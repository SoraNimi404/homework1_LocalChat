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

#define BUF_SIZE (4096)
#define MAX_CLIENTS (20)

sem_t human_number; // 当前聊天人数

// 服务端文件描述符
int svr_fd;

int cli_fd[MAX_CLIENTS + 1] = {}; // 群友数量，有一个机器人负责播报

int main() {
    // 初始化信号量
    if (sem_init(&human_number, 0, 0) != 0) {
        perror("sem_init");
        return -1;
    }

    // 创建socket对象
    printf("[system]创建socket对象...\n");
    svr_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > svr_fd) {
        perror("socket");
        sem_destroy(&human_number); // 销毁信号量
        return -1;
    }

    // ... 其他服务器设置代码

    // 销毁信号量
    sem_destroy(&human_number);

    return 0;
}
