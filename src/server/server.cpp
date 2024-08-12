
#include "./server.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <string_view>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <chrono>

void server::start() {
  struct sockaddr_in local_addr, client_addr;
// 绑定本地地址和端口
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(8081);

  
  int listen_fd;
  socklen_t client_addr_len = sizeof(client_addr);

   // 创建本地监听套接字
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    std::cerr << "Failed to create listen socket" << std::endl;
    return;
  }

  if (bind(listen_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
    std::cerr << "Failed to bind" << std::endl;
    close(listen_fd);
    return ;
  }

  // 开始监听
  if (listen(listen_fd, 5) < 0) {
    std::cerr << "Failed to listen" << std::endl;
    close(listen_fd);
    return;
  }

  int client_fd =
      accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  int n;
  char buffer[1024];
  std::string_view message = "Hello, client!";
  while (1) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_c);
    n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n <= 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send(client_fd, message.data(), message.size(), 0);
    std::string_view message(buffer, n);
    std::cout << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
  }
}