# 1. vector 向量的使用
常用的函数  
vector<type>::iterator it  -- 迭代器  
vector.pop_back()          --删除向量最后一个成员  
vector.push_back(value)    --添加成员到向量最后  
vector.size()              -- size_t 向量成员数量  
vector.resize(num, value)  --调整向量成员个数,保留前num个，成员值改为value  
vector.resize(num)         --调整向量成员个数,保留前num个  
vector.front()             --type, 获取向量第一个成员值  
vector.back()              --type,获取向量最后一个成员值  
vector.empty()             --bool, 判断是否位空  
vector.clear()             --清空向量  

void assign(const_iterator first,const_iterator last);  
void assign(size_type n,const T& x = T());  
将区间[first,last)的元素赋值到当前的vector容器中，或者赋n个值为x的元素到vector容器中，这个容器会清除掉vector容器中以前的内容  

# 2.测试程序

# 2.1 vector 成员位int
```
#include<iostream>
#include<vector>
using namespace std;

void printVector1(vector<int> vt1)
{
    vector<int>::iterator it;

    if(vt1.empty())
    {
        cout<<"the vector is empty";
    }

    for(it=vt1.begin(); it!=vt1.end(); it++)
    {
        cout<<*it<<",";
    }
    cout<<endl;
}

void printVector2(vector<int> vt1)
{
    int i = 0;
    if(vt1.empty())
    {
        cout<<"the vector is empty";
    }

    for(i=0; i<vt1.size(); i++)
    {
        cout<<vt1[i]<<",";
    }
    cout<<endl;
}


int main()
{
    vector<int> vt1(5,2);
    int i = 0;
    int max = 10;
    for(i=0; i<max; i++)
    {
        vt1.push_back(i);
    }
    printVector1(vt1);
    printVector2(vt1);

    vt1.resize(10);
    //vt1.pop();   -- 无此函数
    vt1.pop_back();
    printVector2(vt1);

    vt1.clear();
    printVector2(vt1);
    vt1.push_back(1);

    vector<int> vt2;
    vt2.assign(vt1.begin(), vt1.end());
    printVector2(vt2);
    
}
```
执行结果：  
[wl@host122 cpp]$ ./vector1  
2,2,2,2,2,0,1,2,3,4,5,6,7,8,9,  
2,2,2,2,2,0,1,2,3,4,5,6,7,8,9,  
2,2,2,2,2,0,1,2,3,  
the vector is empty  
1,  


# 2.2 vector成员位结构体

```
#include<iostream>
#include<vector>
#include<cstring>
#include<stdio.h>
using namespace std;

typedef struct T_ST1
{
    int a;
    int b;
    char *p;
/*
    T_ST1()
    {
        p = new char(10);
        strcpy(p, "1234567890");
        cout<<"construction"<<endl;
    }
    ~T_ST1()
    {
        delete p;
        cout<<"destruction"<<endl;
    }
*/
}st1;

int main()
{
    st1 *x1 = new st1;
    cout<<"x1 addr = "<<x1<<endl;

    x1->a = 1;
    x1->b=2;
    x1->p = new char(10);
    strcpy(x1->p, "123456789a");

    vector<st1 *> v1;
    v1.push_back(x1);

    st1 *xx1 = v1.front();
    cout<<"xx1 addr = "<<xx1<<endl;
    cout<<xx1->a<<','<<xx1->b<<','<<xx1->p<<endl;

    delete x1->p;
    delete x1;

}
```
执行结果  
[wl@host122 cpp]$ ./vector2  
x1 addr = 0x2065010  
xx1 addr = 0x2065010  
1,2,123456789a  


# 2.3 vector成员位结构退，结构体有构造函数及析构函数

vector中保存的结构体的指针地址，而不是结构体对象。  
结构体：在new 时调用了结构体中的构造函数，  
在delete时调用了结构体中的析构函数  

```
#include<iostream>
#include<vector>
#include<cstring>
#include<stdio.h>
using namespace std;

typedef struct T_ST1
{
    int a;
    int b;
    char *p;

    T_ST1()
    {
        p = new char(10);
        strcpy(p, "1234567890");
        cout<<"construction"<<endl;
    }
    ~T_ST1()
    {
        delete p;
        cout<<"destruction"<<endl;
    }

}st1;

int main()
{
    st1 *x1 = new st1;
    cout<<"x1 addr = "<<x1<<endl;

    x1->a = 1;
    x1->b=2;
//    x1->p = new char(10);
    strcpy(x1->p, "123456789a");

    vector<st1 *> v1;
    v1.push_back(x1);

    st1 *xx1 = v1.front();
    cout<<"xx1 addr = "<<xx1<<endl;
    cout<<xx1->a<<','<<xx1->b<<','<<xx1->p<<endl;

  //  delete x1->p;
    delete x1;

}

```
执行结果  
[wl@host122 cpp]$ ./vector3  
construction  
x1 addr = 0x11c1010  
xx1 addr = 0x11c1010  
1,2,123456789a  
destruction   

# 2.4 vector中保存结构体对象
如果vector中保存的结构体对象，而非指针，那么是否可以在结构体中包含构造函数及析构函数，  
代码如下：  

```
#include<iostream>
#include<vector>
#include<cstring>
#include<stdio.h>
using namespace std;

typedef struct T_ST1
{
    int a;
    int b;
    char *p;

    T_ST1()
    {
        p = new char(10);
        strcpy(p, "1234567890");
        cout<<"construction"<<endl;
    }
    ~T_ST1()
    {
     //   delete p;
     //   p = NULL;
        cout<<"destruction"<<endl;
    }

}st1;

int main()
{
    //st1 *x1 = new st1;
    st1 x1;
    cout<<"x1 addr = "<<&x1<<endl;

    x1.a = 1;
    x1.b=2;
//    x1->p = new char(10);
    strcpy(x1.p, "123456789a");

    st1 x2;
    x2.a = 2;
    x2.b = 12;

    st1 x3;
    x3.a = 3;
    x3.b = 13;

    cout<<"--1"<<endl;
 
    vector<st1> v1;
    v1.push_back(x1);
    v1.push_back(x2);
    v1.push_back(x3);

    cout<<"--2"<<endl;
//    st1 *xx1 = &(v1.front());
//    cout<<"xx1 addr = "<<xx1<<endl;
//    cout<<xx1.a<<','<<xx1.b<<','<<xx1.p<<endl;

  //  delete x1->p;
  //  delete x1;

      getchar();
}
```
执行结果： 
```
[wl@host122 cpp]$ ./vector4
construction
x1 addr = 0x7ffd97522d60
construction
construction
--1
destruction
destruction
destruction
--2

destruction
destruction
destruction
destruction
destruction
destruction
```
解释如下：  
st1 x3; -- 会调用构造函数  
v1.push_back(x1);  --由于vector是一个连续分配的内存空间，这里可能需要扩容，则可能涉及  
原有数据的销毁，这样就会调用析构函数。  
vector在程序结束时会调用自身对应的析构函数，释放元素对象占用的内存空间，这样可能会调用  
元素的析构函数。  
在push_back前，不断查看&(v1.front())的地址，会发现每次push_back该地址都会变化，也就是  
每次push_back该vector都在经历“重新配置、元素移动、释放原空间”。  
使用vector迭代器时要时刻注意vector是否发生了扩容，一旦扩容引起了空间重新配置，  
指向原vector的所有迭代器都将失效。　  

```
(gdb) bt
#0  std::_Destroy<T_ST1*, T_ST1> (__first=0x6040c0, __last=0x6040f0) at /usr/include/c++/4.8.2/bits/stl_construct.h:151
#1  0x0000000000400e74 in std::vector<T_ST1, std::allocator<T_ST1> >::~vector (this=0x7fffffffe0e0, __in_chrg=<optimized out>)
    at /usr/include/c++/4.8.2/bits/stl_vector.h:415
#2  0x0000000000400cb8 in main () at vector4.cpp:49
(gdb) 


(gdb) bt
#0  T_ST1::~T_ST1 (this=0x6040c0, __in_chrg=<optimized out>) at vector4.cpp:23
#1  0x00000000004018c8 in std::_Destroy<T_ST1> (__pointer=0x6040c0) at /usr/include/c++/4.8.2/bits/stl_construct.h:93
#2  0x000000000040170e in std::_Destroy_aux<false>::__destroy<T_ST1*> (__first=0x6040c0, __last=0x6040f0) at /usr/include/c++/4.8.2/bits/stl_construct.h:103
#3  0x000000000040141d in std::_Destroy<T_ST1*> (__first=0x6040c0, __last=0x6040f0) at /usr/include/c++/4.8.2/bits/stl_construct.h:126
#4  0x0000000000400ff9 in std::_Destroy<T_ST1*, T_ST1> (__first=0x6040c0, __last=0x6040f0) at /usr/include/c++/4.8.2/bits/stl_construct.h:151
#5  0x0000000000400e74 in std::vector<T_ST1, std::allocator<T_ST1> >::~vector (this=0x7fffffffe0e0, __in_chrg=<optimized out>)
    at /usr/include/c++/4.8.2/bits/stl_vector.h:415
#6  0x0000000000400cb8 in main () at vector4.cpp:49


(gdb) bt
#0  T_ST1::~T_ST1 (this=0x604070, __in_chrg=<optimized out>) at vector4.cpp:23
#1  0x00000000004018c8 in std::_Destroy<T_ST1> (__pointer=0x604070) at /usr/include/c++/4.8.2/bits/stl_construct.h:93
#2  0x000000000040170e in std::_Destroy_aux<false>::__destroy<T_ST1*> (__first=0x604070, __last=0x604080) at /usr/include/c++/4.8.2/bits/stl_construct.h:103
#3  0x000000000040141d in std::_Destroy<T_ST1*> (__first=0x604070, __last=0x604080) at /usr/include/c++/4.8.2/bits/stl_construct.h:126
#4  0x0000000000400ff9 in std::_Destroy<T_ST1*, T_ST1> (__first=0x604070, __last=0x604080) at /usr/include/c++/4.8.2/bits/stl_construct.h:151
#5  0x0000000000401233 in std::vector<T_ST1, std::allocator<T_ST1> >::_M_insert_aux (this=0x7fffffffe0e0, __position=..., __x=...)
    at /usr/include/c++/4.8.2/bits/vector.tcc:384
#6  0x0000000000400f1c in std::vector<T_ST1, std::allocator<T_ST1> >::push_back (this=0x7fffffffe0e0, __x=...) at /usr/include/c++/4.8.2/bits/stl_vector.h:913
#7  0x0000000000400c78 in main () at vector4.cpp:51
```

# 3.参考
C++ STL中的vector的内存分配与释放  
https://www.linuxidc.com/linux/2014-05/102381.htm  

C++ 序列式容器之vector  
https://www.linuxidc.com/Linux/2015-07/119821.htm  

List源码解析之Vector 源码分析  
https://www.linuxidc.com/Linux/2017-11/148893.htm  



