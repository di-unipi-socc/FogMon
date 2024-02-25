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
    void handler(int fd, Message &m) override;

    ILeader* parent;
    
    bool notifyAllM(Message &m);

public:
    LeaderConnections(int nThread);
    ~LeaderConnections();

    void initialize(ILeader* parent) override;
    
    bool sendMHello(Message::node ip) override;

    bool sendRemoveNodes(std::vector<Message::node> ips) override;
    bool sendRequestReport(Message::node ip) override;
    bool sendMReport(Message::node ip, vector<Report::report_result> report) override;

    bool sendInitiateSelection(int id) override;
    bool sendStartSelection(int id) override;
    bool sendSelection(Message::leader_update update,Message::node node) override;
    bool sendEndSelection(Message::leader_update update, bool result) override;

    bool sendChangeRoles(Message::leader_update update) override;
};

#endif