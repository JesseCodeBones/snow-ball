#include "./client.hpp"

#include "fcntl.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <chrono>

void client::start() {

  struct sockaddr_in remote_addr;
  int remote_fd = socket(AF_INET, SOCK_STREAM, 0);
  std::cout << "Connected to remote server = " << remote_fd << std::endl;
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_addr.s_addr = inet_addr("43.139.96.180");
  remote_addr.sin_port = htons(3355);

  if (connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) <
      0) {
    std::cerr << "Failed to connect to remote server" << std::endl;
    return;
  }

  char buffer[1024];
  std::string_view message("Hello, server!");

  send(remote_fd, message.data(), 15, 0);
  int n = 0;

  while ((n = recv(remote_fd, buffer, sizeof(buffer), 0)) > 0) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_c);
    std::cout << "Received from server: " << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << " - " << buffer << std::endl;
    send(remote_fd, message.data(), message.size(), 0);
  }
}