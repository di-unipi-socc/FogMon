#ifndef MASTER_CONNECTIONS_HPP_
#define MASTER_CONNECTIONS_HPP_

class IParentMaster;

#include "connections.hpp"
#include "queue.hpp"
#include "message.hpp"
#include "master_storage.hpp"
#include "imaster_node.hpp"

#include <thread>

class MasterConnections : public Connections {
protected:
    void handler(int fd, Message &m);

    IMasterNode* parent;
    
public:
    MasterConnections(int nThread);
    ~MasterConnections();

    void initialize(IMasterNode* parent);
    
    bool sendMHello(Message::node ip);

    bool sendRemoveNodes(std::vector<Message::node> ips);
    bool sendRequestReport(Message::node ip);
    bool sendMReport(Message::node ip, vector<Report::report_result> report);
};

#endif