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

    bool setParam(std::string name, int value);

    virtual void initialize(LeaderFactory* factory = NULL);

    virtual void start(vector<Message::node> mNodes);
    virtual void stop();

    ILeaderConnections* getConnections();
    ILeaderStorage* getStorage();
    Message::node getMyNode();

    virtual bool initSelection(int id);
    virtual bool calcSelection(Message::node from, int id, bool &res);
    virtual bool updateSelection(Message::leader_update update);
    virtual void stopSelection();

    virtual void changeRoles(Message::leader_update update);
    virtual void changeRole(std::vector<Message::node> leaders);
protected:
    void timerFun();

    //for the leader selection algorithms
    Selector selector;

    std::thread timerFunThread;    

    LeaderConnections *connections;
    ILeaderStorage* storage;

    LeaderFactory tFactory;
    LeaderFactory *factory;
};

#endif