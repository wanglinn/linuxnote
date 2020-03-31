# 1. libevent介绍 #
libevent是一个网络通信库，提供一系列API，用来处理定时任务。
可以应用于多线程环境。

头文件 #incldue<event.h>

主要数据结构
struct event ev;

## 1.1 事件通知机制 ##
event_init 初始化  
event_add 将事件添加到监控事件列表中  
event_dispatch 执行函数循环机调度事件  

## 1.2 定时器 ##
struct event ev 通过 evtimer_set 关联自定义函数(事件)：  
evtimer_set(&ev, time_cb, NULL);  
其中time_cb 是用户自定义函数。  

evtimer_add 激活定时器
event_add(&ev, &tv);  
evtimer_del 取消定时器

## 1.3 超时 ##
函数  
timeout_set 超时事件设置  
timeout_add 超时处理激活  
timeout_del 超时处理取消  

## 1.3 test1.cpp ##
~~~
/* test1.cpp
 * g++ -g -o ${1} ${1}.c  -I ~/software/install/libevent/include  
 *    -L ~/software/install/libevent/lib -levent
 */

#include <event.h>

#include <iostream>

struct event ev;
struct timeval tv;

using namespace std;


int num = 0;
void time_cb(int fd, short event, void *argc)
{
    num++;
    cout << "timer wakeup : " << num << endl;
    evtimer_add(&ev, &tv);
    if (num == 5)
      evtimer_del(&ev);
}


int main()
{
    struct event_base *base = event_init();

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    evtimer_set(&ev, time_cb, NULL);
    evtimer_add(&ev, &tv);
    event_dispatch();

    return 0;
}

~~~

