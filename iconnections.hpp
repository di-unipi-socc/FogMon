#ifndef ICONNECTIONS_HPP_
#define ICONNECTIONS_HPP_

class iConnections {
public:
    //start listener for incoming ping and directions
    virtual void start() = 0;
    //stop everything
    virtual void stop() = 0;

    virtual void request(int fd) = 0;
};

#endif