#include "master_node.hpp"

#include <cstdio>

using namespace std;

MasterNode::MasterNode(int nThreads) : server(this), storage(), connections(this, nThreads) {

}

MasterNode::~MasterNode() {

}

void MasterNode::start() {
    this->server.start();
}

void MasterNode::stop() {
    this->server.stop();
}


Storage* MasterNode::getStorage() {
    return &(this->storage);
}

iConnections* MasterNode::getConnections() {
    return (iConnections*)(&(this->connections));
}