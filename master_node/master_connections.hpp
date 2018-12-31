#ifndef MASTER_CONNECTIONS_HPP_
#define MASTER_CONNECTIONS_HPP_

class IParentMaster;

#include "connections.hpp"
#include "queue.hpp"
#include "message.hpp"
#include "master_storage.hpp"
#include "iparent_master.hpp"

#include <thread>

class MasterConnections : public Connections {
protected:
    void handler(int fd, Message &m);

    IParentMaster* parent;
    
public:
    MasterConnections(int nThread);
    ~MasterConnections();

    void initialize(IParentMaster* parent);
    
    bool sendRequestReport(std::string ip);
    bool sendSetToken(std::string ip, int time);
    bool sendMReport(std::string ip, vector<Report::report_result> report);
};

#endif