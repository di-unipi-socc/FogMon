#ifndef MASTER_NODE_HPP_
#define MASTER_NODE_HPP_

class MasterFactory;

#include "master_connections.hpp"
#include "node.hpp"
#include "master_storage.hpp"
#include "server.hpp"
#include "master_factory.hpp"
#include "imaster_node.hpp"

class MasterNode : virtual public IMasterNode, public Node {
public:
    MasterNode(int nThreads);
    ~MasterNode();

    bool setParam(std::string name, int value);

    virtual void initialize(MasterFactory* factory = NULL);

    void start(std::string ip="::1");
    void stop();

    IMasterStorage* getStorage();


protected:

    void timerFun();

    std::thread timerFunThread;

    int timeheartbeat;
    int timePropagation;

    MasterConnections *connections;
    IMasterStorage* storage;

    MasterFactory tFactory;
    MasterFactory *factory;
};

#endif