/*
 * 对应client1.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>

#include "user_type.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5700
#define MAX_LISTEN 1024
#define MAX_BYTES 204800

const int8 *serverfile = "serverUnixSocket";

static inline int32 setSocketOpt(int32 fd)
{
    int32 reuse = 1;
    /* set noblock mode */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        return -1;
    }
    return 0;
}

static int32 doInetBind(const int32 fd, const int8 *ip, const int32 port, int32 domain)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    inet_pton(domain, ip,&addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_family = domain;
    if (bind(fd,(struct sockaddr*)&addr,sizeof(addr)) == -1) {
        perror("bind error: ");
        return -1;
    }

    return 0;
}

static int32 doUnixBind(const int32 fd, const int32 domain)
{
    struct sockaddr_un addr;
    addr.sun_family = domain;
    strcpy(addr.sun_path, serverfile);
    /* bind the serveraddr */
    if (bind(fd,(struct sockaddr*)&addr,sizeof(addr)) == -1)
    {
        perror("bind error: ");
        return -1;
    }

    return 0;
}

static int32 bindSocket(const int32 fd, const int8* ip, const int32 port, const int32 domain)
{
    switch (domain) {
    case AF_INET:
    case AF_INET6:
        if (doInetBind(fd, ip, port, domain) == -1) {\
            return -1;
        }
        break;
    case AF_UNIX:
        if (doUnixBind(fd, domain) == -1) {
            return -1;
        }
        break;
    default:
        return -1;
    }

    return 0;
}

/* create server socket*/
static int32 createServerSocket(const int8 *ip, const int32 port, int32 domain)
{
    int32 fd = socket(domain, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "create socket fail, errno : %d, reasion :%s\n", strerror(errno));
        return -1;
    }
  
    if (setSocketOpt(fd) == -1) {
        return -1;
    }

    if (bindSocket(fd, ip, port, domain) == -1) {
        return -1;
    }

    return fd;
}


static inline void setSignal()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
}

static void showUsage(int8* argv[])
{
    printf("usage:\n");
    printf("\t%s sendNum socket_method\n", argv[0]);
    exit(1);
}

static void getArgs(int argc, int8* argv[], int32* sendNum, int32* domain)
{
    if (argc != 3) {
        showUsage(argv);
    }

    *sendNum = atoi(argv[1]);
    switch(atoi(argv[2])) {
    case 0:
        *domain = AF_UNIX;
        break;
    case 1:
        *domain = AF_INET;
        break;
    case 2:
        *domain = AF_INET6;
        break;
    default:
        *domain = AF_UNIX;
        break;
    }
}

static int32 acceptSocket(const int32 serverFd, int32* clientFd)
{
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    *clientFd = accept(serverFd,(struct sockaddr*)&clientaddr,&clientaddrlen);
    if (*clientFd == -1) {
        fprintf(stdout, "accept error, errno:%d\n", errno);
    }
    fprintf(stdout, "client address:%s, port:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);

    return 0;
}

static int32 sendRecvMsg(const int32 sendNum, const int32 serverFd, const int32 clientFd)
{
    int8 buf[MAX_BYTES];
    int32 times = sendNum;
    while (times-->0) {
        /*receive message */
        int32 nbyte = read(clientFd, buf, MAX_BYTES);
        if (nbyte < 0) {
            fprintf(stderr, "read message from client fail, errno:%d, reason:%s\n", errno, strerror(errno));
            close(clientFd);
            close(serverFd);
            return -1;
        } else {
            /* send the same message to client */
            nbyte = write(clientFd, buf, MAX_BYTES);
            if (nbyte < 0) {
                fprintf(stderr, "read message from client fail, errno:%d, reason:%s\n", errno, strerror(errno));
                close(clientFd);
                close(serverFd);
                return -1;
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    setSignal();

    int32 sendNum;
    int32 domain;
    getArgs(argc, (int8**)argv, &sendNum, &domain);

    /* create server socket */
    int32 serverFd = createServerSocket(SERVER_IP, SERVER_PORT, domain);
    if (serverFd == -1) {
        return -1;
    }

    if (listen(serverFd, MAX_LISTEN) == -1) {
        close(serverFd);
        perror("listen error: ");
        return -1;
    }

    int32 clientFd;
    if (acceptSocket(serverFd, &clientFd) == -1) {
        close(serverFd);
        return -1;
    }

    RECORD_TIME_START();

    if (sendRecvMsg(sendNum, serverFd, clientFd) == -1) {
        close(clientFd);
        close(serverFd);
        return -1;
    }

    RECORD_TIME_END();

    sleep(1);

    /* close the socket */
    close(clientFd);
    close(serverFd);
    if (domain == AF_UNIX) {
        unlink(serverfile);
    }
    return 1;
}
