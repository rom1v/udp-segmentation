#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 12345
#define BUFSIZE 10000

int main(void) {
    uint8_t *buf = malloc(BUFSIZE);
    if (!buf) {
        fprintf(stderr, "OOM");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t sin_size = sizeof(serv_addr);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sin_size) < 0) {
        perror("bind");
        return 1;
    }

    for (;;) {
        // Interrupt by Ctrl+c
        ssize_t len = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &serv_addr, &sin_size);
        if (len < 0) {
            perror("recvfrom");
            return 1;
        }
        printf("Received datagram (%zu bytes)\n", len);
    }

    close(sockfd);

    return 0;
}
