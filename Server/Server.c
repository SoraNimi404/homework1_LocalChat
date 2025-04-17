#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080              // 服务器监听的端口号
#define MAXCLIENTS 100         // 最大客户端连接数
#define BUFFERSIZE 2048        // 数据缓冲区大小

int clientsockets[MAXCLIENTS]; // 存储客户端套接字数组
int nclients = 0;              // 当前连接的客户端数量

pthread_mutex_t clientsmutex = PTHREAD_MUTEX_INITIALIZER; // 客户端数组互斥锁

// 添加客户端到客户端数组
void addclient(int socket) {
    pthread_mutex_lock(&clientsmutex); // 加锁以保护客户端数组
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (clientsockets[i] == 0) {
            clientsockets[i] = socket; // 添加客户端套接字
            nclients++; // 增加客户端计数
            break;
        }
    }
    pthread_mutex_unlock(&clientsmutex); // 解锁
}

// 从客户端数组中移除客户端
void removeclient(int socket) {
    pthread_mutex_lock(&clientsmutex); // 加锁以保护客户端数组
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (clientsockets[i] == socket) {
            clientsockets[i] = 0; // 移除客户端套接字
            nclients--; // 减少客户端计数
            break;
        }
    }
    pthread_mutex_unlock(&clientsmutex); // 解锁
}

// 广播消息给所有客户端
void broadcastmessage(char *message, int sendersocket) {
    pthread_mutex_lock(&clientsmutex); // 加锁以保护客户端数组
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (clientsockets[i] != 0 && clientsockets[i] != sendersocket) {
            send(clientsockets[i], message, strlen(message), 0); // 发送消息
        }
    }
    pthread_mutex_unlock(&clientsmutex); // 解锁
}

// 处理客户端消息的线程函数
void *handleclient(void *socketptr) {
    int socket = *(int *)socketptr;
    free(socketptr); // 释放分配的内存

    char buffer[BUFFERSIZE];
    int readsize;

    // 循环接收客户端消息
    while ((readsize = recv(socket, buffer, BUFFERSIZE, 0)) > 0) {
        buffer[readsize] = '\0'; // 确保字符串结束
        broadcastmessage(buffer, socket); // 广播消息
    }

    if (readsize == 0) {
        printf("Client disconnected: %d\n", socket);
    } else if (readsize == -1) {
        perror("recv failed");
    }

    removeclient(socket); // 移除客户端
    close(socket); // 关闭套接字
    return NULL;
}

// 设置守护进程
void daemonize() {
    pid_t pid = fork(); // 第一次fork

    if (pid < 0) {
        exit(EXIT_FAILURE); // fork失败，退出
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS); // 父进程退出
    }

    if (setsid() < 0) { // 创建新的会话
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN); // 忽略子进程状态变化信号
    signal(SIGHUP, SIG_IGN); // 忽略挂起信号

    pid = fork(); // 第二次fork

    if (pid < 0) {
        exit(EXIT_FAILURE); // fork失败，退出
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS); // 父进程退出
    }

    // 设置文件权限掩码
    umask(0);

    // 更改工作目录
    chdir("/");

    // 关闭所有文件描述符
    for (int i = sysconf(_SC_OPEN_MAX); i >= 0; i--) {
        close(i);
    }
}

int main() {
    int serversocket, newsocket;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t clientaddrsize;
    pthread_t threadid;

    daemonize(); // 设置为守护进程

    // 创建Socket
    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serversocket == -1) {
        perror("Could not create socket");
        return 1;
    }

    // 设置服务器地址结构
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    // 绑定Socket到地址
    if (bind(serversocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind failed");
        return 1;
    }

    // 开始监听
    if (listen(serversocket, MAXCLIENTS) < 0) {
        perror("listen failed");
        return 1;
    }

    printf("Waiting for incoming connections...\n");

    // 接受连接循环
    while ((newsocket = accept(serversocket, (struct sockaddr *)&clientaddr, &clientaddrsize))) {
        if (newsocket < 0) {
            perror("accept failed");
            continue;
        }

        printf("Connection accepted: %d\n", newsocket);

        int *newsock = malloc(sizeof(int));
        *newsock = newsocket;

        addclient(newsocket);

        // 为新客户端创建处理线程
        if (pthread_create(&threadid, NULL, handleclient, (void *)newsock) < 0) {
            perror("Could not create thread");
            return 1;
        }

        pthread_detach(threadid); // 分离线程，不等待线程结束
    }

    if (newsocket < 0) {
        perror("Accept failed");
        return 1;
    }

    return 0;
}
