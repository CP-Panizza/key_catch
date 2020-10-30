#ifndef SPIN_MUTEX_H
#define SPIN_MUTEX_H

#include <atomic>
#include <mutex>
#include <chrono>
#include <condition_variable>

class spin_mutex
{
public:
    spin_mutex()
    {
        flg_.clear(std::memory_order_release);
    }
    explicit spin_mutex(int _try_cnt)
    : try_cnt_(_try_cnt)
    {
        flg_.clear(std::memory_order_release);
    }
    ~spin_mutex() = default;
public:
    void lock()
    {
        for (int millis = 5;; millis = (millis < 100 ? millis << 2 : millis))
        {
            int _try_cnt = try_cnt_;
            for (; _try_cnt > 0 && flg_.test_and_set(std::memory_order_acquire); --_try_cnt);
            if (_try_cnt > 0)
                return;
            std::unique_lock<std::mutex> ulk(mut_);
            cond_.wait_for(ulk, std::chrono::milliseconds(millis));
        }
    }
    bool try_lock()
    {
        int _try_cnt = try_cnt_;
        for (; _try_cnt > 0 && flg_.test_and_set(std::memory_order_acquire); --_try_cnt);
        if (_try_cnt > 0)
            return true;
        return false;
    }
    void unlock()
    {
        flg_.clear(std::memory_order_release);
        cond_.notify_all();
    }
private:
    spin_mutex(const spin_mutex&) = delete;
    spin_mutex(spin_mutex&&) = delete;
    spin_mutex& operator=(const spin_mutex&) = delete;
    spin_mutex& operator=(spin_mutex&&) = delete;
    private:
    std::mutex mut_;
    std::atomic_flag flg_;
    std::condition_variable cond_;
    int try_cnt_{ 200 };
};



//class spin_mutex{
//    std::atomic_flag flag = ATOMIC_FLAG_INIT;
//public:
//    spin_mutex() = default;
//    spin_mutex(const spin_mutex&) = delete;
//    spin_mutex& operator= (const spin_mutex &) = delete;

//    void lock(){
//        while(flag.test_and_set(std::memory_order_acquire));
//    }

//    void unlock(){
//        flag.clear(std::memory_order_release);
//    }
//};

#endif // SPIN_MUTEX_H
