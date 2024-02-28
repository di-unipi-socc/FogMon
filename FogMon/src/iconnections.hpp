#ifndef ICONNECTIONS_HPP_
#define ICONNECTIONS_HPP_

class IAgent;

#include "iagent.hpp"
#include "message.hpp"
#include "istorage.hpp"


class IConnections {
public:
    virtual void initialize(IAgent *parent) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    //put the request on a queue to be processed by another thread
    virtual void request(int fd) = 0;
};

#endif