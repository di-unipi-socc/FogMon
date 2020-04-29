#include "selector.hpp"
#include "leader.hpp"
#include <cmath>

using namespace std;
Selector::Selector(ILeader *leader) {
    this->parent = leader;
    status = FREE;
    this->clusterProc = NULL;
    sleeper.start();
}

Selector::~Selector() {
    this->sleeper.stop();


    if(this->selectionThread.joinable())
        this->selectionThread.join();

    {
        std::lock_guard<std::mutex> lock(this->clusterMutex);
        if(this->clusterProc)
            delete this->clusterProc;
        this->clusterProc = NULL;
    }
}

bool Selector::initSelection(int id) {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);
    if(this->status == READY) {
        if(this->id >= id) {
            return false;
        }
    }
    this->status = CHANGING;
    return true;
}

bool Selector::calcSelection(Message::node from, int id, bool &res) {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

    if(this->status != CHANGING)
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

        if(!sel.empty()) {
            this->parent->getConnections()->sendSelection(sel,from);
        }

        const std::lock_guard<std::mutex> lock(this->selectionMutex);
        status = FREE;
    });

    return true;
}

bool Selector::updateSelection(Message::leader_update update) {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

    if(this->status == STARTED) {
        this->updates.push_back(update);
        return true;
    }
    return false;
}

bool Selector::checkSelection(bool doit) {

    if(doit) {
        this->startSelection();
        return true;
    }
    
    int nF = this->parent->getStorage()->getAllNodes().size();
    int nL = this->parent->getStorage()->getMNodes().size();

    printf("[TESTING] Leaders number: %d\n[TESTING] Follower number: %d\n",nL,nF);

    if(sqrt(nF) >= nL+1) {
        printf("STARTING SELECTION");
        this->startSelection();
        return true;
    }
    try {
    //calculate with a script the update and set the id on it
    const char *args[] = {"./scripts/quality.py",NULL};
    ReadProc * proc = new ReadProc((char**)args);

     {
        std::lock_guard<std::mutex> lock(this->clusterMutex);
        if(this->clusterProc) {
            delete this->clusterProc;
        }
        this->clusterProc = proc;
    }

    int res = proc->waitproc();

    if(res != 0) {
        return false;
    }

    string output = proc->readoutput();
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse((const char*)output.c_str());
    if(!ok)
        return false;
    
    if( !doc.HasMember("quality") || !doc["quality"].IsDouble()) {
        return false;
    }
    
    float quality = (float)doc["quality"].GetDouble();

    if(quality > 3)
        this->startSelection();
        return true;
    }catch(...) {
        printf("Exception in quality test.\n");
    }
    return false;
}

void Selector::stopSelection() {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

    status = FREE;

    std::lock_guard<std::mutex> lock2(this->clusterMutex);
    if(this->clusterProc) {
        delete this->clusterProc;
    }
    this->clusterProc = NULL;
}

Message::leader_update Selector::selection(int id) {

    //calculate with a script the update and set the id on it
    const char *args[] = {"./scripts/cluster.py",NULL};
    ReadProc * proc = new ReadProc((char**)args);

     {
        std::lock_guard<std::mutex> lock(this->clusterMutex);
        if(this->clusterProc) {
            delete this->clusterProc;
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
    
    float quality = (float)doc["quality"].GetDouble();
    vector<Message::node> leaders;
    for (auto& v : doc["new_leaders"].GetArray()) {
        if(!v.IsString())
            return Message::leader_update();
        

        leaders.push_back(Message::node(v.GetString(),"",""));
    }
    int changes = doc["changes"].GetInt();

    return Message::leader_update(leaders,quality,changes,id);
}

void Selector::startSelection() {
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

    if(!this->parent->getConnections()->sendInitiateSelection(this->id)) {
        {
            const std::lock_guard<std::mutex> lock(this->selectionMutex);

            if(status == READY) {
                status = FREE;
            }
        }
        return;
    }

    if(!this->parent->getConnections()->sendStartSelection(this->id)){
        {
            const std::lock_guard<std::mutex> lock(this->selectionMutex);

            if(status == READY) {
                status = FREE;
            }
        }
        this->parent->getConnections()->sendEndSelection(Message::leader_update(),false);
        return;
    }

    {
        const std::lock_guard<std::mutex> lock(this->selectionMutex);

        status = STARTED;
    }
    printf("started selection\n");
    if(this->selectionThread.joinable()) {
        this->selectionThread.join();
    }
    //start thread here
    this->selectionThread = thread([this]{
        auto t1 = std::chrono::high_resolution_clock::now();
        Message::leader_update sel = this->selection(this->id);

        auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();
        
        int numLeaders = this->parent->getStorage()->getMNodes().size();

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

        auto nodes = this->parent->getStorage()->getAllNodes();
        auto tmpS = sel.selected;

        sel.selected.clear();

        for(auto node : nodes) {
            for(auto sele : tmpS) {
                if(sele.id == node.id) {
                    sel.selected.push_back(node);
                    break;
                }
            }
        }

        this->parent->getConnections()->sendEndSelection(sel,true);

        {
            const std::lock_guard<std::mutex> lock(this->selectionMutex);

            status = FREE;
        }

        this->parent->changeRoles(sel);
    });
}