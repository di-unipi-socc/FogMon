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

    std::vector<std::string> requestMNodes(std::string ip); 
    std::vector<std::string> requestNodes(std::string ipS);
    bool sendHello(std::string ipS);
    bool sendUpdate(std::string ipS);

    //return the port for the test
    //ip = ip
    //port = port of this server TODO change it
    int sendStartIperfTest(std::string ip);
    int sendStartEstimateTest(std::string ip);
};

#endif