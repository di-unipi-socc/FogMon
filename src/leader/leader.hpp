#ifndef LEADER_NODE_HPP_
#define LEADER_NODE_HPP_

class LeaderFactory;

#include "leader_connections.hpp"
#include "follower.hpp"
#include "leader_storage.hpp"
#include "server.hpp"
#include "leader_factory.hpp"
#include "ileader.hpp"
#include "selector.hpp"

class Leader : virtual public ILeader, public Follower {
public:
    Leader(Message::node node, int nThreads);
    ~Leader();

    bool setParam(std::string name, int value) override;

    virtual void initialize(LeaderFactory* factory = NULL); //not override

    virtual void start(std::vector<Message::node> mNodes) override;
    virtual void stop() override;

    ILeaderConnections* getConnections() override;
    ILeaderStorage* getStorage() override;
    Message::node getMyNode() override;

    virtual bool initSelection(int id) override;
    virtual bool calcSelection(Message::node from, int id, bool &res) override;
    virtual bool updateSelection(Message::leader_update update) override;
    virtual void stopSelection() override;

    virtual void changeRoles(Message::leader_update update) override;
    virtual void changeRole(std::vector<Message::node> leaders) override;
protected:
    void timerFun();

    //for the leader selection algorithms
    Selector selector;

    int iter;
    std::thread timerFunThread;
    int lastQuality; 

    LeaderConnections *connections;
    ILeaderStorage* storage;

    LeaderFactory tFactory;
    LeaderFactory *factory;
};

#endif