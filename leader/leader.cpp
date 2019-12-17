#include "leader.hpp"

#include "follower.hpp"
#include "selector.hpp"

#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;

Leader::Leader(Message::node node, int nThreads) : Follower(node, nThreads), selector(this) {
    timePropagation = 20;
    timeheartbeat = 120;
    this->nodeS = node;
}

void Leader::initialize(LeaderFactory* fact) {
    if(fact == NULL) {
        this->factory = &this->tFactory;
    }else {
        this->factory = fact;
    }
    this->storage = this->factory->newStorage("leader_node.db", this->nodeS);
    Follower::storage = this->storage;
    this->connections = this->factory->newConnections(this->nThreads);
    this->connections->initialize(this);
    Follower::connections = this->connections;
    Follower::initialize(this->factory);
}

Leader::~Leader() {
    this->stop();
}

void Leader::start(vector<Message::node> mNodes) {
    Follower::start(vector<Message::node>());
    if(!mNodes.empty()) {
        if(!this->connections->sendMHello(mNodes[0])) {
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
    this->timerFunThread = thread(&Leader::timerFun, this);
}

void Leader::stop() {
    Follower::stop();
    if(this->timerFunThread.joinable())
        this->timerFunThread.join();
}

ILeaderConnections* Leader::getConnections() {
    return this->connections;
}

ILeaderStorage* Leader::getStorage() {
    return this->storage;
}

void Leader::timerFun() {
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
        
        this->selector.checkSelection();

        //routine for LeaderNodes
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
        sleeper.sleepFor(chrono::seconds(this->timePropagation));
    }
}

bool Leader::initSelection(int id) {
    this->selector.initSelection(id);
}

bool Leader::calcSelection(Message::node from, int id, bool &res) {
    this->selector.calcSelection(from,id,res);
}

bool Leader::updateSelection(Message::leader_update update) {
    this->selector.updateSelection(update);
}

void Leader::stopSelection() {
    this->selector.stopSelection();
}

bool Leader::setParam(std::string name, int value) {
    if(value <= 0)
        return false;
    
    if(Follower::setParam(name,value))
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

Message::node Leader::getMyNode() {
    return this->nodeS;
}

void Leader::changeRole(vector<Message::node> leaders) {
    bool present = false;
    for(auto node : leaders) {
        if(node == this->getMyNode()) {
            present = true;
        }
    }
    if(!present) {
        this->node->demote();
    }else {
        this->selector.stopSelection();
    }
}

void Leader::changeRoles(Message::leader_update update) {
    this->connections->sendChangeRoles(update);
    this->changeRole(update.selected);
}