#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 5000

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_address_len = sizeof(client_address);

    // Tạo socket và lắng nghe kết nối từ info_client
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Khôngthể tạo socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Không thể bind đến địa chỉ và cổng đã chỉ định");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("Lỗi trong quá trình lắng nghe kết nối");
        exit(EXIT_FAILURE);
    }

    printf("Đang lắng nghe kết nối từ client...\n");

    client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_address_len);
    if (client_socket < 0) {
        perror("Không thể chấp nhận kết nối");
        exit(EXIT_FAILURE);
    }

    // Nhận dữ liệu từ info_client
    char data[1050];
    memset(data, 0, sizeof(data));
    if (recv(client_socket, data, 1050, 0) < 0) {
        perror("Không thể nhận dữ liệu từ client");
        exit(EXIT_FAILURE);
    }

    // Tách dữ liệu thành tên máy tính và danh sách các ổ đĩa
    char computer_name[50];
    char drive_list[1000];
    sscanf(data, "%[^\n]\n%[^\n]", computer_name, drive_list);

    // In ra màn hình tên máy tính và danh sách các ổ đĩa
    printf("Tên máy tính: %s\n", computer_name);
    printf("Danh sách ổ đĩa: \n");
    char *p = strtok(drive_list, "\n");
    printf("Kiem tra o dia: ");
    printf(drive_list);
    while (p != NULL) {
        printf("%s\n", p);
        p = strtok(NULL, "\n");
    }

    // Đóng kết nối và đóng socket
    close(client_socket);
    close(server_socket);
    return 0;
}