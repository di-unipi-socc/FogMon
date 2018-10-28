#include "master_node.hpp"

#include <cstdio>
#include <time.h>
#include <unistd.h>

using namespace std;

MasterNode::MasterNode(int nThreads) : server(this, 5556),  connections(this, nThreads) {
    timer = 20;
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

IConnections* MasterNode::getConnections() {
    return (IConnections*)(&(this->connections));
}

void MasterNode::timerFun() {
    while(this->running) {
        //check database for reports
        vector<string> ips = this->connections.getStorage()->getLRHardware(10, 30);
        int n = ips.size();
        n++;
        for(auto&& ip : ips) {
            this->connections.sendRequestReport(ip);
        }

        ips = this->connections.getStorage()->getLRLatency(10, 30);
        for(auto&& ip : ips) {
            this->connections.sendRequestReport(ip);
        }
        
        int batch = 5;
        ips = this->connections.getStorage()->getLRBandwidth(batch*2, 300);
        vector<string> ips_save;
        for(int i=0,j=0; i<ips.size() && i < batch + j; i++) {
            if(this->connections.sendSetToken(ips[i], this->timer)) {
                ips_save.push_back(ips[i]);
            }else
                j++;
        }

        sleep(this->timer);

        for(auto&& ip : ips_save) {
            this->connections.sendRequestReport(ip);
        }

    }
}

Server* MasterNode::getServer() {
    return &this->server;
}