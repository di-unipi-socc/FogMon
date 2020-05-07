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
    bool valid = false;
    if(mNodes.size() == 0) {
        valid = true;
    }
    for(auto l : mNodes) {
        if(l.id == this->getMyNode().id) {
            if(mNodes.size() == 1) {
                valid = true;
            }
            continue;
        }

        bool res = false;
        for(int i=0; i<5; i++) {
            res = this->connections->sendMHello(l);
            if(res)
                i=5;
            sleeper.sleepFor(chrono::seconds(3));
        }
        if(!res) {
            fprintf(stderr,"cannot connect to the network1\n");
            continue;
        }
        valid = true;
    }

    if(!valid) {
        this->stop();
        exit(1);
    }

    //communicate with the other
    vector<Message::node> ips = this->storage->getMNodes();
    for(auto ip : ips) {
        if(ip.id == this->getMyNode().id)
            continue;

        bool res = false;
        for(int i=0; i<5; i++) {
            res = this->connections->sendMHello(ip);
            if(res)
                i=5;
            sleeper.sleepFor(chrono::seconds(3));
        }
        if(!res) {
            fprintf(stderr,"cannot connect to the network2\n");
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
        vector<Message::node> ips = this->getStorage()->getMLRHardware(100, this->node->timeheartbeat*3);
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

            this->selector.checkSelection();
        }

        if(iter % 10 == 0) {
            this->getStorage()->removeOldNodes(this->node->timeheartbeat*3);
        }
        
        iter++;
        sleeper.sleepFor(chrono::seconds(this->node->timePropagation));
    }
}

bool Leader::initSelection(int id) {
    return this->selector.initSelection(id);
}

bool Leader::calcSelection(Message::node from, int id, bool &res) {
    return this->selector.calcSelection(from,id,res);
}

bool Leader::updateSelection(Message::leader_update update) {
    return this->selector.updateSelection(update);
}

void Leader::stopSelection() {
    this->selector.stopSelection();
}

bool Leader::setParam(std::string name, int value) {
    if(value <= 0)
        return false;
    
    if(Follower::setParam(name,value))
        return true;
    
    if(name == string("start-selection")) {
        this->selector.checkSelection(true);
    }
    
    printf("Param %s: %d\n",name.c_str(),value);
    return false;
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
        this->node->setMNodes(leaders);
        this->node->demote();
    }else {
        this->selector.stopSelection();
        
        sleeper.sleepFor(chrono::seconds(10));

        for(auto node : leaders) {
            if(!this->connections->sendMHello(node)) {
                continue;
            }
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
}

void Leader::changeRoles(Message::leader_update update) {
    if(update.selected.size() == 0)
        return;
    this->connections->sendChangeRoles(update);
}
