#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <arpa/inet.h>
#include <netdb.h>

ABSL_FLAG(std::string, ip, "127.0.0.1", "Server address");
ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");
ABSL_FLAG(uint32_t, loop, 1000, "client call loop times");

std::string convert2IP(std::string ip){

    const char* hostname = ip.c_str();
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // 使用IPv4

    int status = getaddrinfo(hostname, NULL, &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return "";
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip_address, INET_ADDRSTRLEN);

    std::cout << "IP Address of " << hostname << " is: " << ip_address << std::endl;

    freeaddrinfo(res);
    return ip_address;
}
int RunClient(uint16_t port,uint32_t loop,std::string ip){
    ip = convert2IP(ip);
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    int length=25000;
    std::string send_data(length, 'a');
    const char *hello = send_data.c_str();
    char buffer[102400] = {0};

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
    auto s= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
    .count();
    for(int i=0;i<loop;++i){
        // 发送消息给服务器
        send(sock, hello, strlen(hello), 0);
        // std::cout << "Hello message sent" << std::endl;

        // 从服务器接收消息
        valread = read(sock, buffer, 1024);
        // std::cout << "Server: " << buffer << std::endl;
    }
    auto e= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
    .count();
    std::cout<<"loop:"<<loop <<" socket time consume:"<<e-s<<"us"<<std::endl;

    return 0;
}
int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    RunClient(absl::GetFlag(FLAGS_port),absl::GetFlag(FLAGS_loop),absl::GetFlag(FLAGS_ip));
    return 0;
}
