#ifndef MASTER_FACTORY_HPP_
#define MASTER_FACTORY_HPP_

#include "master_connections.hpp"
#include "factory.hpp"
#include "master_storage.hpp"
#include "server.hpp"

class MasterFactory : public Factory {
public:
    virtual IMasterStorage* newStorage(std::string path, Message::node node);
    virtual MasterConnections* newConnections(int nThread);
    virtual Server* newServer(IConnections* conn, int port);
};

#endif