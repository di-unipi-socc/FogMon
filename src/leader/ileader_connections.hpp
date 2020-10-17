#ifndef ILEADER_CONNECTIONS_HPP_
#define ILEADER_CONNECTIONS_HPP_

#include "connections.hpp"

class ILeader;

class ILeaderConnections : virtual public IConnections {
public:
    virtual void initialize(ILeader* parent) = 0;
    
    virtual bool sendMHello(Message::node ip) = 0;

    virtual bool sendRemoveNodes(std::vector<Message::node> ips) = 0;
    virtual bool sendRequestReport(Message::node ip) = 0;
    virtual bool sendMReport(Message::node ip, vector<Report::report_result> report) = 0;

    virtual bool sendInitiateSelection(int id) = 0;
    virtual bool sendStartSelection(int id) = 0;
    virtual bool sendSelection(Message::leader_update update,Message::node node) = 0;
    virtual bool sendEndSelection(Message::leader_update update, bool result) = 0;

    virtual bool sendChangeRoles(Message::leader_update update) = 0;
};

#endif