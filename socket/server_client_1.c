两个文件 pserver1.c 及 pclinet1.c

// pserver1.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <sys/un.h>
#include <signal.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 5700
#define MAXLISTEN 1024
#define MAXLINE 204800
#define MSECOND 1000000

const char *serverfile = "serverunixsocket";

/* create server socket*/

static int createServerSocket(const char *ip, const int port, int domain)
{
  int fd;
  int reuse = 1;
  int retval = 0;
  struct sockaddr_in sockserver;
  struct sockaddr_un sockserverunix;

  fd = socket(domain, SOCK_STREAM, 0);
  if (fd < 0)
  {
        fprintf(stderr, "create socket fail, errno : %d, reasion :%s\n", strerror(errno));
        return -1;
  }
  
  /* set noblock mode */

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
  {
    return -1;
  }
  
  /* set ip and port */
  if (domain == AF_INET || domain == AF_INET6)
  {
          bzero(&sockserver, sizeof(sockserver));
          inet_pton(domain, ip,&sockserver.sin_addr);
          sockserver.sin_port = htons(port);
          sockserver.sin_family = domain;
          /* bind the serveraddr */
          if (bind(fd,(struct sockaddr*)&sockserver,sizeof(sockserver)) == -1)
          {
             perror("bind error: ");
             return -1;
          }
  }
  
  if (domain == AF_LOCAL || domain == AF_UNIX)
  {
          sockserverunix.sun_family = domain;
          strcpy(sockserverunix.sun_path, serverfile);
          /* bind the serveraddr */
          if (bind(fd,(struct sockaddr*)&sockserverunix,sizeof(sockserverunix)) == -1)
          {
             perror("bind error: ");
             return -1;
          }
  }


  listen(fd, MAXLISTEN);  
  
  return fd;
}


int main(int argc, char **argv)
{
        int serverfd;
        int clientfd;
        struct sockaddr_in clientaddr;
        int nbyte;
        int max = 10000;
        int n = 0;
        int domain = AF_INET;
        float timeuse = 0;
        socklen_t clientaddrlen;
        clientaddrlen = sizeof(clientaddr);
        char buf[MAXLINE];
        struct sockaddr_un sockserverunix;
        struct timeval timestart, timeend;

        signal(SIGPIPE, SIG_IGN);
        signal(SIGALRM, SIG_IGN);

        max = atoi(argv[1]);

        switch(atoi(argv[2]))
        {
        case 0:
        domain = AF_LOCAL;
        break;
        case 1:
        domain = AF_UNIX;
        break;
        case 2:
        domain = AF_INET;
        break;
        default:
        domain = AF_LOCAL;
        break;
        }

        /* create server socket */
        serverfd = createServerSocket(SERVERIP, SERVERPORT, domain);
        if (serverfd == -1)
                return -1;
   
        /* accept */
        clientfd = accept(serverfd,(struct sockaddr*)&clientaddr,&clientaddrlen);
        if (clientfd == -1)
        {
                fprintf(stdout, "accept error, errno:%d\n", errno);
        }
  
        fprintf(stdout, "client address:%s, port:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);

        /* start time */
        gettimeofday(&timestart,NULL);
 
        while (max-->0)
        {
                /*receive message */
                nbyte = read(clientfd, buf, MAXLINE);
                if (nbyte < 0)
                {
                        fprintf(stderr, "read message from client fail, errno:%d, reason:%s\n", errno, strerror(errno));
                        close(clientfd);
                        close(serverfd);
                        return -1;
                }
                else
                {
                        //fprintf(stdout, "read %d bytes message from client:%s\n", nbyte, "--");
                        /* send the same message to client */
                        nbyte = write(clientfd, buf, MAXLINE);
                        if (nbyte < 0)
                        {
                                fprintf(stderr, "read message from client fail, errno:%d, reason:%s\n", errno, strerror(errno));
                                close(clientfd);
                                close(serverfd);
                                return -1;
                        }
                }
        }
  
        /* time end */
        gettimeofday(&timeend,NULL);
        timeuse = MSECOND * (timeend.tv_sec - timestart.tv_sec) + timeend.tv_usec - timestart.tv_usec;
        timeuse /= MSECOND;
        fprintf(stdout, "time use %f seconds\n", timeuse);

        sleep(1);

        /* close the socket */
        close(clientfd);
        close(serverfd);
        if (domain == AF_LOCAL || domain == AF_UNIX)
                unlink(serverfile);
        return 1;
}


----------------------------------------------------

// pclient1.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <sys/un.h>
#include <signal.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 5700
#define MAXLISTEN 1024
#define MAXLINE 204800
#define MSECOND 1000000

const char *serverfile = "serverunixsocket";

/* create server socket*/

static int connectServerSocket(const char *ip, const int port, int domain)
{
  int fd;
  int reuse = 1;
  int retval = 0;
  struct sockaddr_in sockserver;
  struct sockaddr_un sockserverunix;

  fd = socket(domain, SOCK_STREAM, 0);
  if (fd < 0)
  {
        fprintf(stderr, "create socket fail, errno : %d, reasion :%s\n", strerror(errno));
        return -1;
  }
  
  /* set noblock mode */

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
  {
    return -1;
  }
  
  /* set ip and port */
  if (domain == AF_INET || domain == AF_INET6)
  {
          bzero(&sockserver, sizeof(sockserver));
          inet_pton(domain, ip,&sockserver.sin_addr);
          sockserver.sin_port = htons(port);
          sockserver.sin_family = domain;

          retval = connect(fd,(struct sockaddr*)&sockserver,sizeof(sockserver));
  }
  
  if (domain == AF_LOCAL || domain == AF_UNIX)
  {
          sockserverunix.sun_family = domain;
          strcpy(sockserverunix.sun_path, serverfile);

          retval = connect(fd,(struct sockaddr*)&sockserverunix,sizeof(sockserverunix));
  }
  
  if (retval < 0)
  {
        fprintf(stderr, "connect to server fail, errno:%d, error:%s\n", errno, strerror(errno));
        return -1;
  }
  
  return fd;
}


int main(int argc, char **argv)
{
        int connectfd;
        int max = 10000;
        int domain = AF_INET;
        struct sockaddr_in clientaddr;
        int nbyte;
        int i = 0;
        float timeuse = 0;
        char buf[MAXLINE];
        struct timeval timestart, timeend;

        signal(SIGPIPE, SIG_IGN);
        signal(SIGALRM, SIG_IGN);

        for(i=0; i<MAXLINE; i++)
                 buf[i] = 'a';


        max = atoi(argv[1]);

        switch(atoi(argv[2]))
        {
        case 0:
        domain = AF_LOCAL;
        break;
        case 1:
        domain = AF_UNIX;
        break;
        case 2:
        domain = AF_INET;
        break;
        default:
        domain = AF_LOCAL;
        break;
        }

        /* connect to server */
        connectfd = connectServerSocket(SERVERIP, SERVERPORT, domain);
        if (connectfd == -1)
        {
                fprintf(stdout, "connect server socket error, errno:%d, reason:%s\n", errno, strerror(errno));
                return -1;
        }

        /* time start */
        gettimeofday(&timestart,NULL);
 
        while(max-- > 0)
        {
                /* write the message to server */
                nbyte = write(connectfd, buf, MAXLINE);
                if (nbyte < 0)
                {
                fprintf(stderr, "write message to server fail, errno:%d, reason:%s\n", errno, strerror(errno));
                close(connectfd);
                return -1;
                }
                /*receive message */
                nbyte = read(connectfd, buf, MAXLINE);
                if (nbyte < 0)
                {
                fprintf(stderr, "read message from server fail, errno:%d, reason:%s\n", errno, strerror(errno));
                close(connectfd);
                return -1;
                }
                else
                {
                        //fprintf(stdout, "read %d bytes message from server:%s\n", nbyte, "--");
                }
        }

        /* time end */
        gettimeofday(&timeend,NULL);
        timeuse = MSECOND * (timeend.tv_sec - timestart.tv_sec) + timeend.tv_usec - timestart.tv_usec;
        timeuse /= MSECOND;
        fprintf(stdout, "time use %f seconds\n", timeuse);

        sleep(1);

        /* close the socket */
        close(connectfd);
        if (domain == AF_LOCAL || domain == AF_UNIX)
                unlink(serverfile);
        return 1;
}

笔记：
1.  若server端先关闭，则client端 执行出错：
read message from server fail, errno:104, reason:Connection reset by peer
2. 若clinet端先关闭，则server端 执行出错：
read message from client fail, errno:32, reason:Broken pipe
3. 注意信号SIGPIPE的处理，若server端先关闭，则client端会产生SIGPIPE信号，直接导致client退出运行；若client先退出，则server端会产生SIGPIPE信号，
导致server端退出运行。
4. 若ip=127.0.0.1,AF_INET,则最大速度是：每次发送204800个字节向上附近，约970MB/s (次数*每次最大发送数据大小/时间)
   若设置AF_LOCAL，则最大速度是：每次发送204800个字节，约1700MB/s (次数*每次最大发送数据大小/时间)
5.若AF_LOCAL，每次发送256000字节，则会发现会hang，所以单次发送量需要有限制：比如准备发送256000字节，则单词write 可以是102400字节，循环直至发送
完毕。
6.经过计算，每次发送应该尽可能大些数据，这样传输效率较高。
7.上面都是经过了用户区间到内核区间的来回切换，如果直接使用内核区间不使用用户区间是否更快的传输数据呢？







