#ifndef MASTER_CONNECTIONS_HPP_
#define MASTER_CONNECTIONS_HPP_

#include "inode.hpp"
#include "iconnections.hpp"
#include "queue.hpp"
#include "message.hpp"

#include <thread>

class MasterConnections : IConnections {
private:
    void handler(int fd, Message &m);
public:
    MasterConnections(INode *parent, int nThread);
    ~MasterConnections();
};

#endif