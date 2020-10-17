#ifndef SLEEPER_HPP
#define SLEEPER_HPP

#include <mutex>
#include <atomic>
#include <chrono>

class Sleeper {
    std::timed_mutex m_;
    std::atomic_bool l_;

    void lock(){
        m_.lock();
        l_ = true;
    }

    void unlock() {
        l_ = false;
        m_.unlock();
    }
public:
    Sleeper() {
        lock();
    }

    ~Sleeper() {
        if(l_) {
            unlock();
        }
    }

    template<class R,class P>
    void sleepFor(const std::chrono::duration<R,P>& time) {
        if(m_.try_lock_for(time)) {
            m_.unlock();
        }
    }

    void start() {
        if(!l_) {
            lock();
        }
    }

    void stop() {
        if(l_) {
            unlock();
        }
    }
};

#endif