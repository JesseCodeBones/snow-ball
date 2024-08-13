
#include "./server.hpp"
#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <string_view>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void server::start() {
  struct sockaddr_in local_addr, client_addr;
  // 绑定本地地址和端口
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(3355);

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
    return;
  }

  // 开始监听
  if (listen(listen_fd, 5) < 0) {
    std::cerr << "Failed to listen" << std::endl;
    close(listen_fd);
    return;
  }
  int client_fd = -1;
  while ((client_fd = accept(listen_fd, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    struct sockaddr_in client_addr;
    uint32_t client_addr_len;
    getpeername(client_fd, (struct sockaddr *)&client_addr, &client_addr_len);

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
    int port = ntohs(client_addr.sin_port);

    std::cout << "Peer IP address: " << ip_str << std::endl;
    std::cout << "Peer port: " << port << std::endl;

    int n = -1;
    char buffer[1024];
    std::string_view message("Hello, client!");
    n = recv(client_fd, buffer, sizeof(buffer), 0);

    if (n <= 0) {
      // close client fd
      close(client_fd);
      continue;
    }

    if (std::string_view(buffer, n) != "Hello, server!") {
      // close client fd
      std::string_view fMessage("F U, you are not one of us!");
      std::cout << fMessage << std::endl;
      send(client_fd, fMessage.data(), fMessage.size(), 0);
      std::cout << "Peer IP address: " << ip_str << std::endl;
      std::cout << "Peer port: " << port << std::endl;
      close(client_fd);
      continue;
    } else {
      remote_fd = client_fd;
      remote_host = ip_str;
      remote_port = port;
      proxy();
    }

    while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
      auto now = std::chrono::system_clock::now();
      std::time_t now_c = std::chrono::system_clock::to_time_t(now);
      std::tm *local_time = std::localtime(&now_c);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      send(client_fd, message.data(), message.size(), 0);
      std::string_view message(buffer, n);
      std::cout << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << " - "
                << "heart beat from: " << ip_str << ":" << port << " - "
                << message << std::endl;
    }
  }
}

void handleRequest(int from_fd, int to_fd) {
  // 数据转发
  char buffer[1024 * 50];
  int n;
  while (1) {
    n = recv(from_fd, buffer, sizeof(buffer), 0);
    std::cout << "handle data:" << n << std::endl;
    if (n <= 0) {
      return;
    }
    std::cout << "from:" << from_fd << "to:" << to_fd << std::endl;
    send(to_fd, buffer, n, 0);
  }
}

void server::async_handle(int client_fd, std::string_view remote_host,
                          int remote_port) {
  if (client_fd < 0) {
    return;
  }
  int remote_fd;
  struct sockaddr_in remote_addr;
  // 连接远程服务器
  remote_fd = socket(AF_INET, SOCK_STREAM, 0);
  std::cout << "Connected to remote server = " << remote_fd << std::endl;
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_addr.s_addr = inet_addr(remote_host.data());
  remote_addr.sin_port = htons(remote_port);

  if (connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) <
      0) {
    std::cerr << "Failed to connect to remote server" << std::endl;
    close(client_fd);
    return;
  }

  std::thread t1(handleRequest, client_fd, remote_fd);
  std::thread t2(handleRequest, remote_fd, client_fd);
  t1.join();
  t2.join();
  std::cout << "disconnect = " << remote_fd << std::endl;
  std::cout << "disconnect = " << client_fd << std::endl;
  close(remote_fd);
  close(client_fd);
}

void server::proxy() {

  int listen_fd, client_fd, remote_fd;
  struct sockaddr_in local_proxy_addr, remote_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  // 创建本地监听套接字
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    std::cerr << "Failed to create listen socket" << std::endl;
    return;
  }
  // 绑定本地地址和端口
  local_proxy_addr.sin_family = AF_INET;
  local_proxy_addr.sin_addr.s_addr = INADDR_ANY;
  local_proxy_addr.sin_port = htons(3366);

  if (bind(listen_fd, (struct sockaddr *)&local_proxy_addr, sizeof(local_proxy_addr)) < 0) {
    std::cerr << "Failed to bind" << std::endl;
    close(listen_fd);
    return;
  }

  // 开始监听
  if (listen(listen_fd, 5) < 0) {
    std::cerr << "Failed to listen" << std::endl;
    close(listen_fd);
    return;
  }

  std::cout << "Listening on port 8080..." << std::endl;

  while (true) {
    // 接受客户端连接
    client_fd =
        accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    std::cout << "Accepted client connection = " << client_fd << std::endl;
    if (client_fd < 0) {
      std::cerr << "Failed to accept client connection" << std::endl;
      continue;
    }
    std::thread(async_handle, client_fd).detach();
  }
  close(listen_fd);
}
