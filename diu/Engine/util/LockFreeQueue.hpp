#ifndef LOCAK_FREE_QUEUE_H_
#define LOCAK_FREE_QUEUE_H_
#include <vector>
#include <atomic>
#include <thread>
#include <assert.h>


//保证T应当是trival
//基于循环数组的有界无锁队列
template <typename T>
class LockFreeArrayQueue
{
public:
    //push or pop 避让策略
    enum class Strategy
    {
        ABANDON, //放弃
        FORCE,   //必须得到
        YIELD,   //必须得到，但会调用yield
    };

public:
    //保证初始化在单线程下完成
    LockFreeArrayQueue(int capacity) : _data(capacity), _capacity(capacity)
    {
        islockfree = capacity == 0 || _data.front().is_lock_free();
    }
    ~LockFreeArrayQueue() {}

    bool is_lock_free()
    {
        return islockfree;
    }

    bool isFull() { return _size.load() == _capacity; }
    bool isEmpty() { return _size.load() == 0; }

    bool push(T val, Strategy strategy = Strategy::FORCE);
    bool pop(T &val, Strategy strategy = Strategy::FORCE);

private:
    //const T Empty = 0;      //元素为0时，表示为空
    const int Exclude = -1; //front or rear为-1时，表示其他线程已加锁，正在操作数据

    const int _capacity; //数组最大容量
    std::vector<std::atomic<T>> _data;
    std::atomic<int> _size = {0};  //当前size
    std::atomic<int> _front = {0}; //头指针
    std::atomic<int> _rear = {0};  //尾指针
    bool islockfree;
};

template <typename T>
bool LockFreeArrayQueue<T>::push(T val, Strategy strategy)
{
    int rear = _rear.load();
    while (true)
    {
        if (rear == Exclude || isFull())
        {
            switch (strategy)
            {
            case Strategy::YIELD:
                std::this_thread::yield();
            case Strategy::FORCE:
                rear = _rear.load();
                continue;
            }
            return false;
        }
        //加rear锁
        if (_rear.compare_exchange_weak(rear, Exclude))
        {
            //已满，失败解锁回退
            if (isFull())
            {
                int excepted = Exclude;
                bool flag = _rear.compare_exchange_weak(excepted, rear);
                assert(flag);
                continue;
            }
            break;
        }
    }
    _data[rear].store(val);
    ++_size; //必须在解锁前面
    int excepted = Exclude;
    //释放rear锁
    bool flag = _rear.compare_exchange_weak(excepted, (rear + 1) % _capacity);
    assert(flag);
    return true;
}

template <typename T>
bool LockFreeArrayQueue<T>::pop(T &val, Strategy strategy)
{
    int front = _front.load();
    while (true)
    {
        if (front == Exclude || isEmpty())
        {
            switch (strategy)
            {
            case Strategy::YIELD:
                std::this_thread::yield();
            case Strategy::FORCE:
                front = _front.load();
                continue;
            }
            return false;
        }
        //加锁
        if (_front.compare_exchange_weak(front, Exclude))
        {
            //空，失败解锁回退
            if (isEmpty())
            {
                int excepted = Exclude;
                bool flag = _front.compare_exchange_weak(excepted, front);
                assert(flag);
                continue;
            }
            break;
        }
    }
    val = _data[front].load();
    //_data[front].store(Empty);
    --_size; //必须在解锁前面
    int excepted = Exclude;
    bool flag = _front.compare_exchange_weak(excepted, (front + 1) % _capacity);
    assert(flag);
    return true;
}
#endif