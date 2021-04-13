#include "leader.hpp"

#include "follower.hpp"
#include "selector.hpp"

#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <math.h>

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "uiconnection.hpp"

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
            sleeper.sleepFor(chrono::seconds(10));
        }
        if(!res) {
            fprintf(stderr,"cannot connect to the network2 (%s)\n",l.ip.c_str());
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
            sleeper.sleepFor(chrono::seconds(10));
        }
        if(!res) {
            fprintf(stderr,"cannot connect to the network2 (%s)\n",ip.ip.c_str());
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
    this->iter = 1;
    this->lastQuality = -random()%10-20;
    while(this->running) {
        //routine for Nodes a
        auto t_start = std::chrono::high_resolution_clock::now();
        
        //check database for reports
        vector<Message::node> ips = this->getStorage()->getMLRHardware(100, this->node->timesilent);
        vector<Message::node> rem;
        for(auto&& node : ips) {
            bool res = this->connections->sendRequestReport(node);
            if(!res) {
                printf("Removing node from this group: %s\n",node.ip.c_str());
                rem.push_back(node);
            }
        }
        {
            auto t_end2 = std::chrono::high_resolution_clock::now();
            auto elapsed_time2 = std::chrono::duration_cast<std::chrono::duration<float>>(t_end2-t_start).count();
            cout << "timerFun1 " << elapsed_time2 << endl;
        }
        //remove the nodes that failed to respond
        this->connections->sendRemoveNodes(rem);
        vector<Message::node> tmp;
        this->getStorage()->updateNodes(tmp,rem);   
        {
            auto t_end2 = std::chrono::high_resolution_clock::now();
            auto elapsed_time2 = std::chrono::duration_cast<std::chrono::duration<float>>(t_end2-t_start).count();
            cout << "timerFun2 " << elapsed_time2 << endl;
        }
        //routine for LeaderNodes
        ips = this->getStorage()->getMNodes();

        int num = ips.size();
        bool force = true;
        if (num<1)
            num = 1;
        int time = (int)(this->node->timePropagation*( log2(num)*5+3 ));
        if (iter < 600/this->node->timePropagation) {
            force = false;
            time += this->node->timePropagation*10;
        }
        printf("Check old nodes %d\n",time);
        
        int num_leaders = 0;
        rem = this->getStorage()->removeOldLNodes(time, num_leaders, force); // remove old leaders that do not update in a logarithmic time
        tmp = this->getStorage()->removeOldNodes(this->node->timesilent); // remove followers that do not update in heartbeat time
        //inform other nodes of the removals
        rem.insert(rem.end(),tmp.begin(),tmp.end());
        if (rem.size() > 0) {
            this->connections->sendRemoveNodes(rem);
            tmp.clear();
            this->getStorage()->updateNodes(tmp,rem);
        }
        {
            auto t_end2 = std::chrono::high_resolution_clock::now();
            auto elapsed_time2 = std::chrono::duration_cast<std::chrono::duration<float>>(t_end2-t_start).count();
            cout << "timerFun3 " << elapsed_time2 << endl;
        }
        int i=0;
        int sent=0;
        while(i < ips.size() && sent < 2) {
            if(ips[i].id == this->nodeS.id) {
                i++;
                continue;
            }
            
            vector<Report::report_result> report = this->getStorage()->getReport();

            if(this->connections->sendMReport(ips[i], report)) {
                sent++;
                printf("Sent to Leader: %s\n",ips[i].ip.c_str());
            }
            i++;
        }
        {
            auto t_end2 = std::chrono::high_resolution_clock::now();
            auto elapsed_time2 = std::chrono::duration_cast<std::chrono::duration<float>>(t_end2-t_start).count();
            cout << "timerFun4 " << elapsed_time2 << endl;
        }
        if(iter % 4 == 0) {
            this->getStorage()->complete();
            {
                vector<Report::report_result> report = this->getStorage()->getReport(true);
                UIConnection conn(this->getMyNode(),this->node->interfaceIp, this->node->session);
                conn.sendTopology(report);
            }
            if((iter % (4*2)) == 0) {
                this->lastQuality +=1;
                bool param = this->lastQuality >= 2;
                if (param)
                    this->lastQuality = 0;
                this->selector.checkSelection(param);
            }
        }
        {
            auto t_end2 = std::chrono::high_resolution_clock::now();
            auto elapsed_time2 = std::chrono::duration_cast<std::chrono::duration<float>>(t_end2-t_start).count();
            cout << "timerFun5 " << elapsed_time2 << endl;
        }
        auto t_end = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(t_end-t_start).count();
        //std::cout << "timerFun1: "<< elapsed_time << " s"<< endl;
        int sleeptime = this->node->timePropagation-elapsed_time;
        if (sleeptime > 0)
            sleeper.sleepFor(chrono::seconds(sleeptime));
        iter++;
        
        //t_end = std::chrono::high_resolution_clock::now();
        //elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(t_end-t_start).count();
        //std::cout << "timerFun2: "<< elapsed_time << " s"<< endl;
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
        this->selector.checkSelection(true,true);
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
        if(node.id == this->getMyNode().id) {
            present = true;
        }
    }
    printf("Change role: %d\n",!present);
    if(!present){
        printf("here: %s %s %s\n",this->getMyNode().id.c_str(), this->getMyNode().ip.c_str(), this->getMyNode().port.c_str());
        for(auto node : leaders) {
            printf("      %s %s %s\n",node.id.c_str(), node.ip.c_str(), node.port.c_str());
        }
    }
    printf("A\n");
    fflush(stdout);
    if(!present) {
        printf("B\n");
        fflush(stdout);
        this->iter = 1;
        this->lastQuality = -random()%10-20;
        this->node->demote(leaders);
    }else {
        this->storage->removeChangeRole(leaders);
        printf("E\n");
        fflush(stdout);
        this->iter = 1;
        this->lastQuality = -random()%10-20;   
        sleeper.sleepFor(chrono::seconds(10));
        printf("F\n");
        fflush(stdout);
        for(auto ip : leaders) {
            if(ip.id == this->getMyNode().id)
                continue;
            if(!this->connections->sendMHello(ip)) {
                sleeper.sleepFor(chrono::seconds(10));
                if(!this->connections->sendMHello(ip)) {
                    fprintf(stderr,"cannot connect to all the other leaders (%s)\n",ip.ip.c_str());
                }
            }
        }
        printf("G\n");
        fflush(stdout);
        this->selector.stopSelection();
        printf("H\n");
        fflush(stdout);
    }
}

void Leader::changeRoles(Message::leader_update update) {
    if(update.selected.size() == 0)
        return;
    this->connections->sendChangeRoles(update);
}
