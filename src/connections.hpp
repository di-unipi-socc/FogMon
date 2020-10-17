#ifndef CONNECTIONS_HPP_
#define CONNECTIONS_HPP_

class IAgent;

#include "iagent.hpp"
#include "queue.hpp"
#include "message.hpp"
#include "istorage.hpp"
#include <thread>
#include "iconnections.hpp"

#ifndef IN6_IS_ADDR_V4MAPPED
#define IN6_IS_ADDR_V4MAPPED(a) \
       ((((a)->s6_words[0]) == 0) && \
        (((a)->s6_words[1]) == 0) && \
        (((a)->s6_word[2]) == 0) && \
        (((a)->s6_word[3]) == 0) && \
        (((a)->s6_word[4]) == 0) && \
        (((a)->s6_word[5]) == 0xFFFF))
#endif

using namespace std;
class Connections  : virtual public IConnections {
protected:
    IAgent *parent;
    Queue<int> queue;

    atomic<bool> running;

    bool getMessage(int fd, Message &m);
    bool sendMessage(int fd, Message &m);
    bool notifyAll(Message &m);

    virtual void handler(int fd,Message &m) = 0;

    void worker();
    int num;
    thread *workers;

    int readS(long fd, void *data, int len);
    int writeS(long fd, const char *data, int len);

    int openConnection(std::string ipS, std::string port="5555");

    std::string getSource(int fd, Message &m);
public:
    Connections(int nThread);
    ~Connections();

    void initialize(IAgent *parent);

    void start();
    void stop();

    //put the request on a queue to be processed by another thread
    void request(int fd);
};

#endif