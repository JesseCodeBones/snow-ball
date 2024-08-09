
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

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

void async_handle(int client_fd) {
  if (client_fd < 0) {
    return;
  }
  int remote_fd;
  struct sockaddr_in remote_addr;
  // 连接远程服务器
  remote_fd = socket(AF_INET, SOCK_STREAM, 0);
  std::cout << "Connected to remote server = " << remote_fd << std::endl;
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  remote_addr.sin_port = htons(9091);

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

int main() {
  int listen_fd, client_fd, remote_fd;
  struct sockaddr_in local_addr, remote_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  // 创建本地监听套接字
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    std::cerr << "Failed to create listen socket" << std::endl;
    return -1;
  }

  // 绑定本地地址和端口
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(8081);

  if (bind(listen_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
    std::cerr << "Failed to bind" << std::endl;
    close(listen_fd);
    return -1;
  }

  // 开始监听
  if (listen(listen_fd, 5) < 0) {
    std::cerr << "Failed to listen" << std::endl;
    close(listen_fd);
    return -1;
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
  return 0;
}