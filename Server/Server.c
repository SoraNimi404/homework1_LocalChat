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
#define SEM_SIZE (20) // Ⱥ����������
 
// �ź���--�ж�Ⱥ������
sem_t sem;
// ������ļ�������
int svr_fd;
// �洢Ⱥ�ѣ���һ����Ϊ�˵�Ⱥ����ʱ����һ�������ӷ���Ϣ
int cli_fd[SEM_SIZE + 1] = {};
 
struct client
{
	/* data */
	char buf[BUF_SIZE];	 // message
	char name[BUF_SIZE]; // name
	int client_fd;		 // fd
};
 
struct client clients[SEM_SIZE];
 
// �ַ����ָ��
void split(char *src, const char *separator, char **dest, int *num)
{
	char *pNext;
	// ��¼�ָ�������
	int count = 0;
	// ԭ�ַ���Ϊ��
	if (src == NULL || strlen(src) == 0)
		return;
	// δ����ָ���
	if (separator == NULL || strlen(separator) == 0)
		return;
	/*
		c����string���к�����
		������
		char *strtok(char *str, const char *delim)
		������
		str -- Ҫ���ֽ��һ��С�ַ������ַ�����
		delim -- �����ָ����� C �ַ�����
		����ֵ��
		�ú������ر��ֽ�ĵ�һ�����ַ��������û�пɼ������ַ������򷵻�һ����ָ�롣
	*/
	char *strtok(char *str, const char *delim);
	// ��õ�һ���ɷָ����ָ���ַ���
	pNext = strtok(src, separator);
	while (pNext != NULL)
	{
		// ���뵽Ŀ���ַ���������
		*dest++ = pNext;
		++count;
		/*
			strtok()�������ַ����ָ��һ����Ƭ�Ρ�����sָ�����ָ���ַ���������delim��Ϊ�ָ��ַ����а����������ַ���
			��strtok()�ڲ���s���ַ����з��ֲ���delim�а����ķָ��ַ�ʱ,��Ὣ���ַ���Ϊ\0 �ַ���
			�ڵ�һ�ε���ʱ��strtok()����������s�ַ���������ĵ����򽫲���s���ó�NULL��
			ÿ�ε��óɹ��򷵻�ָ�򱻷ָ��Ƭ�ε�ָ�롣
		*/
		pNext = strtok(NULL, separator);
	}
	*num = count;
}
 
// Ⱥ������
void *send_all(char *buf)
{
	for (int i = 0; i < SEM_SIZE; i++)
	{
		// ��ֵΪ-1����û�д�Ⱥ�ѣ���ʾ�Ѿ��˳���δ��ռ��
		if (-1 != cli_fd[i])
		{
			printf("%s\n", buf);
			printf("send to %d\n", cli_fd[i]);
			write(cli_fd[i], buf, strlen(buf) + 1);
		}
	}
}
 
/**
 * ���͸�ָ�����û�
 * char *buf ���͵���Ϣ
 * int fd ����˭
 */
void *send_one(char *buf, int fd)
{
	printf("send to %d : %s\n",fd, buf);
	// printf("send to %d\n", fd);
	write(fd, buf, strlen(buf) + 1);
}
 
// ����˽��պ���
void *server(void *arg)
{
 
	int fd = *(int *)arg;
	char buf[BUF_SIZE];
	char name[BUF_SIZE], ts[BUF_SIZE];
	
	
 
	// ��ȡ�ǳ�
	read(fd, clients[fd].name, sizeof(name));
	clients[fd].client_fd = fd;
 
	// printf("clients[fd].name = %s\n", clients[fd].name);
	printf("���clients[fd].client_fd = %d\n", clients[fd].client_fd);
	sprintf(ts, "[system]���һ�ӭ %s ����Ⱥ��", clients[fd].name);
	send_all(ts);
 
	for (;;)
	{
		// ������Ϣ,����Ϣʱ������
		int recv_size = read(fd, clients[fd].buf, sizeof(buf));
 
		
		// �յ��˳���Ϣ
		if (0 >= recv || NULL != strstr(clients[fd].buf, "quit"))
		{
			sprintf(ts, "[system]���� %s �뿪Ⱥ��\n", clients[fd].name);
			int index = 0;
			// �ҵ�Ҫ�˳����Ǹ��ˣ���������Ϊ-1
			for (; index < SEM_SIZE; index++)
			{
				if (cli_fd[index] == fd)
				{
					cli_fd[index] = -1;
					break;
				}
			}
			// Ⱥ��XXX�˳���������ʾ��Ϣ
			send_all(ts);
 
			// Ⱥ���˳����ź���+1
			int n;
			sem_post(&sem);
			sem_getvalue(&sem, &n);
 
			printf("[system] %s �뿪Ⱥ��,Ⱥ�Ļ�ʣ%d��\n", clients[fd].name, SEM_SIZE - n);
			strcpy(clients[fd].buf, "quit");
 
			write(fd, clients[fd].buf, strlen(clients[fd].buf) + 1);
			close(fd);
			pthread_exit(NULL);
		}
 
		// �������ͻ���Ⱥ��
		//    ------/send name message
		if (0 >= recv || NULL != strstr(clients[fd].buf, "send"))// ������
		{ 	
			char str[100];
			char *p[10]={0};
			int num=0,i;
			//����
			strcpy(str,clients[fd].buf);
			// printf("clients[fd].buf1 = %s\n", clients[fd].buf); //send name message
			// �� clients[fd].buf���н�ȡ
			split(clients[fd].buf,":",p,&num);
			// printf("clients[fd].buf1 = %s\n", clients[fd].buf); //send
			// printf("split str is = %s\n",p[1]);
			// Ȼ�����������бȽ� �ҵ���name��ͬ������Ԫ�أ�ȡ�������з���
			for(i = 0;i < SEM_SIZE; i++) {
 
 
                                // �ж����֣�ȥ����Ӧclient_fd���з���
				if(NULL != strstr(clients[i].name,p[2])){
					printf("client[%d].name3 = %s\n",i,clients[i].name);
					char msg[200];
					sprintf(msg,"[���Ļ�]%s:%s",clients[fd].name,p[3]);
					send_one(msg, clients[i].client_fd);
                                }
                        }
		}
		else
		{ // Ⱥ��
			send_all(clients[fd].buf);
		}
	}
}
 
/**
 * quit
 */
void sigint(int signum)
{
	close(svr_fd);
	sem_destroy(&sem);
	printf("[system]�������ر�\n");
	exit(0);
}
 
int main()
{
	signal(SIGINT, sigint);
	// ��ʼ���ź�����Ⱥ������SEM_SIZE��
	sem_init(&sem, 0, SEM_SIZE);
 
	// ����socket����
	printf("[system]����socket����...\n");
	svr_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > svr_fd)
	{
		perror("socket");
		return -1;
	}
 
 
	//�˿ڸ��ú���������˿ںű�ϵͳռ�õ����
	int on = 1;
	int gg = setsockopt(svr_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(gg==-1)
	{
		perror("setsockopt");
		return -1;
	}
 
	// ׼��ͨ�ŵ�ַ(�Լ�)
	printf("[system]׼��ͨ�ŵ�ַ...\n");
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6666);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t addrlen = sizeof(addr);
 
	// ��socket�������ַ
	printf("[system]��socket�������ַ...\n");
	if (bind(svr_fd, (struct sockaddr *)&addr, addrlen))
	{
		perror("bind");
		return -1;
	}
 
	// ���ü������ų�����
	printf("[system]���ü���");
	if (listen(svr_fd, 10))
	{
		perror("listen");
		return -1;
	}
 
	printf("[system]�ȴ��ͻ�������...\n");
	// ����ʼֵ��ȫΪ-1����ʾ������λ��û����ռ��
	memset(cli_fd, -1, sizeof(cli_fd));
	for (;;)
	{
		int sem_num;
		sem_getvalue(&sem, &sem_num);
 
		// �ҵ�û����ռ�������λ
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
			printf("[system]����������%d�ſͻ�������ʧ��\n", cli_fd[index]);
			sprintf(buf, "[system]�����������ͻ�������ʧ��");
			write(cli_fd[index], buf, strlen(buf) + 1);
			close(cli_fd[index]);
			cli_fd[index] = -1;
		}
		else
		{
			sem_trywait(&sem);
			sem_getvalue(&sem, &sem_num);
			char msg[BUF_SIZE] = {};
			printf("[system]%d�ſͻ������ӳɹ�,��ǰ��������%d��\n", cli_fd[index], SEM_SIZE - sem_num);
			sprintf(msg, "[system]�ͻ������ӳɹ�,��ǰ��������%d��\n", SEM_SIZE - sem_num);
			write(cli_fd[index], msg, strlen(msg) + 1);
 
			// �����߳̿ͻ���
			pthread_t tid;
			pthread_create(&tid, NULL, server, &cli_fd[index]);
		}
	}
}