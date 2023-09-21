#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

int RunServer(uint16_t port) {
  std::cout<<"========== mydebug: start socket server port:"<<port<<std::endl;
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[102400] = {0};
  int length=25000;
  std::string send_data(length, 'a');
  const char *hello = "Hello from server";

  // 创建 Socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      perror("socket failed");
      return 1;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  // 绑定端口
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
      perror("bind failed");
      return 1;
  }

  // 监听
  if (listen(server_fd, 3) < 0) {
      perror("listen failed");
      return 1;
  }

  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
      perror("accept failed");
      return 1;
  }

  while(true){
  // 从客户端接收消息
  read(new_socket, buffer, 1024);
  // std::cout << "Client: " << buffer << std::endl;

  // 发送消息给客户端
  send(new_socket, hello, strlen(hello), 0);
  // std::cout << "Hello message sent" << std::endl;
  }

  return 0;
}
int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  RunServer(absl::GetFlag(FLAGS_port));
  return 0;
}
