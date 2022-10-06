/*
 * 对应 server1.c
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
#define MAX_BYTES 204800

const int8 *serverfile = "serverUnixSocket";

static inline int32 doInetConn (const int32 fd, const int8 *ip, const int32 port, int32 domain)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    inet_pton(domain, ip,&addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_family = domain;
    return connect(fd,(struct sockaddr*)&addr,sizeof(addr));
}

static inline int32 doLocalConn(const int32 fd, const int32 domain)
{
    struct sockaddr_un addr;
    addr.sun_family = domain;
    strcpy(addr.sun_path, serverfile);
    return connect(fd,(struct sockaddr*)&addr,sizeof(struct sockaddr_un));
}

static int32 conn(const int32 fd, const int8 *ip, const int32 port, int32 domain)
{
    int32 res = -1;
    switch (domain) {
        case AF_INET:
        case AF_INET6:
            res = doInetConn(fd, ip, port, domain);
            break;
        case AF_UNIX:
            res = doLocalConn(fd, domain);
            break;
        default:
            return -1;
    }
  
    if (res < 0) {
        fprintf(stderr, "connect to server fail, errno:%d, error:%s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

static inline int32 setSocketOpt(int32 fd)
{
    int32 reuse = 1;
    /* set noblock mode */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        return -1;
    }
    return 0;
}

/* create server socket*/
static int32 connServer(const int8 *ip, const int32 port, int32 domain)
{
    int32 fd = socket(domain, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "create socket fail, errno : %d, reasion :%s\n", strerror(errno));
        return -1;
    }

    if (conn(fd, ip, port, domain) == -1) {
        return -1;
    }

    if (setSocketOpt(fd) == -1) {
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

static int32 sendRecvMsg(int32 sendNum, int32 connFd, int8* buf)
{
    while(sendNum-- > 0) {
        /* write the message to server */
        int32 nbyte = write(connFd, buf, MAX_BYTES);
        if (nbyte < 0) {
            fprintf(stderr, "write message to server fail, errno:%d, reason:%s\n", errno, strerror(errno));
            close(connFd);
            return -1;
        }
        /*receive message */
        nbyte = read(connFd, buf, MAX_BYTES);
        if (nbyte < 0) {
            fprintf(stderr, "read message from server fail, errno:%d, reason:%s\n", errno, strerror(errno));
            close(connFd);
            return -1;
        }
    }

    return 0;
}

int32 main(int argc, char *argv[])
{
    setSignal();

    int8 buf [MAX_BYTES];
    for(int32 i=0; i<MAX_BYTES; i++) {
        buf[i] = 'a';
    }

    int32 sendNum;
    int32 domain;
    getArgs(argc, (int8**)argv, &sendNum, &domain);

    /* connect to server */
    int32 connFd = connServer(SERVER_IP, SERVER_PORT, domain);
    if (connFd == -1) {
        fprintf(stdout, "connect server socket error, errno:%d, reason:%s\n", errno, strerror(errno));
        return -1;
    }

    RECORD_TIME_START();
 
    sendRecvMsg(sendNum, connFd, buf);

    RECORD_TIME_END();

    sleep(1);
    /* close the socket */
    close(connFd);
    if (domain == AF_UNIX) {
        unlink(serverfile);
    }

    return 1;
}
