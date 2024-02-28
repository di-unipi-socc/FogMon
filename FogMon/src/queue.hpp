#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <atomic>


/**
 * A simple thread safe FIFO queue
*/
template<typename E>
class Queue {
private:
    struct queue_elem_t {
        E info;
        queue_elem_t *next;
    };

    queue_elem_t *queue_h;
    queue_elem_t *queue_l;

    std::mutex mutex;
    std::condition_variable threadWait;

    std::atomic<bool> stop;
public:
    Queue() {
        stop = false;
        queue_h = nullptr;
        queue_l = nullptr;
    }

    ~Queue() {
        for (queue_elem_t* p=this->queue_h;p!=nullptr;) {
            queue_elem_t* next = p->next;
            delete p;
            p = next;
        }
    }
    
    bool push(E info) {
        if (this->stop.load()) {
            return false;
        }

        std::unique_lock<std::mutex> lock(this->mutex);
        queue_elem_t *new_el = new queue_elem_t;
        new_el->info = info;
        new_el->next = nullptr;

        if( this->queue_l == nullptr )
        {
            this->queue_l = new_el;
            this->queue_h = new_el;
        }else
        {
            this->queue_l->next = new_el;
            this->queue_l = new_el;
        }

        this->threadWait.notify_one();
        return true;
    }

    bool pop(E* ret) {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->threadWait.wait(lock, [=]{
            return this->queue_h != nullptr || this->stop.load();
        });

       if(this->queue_h!=nullptr)
        { // there is a new element to pop
            *ret = this->queue_h->info;
            queue_elem_t* old_head = this->queue_h;
            this->queue_h = this->queue_h->next;
            delete old_head;

            if(this->queue_h==nullptr) {
                this->queue_l = nullptr;
            }
            return true;
        }
        else
        { // the queue was closed
            return false;
        }
    }

    void startqueue() {
        this->stop = false;
    }

    void stopqueue() {
        this->stop = true;
        this->threadWait.notify_all();
    }
};

#endif