# libevent 测试

## IO模型

### BIO（Blocking I/O）：阻塞I/O

* 同步并阻塞
* 一个连接一个线程
* 适用于：连接数比较小且固定。
* 流 I/O，以流的方式处理数据。

### NIO（Non-Blocking IO）：非阻塞I/O

* 同步非阻塞
* 适用于：连接数目多且连接比较短。
* 块 I/O，以块的方式处理数据。
* 可实现一种模式：Reactor。
* 内核将可读可写事件通知应用，由应用主动发起读写操作。

### AIO（Asynchronous I/O）：异步I/O

* 异步非阻塞
* 适用于：连接数目多且连接比较长。
* 块 I/O，以块的方式处理数据。
* 可实现两种模式：Reactor 和 Proactor。
* 内核将读完成事件通知应用，读操作由内核完成，应用只需操作数据即可；应用做异步写操作时立即返回，内核会进行写操作排队并执行写操作。

## 线程模型

### Reactor 单线程模型

### Reactor 多线程模型

### Reactor 主从多线程模型

## 安装依赖库

```bash
sudo apt install libevent-dev
sudo apt install libfmt-dev
```

## 参考资料

* [Netty's efficient reactor thread model](https://javamana.com/2021/09/20210909151058484V.html)
* [彻底搞懂 netty 线程模型](https://www.cnblogs.com/luoxn28/p/11875340.html)
* [libevent 线程池的设计](https://www.jianshu.com/p/d9e161acdd40)
* [多线程使用libevent：那些花里胡哨的技巧](https://segmentfault.com/a/1190000040121458)
* [libevent服务端，单线程应用](https://blog.csdn.net/c1s2d3n4cs/article/details/124255380)
* [理解什么是BIO/NIO/AIO](https://segmentfault.com/a/1190000037714804)
* [JAVA中BIO、NIO、AIO的分析理解](https://developer.aliyun.com/article/726698)
