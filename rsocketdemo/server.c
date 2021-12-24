/*socket rdma服务器端*/
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <rdma/rdma_cma.h>
#include <rdma/rsocket.h>
#include <pthread.h>

static const char *port = "5556";
static const char *src_addr = "172.20.10.11";
/*
 监听后，一直处于accept阻塞状态，
 直到有客户端连接，
 当客户端如数quit后，断开与客户端的连接
 */

int poll_timeout = 0;
int ret = 0;

int do_poll(struct pollfd *fds, int timeout)
{
	int ret;

	do {
		ret = rpoll(fds, 1, timeout);
	} while (!ret);

	return ret == 1 ? (fds->revents & (POLLERR | POLLHUP)) : ret;
}

int serverSocket;
int process(int);

int main()
{
	//调用socket函数返回的文件描述符

	//声明两个套接字sockaddr_in结构体变量，分别表示客户端和服务器
	struct sockaddr_in server_addr;
	struct sockaddr_in clientAddr;
	int addr_len = sizeof(clientAddr);
	int client;

	int iDataNum;

	struct rdma_addrinfo *rai = NULL;
	static struct rdma_addrinfo rai_hints;
	rai_hints.ai_port_space = RDMA_PS_TCP;
	rai_hints.ai_flags |= RAI_PASSIVE;
	ret = rdma_getaddrinfo(src_addr, port, &rai_hints, &rai);
	if (ret) {
		printf("getaddrinfo: %s\n", gai_strerror(ret));
		return ret;
	}
	serverSocket = rsocket(AF_INET, SOCK_STREAM, 0);
	printf("rsocket got: %d\n", serverSocket);

	if (serverSocket < 0) {
		perror("socket");
		return 1;
	}

	bzero(&server_addr, sizeof(server_addr));

	ret = rbind(serverSocket, rai->ai_src_addr, rai->ai_src_len);
	// rbind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr))

	if (ret) {
		perror("rbind err");
		return 1;
	}
	//设置服务器上的socket为监听状态
	if (rlisten(serverSocket, 5) < 0) {
		perror("listen");
		return 1;
	}
	printf("Listening on: %s %s\n", inet_ntoa(server_addr.sin_addr), port);
	// listen fd 设为非阻塞
	rfcntl(serverSocket, F_SETFL, O_NONBLOCK);

	struct pollfd lfds;
	lfds.fd = serverSocket;
	lfds.events = POLLIN;
	pthread_t thread_id;
	do {
		printf("will rpoll for accept(%d)\n", serverSocket);

		ret = do_poll(&lfds, poll_timeout);
		if (ret) {
			perror("rpoll");
			continue;
		}

		client = raccept(serverSocket, (struct sockaddr *)&clientAddr,
				 (socklen_t *)&addr_len);
		printf("client is %d", client);
		if (client < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			continue;
		}
		//inet_ntoa   ip地址转换函数，将网络字节序IP转换为点分十进制IP
		//表达式：char *inet_ntoa (struct in_addr);
		printf("IP is %s\n", inet_ntoa(clientAddr.sin_addr));
		printf("Port is %d\n", htons(clientAddr.sin_port));
		printf("\n");
		pthread_create(&thread_id, NULL, process, client);

	} while (1);
	return 0;
}

int process(int cli)
{
	while (1) {
		printf("recv client data...\n");
		struct pollfd afds;
		int poll_timeout = 0;
		afds.fd = cli;
		int iDataNum = 0, recvSum = 0, sendSum = 0;
		char buffer[20000];
		while (1) {
			// 循环poll 直到获取到pollin 事件
			afds.events = POLLIN;

			ret = rpoll(&afds, 1, poll_timeout);
			if (ret > 0) {
				printf("before rrecv rpolled %d\n",
				       afds.revents);
				printf("will rrecv\n");
				iDataNum = rrecv(cli, buffer, 1024, 0);
				if (iDataNum > 0) {
					recvSum += iDataNum;
				} else if (iDataNum == 0) {
					printf("recv EPOLLHUP！\n");
					rclose(cli);
					return iDataNum;
				} else if (errno == EWOULDBLOCK ||
					   errno == EAGAIN) {
					continue;
				}

				buffer[iDataNum] = '\0';
				if (strcmp(buffer, "quit") == 0)
					return iDataNum;
				printf("rrecv bytes: %d, data is %s\n",
				       iDataNum, buffer);
				break;
			} else {
				continue;
			}
		}

		printf("will send data to client...\n");
		while (1) {
			afds.events = POLLOUT;
			ret = rpoll(&afds, 1, poll_timeout);
			if (ret > 0) {
				printf("before rsend rpolled %d\n",
				       afds.revents);
				printf("will rsend\n");
				ret = rsend(cli, buffer, iDataNum, 0);
				if (ret > 0) {
					sendSum += ret;
				} else if (ret == 0) {
					printf("recv EPOLLHUP！\n");
					rclose(cli);
					return ret;
				} else if (errno == EWOULDBLOCK ||
					   errno == EAGAIN) {
					continue;
				}

				printf("rsend bytes: %d, data is %s\n\n",
				       iDataNum, buffer);
				break;
			} else {
				continue;
			}
		}
	}
}