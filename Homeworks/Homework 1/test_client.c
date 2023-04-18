#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 5000

int main() {
    int client_socket;
    struct sockaddr_in server_address;

    // Tạo socket và kết nối đến info_server
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Không thể kết nối đến server");
        exit(EXIT_FAILURE);
    }

    // Nhập tên máy tính và danh sách các ổ đĩa từ bàn phím
    char computer_name[50];
    char drive_list[1000];
    char drive_name[50], drive_size[50];
    printf("Nhập tên máy tính: ");
    fgets(computer_name, 50, stdin);
    computer_name[strcspn(computer_name, "\n")] = '\0';

    printf("Nhập danh sách ổ đĩa (để kết thúc nhập, nhập q tại tên ổ đĩa): \n");
    int count = 0;
    int drive_numbers;
    printf("Số lượng ổ đĩa: ");
    // printf("test 1");
    scanf("%d", &drive_numbers);
    // printf(drive_numbers);
    getchar();
    // fgets(drive_numbers, 50, stdin);
    // printf("test 2");
    for (int i=0;i<drive_numbers;i++){
        printf("Tên ổ đĩa: ");
        fgets(drive_name, 50, stdin);
        drive_name[strcspn(drive_name, "\n")] = '\0';
        if (strcmp(drive_name, "q") == 0) {
            break;
        }
        printf("Kích thước ổ đĩa: ");
        fgets(drive_size, 50, stdin);
        drive_size[strcspn(drive_size, "\n")] = '\0';
        sprintf(drive_list + strlen(drive_list), "%s %s\n", drive_name, drive_size);
        count++;
    }
    // while (1) {
    //     printf("Tên ổ đĩa: ");
    //     fgets(drive_name, 50, stdin);
    //     drive_name[strcspn(drive_name, "\n")] = '\0';
    //     if (strcmp(drive_name, "q") == 0) {
    //         break;
    //     }
    //     printf("Kích thước ổ đĩa: ");
    //     fgets(drive_size, 50, stdin);
    //     drive_size[strcspn(drive_size, "\n")] = '\0';
    //     sprintf(drive_list + strlen(drive_list), "%s %s\n", drive_name, drive_size);
    //     count++;
    // }

    // Đóng gói dữ liệu thành một chuỗi và gửi đến info_server
    printf(drive_list);
    char data[1050];
    sprintf(data, "%s\n%s", computer_name, drive_list);
    if (send(client_socket, data, strlen(data), 0) < 0) {
        perror("Không thể gửi dữ liệu đến server");
        printf(data);
        exit(EXIT_FAILURE);
    }

    // Đóng kết nối và đóng socket
    close(client_socket);
    return 0;
}