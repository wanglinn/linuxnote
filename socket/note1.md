#1 代码

client1.c   server1.c  user_type.h

#2 笔记
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
  《Linux高性能服务器编程》这本书的第6章提供了一些函数，可以直接使用内核区，还没测试。
  零拷贝 参考：
  https://blog.csdn.net/lovekatherine/article/details/1540291
8. 将上面的临时区间buf 改为建在共享内存shared buffer中，测试发现，没有提高效率。
9. 跨机器通信代价很大，比如每次传输1024字节，传输100000次，本机TCP通信用时2秒多，跨机器用了9.5秒。
10.write 函数里面写的长度，不同机器发生阻塞的情况不同，这个单次write长度还是需要仔细研究。
11. 阻塞与非阻塞，一般用在I/O多路复用中，参考：
https://www.cnblogs.com/senior-engineer/p/6163677.html
https://www.cnblogs.com/msb-/articles/6042413.html
上面5的问题应该是：单次发送过大，对端每次读只读取一部分数据就返回，这样内核缓冲区有可能被占满，由于读取一次就返回当再次执行write函数则无法写入，
从而发生hang 问题，所以read write 需要循环直到read返回0或者write返回0表示读结束或者写结束。
:: 将read, write 改为不断循环直至返回为0， 发现发送基本上都可以一次发送完毕，但是接收需要不断循环每次只能接收xxkb 是个动态调整的过程。
