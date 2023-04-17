#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <log_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    char log_data[BUFFER_SIZE];
    time_t current_time;
    struct tm *time_info;
    FILE *log_file;

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ và cổng
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind địa chỉ và cổng vào socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe yêu cầu từ client
    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s:%s\n", inet_ntoa(server_addr.sin_addr), argv[1]);

    // Vòng lặp vô hạn để lắng nghe yêu cầu từ client
    while (1) {
        socklen_t client_len = sizeof(client_addr);

        // Chấp nhận kết nối từ client
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) == -1) {
            perror("accept failed");
            continue;
        }

        printf("Connected by %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while(1){
            // Nhận dữ liệu từ client
            if (recv(client_fd, buffer, BUFFER_SIZE, 0) == -1) {
                perror("recv failed");
                close(client_fd);
                continue;
            }

            // Lấy thời gian hiện tại
            current_time = time(NULL);
            time_info = localtime(&current_time);

            // Tạo chuỗi dữ liệu để ghi vào file
            sprintf(log_data, "%s %04d-%02d-%02d %02d:%02d:%02d %s\n", inet_ntoa(client_addr.sin_addr), time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday, time_info->tm_hour, time_info->tm_min, time_info->tm_sec, buffer);

            // In dữ liệu ra màn hình
            printf("%s", log_data);

            // Ghi dữ liệu vào file log
            if ((log_file = fopen(argv[2], "a")) == NULL) {
                perror("fopen failed");
                close(client_fd);
                continue;
            }

            fprintf(log_file, "%s", log_data);
            fclose(log_file);
        }
    // Đóng kết nối với client
    close(client_fd);    
    }
    return 0;
}