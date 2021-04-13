#ifndef IAGENT_HPP_
#define IAGENT_HPP_

#include "node.hpp"
#include "message.hpp"
#include "server.hpp"
#include "istorage.hpp"
#include "sleeper.hpp"


#include <string>

#include <condition_variable>
#include <mutex>
#include <chrono>
#include <atomic>
#include <vector>

class Node;
class IAgent {
public:

    IAgent();
    virtual ~IAgent();

    //start listener for incoming ping and directions
    virtual void start(std::vector<Message::node> mNodes);

    //stop everything
    virtual void stop();

    void setParent(Node * node);

    virtual bool setParam(std::string name, int value) = 0;

    virtual IConnections* getConnections() = 0;
    virtual IStorage* getStorage() = 0;

    virtual void setMyId(std::string id) = 0;    
    virtual Message::node getMyNode() = 0;

    virtual int getIperfPort() = 0;
    virtual int getEstimatePort() = 0;
    
    virtual Server* getServer() = 0;

    virtual void changeRole(std::vector<Message::node> leaders) = 0;
    Node *node;
protected:
    Message::node nodeS;

    virtual int startIperf() = 0;
    virtual int startEstimate() = 0;

    //mutex/condition for termination and wait_for
    Sleeper sleeper;

    std::atomic_bool running;
};

#endif