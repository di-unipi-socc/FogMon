#ifndef SERVER_HPP_
#define SERVER_HPP_
#include <string>
#include <thread>

#include "iconnections.hpp"
#include "storage.hpp"
#include "inode.hpp"

#include <thread>

class Server {
private:
    bool running;

    uint16_t portC;

    std::thread listenerThread;

    INode* node;

    //fd to wake up the listener thread during poll
    int efd;
public:
    Server(INode *node, int port);
    ~Server();

    void start();
    void stop();

    void listener();
};

#endif