#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int client_socket, port_number;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Kiểm tra định dạng tham số dòng lệnh
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Tạo socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ máy chủ
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    port_number = atoi(argv[2]);
    server_address.sin_port = htons(port_number);

    // Kết nối đến máy chủ
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Nhập dữ liệu từ bàn phím và gửi đến máy chủ
    while (1) {
        printf("Enter a message (type 'quit' to exit): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // // Loại bỏ ký tự xuống dòng cuối cùng
        // buffer[strcspn(buffer, "\n")] = 0;
        // Kiểm tra nếu người dùng muốn thoát
        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        // Gửi dữ liệu đến máy chủ
        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
            perror("Error sending data to server");
            exit(EXIT_FAILURE);
        }
    }

    // Đóng kết nối và thoát chương trình
    close(client_socket);
    return 0;
}