#ifndef LEADER_FACTORY_HPP_
#define LEADER_FACTORY_HPP_

#include "leader_connections.hpp"
#include "factory.hpp"
#include "leader_storage.hpp"
#include "server.hpp"

class LeaderFactory : public Factory {
public:
    virtual ILeaderStorage* newStorage(std::string path, Message::node node) override;
    virtual LeaderConnections* newConnections(int nThread) override;
    virtual Server* newServer(IConnections* conn, int port) override;
};

#endif