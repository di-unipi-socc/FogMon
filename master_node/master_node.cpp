#include "master_node.hpp"

#include <cstdio>
#include <time.h>
#include <unistd.h>

using namespace std;

MasterNode::MasterNode(int nThreads) : server(this, 5556), storage(), connections(this, nThreads) {
    timer = 5;
    running = false;
}

MasterNode::~MasterNode() {
    this->stop();
}

void MasterNode::start() {
    this->running = true;
    this->timerThread = thread(&MasterNode::timerFun, this);
    this->server.start();
}

void MasterNode::stop() {
    this->running = false;
    this->server.stop();
    if(this->timerThread.joinable())
        this->timerThread.join();
}


Storage* MasterNode::getStorage() {
    return &(this->storage);
}

IConnections* MasterNode::getConnections() {
    return (IConnections*)(&(this->connections));
}

void MasterNode::timerFun() {
    while(this->running) {
        //check database for reports

        sleep(this->timer);
    }
}