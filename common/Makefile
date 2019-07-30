1. sample1
[wl@host122 02]$ cat defs.h
#include <stdio.h>

int v1;

[wl@host122 02]$ cat defs.c
#include "defs.h"

void func1()
{
  printf("func1\n");
}
[wl@host122 02]$ cat main.c 
#include <stdio.h>
#include "defs.h"

int main()
{
  v1 = 2;
  printf("%d\n", v1);
 
  func1();

}

[wl@host122 02]$ cat Makefile 
main : main.o defs.o
        cc -o main main.o defs.o

defs.o : defs.c
        cc -c defs.c

main.o : main.c
        cc -c main.c



参考：

https://blog.csdn.net/baidu_38172402/article/details/88864517





