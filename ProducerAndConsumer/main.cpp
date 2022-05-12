/*
 * @Author: 2053285 罗斌江 2200656244@qq.com
 * @Date: 2022-05-10 16:01:02
 * @LastEditors: 2053285 罗斌江 2200656244@qq.com
 * @LastEditTime: 2022-05-10 22:17:15
 * @FilePath: 
 * @Description:生产者消费者模型
 */
#include<iostream>
#include<vector>
#include<thread>
#include<mutex>
#include<random>
#include<windows.h>
#include<conio.h>
#include"cmd_console_tools.h"

#define CAPACITY 20
#define PNum 3
#define CNum 3

#define DEFAULT -1
#define RED 0
#define GREEN 1
#define YELLOW 2
#define WHITE 3
#define LIGHTGREEN 4

using std::cout;
using std::endl;
using std::vector;
using std::thread;
using std::string;

typedef int semaphore;//信号量

thread::id Main;

int* buffer;
int in = 0;
int out = 0;

vector<int>products;//存储生产
vector<int>goods;

//semaphore empty=0;
//semaphore full=CAPACITY;

semaphore empty = CAPACITY;
semaphore full = 0;


std::mutex buffermtx;
std::mutex pmtx;//每次只有一个生产者
std::mutex cmtx;//每次只要

void setColor(int color) {
    switch (color) {
        case RED: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED);//设置红色
            break;
        }
        case GREEN: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_GREEN);//设置绿色
            break;
        }
        case YELLOW: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);//设置红色和绿色相加
            break;
        }
        case WHITE: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);//设置三色相加
            break;
        }
        case LIGHTGREEN: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);//设置绿色和蓝色相加
            break;
        }
        default: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY);//设置颜色，没有添加颜色，故为原色
        }
    }
}

void PressToContinue() {
    setColor(WHITE);
    cout << "\n按下Enter键以继续\n";
    cout << "按空格键退出\n";
    if (_getch() == ' ') {
        exit(0);
    }
}

void OutputBuffer(int start, bool id) {
    //index
    cct_gotoxy(0, 5);
    setColor(YELLOW);
    cout << "  Index  : ";// len 11
    for (int i = 0; i < CAPACITY; i++) {
        if (i < 10)
            cout << " ";
        cout << i << " ";
    }
    cout << '\n';//cct_gotoxy(0,6)
    //buffer 
    cout << "Buffer[] : ";//len 11
    setColor(LIGHTGREEN);
    for (int i = 0; i < CAPACITY; i++) {
        if (buffer[i] < 10)
            cout << " ";
        cout << buffer[i] << " ";
    }
    cout << '\n';
    if (id) {
        //producer
        setColor(GREEN);
        if (start < in) {
            cct_gotoxy(11 + start * 3, 7);
            for (int i = 0; i < products.size(); i++) {
                if (products[i] < 10) {
                    cout << " " << products[i];
                }
                else {
                    cout << products[i];
                }
                cout << " ";
            }
            cct_gotoxy(12 + 3 * in, 7);
            cout << "\\";
        }
        else {
            int size = products.size();
            int tail = CAPACITY - start;
            int head = in;
            cct_gotoxy(11, 7);
            for (int i = 0; i < head; i++) {
                if (products[i + tail] < 10) {
                    cout << " " << products[i + tail];
                }
                else {
                    cout << products[i + tail];
                }
                cout << " ";
            }
            cct_gotoxy(12 + 3 * in, 7);
            cout << "\\";
            cct_gotoxy(11 + start * 3, 7);
            for (int i = 0; i < tail; i++) {
                if (products[i] < 10) {
                    cout << " " << products[i];
                }
                else {
                    cout << products[i];
                }
                cout << " ";
            }
        }
        setColor(RED);
        cct_gotoxy(12 + 3 * out, 8);
        cout << "/";
    }
    else {
        //Consumer
        setColor(GREEN);
        cct_gotoxy(12 + 3 * in, 7);
        cout << "\\";
        setColor(RED);
        if (start < out) {
            cct_gotoxy(11 + start * 3, 8);
            for (int i = 0; i < goods.size(); i++) {
                if (goods[i] < 10) {
                    cout << " " << goods[i];
                }
                else {
                    cout << goods[i];
                }
                cout << " ";
            }
            cct_gotoxy(12 + 3 * out, 8);
            cout << "/";
        }
        else {
            int size = goods.size();
            int tail = CAPACITY - start;
            int head = out;
            cct_gotoxy(11, 8);
            for (int i = 0; i < head; i++) {
                if (goods[i + tail] < 10) {
                    cout << " " << goods[i + tail];
                }
                else {
                    cout << goods[i + tail];
                }
                cout << " ";
            }
            cct_gotoxy(12 + 3 * out, 8);
            cout << "/";
            cct_gotoxy(11 + start * 3, 8);
            for (int i = 0; i < tail; i++) {
                if (goods[i] < 10) {
                    cout << " " << goods[i];
                }
                else {
                    cout << goods[i];
                }
                cout << " ";
            }
        }
    }

}


int RandNumber(int bound) {
    if (bound == 1) {
        return 1;
    }
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int>distr(1, bound);
    return distr(eng);
}

int RandNumber() {
    return RandNumber(20);
}

void stop() {
    //Sleep(RandNumber() * 100);
    Sleep(2000);
}

void randomStop() {
    Sleep(RandNumber() * 100);
}



void Produce() {
    std::unique_lock<std::mutex> lock(buffermtx);
    if (empty > 0) {
        PressToContinue();
        const int x = RandNumber(empty);
       
        int start = in;
        //std::uniform_int_distribution<unsigned int> x(1, empty);
        empty -= x;
        cct_cls();
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
        //cout << x << " " << empty << " " << full <<" " <<in<<" "<<out<< "\n";
        //cout << "Producer" << std::this_thread::get_id() << "从" << start << "处开始生产" << x << "个商品" << endl;
        cout << "[Producer][" << std::this_thread::get_id() << "]";
        setColor(WHITE);
       
        cout << "从" << start << "处开始生产了" << x << "个商品。" << endl;
        OutputBuffer(start, 1);
    }
}

void Producer() {
    while (1) {
        std::unique_lock<std::mutex> lock(pmtx);
        while (empty <= 0) {

        }
        Produce();
        lock.unlock();
        randomStop();
    }
}

void Consume() {
    std::unique_lock<std::mutex> lock(buffermtx);
    if (full > 0) {
        PressToContinue();
        const int x = RandNumber(full);
        
        int start = out;
        full = full - x;
        cct_cls();
        cct_gotoxy(0, 1);
        setColor(WHITE);
        cout << "Main Thread id : " << Main;
        cct_gotoxy(0, 2);
        cout << "Operating Thread : ";
        setColor(RED);
        cout << std::this_thread::get_id() << "\n";
        //cout << "Consumer" << std::this_thread::get_id() << "\n";
        goods.clear();
        for (int i = 0; i < x; i++) {
            goods.push_back(buffer[out]);
            buffer[out] = 0;
            out = (out + 1) % CAPACITY;
        }
        empty = empty + x;
        cout << "[Consumer][" << std::this_thread::get_id() << "]";
        setColor(WHITE);
       
        cout << "从" << start << "处开始消费了" << x << "个商品。" << endl;
        //cout << x << " " << empty << " " << full << " " << in << " " << out << "\n";
        OutputBuffer(start, 0);
    }
}

void Consumer() {
    while (1) {
        std::unique_lock<std::mutex> lock(cmtx);
        while (full <= 0) {

        }
        Consume();
        lock.unlock();

        randomStop();
    }
}

int main(int argc, char* argv[]) {
    Main = std::this_thread::get_id();
    cout << "main thread id:" << Main << endl;

    buffer = new int[CAPACITY];
    for (int i = 0; i < CAPACITY; i++) {
        buffer[i] = 0;//init
    }
    thread th[PNum + CNum];
    for (int i = 0; i < PNum; i++) {
        th[i] = thread(Producer);
    }
    for (int i = 0; i < CNum; i++) {
        th[i + PNum] = thread(Consumer);
    }
    for (auto& i : th) {
        i.join();
    }
    delete[]buffer;

    return 0;
}