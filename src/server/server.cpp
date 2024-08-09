
#include "./server.hpp"
#include "stdio.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

static int proxy_server_fd, public_server_fd, remote_client_socket, proxy_request_socket;



void proxyHandler(int client_socket) {
    char buffer[1024] = {0};
    const char* response = "Message received by server\n";
    // 接收客户端消息
    read(client_socket, buffer, 1024);
    std::cout << "Message from client: " << buffer << std::endl;
    // 发送响应消息到客户端
    send(remote_client_socket, buffer, 1024, 0);
}


void clientHandler(int client_socket) {
    char buffer[1024] = {0};
    const char* response = "Message received by server\n";
    // 接收客户端消息
    read(client_socket, buffer, 1024);
    std::cout << "Message from client: " << buffer << std::endl;
    // 发送响应消息到客户端
    send(remote_client_socket, buffer, 1024, 0);
}


void server::start() {
        
    struct sockaddr_in address;
    struct sockaddr_in public_server_address;
    int addrlen = sizeof(address);
    int opt = 1;
    char buffer[1024] = {0};
    const char* hello = "Hello from server";

    // 创建套接字
    if ((proxy_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if ((public_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项
    if (setsockopt(proxy_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))
    || setsockopt(public_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(1997);

    public_server_address.sin_family = AF_INET;
    public_server_address.sin_addr.s_addr = INADDR_ANY;
    public_server_address.sin_port = htons(1989);

    // 绑定套接字
    if (bind(proxy_server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (bind(public_server_fd, (struct sockaddr *)&public_server_address, sizeof(public_server_address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(proxy_server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(public_server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // // 接受连接
    // if ((remote_client_socket = accept(proxy_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }

     while (1) {
        // 接受连接
        if ((proxy_request_socket = accept(public_server_fd, (struct sockaddr *)&public_server_address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // 创建新线程处理客户端请求
        std::thread clientThread(proxyHandler, proxy_request_socket);
        clientThread.detach(); // 分离线程，使其在后台运行
    }

    while (1) {
        // 接受连接
        if ((remote_client_socket = accept(proxy_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // 创建新线程处理客户端请求
        std::thread clientThread(clientHandler, proxy_request_socket);
        clientThread.detach(); // 分离线程，使其在后台运行
    }

    // 发送消息到客户端
    send(remote_client_socket, hello, strlen(hello), 0);
    std::cout << "Hello message sent\n";
}