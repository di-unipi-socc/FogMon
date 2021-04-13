#ifndef LEADER_CONNECTIONS_HPP_
#define LEADER_CONNECTIONS_HPP_

#include "follower_connections.hpp"
#include "message.hpp"
#include "leader_storage.hpp"
#include "ileader.hpp"
#include "ileader_connections.hpp"

#include <thread>

class LeaderConnections : public FollowerConnections, virtual public ILeaderConnections {
protected:
    void handler(int fd, Message &m);

    ILeader* parent;
    
    bool notifyAllM(Message &m);

public:
    LeaderConnections(int nThread);
    ~LeaderConnections();

    void initialize(ILeader* parent);
    
    bool sendMHello(Message::node ip);

    bool sendRemoveNodes(std::vector<Message::node> ips);
    bool sendRequestReport(Message::node ip);
    bool sendMReport(Message::node ip, vector<Report::report_result> report);

    bool sendInitiateSelection(int id);
    bool sendStartSelection(int id);
    bool sendSelection(Message::leader_update update,Message::node node);
    bool sendEndSelection(Message::leader_update update, bool result);

    bool sendChangeRoles(Message::leader_update update);
};

#endif