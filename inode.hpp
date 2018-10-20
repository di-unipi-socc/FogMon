#ifndef INODE_HPP_
#define INODE_HPP_

class INode;

#include "istorage.hpp"
#include "iconnections.hpp"

class INode {
public:
    //start listener for incoming ping and directions
    virtual void start() = 0;
    //stop everything
    virtual void stop() = 0;

    virtual IConnections* getConnections() = 0;
};

#endif