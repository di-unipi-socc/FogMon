#ifndef ICONNECTIONS_HPP_
#define ICONNECTIONS_HPP_

class IConnections;

#include "inode.hpp"
#include "queue.hpp"
#include "message.hpp"

#include <thread>

using namespace std;
class IConnections {
protected:
    INode *parent;
    Queue<int> queue;

    atomic<bool> running;

    bool getMessage(int fd, Message &m);
    bool sendMessage(int fd, Message &m);

    virtual void handler(int fd,Message &m) = 0;

    void worker();
    int num;
    thread *workers;

    int readS(long fd, void *data, int len);
    int writeS(long fd, const char *data, int len);

    int openConnection(std::string ipS);
public:
    IConnections(INode *parent, int nThread);
    ~IConnections();
    void start();
    void stop();

    //put the request on a queue to be processed by another thread
    void request(int fd);
};

#endif