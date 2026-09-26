# futex 锁机制
  看oracle文档描述，使用了信号量，sem_post, sem_wait, 和AI 讨论了背后的锁机制，内核层面的futex等待队列。
：：理解了信号量背后的等待处理的futex锁机制， 线程锁背后的锁机制。

参考
什么是fetux (https://zhuanlan.zhihu.com/p/568678633)
Futex详解：用户态锁、内核态等待、pthread_mutex实现 (https://blog.csdn.net/2301_78967994/article/details/153686408)

# uint64_t d = atomic_load_relaxed (&sem->data);   // 只是先瞄一眼（可能旧值，无所谓）
：：为啥不写成 uint64_t d = sem->data; 现在64 位数值赋值 也是原子性的吧？ 之前不是原子性


# 普通C 程序 也就是用户态 可以调用 atomic_load_relaxed ， atomic_compare_exchange_weak_acquire 吗？

好问题。在 x86-64 上你的直觉是对的：**对齐的 64 位 `mov` 硬件层面确实不撕裂**。但直接写 `uint64_t d = sem->data;` 仍然是错的，因为它赌的是"硬件碰巧如此"，而 C 语言层面这是**未定义行为（data race）**。`atomic_load_relaxed` 买的不是 x86 的原子性，而是三样更根本的保证。

## 三层原因

### 1. C 语义层：数据竞争 = UB

C11 内存模型规定：两个线程访问同一对象，至少一个是写，且没有 happens-before 关系 → **未定义行为**。`sem->data` 被其他线程的原子 RMW 并发修改，你用普通 load 读它，就构成 data race。

UB 的恐怖之处不在于"这次运行结果错"，而在于**编译器获得license做任意优化**：

```c
uint64_t d = sem->data;   // 普通 load，编译器认为没人跟它抢
```

编译器可以假设无竞争，于是：

- 认为 `sem->data` 在这条语句前后不会变（没有同步关系证明它会变），把 load 提前/延后、合并、缓存复用；
- 在循环里可能直接复用第一次读到的值，不再真正去内存读。

代码"看起来读了一次内存"，实际可能根本没读。行为取决于优化级别和编译器心情。

### 2. 硬件层：x86-64 的原子是 ISA 特性，不是 C 保证

- **x86-64**：对齐 64 位 load/store 是单指令且 CPU 保证原子（Pentium 之后的语义）。碰巧安全。
- **32 位 ARM**：64 位 load 编译成两条指令：

```asm
ldr r0, [p]      ; 低 32 位
ldr r1, [p, #4]  ; 高 32 位   ← 中间可能被抢占/中断
```

另一个核此刻恰好低 32 位写一次、高 32 位写一次，你会读到一个**从未存在过的混合值**：低半是新的、高半是旧的。`(d & SEM_VALUE_MASK)` 可能误判有 token，或者 mask 算出的计数是乱码。

`atomic_load` 在 ARM 上会生成 `ldrd`/`ldrexd` 或加 barrier 的序列，保证 64 位整体读出。

### 3. 内存序层：原子性 ≠ 同步协议

就算 load 本身原子，你也得告诉编译器和 CPU **这次访问在协议中扮演什么角色**：

- `relaxed`：只保证"完整读出最新某个值 + 不撕裂"，不建立顺序——用于"瞄一眼";
- 如果不加任何标注（普通 load），编译器/CPU 可能把它重排到其他操作的前后，破坏和 `sem_post` 之间靠 acquire/release 搭建的同步。

## 顺便说一个细节

如果变量声明成 C11 `_Atomic uint64_t`，那么 `d = sem->data` 其实**是合法的**，会被解释为 **seq_cst** 原子 load——但语义太强了（seq_cst 要额外阻止重排，比 relaxed 贵），而且 glibc 内部 atomic 宏层的变量并不是 `_Atomic` 类型，所以必须显式走宏。

## 一句话总结

> `uint64_t d = sem->data` 在 x86-64 上"碰巧对"：硬件单指令不撕裂。但 C 编译器眼里这是 data race，可以拆分、缓存、重排，32 位平台上还会物理撕裂；`atomic_load_relaxed` 是用 C 语义正式声明"这是一次不可分割的、按 relaxed 协议参与的原子读"——买的是语言契约和可移植性，不只是那一条机器指令。


# AI回答
可以，而且这正是它们的主要用途场景。但要分清两件事：**"能不能用原子操作"** 和 **"能不能用这两个名字"**。

## 1. 这两个名字本身：不能直接用

`atomic_load_relaxed`、`atomic_compare_exchange_weak_acquire` 是 **glibc 内部宏**（定义在 glibc 源码的 `include/atomic.h`），不是公开 API，普通程序包含 `<stdio.h>` 之类是找不到它们的。普通用户态程序应该用**标准 C11 原子接口**：

```c
#include <stdatomic.h>

atomic_uint_fast64_t sem_data = 0;   // 声明为 _Atomic 类型

uint64_t d = atomic_load_explicit(&sem_data, memory_order_relaxed);

uint64_t expected = d;
bool ok = atomic_compare_exchange_weak_explicit(
              &sem_data, &expected, d - 1,
              memory_order_acquire,      // 成功时的内存序
              memory_order_relaxed);     // 失败时的内存序
```

对照关系：

| glibc 内部宏 | 标准 C11 | gcc builtin |
|---|---|---|
| `atomic_load_relaxed(p)` | `atomic_load_explicit(p, memory_order_relaxed)` | `__atomic_load_n(p, __ATOMIC_RELAXED)` |
| `atomic_compare_exchange_weak_acquire(p,e,d)` | `atomic_compare_exchange_weak_explicit(p,e,d,memory_order_acquire,memory_order_relaxed)` | `__atomic_compare_exchange_n(p,e,d,1,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED)` |

编译不需要特殊选项，`gcc test.c`（默认 gnu11）即可，或者显式 `-std=c11`。

## 2. 原子操作用户态完全可用，且通常不进内核

C11 原子操作（以及 gcc 的 `__atomic_*` builtins）由**编译器直接生成硬件指令**：

- x86-64：`lock cmpxchg`、`xadd`、`mov`（带 lock 前缀的）
- ARM：`ldaxr/stlxr`、`ldrex/strex` 等

整条路径是：你的 C 代码 → 编译器 → 几条 CPU 指令。**没有任何系统调用**。只有在"需要睡眠等待"的场景（如锁被占用），软件层面才会在原子操作失败后再调用 `futex` 陷入内核——原子操作本身永远用户态完成。

## 3. 一个完整的用户态例子

```c
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

atomic_int lock = 0;   // 0=空闲, 1=持有

void acquire(void)
{
    int expected;
    while (1) {
        expected = 0;
        // 原子地：如果 lock==0 就置 1，成功则拿到锁
        if (atomic_compare_exchange_weak_explicit(&lock, &expected, 1,
                memory_order_acquire, memory_order_relaxed))
            return;
        // 失败：自选等待（真实场景应 pause + futex）
    }
}

void release(void)
{
    atomic_store_explicit(&lock, 0, memory_order_release);
}

int counter = 0;

void *worker(void *arg)
{
    for (int i = 0; i < 1000000; i++) {
        acquire();
        counter++;
        release();
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker, NULL);
    pthread_create(&t2, NULL, worker, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter = %d (期望 2000000)\n", counter);
    return 0;
}
```

编译：

```bash
gcc -O2 test.c -lpthread -o test
```

这就是无锁/细粒度同步的标准用户态写法。之前分析的 glibc `sem_wait` 快路径，`pthread_mutex_lock` 的无竞争路径，底层都是同样的东西：C 原子操作生成 `lock cmpxchg`，抢不到才 `futex`。

## 4. 演进脉络（了解即可）

- `__sync_*` builtins：gcc 4.1 时代的老接口，只提供 full barrier，已过时；
- `__atomic_*` builtins：gcc 4.7+，可指定内存序，glibc 内部大量在用；
- `<stdatomic.h>`：C11 标准（2011），新代码应使用它；
- C++ 侧对应 `<atomic>` + `std::memory_order_relaxed/acquire/...`。

一句话：**用户态不仅可以，而且本来就该用原子操作；只是要用标准 `<stdatomic.h>` 的名字，别抄 glibc 内部宏。**


 

