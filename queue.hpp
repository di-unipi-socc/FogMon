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
    typedef struct queue_str {
        E info;
        struct queue_str *next;
    }queue_elem_t;

    queue_elem_t *queue_h;
    queue_elem_t *queue_l;

    std::mutex mutex;
    std::condition_variable threadWait;

    std::atomic<bool> stop;
public:
    Queue() {
        stop = false;
        queue_h = NULL;
        queue_l = NULL;
    }

    int push(E info) {
        std::unique_lock<std::mutex> lock(this->mutex);

        queue_elem_t *new_el = new queue_elem_t;
        new_el->info = info;
        new_el->next = NULL;

        if( this->queue_l == NULL )
        {
            this->queue_l = new_el;
            this->queue_h = new_el;
        }else
        {
            this->queue_l->next = new_el;
            this->queue_l = new_el;
        }

	    lock.unlock();
        this->threadWait.notify_one();	
        return 0;
    }

    int pop(E* ret) {
        int err=0;
        std::unique_lock<std::mutex> lock1(this->mutex);

        while( this->queue_h == NULL )
        {
            if(this->stop.load())
            {
                err = -1;
                break;
            }
            this->threadWait.wait(lock1);
        }
        if(err>=0)
        {
            if(this->queue_h!=NULL)
            {
                *ret = this->queue_h->info;
                this->queue_h = this->queue_h->next;
            }
            if(this->queue_h==NULL)
                this->queue_l = NULL;
        }

        lock1.unlock();

        return err;
    }

    void startqueue() {
        this->stop = false;
    }

    void stopqueue() {
        std::unique_lock<std::mutex> lock(this->mutex);

        while( this->queue_h )
        {
            this->queue_l = this->queue_h->next;
            delete this->queue_h;
            this->queue_h = this->queue_l;
        }
        this->stop = true;
        
        this->threadWait.notify_all();

        lock.unlock();
    }
};



#endif