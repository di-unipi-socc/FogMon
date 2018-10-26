#ifndef MASTER_CONNECTIONS_HPP_
#define MASTER_CONNECTIONS_HPP_

#include "inode.hpp"
#include "iconnections.hpp"
#include "queue.hpp"
#include "message.hpp"
#include "master_storage.hpp"

#include <thread>

class MasterConnections : IConnections {
private:
    void handler(int fd, Message &m);

    MasterStorage storage;
    
public:
    MasterConnections(INode *parent, int nThread);
    ~MasterConnections();

    MasterStorage* getStorage();

    bool sendRequestReport(std::string ip);
    bool sendSetToken(std::string ip, int time);
};

#endif