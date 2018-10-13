#ifndef INODE_HPP_
#define INODE_HPP_

#include "storage.hpp"
#include "iconnections.hpp"

class iNode {
public:
    //start listener for incoming ping and directions
    virtual void start() = 0;
    //stop everything
    virtual void stop() = 0;

    virtual Storage* getStorage() = 0;
    virtual iConnections* getConnections() = 0;
};

#endif