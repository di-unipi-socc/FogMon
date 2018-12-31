#ifndef CONNECTIONS_HPP_
#define CONNECTIONS_HPP_

#include "inode.hpp"
#include "iconnections.hpp"
#include "queue.hpp"
#include "message.hpp"
#include "storage.hpp"

#include <thread>

class Connections : public IConnections {
protected:
    void handler(int fd, Message &m);

    INode* parent;

public:
    Connections(int nThread);
    ~Connections();

    void initialize(INode *parent);

    //ip:port
    bool sendHello(std::string ipS, std::string portS);
    bool sendUpdate(std::string ipS, std::string portS);

    //return the port for the test
    int sendStartIperfTest(std::string ip);
    int sendStartEstimateTest(std::string ip);
};

#endif