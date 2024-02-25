#include "selector.hpp"
#include "leader.hpp"
#include <cmath>

#include "uiconnection.hpp"

using namespace std;
Selector::Selector(ILeader *leader) {
    this->parent = leader;
    status = FREE;
    this->clusterProc = NULL;
    sleeper.start();
    this->id = 0;
    this->last = std::chrono::high_resolution_clock::now();
}

Selector::~Selector() {
    this->sleeper.stop();
    const std::lock_guard<std::mutex> lock(this->selectionMutex);

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
    printf("init selection %d >? %d\n",id,this->id);
    const std::lock_guard<std::mutex> lock(this->selectionMutex);
    if(this->status == READY) {
        if(this->id >= id) {
            if(!this->checkOld())
                return false;
        }
    }else if(this->status != FREE) {
        if(!this->checkOld())
            return false;
    }
    printf("init selection true\n");
    this->last = std::chrono::high_resolution_clock::now();
    this->status = CHANGING; // TODO: change after some time CHANGING to FREE if no calcSelection arrived
    return true;
}

bool Selector::checkOld() {
    if (status == FREE) {
        return true;
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(now-this->last).count();

    if(elapsed_time > 60*2) {
        printf("old selection stopping\n");
        status = FREE;

        std::lock_guard<std::mutex> lock2(this->clusterMutex);
        if(this->clusterProc) {
            delete this->clusterProc;
        }
        this->clusterProc = NULL;
        printf("old selection stopped\n");
        return true;
    }
    return false;
}

bool Selector::calcSelection(Message::node from, int id, bool &res) {
    printf("calc selection\n");
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
        int formula = this->parent->node->leaderFormula;
        Message::leader_update sel = this->selection(id, formula);

        if(!sel.empty()) {
            this->parent->getConnections()->sendSelection(sel,from);
        }
        {
            status = FREE;
        }
    });

    return true;
}

bool Selector::updateSelection(Message::leader_update update) {
    const std::lock_guard<std::mutex> lock(this->selectionMutex);
    printf("updating selection\n");
    if(this->status == STARTED) {
        this->updates.push_back(update);
        return true;
    }
    return false;
}

bool Selector::checkSelection(bool qualityCheck, bool doit) {

    if(doit) {
        printf("STARTING SELECTION (forced)\n");
        this->startSelection();
        return true;
    }
    
    int nF = this->parent->getStorage()->getAllNodes().size();
    int nL = this->parent->getStorage()->getMNodes().size();

    printf("[TESTING] Leaders number: %d\n[TESTING] Follower number: %d\n",nL,nF);

    bool check = false;
    int formula = this->parent->node->leaderFormula;
    if (formula == -2)
        check = trunc(sqrt(nF)*2) >= nL+1;
    else if (formula == -1)
        check = trunc(sqrt(nF)/2) >= nL+1;
    else if(formula > 0)
        check = formula != nL;
    else
        check = trunc(sqrt(nF)) >= nL+1;


    if(check) {
        printf("STARTING SELECTION (not enough nodes)\n");
        this->startSelection();
        return true;
    }
    if(qualityCheck) {
        try {
            //calculate with a script the update and set the id on it
            vector<string> args = {"./scripts/quality.py"};
            ReadProc * proc = new ReadProc(args);

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
            printf("quality check (cost = %f)\n",quality);
            if(quality > 20) {
                printf("STARTING SELECTION (bad quality)\n");
                this->startSelection();
                return true;
            }
        }catch(...) {
            printf("Exception in quality test.\n");
        }
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
    printf("stopped selection\n");
}

Message::leader_update Selector::selection(int id, int formula) {
    //calculate with a script the update and set the id on it
    
    //flush the storage so the script can read the new data
    this->parent->getStorage()->flush();
    
    vector<string> args = {"/usr/bin/python3","./scripts/cluster.py",std::to_string(formula)};
    ReadProc * proc = new ReadProc(args);

     {
        std::lock_guard<std::mutex> lock(this->clusterMutex);
        if(this->clusterProc) {
            delete this->clusterProc;
        }
        this->clusterProc = proc;
    }

    proc->waitproc();
    int res = proc->getexitcode();
    printf("selection result: %d\n",res);
    fflush(stdout);
    string output = proc->readoutput();
    printf("selection output: %s\n",output.c_str());
    fflush(stdout);
    if(res != 0) {
        return Message::leader_update();
    }
    
    
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
    printf("quality: %f\n",quality);
    vector<Message::node> leaders;
    for (auto& v : doc["new_leaders"].GetArray()) {
        if(!v.IsString()) {
            printf("empty [not string]: %s\n",output.c_str());
            return Message::leader_update();
        }
        

        leaders.push_back(Message::node(v.GetString(),"",""));
    }
    int changes = doc["changes"].GetInt();

    if (quality == 0 && changes == 0 && leaders.size() == 0) {
        printf("empty [zero]: %s\n",output.c_str());
        return Message::leader_update();
    }
    return Message::leader_update(leaders,quality,changes,id);
}

void Selector::startSelection() {
    printf("starting selection\n");
    this->updates.clear();

    {
        const std::lock_guard<std::mutex> lock(this->selectionMutex);

        if(status != FREE) {
            if(!this->checkOld()) {
                printf("aborted selection1\n");
                return;
            }
        }
        this->last = std::chrono::high_resolution_clock::now();
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
        printf("aborted selection2\n");
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
        printf("aborted selection3\n");
        return;
    }

    {
        const std::lock_guard<std::mutex> lock(this->selectionMutex);

        status = STARTED;
    }
    {
        const std::lock_guard<std::mutex> lock(this->selectionMutex);
        printf("started selection\n");
        if(this->selectionThread.joinable()) {
            this->selectionThread.join();
        }
        //start thread here
        this->selectionThread = thread([this]{
            auto t1 = std::chrono::high_resolution_clock::now();
            int formula = this->parent->node->leaderFormula;
            Message::leader_update sel = this->selection(this->id, formula);

            auto t2 = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();
            
            int numLeaders = this->parent->getStorage()->getMNodes().size();

            //wait remaining time
            this->sleeper.sleepFor(chrono::seconds(duration*3+20));

            for(auto update : this->updates) {
                printf("possible: (cost = %f, changes = %d, id = %d)\n",update.cost,update.changes,update.id);
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
            if ( sel.cost == 0 and sel.changes == 0 and sel.selected.size() == 0) {
                printf("zero!!!!\n");
                this->parent->getConnections()->sendEndSelection(Message::leader_update(),false);
                printf("aborted selection zero\n");
                return;
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
            for(auto n : sel.selected) {
                printf("%s  %s  %s\n",n.id.c_str(),n.ip.c_str(),n.port.c_str());
            }
            printf("ending selection\n");
            this->parent->getConnections()->sendEndSelection(sel,true);

            {
                status = FREE;
            }
            printf("preend selection\n");
            this->parent->changeRoles(sel);
            printf("ended selection\n");
            {
                UIConnection conn(this->parent->getMyNode(),this->parent->node->interfaceIp, this->parent->node->session);
                conn.sendChangeRole(sel);
            }
        });
    }
}