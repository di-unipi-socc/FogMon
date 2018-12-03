#ifndef CONNECTIONS_HPP_
#define CONNECTIONS_HPP_

class Node;

#include "iconnections.hpp"
#include "queue.hpp"
#include "message.hpp"
#include "storage.hpp"

#include <thread>

class Connections : public IConnections {
protected:
    void handler(int fd, Message &m);

    Storage storage;
    Node* parent;

public:
    Connections(Node *parent, int nThread);
    ~Connections();

    //ip:port
    bool sendHello(std::string ipS, std::string portS);
    bool sendUpdate(std::string ipS, std::string portS);

    //return the port for the test
    int sendStartBandwidthTest(std::string ip);

    Storage* getStorage();
};

#endif