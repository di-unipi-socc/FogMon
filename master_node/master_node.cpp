#include "master_node.hpp"

#include "node.hpp"
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>

using namespace std;

MasterNode::MasterNode(int nThreads) : Node("localhost", "5555", nThreads) {
    timeTimerFun = 20;
}

void MasterNode::initialize(MasterFactory* fact) {
    if(fact == NULL) {
        this->factory = &this->tFactory;
    }else {
        this->factory = fact;
    }
    this->storage = this->factory->newStorage("master_node.db");
    Node::storage = this->storage;
    this->connections = this->factory->newConnections(this->nThreads);
    this->connections->initialize(this);
    Node::connections = this->connections;
    Node::initialize(this->factory);
}

MasterNode::~MasterNode() {
    this->stop();
}

void MasterNode::start() {
    Node::start();
    this->timerFunThread = thread(&MasterNode::timerFun, this);
}

void MasterNode::stop() {
    Node::stop();
    if(this->timerFunThread.joinable())
        this->timerFunThread.join();
}

IMasterStorage* MasterNode::getStorage() {
    return this->storage;
}

void MasterNode::timerFun() {
    try{
    while(this->running) {
        //check database for reports
        vector<string> ips = this->getStorage()->getLRHardware(10, 30);
        int n = ips.size();
        n++;
        for(auto&& ip : ips) {
            this->connections->sendRequestReport(ip);
        }

        ips = this->getStorage()->getLRLatency(10, 30);
        for(auto&& ip : ips) {
            this->connections->sendRequestReport(ip);
        }
        
        int batch = 5;
        ips = this->getStorage()->getLRBandwidth(batch*2, 300);
        vector<string> ips_save;
        for(int i=0,j=0; i<ips.size() && i < batch + j; i++) {
            if(this->connections->sendSetToken(ips[i], this->timeTimerFun)) {
                ips_save.push_back(ips[i]);
            }else
                j++;
        }

        sleep(this->timeTimerFun);

        for(auto&& ip : ips_save) {
            this->connections->sendRequestReport(ip);
        }

    }
    }catch(...) {
        int a=0;
    }
}