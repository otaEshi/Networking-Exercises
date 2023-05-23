#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <time.h>

#define MAX_CLIENTS 64

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <recv_ip> <recv_port> <listen_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *destIP = argv[1];
    int dest_port = atoi(argv[2]);
    int recv_port = atoi(argv[3]);

    int other_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int host = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in send_to_addr;
    send_to_addr.sin_family = AF_INET;
    send_to_addr.sin_addr.s_addr = inet_addr(destIP);
    send_to_addr.sin_port = htons(dest_port);

    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(recv_port);

    bind(other_fd, (struct sockaddr *)&send_to_addr, sizeof(send_to_addr));

    printf("Recieving on port %d...\n", recv_port);
    printf("Seding to %s:%d...\n", destIP, dest_port);

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 2;

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = other_fd;
    fds[1].events = POLLIN;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            fgets(buf, sizeof(buf), stdin);
            int ret = sendto(other_fd, buf, strlen(buf), 0,
                             (struct sockaddr *)&local_addr, sizeof(local_addr));
        }

        if (fds[1].revents & POLLIN)
        {
            ret = recv(other_fd, buf, sizeof(buf), 0);
            if (ret <= 0)
                break;
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }
    return 0;
}