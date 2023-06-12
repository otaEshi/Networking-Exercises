#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void *thread_proc(void *);

int main()
{
    // Create a socket for the server
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Set up the server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Bind the socket to the server address
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    int num_threads = 8;
    pthread_t thread_id;
    for (int i = 0; i < num_threads; i++)
        pthread_create(&thread_id, NULL, thread_proc, &listener);

    pthread_join(thread_id, NULL);

    // Close the listener socket
    close(listener);

    return 0;
}

// Thread procedure for handling a client
void *thread_proc(void *param)
{
    int listener = *(int *)param;
    char buf[256];

    while (1)
    {
        // Accept a new client connection
        int client = accept(listener, NULL, NULL);
        printf("New client accepted: %d\n", client);

        // Receive data from the client
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            continue;
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        // Trả lại kết quả cho client
        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
        send(client, msg, strlen(msg), 0);

        // Đóng kết nối
        close(client);
    }
}
