#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define MAX_FILES 100
#define BUFFER_SIZE 1024

void handle_client(int client_socket, const char* folder_path) {
    // Lấy danh sách các file trong thư mục
    char file_list[MAX_FILES][BUFFER_SIZE];
    int num_files = 0;
    
    DIR* dir;
    struct dirent *entry;
    
    if ((dir = opendir(folder_path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                strcpy(file_list[num_files], entry->d_name);
                num_files++;
            }
        }
        closedir(dir);
    }
    
    if (num_files > 0) {
        // Gửi danh sách file cho client
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "OK %d\r\n", num_files);
        send(client_socket, response, strlen(response), 0);
        
        for (int i = 0; i < num_files; i++) {
            send(client_socket, file_list[i], strlen(file_list[i]), 0);
            send(client_socket, "\r\n", 2, 0);
        }
        send(client_socket, "\r\n\r\n", 4, 0);
        
        // Nhận tên file từ client
        char filename[BUFFER_SIZE];
        recv(client_socket, filename, BUFFER_SIZE, 0);
        filename[strcspn(filename, "\r\n")] = '\0';
        
        int file_found = 0;
        for (int i = 0; i < num_files; i++) {
            if (strcmp(filename, file_list[i]) == 0) {
                file_found = 1;
                break;
            }
        }
        
        if (file_found) {
            // File tồn tại, gửi nội dung file cho client
            char file_path[BUFFER_SIZE];
            snprintf(file_path, BUFFER_SIZE, "%s/%s", folder_path, filename);
            
            FILE* file = fopen(file_path, "rb");
            
            if (file != NULL) {
                // Gửi thông báo OK và kích thước file
                fseek(file, 0L, SEEK_END);
                long file_size = ftell(file);
                rewind(file);
                
                snprintf(response, BUFFER_SIZE, "OK %ld\r\n", file_size);
                send(client_socket, response, strlen(response), 0);
                
                // Gửi nội dung file
                char buffer[BUFFER_SIZE];
                size_t read_size;
                
                while ((read_size = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                    send(client_socket, buffer, read_size, 0);
                }
                
                fclose(file);
            }
        } else {
            // File không tồn tại
            char error_response[] = "ERROR File Not Found\r\n";
            send(client_socket, error_response, strlen(error_response), 0);
        }
    } else {
        // Không có file trong thư mục
        char error_response[] = "ERROR No files to download\r\n";
        send(client_socket, error_response, strlen(error_response), 0);
    }
    
    // Đóng kết nối với client
    close(client_socket);
}

void start_server(const char* folder_path) {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    
    // Tạo socket server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }
    
    // Thiết lập thông tin server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(4123);
    
    // Gắn socket với địa chỉ và cổng
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        exit(1);
    }
    
    // Lắng nghe kết nối từ client
    if (listen(server_socket, 5) < 0) {
        perror("Error listening for connections");
        exit(1);
    }
    
    printf("Server is running...\n");
    
    while (1) {
        // Chấp nhận kết nối từ client
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            exit(1);
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connected: %s\n", client_ip);
        
        // Xử lý client trong một tiến trình riêng biệt
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            close(server_socket);
            handle_client(client_socket, folder_path);
            exit(0);
        } else if (pid < 0) {
            perror("Error creating child process");
            exit(1);
        }
        
        close(client_socket);
    }
    
    close(server_socket);
}

// int main() {
//     const char* folder_path = ".";
//     start_server(folder_path);
    
//     return 0;
// }

int main() {
    char folder_path[100];

    printf("Enter the folder path: ");
    fgets(folder_path, sizeof(folder_path), stdin);
    folder_path[strcspn(folder_path, "\n")] = '\0';

    start_server(folder_path);
    
    return 0;
}