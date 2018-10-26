#ifndef INODE_HPP_
#define INODE_HPP_

#include "istorage.hpp"
#include "server.hpp"
#include "iconnections.hpp"

class INode {
public:
    //start listener for incoming ping and directions
    virtual void start() = 0;
    //stop everything
    virtual void stop() = 0;

    virtual IConnections* getConnections() = 0;
    virtual void setMyIp(std::string ip) = 0;
    virtual std::string getMyIp() = 0;
    virtual Server* getServer() = 0;
};

#endif