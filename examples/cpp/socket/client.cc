#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

ABSL_FLAG(std::string, ip, "127.0.0.1", "Server address");
ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

int RunClient(uint16_t port,std::string ip){
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    const char *hello = "Hello from client";
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // 将 IP 地址从字符串转换为网络地址
    if(inet_pton(AF_INET,ip.c_str(), &serv_addr.sin_addr)<=0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // 发送消息给服务器
    send(sock, hello, strlen(hello), 0);
    std::cout << "Hello message sent" << std::endl;

    // 从服务器接收消息
    valread = read(sock, buffer, 1024);
    std::cout << "Server: " << buffer << std::endl;

    return 0;
}
int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    RunClient(absl::GetFlag(FLAGS_port),absl::GetFlag(FLAGS_ip));
    return 0;
}
