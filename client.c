#include <arpa/inet.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 12345
#define DATA_SIZE 10000
static const uint16_t SEGMENT_SIZE = 1200;

int main(void) {
    uint8_t *data = malloc(DATA_SIZE);
    if (!data) {
        fprintf(stderr, "OOM\n");
        return 1;
    }

    for (size_t i = 0; i < DATA_SIZE; ++i) {
        // Fill with some data
        data[i] = (uint8_t) i;
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
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    struct iovec iov = {
        .iov_base = data,
        .iov_len = DATA_SIZE,
    };

    uint8_t control[CMSG_SPACE(sizeof(SEGMENT_SIZE))];
    struct msghdr msg = {
        .msg_name = &serv_addr,
        .msg_namelen = sizeof(struct sockaddr_in),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = control,
        .msg_controllen = sizeof(control),
        .msg_flags = 0,
    };

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_UDP;
    cmsg->cmsg_type = UDP_SEGMENT;
    cmsg->cmsg_len = CMSG_LEN(sizeof(SEGMENT_SIZE));
    void *cmsg_data = CMSG_DATA(cmsg);
    memcpy(cmsg_data, &SEGMENT_SIZE, sizeof(SEGMENT_SIZE));

    ssize_t len = sendmsg(sockfd, &msg, 0);
    if (len < 0) {
        perror("sendmsg");
        return 1;
    }

    printf("%zu bytes sent (segment_size=%" PRIu16 ")\n", len, SEGMENT_SIZE);

    close(sockfd);

    return 0;
}
