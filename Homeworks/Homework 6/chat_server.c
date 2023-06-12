#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENTS 64
struct client_info
{
    int fd;
    char client_name[32];
    char client_id[32];
    int is_set_info;
};
struct client_info clients[MAX_CLIENTS] = {0};
int num_clients = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_thread(void *);

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

    printf("Server started. Listening on port 9000\n");

    while (1)
    {
        // Accept a new client connection
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        pthread_mutex_lock(&clients_mutex);
        if (num_clients < MAX_CLIENTS)
        {
            printf("New client connected: %d\n", client);
            clients[num_clients].fd = client;
            num_clients++;
        }
        else
        {
            printf("Too many connections\n");
            close(client);
        }
        pthread_mutex_unlock(&clients_mutex);
        // Create a new thread to handle the client
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    // Close the listener socket
    close(listener);

    return 0;
}

// Thread procedure for handling a client
void *client_thread(void *param)
{
    int client = *(int *)param;
    char *client_id;
    char *client_name;

    char buf[256];
    char message[1024];

    int i = 0;
    for (int j = 0; j < MAX_CLIENTS; j++)
    {
        if (clients[j].fd == client)
        {
            i = j;
            break;
        }
        if (j == MAX_CLIENTS - 1)
        {
            printf("Failed at line 11x");
            exit(0);
        }
    }

    while (1)
    {
        // Receive data from the client
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        if (clients[i].is_set_info == 0)
        {
            buf[strcspn(buf, "\n")] = '\0';
            printf("Received from client %d: %s\n", clients[i].fd, buf);
            client_id = strtok(buf, ":");
            client_name = strtok(NULL, ":");
            if (client_name == NULL || client_id == NULL || strlen(client_name) == 0 || strlen(client_id) == 0)
            {
                send(clients[i].fd, "Invalid input format. Please send 'client_id: client_name' again\n", strlen("Invalid input format. Please send 'client_id: client_name' again\n"), 0);
                close(clients[i].fd);
                continue;
            }
            strcpy(clients[i].client_name, client_name);
            strcpy(clients[i].client_id, client_id);
            clients[i].is_set_info = 1;
        }
        else if (clients[i].is_set_info == 1)
        {
            buf[strcspn(buf, "\n")] = '\0';
            printf("Received from client %d: %s\n", clients[i].fd, buf);
            if (buf[0] == '@')
            {
                // Tin nhắn riêng
                char *recipient_id = strtok(buf + 1, ":");
                char *message_content = strtok(NULL, ":");
                if (recipient_id != NULL && message_content != NULL)
                {
                    int recipient_fd;
                    for (int j = 1; j < num_clients; j++)
                    {
                        if (j == i)
                            continue;
                        if (strcmp(clients[j].client_id, recipient_id) == 0)
                        {
                            recipient_fd = clients[j].fd;
                            break;
                        }
                    }
                    snprintf(message, sizeof(message), "%s: %s\n", clients[i].client_name, message_content);
                    send(recipient_fd, message, strlen(message), 0);
                }
                else
                {
                    send(clients[i].fd, "Invalid input format. Please send '@recipient_id: message_content' again\n", strlen("Invalid input format. Please send '@recipient_id: message_content' again\n"), 0);
                }
            }
            else
            {
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                snprintf(message, sizeof(message), "%4d-%2d-%2d %2d:%2d:%2d %s: %s\n",
                         t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                         t->tm_hour, t->tm_min, t->tm_sec, clients[i].client_name, buf);

                for (int j = 0; j < num_clients; j++)
                {
                    if (j == i)
                        continue;
                    ret = send(clients[j].fd, message, strlen(message), 0);
                    if (ret == -1)
                    {
                        perror("send() failed");
                        return NULL;
                    }
                }
            }
        }
        else
        {
            perror("Error is_set_info");
            return NULL;
        }
    }

    // Close the client socket
    close(client);
}
