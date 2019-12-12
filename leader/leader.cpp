#include "leader.hpp"

#include "follower.hpp"
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>
#include <cmath>

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;

Leader::Leader(Message::node node, int nThreads) : Follower(node, nThreads) {
    timePropagation = 20;
    timeheartbeat = 120;
    status = FREE;
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
    vector<Message::node> vec;
    vec.push_back(this->nodeS);
    Follower::start(vec);
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
        
        int nF = this->storage->getAllNodes().size();
        int nL = this->storage->getMNodes().size();
        
        if(sqrt(nF) > nL+1) {
            printf("Leaders number: %d\nFollower number: %d\n",nL,nF);
            this->startSelection();
        }

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
    const std::lock_guard<std::mutex> lock(this->selectionMutex);
    if(this->status == READY) {
        if(this->id >= id) {
            return false;
        }
    }
    this->status = CHANGING;
    return true;
}

bool Leader::calcSelection(Message::node from, int id, bool &res) {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

    if(this->status != FREE)
        return false;
    
    //calculate the selection and send it back

    //TODO: decide a probability to do the calc

    bool notDo = false;

    //not doing it?
    if(notDo) {
        res = false;
        return true;
    }
    res = true;

    status = RECEIVED;

    //remove the thread
    //if status 0 the thread does not exist anymore so no deadlock
    if(this->selectionThread.joinable()) {
        this->selectionThread.join();
    }

    //start thread to calculate and send
    this->selectionThread = thread([this,id,from]{

        Message::leader_update sel = this->selection(id);

        this->connections->sendSelection(sel,from);

        const std::lock_guard<std::mutex> lock(this->selectionMutex);
        status = FREE;
    });

    return true;
}

bool Leader::updateSelection(Message::leader_update update) {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

    if(this->status == STARTED) {
        this->updates.push_back(update);
        return true;
    }
    return false;
}

Message::leader_update Leader::selection(int id) {
    
    //calculate with a script the update and set the id on it
    char *args[] = {"./script/cluster.py"};
    ReadProc * proc = new ReadProc(args);

     {
        std::lock_guard<std::mutex> lock(this->mTest);
        if(this->clusterProc) {
            delete proc;
            return Message::leader_update();
        }
        this->clusterProc = proc;
    }

    int res = proc->waitproc();

    if(res != 0) {
        return Message::leader_update();
    }

    string output = proc->readoutput();
    
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse((const char*)output.c_str());
    if(!ok)
        return Message::leader_update();
    
    if( !doc.HasMember("quality") || !doc["quality"].IsDouble() ||
        !doc.HasMember("new_leaders") || !doc["new_leaders"].IsArray() ||
        !doc.HasMember("changes") || !doc["changes"].IsInt()) {
        return Message::leader_update();
    }
    
    float quality = (float)doc["changes"].GetDouble();
    vector<Message::node> leaders;
    for (auto& v : doc["new_leaders"].GetArray()) {
        Message::node node;
        if(!node.setJson(v)) {
            return Message::leader_update();
        }
        leaders.push_back(node);
    }
    int changes = doc["changes"].GetInt();

    return Message::leader_update(leaders,quality,changes,id);
}

void Leader::startSelection() {
    printf("starting selection\n");
    this->updates.clear();

    {
        const std::lock_guard<std::mutex> lock(this->selectionMutex);

        if(status != FREE) {
            return;
        }
        this->id = random();
        status = READY;
    }

    if(!this->connections->sendInitiateSelection(this->id)) {
        return;
    }

    if(!this->connections->sendStartSelection(this->id)){
        this->connections->sendEndSelection(Message::leader_update(),false);
    }

    {
        const std::lock_guard<std::mutex> lock(this->selectionMutex);

        status = STARTED;
    }
    printf("started selection\n");
    //start thread here
    this->selectionThread = thread([this]{
        auto t1 = std::chrono::high_resolution_clock::now();
        Message::leader_update sel = this->selection(this->id);

        auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();
        
        int numLeaders = this->storage->getMNodes().size();

        //wait remaining time
        this->sleeper.sleepFor(chrono::seconds(10*numLeaders - duration));

        for(auto update : this->updates) {
            if(update.id != this->id) {
                continue;
            }
            //select best
            if(update.cost < sel.cost) {
                sel = update;
            }else if(update.cost == sel.cost && update.changes < sel.changes) {
                sel = update;
            }
        }
        printf("selected (cost = %f, changes = %d):\n",sel.cost,sel.changes);
        for(auto n : sel.selected) {
            printf("%s  %s  %s\n",n.id.c_str(),n.ip.c_str(),n.port.c_str());
        }
        this->connections->sendEndSelection(sel,true);

        {
            const std::lock_guard<std::mutex> lock(this->selectionMutex);

            status = FREE;
        }

        this->connections->sendChangeRoles(sel);
    });
}

void Leader::stopSelection() {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

    status = FREE;
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
        this->status = FREE;
    }
}

void Leader::changeRoles(Message::leader_update update) {
    this->connections->sendChangeRoles(update);
    this->changeRole(update.selected);
}