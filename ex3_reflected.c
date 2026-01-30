//
// Created by yairt on 16/12/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5555
#define BUFFER_SIZE 65536

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    int opt = 1;

    // Initialize socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR & SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        exit(EXIT_FAILURE);
    }

    // Bind and listen
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 1) < 0) {
        exit(EXIT_FAILURE);
    }

    // Accept
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        exit(EXIT_FAILURE);
    }

    // Read and save
    if (read(new_socket, buffer, BUFFER_SIZE) > 0) {
        // Find start of body (skip HTTP headers)
        char *body = strstr(buffer, "\r\n\r\n");
        if (body) body += 4;
        else body = buffer;

        FILE *fp = fopen("spoofed-reflected.txt", "w");
        if (fp) {
            fprintf(fp, "%s", body);
            fclose(fp);
        }
    }

    // Close sockets
    close(new_socket);
    close(server_fd);
    return 0;
}