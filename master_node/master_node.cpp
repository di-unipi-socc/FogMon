#include "master_node.hpp"

#include "node.hpp"
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>

using namespace std;

MasterNode::MasterNode(Message::node node, int nThreads) : Node(node, node.port, nThreads) {
    timePropagation = 20;
    timeheartbeat = 120;
}

void MasterNode::initialize(MasterFactory* fact) {
    if(fact == NULL) {
        this->factory = &this->tFactory;
    }else {
        this->factory = fact;
    }
    this->storage = this->factory->newStorage("master_node.db", this->nodeS);
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

void MasterNode::start(Message::node *node) {
    Node::start();
    if(node != NULL) {
        if(!this->connections->sendMHello(*node)) {
            fprintf(stderr,"cannot connect to the network\n");
            this->stop();
            exit(1);
        }
        //communicate with the other
        vector<Message::node> ips = this->storage->getMNodes();
        for(auto ip : ips) {
            if(ip.id == this->getMyNode().id)
                continue;
            if(!this->connections->sendMHello(ip)) {
                fprintf(stderr,"cannot connect to the network\n");
                this->stop();
                exit(1);
            }
        }
    }
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
        vector<Message::node> ips = this->getStorage()->getMLRHardware(100, this->timeheartbeat);
        vector<Message::node> rem;
        for(auto&& node : ips) {
            bool res = this->connections->sendRequestReport(node);
            if(!res) {
                printf("Removing node from this group: %s\n",node.id.c_str());
                rem.push_back(node);
            }
        }
        //remove the nodes that failed to respond
        this->connections->sendRemoveNodes(ips);
        vector<Message::node> tmp;
        this->getStorage()->updateNodes(tmp,rem);

        //routine for MasterNodes
        ips = this->getStorage()->getMNodes();
        int i=0;
        int sent=0;
        while(i < ips.size() && sent < 1) {
            if(ips[i].id == this->nodeS.id) {
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

Message::node MasterNode::getMyNode() {
    return this->nodeS;
}