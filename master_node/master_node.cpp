#include "master_node.hpp"

#include "node.hpp"
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>

using namespace std;

MasterNode::MasterNode(int nThreads) : Node("::1", nThreads) {
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
    exit(0);
}

void MasterNode::start(std::string ip) {

    if(ip != string("::1")) {
        if(!this->connections->sendMHello(ip)) {
            perror("cannot connect to the network");
            this->stop();
            exit(1);
        }
    }
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
    while(this->running) {
        //routine for Nodes
        
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

        //routine for MasterNodes
        ips = this->getStorage()->getMNodes();
        int i=0;
        int sent=0;
        while(i < ips.size() && sent < 1) {
            if(ips[i] == this->myIp) {
                i++;
                continue;
            }

            vector<Report::report_result> report = this->getStorage()->getReport();
            if(this->connections->sendMReport(ips[i], report)) {
                sent++;
            }
            i++;
        }

        //test old nodes that do not answer since long time
        ips = this->getStorage()->getLRHardware(100, 120);
        vector<string> rem;
        for(auto&& ip : ips) {
            bool res = this->connections->sendRequestReport(ip);
            if(!res) {
                rem.push_back(ip);
            }
        }
        //remove the nodes that failed to respond
        this->connections->sendRemoveNodes(ips);
        vector<string> tmp;
        this->getStorage()->updateNodes(tmp,rem);

        sleep(this->timeTimerFun);
    }
}