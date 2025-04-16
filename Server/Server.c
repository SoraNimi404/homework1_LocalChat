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
//又称socket描述字
int svr_fd;

int cli_fd[MAX_CLIENTS + 1] = {}; // 群友数量，多一个机器人负责播报


struct client{
	/* data */
	char buf[BUF_SIZE];	 // message
	char name[BUF_SIZE]; // name
	int client_fd;		 // fd
};

int main() {
    // 初始化信号量
    if (sem_init(&human_number, 0, 0) != 0) {
        perror("sem_init");
        return -1;
    }

    // 创建socket对象
    printf("[system]创建socket对象...\n");
    svr_fd = socket(AF_INET, SOCK_STREAM, 0);
    //AF_INET ipv4协议，SOCK_STREAM 基于有保障的链接方式，这里是TCP 0则是与第二个参数同步
    if ( svr_fd < 0) {
        perror("socket");
        sem_destroy(&human_number); // 销毁信号量
        return -1;
    }
    //端口复用函数：解决端口号被系统占用的情况
	int on = 1;
	int gg = setsockopt(svr_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(gg==-1)
	{
		perror("setsockopt");
		return -1;
	}
 
	// 准备通信地址(自己)
	printf("[system]准备通信地址...\n");
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6666);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t addrlen = sizeof(addr);
 
	// 绑定socket对象与地址
	printf("[system]绑定socket对象与地址...\n");
	if (bind(svr_fd, (struct sockaddr *)&addr, addrlen)){
		perror("bind");
		return -1;
	}
 
	// 设置监听和排除数量
	printf("[system]设置监听");
	if (listen(svr_fd, 10)){
		perror("listen");
		return -1;
	}
 
	printf("[system]等待客户端链接...\n");
	// 将初始值置全为-1，表示该聊天位置没有人占领
	memset(cli_fd, -1, sizeof(cli_fd));
	while(1){
		int sem_num;
		sem_getvalue(&human_number, &sem_num);
 
		// 找到没有人占领的聊天位
		int index = 0;
		while (-1 != cli_fd[index])
			index++;
		cli_fd[index] = accept(svr_fd, (struct sockaddr *)&addr, &addrlen);
 
		if (0 > cli_fd[index])
		{
			perror("accept");
			return -1;
		}
 
		char buf[BUF_SIZE];
		if (0 >= sem_num)
		{
			printf("[system]人数已满，%d号客户端链接失败\n", cli_fd[index]);
			sprintf(buf, "[system]人数已满，客户端链接失败");
			write(cli_fd[index], buf, strlen(buf) + 1);
			close(cli_fd[index]);
			cli_fd[index] = -1;
		}
		else
		{
			sem_trywait(&sem);
			sem_getvalue(&sem, &sem_num);
			char msg[BUF_SIZE] = {};
			printf("[system]%d号客户端链接成功,当前聊天人数%d人\n", cli_fd[index], SEM_SIZE - sem_num);
			sprintf(msg, "[system]客户端链接成功,当前聊天人数%d人\n", SEM_SIZE - sem_num);
			write(cli_fd[index], msg, strlen(msg) + 1);
 
			// 创建线程客户端
			pthread_t tid;
			pthread_create(&tid, NULL, server, &cli_fd[index]);
		}
	}
    
    sem_destroy(&human_number);

    return 0;
}
