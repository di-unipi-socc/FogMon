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

    std::vector<Message::node> requestMNodes(Message::node ipS); 
    std::vector<Message::node> requestNodes(Message::node ipS);
    bool sendHello(Message::node ipS);
    bool sendUpdate(Message::node ipS);

    //return the port for the test
    //ip = ip
    //port = port of this server TODO change it
    int sendStartIperfTest(Message::node ip);
    int sendStartEstimateTest(Message::node ip, std::string &myIp);
};

#endif