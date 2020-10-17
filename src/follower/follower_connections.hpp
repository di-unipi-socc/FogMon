#ifndef FOLLOWER_CONNECTIONS_HPP_
#define FOLLOWER_CONNECTIONS_HPP_

#include "iagent.hpp"
#include "iconnections.hpp"
#include "connections.hpp"
#include "message.hpp"
#include "storage.hpp"

#include <optional>
#include <thread>

class FollowerConnections : public Connections {
protected:
    void handler(int fd, Message &m);

    IAgent* parent;

public:
    FollowerConnections(int nThread);
    ~FollowerConnections();

    void initialize(IAgent *parent);

    std::vector<Message::node> requestMNodes(Message::node ipS); 
    std::vector<Message::node> requestNodes(Message::node ipS);
    bool sendHello(Message::node ipS);
    std::optional<std::pair<int64_t,Message::node>> sendUpdate(Message::node ipS, std::pair<int64_t,Message::node> update);

    //return the port for the test
    //ip = ip
    //port = port of this server TODO change it
    int sendStartIperfTest(Message::node ip);
    int sendStartEstimateTest(Message::node ip, std::string &myIp);
};

#endif