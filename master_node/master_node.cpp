#include "master_node.hpp"

#include "node.hpp"
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>

using namespace std;

MasterNode::MasterNode(std::string name, int nThreads) : Node("::1", nThreads) {
    this->name = name;
    timePropagation = 20;
    timeheartbeat = 120;
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
            fprintf(stderr,"cannot connect to the network\n");
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
    int iter = 1;
    while(this->running) {
        //routine for Nodes
        
        //check database for reports
        vector<string> ips = this->getStorage()->getLRHardware(100, this->timeheartbeat);
        vector<string> rem;
        for(auto&& ip : ips) {
            bool res = this->connections->sendRequestReport(ip);
            if(!res) {
                printf("Removing node from this group: %s\n",ip.c_str());
                rem.push_back(ip);
            }
        }
        //remove the nodes that failed to respond
        this->connections->sendRemoveNodes(ips);
        vector<string> tmp;
        this->getStorage()->updateNodes(tmp,rem);

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

        if(iter % 5 == 0) {
            this->getStorage()->complete();
        }
        
        iter++;
        sleep(this->timePropagation);
    }
}

bool MasterNode::setParam(std::string name, int value) {
    if(value <= 0)
        return false;
    
    if(Node::setParam(name,value))
        return true;
    
    if(name == string("heartbeat")) {
        this->timeheartbeat = value;
    }else if(name == string("time-propagation")) {
        this->timePropagation = value;
    }else{
        return false;
    }
    printf("Param %s: %d\n",name.c_str(),value);
    return true;
}
