#ifndef INODE_HPP_
#define INODE_HPP_

#include "server.hpp"
#include "istorage.hpp"
#include <string>

class INode {
public:
    //start listener for incoming ping and directions
    virtual void start() = 0;
    //stop everything
    virtual void stop() = 0;

    virtual IConnections* getConnections() = 0;
    virtual IStorage* getStorage() = 0;

    virtual void setMyIp(std::string ip) = 0;
    virtual std::string getMyIp() = 0;
    
    virtual int startIperf() = 0;
    virtual int startEstimate() = 0;
    
    virtual Server* getServer() = 0;

    void filterIp(std::vector<std::string> &list) {
        for(int i=0; i<list.size(); i++)
        {
            if(list[i]==std::string("::1")||list[i]==std::string("127.0.0.1"))
                list[i] = this->ipS;
        }
    }
protected:
    std::string ipS;
};

#endif