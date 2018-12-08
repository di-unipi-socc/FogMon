#ifndef MASTER_NODE_HPP_
#define MASTER_NODE_HPP_

class MasterFactory;

#include "master_connections.hpp"
#include "node.hpp"
#include "master_storage.hpp"
#include "server.hpp"
#include "master_factory.hpp"
#include "iparent_master.hpp"

class MasterNode : virtual public IParentMaster, public Node {
public:
    MasterNode(int nThreads);
    ~MasterNode();

    virtual void initialize(MasterFactory* factory = NULL);

    void start();
    void stop();

    IMasterStorage* getStorage();


protected:

    void timerFun();

    std::thread timerFunThread;

    unsigned int timeTimerFun;

    MasterConnections *connections;
    IMasterStorage* storage;

    MasterFactory tFactory;
    MasterFactory *factory;
};

#endif