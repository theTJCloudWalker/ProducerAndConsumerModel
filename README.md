# 设计方案
同济大学2022年操作系统第一次作业
## 任务描述 

-  有界缓冲区有20个存储单元，存取数据为1-20这20个整型数；

- 当生产者和消费者对有界缓冲区进行操作后,即时显示有界缓冲区的全部内容、当前位置、生产者/消费者进程的标识符；

- 生产者和消费者各有两个以上；

## 界面

![image-20220514223256862](https://gitee.com/thecloudwalker/img/raw/master/markdown/image-20220514223256862.png)

- 符号及色彩标识

  >- 绿色表示与生产者相关，红色表示与消费者相关
  >
  >- 用绿色\表示生产者即将填入数据的位置，红色/表示消费者下次消费开始的位置，以此表示当前位置

- 缓冲区状态

  >用下述两者展示缓冲区的全部内容：
  >
  >黄色的index行代表大小为20的缓冲区各存储单元下标；
  >
  >蓝色的Buffer[]代表对应buffer[index]处存储的整型数；

- 进程标识

  >程序运行在windows系统下，故用进程id标识进程
  >
  >- main函数为主进程，id显示在第二行；
  >- 正在对缓冲区操作的进程id显示在第三行；

- 操作显示

  >与生产者指针与消费者指针同行会有当前线程操作的数据，不同的是，生产者将新生成的数字列出，并填入缓冲区；而被消费者操作过的缓冲区会置零(合法的数字是1-20)，被消费的缓冲区数据会列在对应消费者的行。
  >
  >另外，在正在运行的进程id下一行，会显示

- 单步运行

  >借助_getch()函数实现任意键单步执行执行，空格键退出的功能。

## 算法

最基础的生产者消费者问题由两个进程共享一个公共的固定大小的缓冲区。其中一个是生产者，将信息放入缓冲区，相当于只写；另外一个是消费者，从缓冲区中取出消息，相当于只读。

我们可以使用P,V操作实现生产者和消费者线程的功能。

~~~
semaphore mutex = 1;//互斥信号量 实现对缓冲区的互斥访问
semaphore empty = n;//同步信号量 表示空闲缓冲区的数量
semaphore full = 0;//同步信号量 表示产品数量 也是非空缓冲区的数量

producer(){
	//生产者	
	while(1){
		生产一个产品
		P(empty);
		P(mutex);
		把产品放入缓存区
		V(mutex);
		V(full);
	}
}

consumer(){
	//消费者	
	while(1){
		P(full);
		P(mutex);
		从缓冲区取出一个产品
		V(mutex);
		V(empty);
		使用产品
	}
}
~~~

当缓冲区被写满时，生产者线程不能进行操作，只能等待可写空间；缓冲区为空时，消费者不能进行操作，只能等待可读空间；缓冲区是临界资源，各进程只能互斥地访问。这可以借助互斥信号量来完成；

需要注意的是，实现互斥的P操作一定要在实现同步的P操作之后，即先同步，后互斥，否则会出现生产者和消费者循环等待对方唤醒自己的死锁。

上述的代码只能实现各线程按固定顺序依次读写缓冲区，真实的生产情况下生产者与消费者出现的时机是随机的，每次读写的缓冲区数据量也是不确定的，因此这里借助互斥锁与c++11的random库。

~~~
typedef int semaphore;//自定义信号量

semaphore empty = CAPACITY;
semaphore full = 0;

//生产者函数
void Producer() {
    while (1) {
        while (empty <= 0) {

        }
        std::unique_lock<std::mutex> lock(pmtx);
        Produce();//生产操作
        lock.unlock();
        randomStop();
    }
}

//消费者函数
void Consumer() {
    while (1) {
        while (full <= 0) {

        }
        std::unique_lock<std::mutex> lock(cmtx);
        
        Consume();//消费操作
        lock.unlock();//解锁，开放给其他消费者线程

        randomStop();//休眠随机时间（100-2000）
    }
}
~~~

本项目使用三个互斥锁来实现各线程之间的同步，其中，pmtx和cmtx用来确保同一时间消费者和生产者各自最多只有一个线程可以尝试访问缓冲区，而第三个互斥锁buffermtx则保证了同一时间只有一个线程对缓冲区进行操作。

其中Produce()和Consume()函数内部对信号量empty和full进行修改，借助c++11的random库确定每次修改的数据量，到达随机生产消费数目的效果。

## 实现

### 界面

cmd界面的设计和多种色彩输出的实现依赖于cmd_console_tools.h和cmd_console_tools.cpp中给出的gotoxy()等工具函数，这两个文件来自于大一高级语言程序设计沈坚老师，仅使用，未作修改。

gotoxy(int,int)函数可以将光标移动到(x,y)位置，实现固定格式的输出。

缓冲区信息的输出由OutputBuffer()函数实现

### 算法

在生产者、消费者获得对应的权限后，进入对应的生产、消费函数，尝试获取buffer 的操作权限，需要注意的是，本算法使用的unique_lock<mutex>方式会在互斥的情况下阻塞该线程，导致缓冲区可用时信号量的值可能不再可用，故在生产、消费函数内部操作前重新确定信号量值的合法性，仅在信号量合法时继续执行。

以生产函数为例

~~~
//生产操作
void Produce() {
    std::unique_lock<std::mutex> lock(buffermtx);
    if (empty <= 0)
        return;
    PressToContinue();
    const int x = RandNumber(empty);//确定本次操作缓冲区的大小

    int start = in;//记录操作起始位置
   
    empty -= x;//信号量根据生产的数量减少
    cct_cls();//刷新界面
    cct_gotoxy(0, 1);
    setColor(WHITE);
    cout << "Main Thread id : " << Main;
    cct_gotoxy(0, 2);
    cout << "Operating Thread : ";
    setColor(GREEN);
    //cout << "Producer" << std::this_thread::get_id() << "\n";
    cout << std::this_thread::get_id() << "\n";

    products.clear();
    for (int i = 0; i < x; i++) {
        buffer[in] = RandNumber();
        products.push_back(buffer[in]);
        in = (in + 1) % CAPACITY;
    }
    full += x;
    
    cout << "[Producer][" << std::this_thread::get_id() << "]";
    setColor(WHITE);

    cout << "从" << start << "处开始生产了" << x << "个商品。" << endl;
    OutputBuffer(start, 1);

}
~~~

这里可以发现，当按下enter继续运行时，会输出本次运行的结果；

生产函数会在信号量值合法的情况下利用RandNumber(empty)函数确定本线程本次生产的数目，RandNumber(empty)函数会返回1-empty的任意值，在生产之初要记录当前生产者in指针所在位置，便于显示操作的信息，随后按自上至下，自左向右的顺序打印界面信息，用全局数组products记录下本次操作生成的数据，在OutputBuffer函数输出缓冲区信息。

多线程的开辟使用了windows的thread库，并以阻塞主线程的方式开始运行。

由于生产者、消费者函数都是死循环，可以单步持续观察各线程对缓冲区的操作。

## 运行截图

### 进入应用

![image-20220515180916467](https://gitee.com/thecloudwalker/img/raw/master/markdown/image-20220515180916467.png)

### 单步执行

![image-20220515180949438](https://gitee.com/thecloudwalker/img/raw/master/markdown/image-20220515180949438.png)

![image-20220515181008830](https://gitee.com/thecloudwalker/img/raw/master/markdown/image-20220515181008830.png)

![image-20220515181021341](https://gitee.com/thecloudwalker/img/raw/master/markdown/image-20220515181021341.png)
