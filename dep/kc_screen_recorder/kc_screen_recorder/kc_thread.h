//
// Created by cmj on 2020/10/30.
//

#ifndef KC_THREAD_KC_THREAD_H
#define KC_THREAD_KC_THREAD_H

#include <thread>
#include <functional>
#include <windows.h>

class kc_thread_bass{
public:
    kc_thread_bass() {
        task = std::function<void()>(std::bind(&kc_thread_bass::run, this));
    }

    virtual void run(){

    }

    void start(int nPriority){
        priority = nPriority;
        std::thread t(this->task);
        ::SetThreadPriority((HANDLE)t.native_handle(), priority);
        t.detach();
    }


    bool set_priority(int nPriority){
        priority = nPriority;
    }

protected:
    std::function<void()> task;
    int priority = THREAD_PRIORITY_NORMAL;
};





#endif //KC_THREAD_KC_THREAD_H
