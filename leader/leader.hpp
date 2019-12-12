#ifndef LEADER_NODE_HPP_
#define LEADER_NODE_HPP_

class LeaderFactory;

#include "leader_connections.hpp"
#include "follower.hpp"
#include "leader_storage.hpp"
#include "server.hpp"
#include "leader_factory.hpp"
#include "ileader.hpp"

class Leader : virtual public ILeader, public Follower {
public:
    Leader(Message::node node, int nThreads);
    ~Leader();

    bool setParam(std::string name, int value);

    virtual void initialize(LeaderFactory* factory = NULL);

    virtual void start(vector<Message::node> mNodes);
    virtual void stop();

    ILeaderStorage* getStorage();
    Message::node getMyNode();

    virtual bool initSelection(int id);
    virtual bool calcSelection(Message::node from, int id, bool &res);
    virtual bool updateSelection(Message::leader_update update);
    virtual void stopSelection();

    virtual bool changeRoles(Message::leader_update update);
    virtual void changeRole(std::vector<Message::node> leaders);
protected:
    void timerFun();

    std::thread timerFunThread;

    typedef enum Status{FREE, READY, STARTED, RECEIVED, CHANGING};

    Message::leader_update selection(int id);
    void startSelection();
    
    std::vector<Message::leader_update> updates;
    std::thread selectionThread;
    std::mutex selectionMutex;
    Status status;
    int id;
    ReadProc *clusterProc;
    std::mutex clusterMutex;


    

    int timeheartbeat;
    int timePropagation;

    LeaderConnections *connections;
    ILeaderStorage* storage;

    LeaderFactory tFactory;
    LeaderFactory *factory;
};

#endif