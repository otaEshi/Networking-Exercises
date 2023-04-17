#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_LENGTH 1024

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char mssv[10], hoten[50], ngaysinh[20], diem[5];

    if (argc != 3) {
        fprintf(stderr, "Sử dụng: %s <địa chỉ server> <cổng server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Tạo socket client
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Không thể chuyển đổi địa chỉ IP");
        exit(EXIT_FAILURE);
    }

    // Kết nối đến server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Không thể kết nối đến server");
        exit(EXIT_FAILURE);
    }

    while(1){
        // Nhập thông tin sinh viên từ bàn phím
        char mssv[MAX_LENGTH], hoten[MAX_LENGTH], ngaysinh[MAX_LENGTH], diem[MAX_LENGTH];
        memset(mssv, 0, MAX_LENGTH);
        memset(hoten, 0, MAX_LENGTH);
        memset(ngaysinh, 0, MAX_LENGTH);
        memset(diem, 0, MAX_LENGTH);

        printf("Nhập thông tin sinh viên:\n");
        printf("\t- MSSV: ");
        fgets(mssv, MAX_LENGTH, stdin);
        mssv[strcspn(mssv, "\n")] = 0;

        printf("\t- Họ tên: ");
        fgets(hoten, MAX_LENGTH, stdin);
        hoten[strcspn(hoten, "\n")] = 0;

        printf("\t- Ngày sinh: ");
        fgets(ngaysinh, MAX_LENGTH, stdin);
        ngaysinh[strcspn(ngaysinh, "\n")] = 0;

        printf("\t- Điểm trung bình các môn học: ");
        fgets(diem, MAX_LENGTH, stdin);
        diem[strcspn(diem, "\n")] = 0;

        // Gửi thông tin sinh viên đến server
        sprintf(buffer, "%s %s %s %s", mssv, hoten, ngaysinh, diem);
        write(client_socket, buffer, strlen(buffer));
        printf("Do you want to continue? (y/n): ");
        char c;
        c = getchar();
        while (getchar() != '\n'); // loại bỏ ký tự '\n' thừa trong buffer

        while (1){
            if (c == 'y' || c == 'Y' || c == 'n' || c == 'N') {
                printf("test");
                break;
            }
            printf("Do you want to continue? (y/n): ");
            c = getchar();
            while (getchar() != '\n');
        }
        if (c == 'y' || c == 'Y') {
            continue;
        }
        break;
    }


    // Đóng kết nối
    close(client_socket);

    return 0;
}