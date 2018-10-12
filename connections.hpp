#ifndef CONNECTIONS_HPP_
#define CONNECTIONS_HPP_

class Elf;

#include "queue.hpp"
#include "message.hpp"
#include <atomic>
#include <thread>

using namespace std;
class Connections {
private:
    Elf *parent;
    Queue<int> queue;

    atomic<bool> running;

    bool getMessage(int fd, Message * m);

    void handler(int fd);

    void worker();
    int num;
    thread *workers;
public:
    Connections(Elf *elf, int nThread);
    ~Connections();
    void start();
    void stop();

    //put the request on a queue to be processed by another thread
    void request(int fd);
};

#endif