#ifndef IFACTORY_HPP_
#define IFACTORY_HPP_

#include "iconnections.hpp"
#include "istorage.hpp"
#include "server.hpp"

#include <string>

class IFactory {
public:
    virtual IStorage* newStorage(std::string path) = 0;
    virtual IConnections* newConnections(int nThread) = 0;
    virtual Server* newServer(IConnections* conn, int port) = 0;
};

#endif