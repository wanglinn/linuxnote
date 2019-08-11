/*
* check the port can be connect or not
*/

#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct nodeInfo
{
	int port;
	char *address;
	int sleepMs;
} nodeInfo;

#define CONNECT_OK '1'
#define CONNECT_FAIL '0'

static void showUsage(char *progname);
static int check_port_connect_status(int port, char *address);
static void mgr_check_node_status(void *arg);
static void print_node_result(char *statusArray, int totalNum, int startPort);

static void showUsage(char *progname)
{
	printf("usage:\n"
		   "    -p port    the start port number\n"
		   "    -h host    the address\n"
		   "    -t timeMs  the interval time, unit ms\n"
		   "    -N number  the port from port to port+N\n");
}

/*
* check the port can connect or not
*
*/
static int check_port_connect_status(int port, char *address)
{
	int ret = 1;
	struct sockaddr_in serv_addr;
	int fd;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		fprintf(stderr, "create socket fail : %s\n", strerror(errno));
		return 0;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(address);
	ret = connect(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	close(fd);

	return ret == -1 ? 0 : 1;
}

/*
* check node running status
*/
static void mgr_check_node_status(void *arg)
{
	const int loopMax = 3;
	int result = 1;
	char *res;
	int i = 0;
	nodeInfo *pt_arg;

	res = (char *)malloc(sizeof(char) * 1);
	if (!res)
	{
		fprintf(stderr, "malloc fail : %s\n", strerror(errno));
		exit(1);
	}

	pt_arg = (nodeInfo *)arg;

	while (i++ < loopMax)
	{
		result = check_port_connect_status(pt_arg->port, pt_arg->address);
		if (result)
			break;
		usleep(pt_arg->sleepMs * 1000);
	}
	res[0] = result ? CONNECT_OK : CONNECT_FAIL;
	pthread_exit((void *)res);
}

/*
*
*/
static void print_node_result(char *statusArray, int totalNum, int startPort)
{
	int i = 0;
	while (i < totalNum)
	{
		if (statusArray[i] == CONNECT_OK)
			fprintf(stdout, "connect ok, port=%d\n", startPort + i);
		i++;
	}
}

int main(int argc, char *argv[])
{

	int totalNum = 50;
	int startPort = 1000;
	char address[32] = "127.0.0.1";
	int timeMs = 30;

	if (argc < 3)
	{
		showUsage(argv[0]);
		exit(1);
	}

	char ch;
	while ((ch = getopt(argc, argv, "p:h:t:N:")) != -1)
	{
		switch (ch)
		{
		case 'p':
			startPort = atoi(optarg);
			break;
		case 'h':
			strncpy(address, optarg, 31);
			address[strlen(optarg)] = '\0';
			break;
		case 't':
			timeMs = atoi(optarg);
			break;
		case 'N':
			totalNum = atoi(optarg);
			break;
		default:
			showUsage(argv[0]);
			exit(1);
		}
	}

	int k = 0;
	const int unit = 1000;
	for (k = 0; k < totalNum / unit; k++)
	{
		handle_connect_port(startPort + k * unit, unit, timeMs, address);
	}
	handle_connect_port(startPort + k * unit, totalNum % unit, timeMs, address);
}

/*
* check port connect status, from startPort to startPort + totalNum,
* int the function , create totalNum thread, so totalNum should not be too larger
*/
handle_connect_port(int startPort, int totalNum, int timeMs, char *address)
{
	pthread_t *pthreadIdArray;
	nodeInfo *pt_arg;
	char *statusArray;
	void *rest;
	int i;

	pthreadIdArray = (pthread_t *)malloc(sizeof(pthread_t) * totalNum);
	if (!pthreadIdArray)
	{
		fprintf(stderr, "malloc fail : %s\n", strerror(errno));
		exit(1);
	}

	statusArray = (char *)malloc(sizeof(char) * totalNum);
	if (!statusArray)
	{
		free(pthreadIdArray);
		fprintf(stderr, "malloc fail : %s\n", strerror(errno));
		exit(1);
	}
	pt_arg = (nodeInfo *)malloc(sizeof(nodeInfo) * totalNum);
	if (!pt_arg)
	{
		free(pthreadIdArray);
		free(statusArray);
		fprintf(stderr, "malloc fail : %s\n", strerror(errno));
		exit(1);
	}

	i = 0;
	int j = 0;
	while (i < totalNum)
	{
		pt_arg[i].port = startPort + i;
		pt_arg[i].address = (char *)malloc(sizeof(char) * strlen(address) + 1);
		if (!pt_arg[i].address)
		{
			j = 0;
			while (j++ < i)
				free(pt_arg[j].address);
			fprintf(stderr, "malloc fail : %s\n", strerror(errno));
			exit(1);
		}
		memcpy(pt_arg[i].address, address, strlen(address));
		pt_arg[i].address[strlen(address)] = '\0';
		pt_arg[i].sleepMs = timeMs;
		i++;
	}

	i = 0;
	/* create thread */
	while (i < totalNum)
	{
		pthread_create(&pthreadIdArray[i], NULL, (void *)mgr_check_node_status, &pt_arg[i]);
		i++;
	}

	i = 0;
	/* join thread */
	while (i < totalNum)
	{
		pthread_join(pthreadIdArray[i], &rest);
		statusArray[i] = *(char *)rest;
		free(pt_arg[i].address);
		free(rest);
		i++;
	}

	free(pthreadIdArray);
	free(pt_arg);

	/* pirnt result */
	print_node_result(statusArray, totalNum, startPort);
	free(statusArray);

	return 1;
}
