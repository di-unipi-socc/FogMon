#ifndef FACTORY_HPP_
#define FACTORY_HPP_

#include "follower_connections.hpp"
#include "storage.hpp"
#include "server.hpp"
#include "ifactory.hpp"

class Factory : public IFactory {
public:
    virtual IStorage* newStorage(std::string path, Message::node node) override;
    virtual FollowerConnections* newConnections(int nThread) override;
    virtual Server* newServer(IConnections* conn, int port) override;
};

#endif