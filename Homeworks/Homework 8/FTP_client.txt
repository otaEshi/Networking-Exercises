#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
char *get_filename(char *filename);

int main(int argc, char const *argv[])
{
    if (argc != 7)
    {
        printf("Wrong or not enough args. Exiting...");
        return 1;
    }

    int ctrl_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in ctrl_addr;
    ctrl_addr.sin_family = AF_INET;
    ctrl_addr.sin_addr.s_addr = inet_addr(argv[1]);
    ctrl_addr.sin_port = htons(atoi(argv[2]));

    if (connect(ctrl_socket, (struct sockaddr *)&ctrl_addr, sizeof(ctrl_addr)))
    {
        perror("ctrl_socket connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhan xau chao tu server
    int ret = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;

    sprintf(buf, "USER %s\r\n", argv[3]);
    send(ctrl_socket, buf, strlen(buf), 0);

    ret = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    // puts(buf);

    // Gui lenh PASS
    sprintf(buf, "PASS %s\r\n", argv[4]);
    send(ctrl_socket, buf, strlen(buf), 0);

    ret = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    // puts(buf);

    if (strncmp(buf, "230", 3) != 0)
    {
        printf("Wrong username or password. Exitting...\n");
        return 1;
    }

    int data_socket;
    if (strcmp(argv[5], "PASV") == 0)
    {
        // Gui lenh PASV
        send(ctrl_socket, "PASV\r\n", 6, 0);
        ret = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[ret] = 0;
        puts(buf);

        // Xu ly ket qua lenh PASV
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        int n = pos2 - pos1;
        memcpy(buf, pos1, n);
        buf[n] = 0;

        char *p = strtok(buf, ",");
        int i1 = atoi(p);
        p = strtok(NULL, ",");
        int i2 = atoi(p);
        p = strtok(NULL, ",");
        int i3 = atoi(p);
        p = strtok(NULL, ",");
        int i4 = atoi(p);
        p = strtok(NULL, ",");
        int p1 = atoi(p);
        p = strtok(NULL, ",");
        int p2 = atoi(p);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        sprintf(buf, "%d.%d.%d.%d", i1, i2, i3, i4);
        data_addr.sin_addr.s_addr = inet_addr(buf);
        data_addr.sin_port = htons(p1 * 256 + p2);

        data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("data_socket connect() failed");
            return 1;
        }
    }
    else
    {
        // ESPV
        // printf("IS not handled yet. Exitting...\n");
        send(ctrl_socket, "EPSV\r\n", 6, 0);
        ret = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[ret] = 0;
        puts(buf);

        // Xu li thu cong
        char *pos1 = strchr(buf, '(') + 4;
        char *pos2 = strchr(pos1, ')') - 1;
        int n = pos2 - pos1;
        memcpy(buf, pos1, n);
        buf[n] = 0;
        int port = atoi(buf);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr(argv[1]);
        data_addr.sin_port = htons(port);

        data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("data_socket connect() failed");
            return 1;
        }
    }

    ret = sprintf(buf, "STOR %s\r\n", argv[6]);
    send(ctrl_socket, buf, ret, 0);
    ret = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    FILE *f = fopen(argv[6], "rb");
    if (!f)
    {
        perror("fopen() failed\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Size: %ld\n", size);
    while (1)
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        send(data_socket, buf, ret, 0);
    }
    fclose(f);
    close(data_socket);

    ret = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    return 0;

    send(ctrl_socket, "QUIT\r\n", 6, 0);
    ret = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    // Ket thuc, dong socket
    close(ctrl_socket);
}