#include "follower.hpp"
#include <sys/statvfs.h>
#include <sys/sysinfo.h>

#include <sstream>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>

#include <future>
#include <chrono>

#include <unistd.h>

#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <sys/eventfd.h>
#include <signal.h>

#include <sigar.h>
#include "rapidjson/document.h"

#include "microbit/microbit_discoverer.hpp"

#include "storage.hpp"
#include "node.hpp"

using namespace std;
using namespace rapidjson;

Follower::Follower(Message::node node, int nThreads) : IAgent() {
    this->nThreads = nThreads;

    this->storage = NULL;
    this->connections = NULL;
    this->server = NULL;
    this->pIperf = NULL;
    this->pAssoloRcv = NULL;
    this->pAssoloSnd = NULL;
    this->pTest = NULL;
    this->myNode = node;
}

void Follower::initialize(Factory* fact) {
    if(fact == NULL) {
        this->factory = &this->tFactory;
    }else {
        this->factory = fact;
    }
    if(this->storage == NULL)
        this->storage = this->factory->newStorage("leader_node.db");
    if(this->connections == NULL) {
        this->connections = this->factory->newConnections(this->nThreads);
    }
    this->connections->initialize(this);
    this->server = this->factory->newServer(this->connections,5555);
}

Follower::~Follower() {
    this->stop();
    try{
        delete this->connections;
        this->connections = NULL;
    }catch(...) {}
    try{
        delete this->storage;
        this->storage = NULL;
    }catch(...) {}
    try{
        delete this->server;
        this->server = NULL;
    }catch(...) {}
}

void Follower::start(vector<Message::node> mNodes) {
    IAgent::start(mNodes);
    this->server->start();
    srandom(time(nullptr));

    if(this->startEstimate() != 0) {
        fprintf(stderr,"Cannot start the estimate\n");
        this->stop();
        exit(1);
    }
    if(this->startIperf() != 0) {
        fprintf(stderr,"Cannot start iperf3\n");
        this->stop();
        exit(1);
    }

    this->getHardware();

    if(!selectServer(mNodes)) {
        fprintf(stderr,"Cannot connect to the main node\n");
        this->stop();
        exit(1);
    }

    this->timerThread = thread(&Follower::timer, this);
    this->timerTestThread = thread(&Follower::TestTimer, this);
}

void signalHandler( int signum ) {
   cout << "Interrupt signal (" << signum << ") received.\n"; 
}


void Follower::stop() {
    IAgent::stop();
    if(this->pIperf)
        delete this->pIperf;
    if(this->pAssoloRcv)
        delete this->pAssoloRcv;
    if(this->pAssoloSnd)
        delete this->pAssoloSnd;
    {
        std::lock_guard<std::mutex> lock(this->mTest);
        if(this->pTest)
            delete this->pTest;
        this->pTest = NULL;
    }
    this->pIperf = NULL;
    this->pAssoloRcv = NULL;
    this->pAssoloSnd = NULL;
    if(this->timerThread.joinable())
    {
        this->timerThread.join();
    }
    if(this->timerTestThread.joinable())
    {
        this->timerTestThread.join();
    }
    if(this->server)
        this->server->stop();
    if(this->connections)
        this->connections->stop();
    if(this->storage)
        this->storage->close();
}

bool Follower::selectServer(vector<Message::node> mNodes) {
    //ask the MNodes list and select one MNode with the min latency
    cout << "Selecting server..." << endl;
    
    //if is a leader connect to it
    if(!this->node->isFollower()) {
        if(!this->connections->sendHello(this->nodeS))
            return false;
        return true;
    }

    vector<Message::node> res;
    int i=0;
    while(res.empty() && i<mNodes.size()) {
        cout << "trying "<<mNodes[i].ip<<endl;
        for(int k=0; k<5; k++) {
            res = this->connections->requestMNodes(mNodes[i]);
            if(res.size() != 0)
                k=5;
            sleeper.sleepFor(chrono::seconds(3));
        }
        
        for(int j=0; j<res.size(); j++)
        {
            if(res[j].ip==std::string("::1")||res[j].ip==std::string("127.0.0.1"))
                res[j].ip = mNodes[i].ip;
        }
        i++;
    }
    if(!res.empty())
        this->node->setMNodes(res);
    else {
        //try connecting anyway
        res = mNodes;
    }

    //try every element of res for a server and select the closest
    while(!res.empty()) {
        int imin=0;
        unsigned int min = (unsigned int)this->testPing(res[imin].ip);

        for(int i=1; i<res.size(); i++) {
            unsigned int tmp = (unsigned int)this->testPing(res[i].ip);
            if(tmp < min) {
                imin = i;
                min = tmp;
            }
        }
        this->nodeS = res[imin];
        if(!this->connections->sendHello(this->nodeS)) {
            res.erase(res.begin()+imin);
        }else
            return true;
        
    }
    return false;
}

bool Follower::checkServer(vector<Message::node> mNodes) {
    vector<Message::node> res = mNodes;

    unsigned int curr = (unsigned int)this->testPing(this->nodeS.ip);

    int imin=0;
    unsigned int min = (unsigned int)this->testPing(res[imin].ip);

    for(int i=1; i<res.size(); i++) {
        unsigned int tmp = (unsigned int)this->testPing(res[i].ip);
        if(tmp < min) {
            imin = i;
            min = tmp;
        }
    }

    if( curr > min + 5) {
        float mean = 0;
        float meanCurr = 0;
        for(int i=0; i<5; i++) {
            unsigned int val = (unsigned int)this->testPing(res[imin].ip);
            mean += val;
            unsigned int valCurr = (unsigned int)this->testPing(this->nodeS.ip);
            meanCurr += valCurr;
        }
        mean/=5;
        meanCurr/=5;

        if(meanCurr > mean + 5) {
            return true;
        }
    } 
    
    return false;
}

IConnections* Follower::getConnections() {
    return (IConnections*)(&(this->connections));
}

IStorage* Follower::getStorage() {
    return this->storage;
}

int Follower::startIperf() {

    int port = random()%1000 + 5600;

    port = 5201;

    char command[1024];
    sprintf(command, "%d", port);

    char *args[] = {(char*)"/bin/iperf3", (char*)"-s",(char*)"-p",command, NULL };
    ReadProc *proc = new ReadProc(args);
    sleeper.sleepFor(chrono::milliseconds(50));
    int res = proc->nowaitproc();

    if(res != 0) {
        this->portIperf = port;
        this->pIperf = proc;
        return 0;
    }
    cout << proc->readoutput();
    return -1;
}

int Follower::startEstimate() {
    
    
    int port = random()%2000 + 5600;

    port = 8366;

    char *args1[] = {(char*)"./assolo_rcv", NULL };
    ReadProc *proc1 = new ReadProc(args1);

    char command[256];
    sprintf(command, "-U %d", port);

    char *args2[] = {(char*)"./assolo_snd", command, NULL };
    ReadProc *proc2 = new ReadProc(args2);

    sleeper.sleepFor(chrono::milliseconds(50));
    int res1 = proc1->nowaitproc();
    int res2 = proc2->nowaitproc();

    if(res1 != 0 && res2 != 0) {
        this->pAssoloRcv = proc1;
        this->pAssoloSnd = proc2;
        this->portAssolo = port;
        return 0;
    }else {
        delete proc1;
        delete proc2;

    }
    return -1;
}

float Follower::testBandwidthIperf(string ip, int port) {
    char command[256];
    if(port > 0) {
        sprintf(command, "iperf3 -c %s -p %d -t 1 -i 1 -J 2>&1", ip.c_str(), port);
    }else
        sprintf(command, "iperf3 -c %s -t 1 -i 1 -J 2>&1", ip.c_str());
    string mode = "r";
    string output;

    std::stringstream sout;

    // Run Popen
    FILE *in;
    char buff[512];

    // Test output
    if(!(in = popen(command, mode.c_str()))){
        return -1;
    }

    // Parse output
    while(fgets(buff, sizeof(buff), in)!=NULL){
        sout << buff;
    }

    // Close
    int exit_code = pclose(in);

    // set output
    output = sout.str();
    if(exit_code == 0) {
        Document doc;
        ParseResult ok = doc.Parse((const char*)output.c_str());
        if(!ok)
            return -1;
        
        if( !doc.HasMember("end") && !doc["end"].IsObject() &&
            !doc["end"].HasMember("sum_received") && !doc["end"]["sum_received"].IsObject() &&
            !doc["end"]["sum_received"].HasMember("bits_per_second") && !doc["end"]["sum_received"]["bits_per_second"].IsFloat())
            return -1;

        float val = doc["end"]["sum_received"]["bits_per_second"].GetFloat();

        cout << "iperf3 " << ip << " bps " << val << " kbps " << val /1000 <<endl;
        //this->storage->saveBandwidthTest(ip, val/1000, 0);
        return val/1000;
    }
    return -1;
}

float Follower::testBandwidthEstimate(string ip, string myIp, float old) {
    
    if(old < 0) {
        return -1;
    }

    char command[1024];
    sprintf(command, "%d", this->portAssolo);
    old = old/1000; //convert to mbps
    if(old < 1.0) {
        old = 5.0;
    }
    const char *args[] = {"./assolo_run","-R",(char*)ip.c_str(),"-S",(char*)myIp.c_str(),"-J", "3", "-t", "30", "-u", (char*)to_string(old*20).c_str(), "-l", (char*)to_string(old/5).c_str(), "-U",command, NULL };
    ReadProc *proc = new ReadProc((char**)args);


    {
        std::lock_guard<std::mutex> lock(this->mTest);
        if(this->pTest) {
            delete proc;
            return -1;
        }
        this->pTest = proc;
    }
    

    while(proc->nowaitproc() != 0) {
        sleeper.sleepFor(chrono::milliseconds(100));
    }

    int exit_code = 0;

    // set output
    string output = proc->readoutput();
    char buff[512];
    float ret = -1;
    if(exit_code == 0) {
        std::regex reg("Opening file: ([0-9a-zA-Z_\\.]*)\n");

        std::smatch m;
        
        string file = "";

        while (std::regex_search (output,m,reg)) {           
            file = m[1].str();
            output = m.suffix().str();
        }
        if(!file.empty())
        {
            FILE *in = fopen(file.c_str(),"r");
            if(in != NULL)
            {
                std::stringstream sout;
                while(fgets(buff, sizeof(buff), in)!=NULL){
                    sout << buff;
                }
                output = sout.str();
                unlink(file.c_str());
                {
                    std::regex reg("([0-9\\.]*) ([0-9\\.]*)\n");

                    std::smatch m;
                    
                    float mean = 0;
                    int num = 0;
                    while (std::regex_search (output,m,reg)) {
                        try {
                        mean += stof(m[2].str());
                        num++;
                        }catch(...) {
                            
                        }
                        output = m.suffix().str();
                    }
                    if(num>0)  {
                        mean = mean/num;
                        cout << "estimate " << ip << " mbps " << mean << " kbps " << mean * 1000 << endl;
                        ret = mean*1000;
                    }    
                }
            }
            fclose(in);
        }
    }

    {
        std::lock_guard<std::mutex> lock(this->mTest);
        if(this->pTest)
            delete this->pTest;
        this->pTest = NULL;
    }
    return ret;
}

int Follower::testPing(string ip) {
    char command[1024];
    sprintf(command, "ping -c 3 %s 2>&1", ip.c_str());
    string mode = "r";
    string output;

    std::stringstream sout;

    // Run Popen
    FILE *in;
    char buff[512];

    // Test output
    if(!(in = popen(command, mode.c_str()))){
        return -1;
    }

    // Parse output
    while(fgets(buff, sizeof(buff), in)!=NULL){
        sout << buff;
    }

    // Close
    int exit_code = pclose(in);

    // set output
    output = sout.str();

    if(exit_code == 0) {
        std::regex reg("time=([0-9\\.]*) ms");

        std::smatch m;
        
        while (std::regex_search (output,m,reg)) {
            //this->storage->saveLatencyTest(ip, stoi(m[1]));
            cout << ip << " ms " << stoi(m[1].str()) << endl;
            return stoi(m[1].str());
            //output = m.suffix().str();
        }
    }
    return -1;
}

void Follower::getHardware() {

    int status, i;
    sigar_t *sigar;
    sigar_cpu_t cpuT1;
    sigar_cpu_t cpuT2;
    sigar_cpu_list_t cpulist;

    sigar_open(&sigar);
    sigar_file_system_usage_t disk;
    sigar_file_system_usage_get(sigar,"/",&disk);

    status = sigar_cpu_list_get(sigar, &cpulist);
    if (status != SIGAR_OK) {
        printf("cpu_list error: %d (%s)\n",
               status, sigar_strerror(sigar, status));
        exit(1);
    }

    status = sigar_cpu_get(sigar, &cpuT1);
    if (status != SIGAR_OK) {
        printf("cpu error: %d (%s)\n",
               status, sigar_strerror(sigar, status));
        exit(1);
    }
    sleeper.sleepFor(chrono::seconds(1));
    status = sigar_cpu_get(sigar, &cpuT2);
    if (status != SIGAR_OK) {
        printf("cpu error: %d (%s)\n",
               status, sigar_strerror(sigar, status));
        exit(1);
    }

    unsigned long long diffIdle = cpuT2.idle - cpuT1.idle;
    unsigned long long totaldiff = cpuT2.total - cpuT1.total + cpuT2.user - cpuT1.user + cpuT2.sys - cpuT1.sys;

    sigar_mem_t mem;
    sigar_mem_get(sigar,&mem);

    Report::hardware_result hardware;
    hardware.cores = cpulist.number;
    hardware.mean_free_cpu = ((float)diffIdle)/(totaldiff);
    hardware.memory = mem.total;
    hardware.mean_free_memory = mem.actual_free;
    hardware.disk = disk.total;
    hardware.mean_free_disk = disk.avail;

    this->storage->saveHardware(hardware);

    sigar_cpu_list_destroy(sigar, &cpulist);

    sigar_close(sigar);
}

void Follower::timer() {
    int iter=0;
    while(this->running) {
        //generate hardware report and send it
        this->getHardware();

        std::optional<std::pair<int64_t,Message::node>> ris = this->connections->sendUpdate(this->nodeS, this->update);

        if(ris == nullopt) {
            ris = this->connections->sendUpdate(this->nodeS,this->update);
            if(ris == nullopt) {
                //change server
                cout << "Changing server..." << endl;
                if(!selectServer(this->node->getMNodes())) {
                    cout << "Failed to find a server!!!!!!!!" << endl;
                }
                iter=0;
            }
        }

        if(ris != nullopt) {
            this->update.first= (*ris).first;
            this->update.second= (*ris).second;
        }

        //every 10 iterations ask the nodes in case the server cant reach this network
        if(iter%10 == 0) {
            vector<Message::node> ips = this->connections->requestNodes(this->nodeS);
            vector<Message::node> tmp = this->getStorage()->getNodes();
            vector<Message::node> rem;

            for(auto node : tmp) {
                bool found = false;
                int i=0;
                while(!found && i<ips.size()) {
                    if(node.id == ips[i].id)
                        found = true;
                    i++;
                }
                if(!found) {
                    rem.push_back(node);
                }
            }

            this->getStorage()->updateNodes(ips,rem);
        }

        //every leaderCheck iterations update the MNodes
        if(iter% this->node->leaderCheck == 9) {
            vector<Message::node> res = this->connections->requestMNodes(this->nodeS);
            if(!res.empty()) {
                for(int j=0; j<res.size(); j++)
                {
                    if(res[j].ip==std::string("::1")||res[j].ip==std::string("127.0.0.1"))
                        res[j].ip = this->nodeS.ip;
                }
                this->node->setMNodes(res);
            }
            cout << "Check server" << endl;
            bool change = this->checkServer(res);
            if(change) {
                cout << "Changing server" << endl;
                if(!selectServer(res)) {
                    cout << "Failed to find a server!!!!!!!!" << endl;
                }
            }
            cout << "no change" << endl;
        }

        sleeper.sleepFor(chrono::seconds(this->node->timeReport));
        iter++;
    }
}

void Follower::testIoT() {
    MicrobitDiscoverer discoverer;
    vector<IThing*> things = discoverer.discover();

    for(int i=0; i<things.size(); i++) {
        this->getStorage()->addIot(things[i]);
    }

    for(int i=0; i<things.size(); i++) {
        delete things[i];
    }
}

void Follower::TestTimer() {
    int iter=0;
    while(this->running) {
        //monitor IoT
        if(iter%4 == 0)
            this->testIoT();

        //get list ordered by time for the latency tests
        //test the least recent
        vector<Message::node> ips = this->storage->getLRLatency(this->node->maxPerLatency, this->node->timeLatency);

        for(auto node : ips) {
            if(this->myNode.id == node.id)
                continue;
            int val = this->testPing(node.ip);
            if(val >= 0) {
                this->storage->saveLatencyTest(node, val);
            }
        }
        //test bandwidth
        //get 10 nodes tested more than 300 seconds in the past
        ips = this->storage->getLRBandwidth(this->node->maxPerBandwidth + 5, this->node->timeBandwidth);
        int i=0;
        int tested=0;
        while(i < ips.size() && tested < 1) {
            if(this->myNode.id == ips[i].id) {
                i++;
                continue;
            }
            Report::test_result last;
            int state = this->storage->getTestBandwidthState(ips[i], last);
            float val = this->testBandwidth(ips[i], last.mean, state);
            if(val < 0) {
                val = this->testBandwidth(ips[i], last.mean, state);
            }

            if(val >= 0) {
                this->storage->saveBandwidthTest(ips[i], val, state);
                tested++;
            }
            i++;
        }

        sleeper.sleepFor(chrono::seconds(this->node->timeTests));
        iter++;
    }
}


float Follower::testBandwidth(Message::node ip, float old, int &state) {
    float result = -1;
    int port;
    string myIp;
    switch(state) {
        case 0: //base state
            port = this->connections->sendStartIperfTest(ip);
            if(port != -1) {
                result = this->testBandwidthIperf(ip.ip,port);
                if(result >= 0)
                    state = 1;
            }
        break;
        case 1: //a test is already done
            port = this->connections->sendStartEstimateTest(ip, myIp);
            if(port != -1) {
                result = this->testBandwidthEstimate(ip.ip,myIp,old);
                if(result >= 0 && abs(result - old)/old < 0.3) {
                    state = 2;
                    result = old; //return the old result because is still reliable
                }else {
                    state = 3;
                    result = -1;
                }
            }
        break;
        case 2: //estimate succeeded
            port = this->connections->sendStartEstimateTest(ip, myIp);
            if(port != -1) {
                result = this->testBandwidthEstimate(ip.ip, myIp,old);
                if(result >= 0 && abs(result - old)/old < 0.3) {
                    state = 0;
                    result = old; //return the old result because is still reliable
                }else {
                    state = 3;
                    result = -1;
                }
            }
        break;
        case 3: //estimate failed
            port = this->connections->sendStartIperfTest(ip);
            if(port != -1) {
                result = this->testBandwidthIperf(ip.ip,port);
                if(result >= 0)
                    state = 0;
            }
    }
    return result;
}

void Follower::setMyId(std::string id) {
    this->myNode.id = id;
}

Message::node Follower::getMyNode() {
    return this->myNode;
}

Server* Follower::getServer() {
    return this->server;
}

bool Follower::setParam(std::string name, int value) {
    if(value <= 0)
        return false;

    printf("Param %s: %d\n",name.c_str(),value);
    return false;
}

int Follower::getIperfPort() {
    return this->portIperf;
}

int Follower::getEstimatePort() {
    return this->portAssolo;
}

void Follower::changeRole(vector<Message::node> leaders) {
    cout << "Change role arrived:" << endl;
    fflush(stdout);
    this->node->setMNodes(leaders);
    for(auto l : leaders) {
        cout << l.id << endl;
        if(l.id == this->myNode.id) {
            cout << "match!" << endl;
            this->node->setMNodes(leaders);
            this->node->promote();
            return;
        }
    }   
}