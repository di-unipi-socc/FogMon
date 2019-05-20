#include "node.hpp"
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

using namespace std;
using namespace rapidjson;

Node::Node(Message::node node, string port, int nThreads) {
    this->nThreads = nThreads;
    this->running = false;
    this->timeReport = 30;
    this->timeTests = 30;
    this->timeLatency = 30;
    this->maxPerLatency = 100;
    this->timeBandwidth = 600;
    this->maxPerBandwidth = 1; 
    this->nodeS = node;
    this->storage = NULL;
    this->connections = NULL;
    this->server = NULL;
    this->pIperf = NULL;
    this->pAssoloRcv = NULL;
    this->pAssoloSnd = NULL;
    this->pTest = NULL;
    this->myNode.port = port;
    this->myNode.id = "";
}

void Node::initialize(Factory* fact) {
    if(fact == NULL) {
        this->factory = &this->tFactory;
    }else {
        this->factory = fact;
    }
    if(this->storage == NULL)
        this->storage = this->factory->newStorage("node.db");
    if(this->connections == NULL) {
        this->connections = this->factory->newConnections(this->nThreads);
        this->connections->initialize(this);
    }
    this->server = this->factory->newServer(this->connections,5555);
}

Node::~Node() {
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
    exit(0);
}

void Node::start() {
    this->running = true;
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
    mNodes = this->connections->requestMNodes(this->nodeS);
    if(!selectServer()) {
        fprintf(stderr,"Cannot connect to the main node\n");
        this->stop();
        exit(1);
    }

    this->timerThread = thread(&Node::timer, this);
    this->timerTestThread = thread(&Node::TestTimer, this);
}

void signalHandler( int signum ) {
   cout << "Interrupt signal (" << signum << ") received.\n"; 
}


void Node::stop() {
    if(this->pIperf)
        delete this->pIperf;
    if(this->pAssoloRcv)
        delete this->pAssoloRcv;
    if(this->pAssoloSnd)
        delete this->pAssoloSnd;
    {
        std::unique_lock<std::mutex> lock(this->mTest);
        if(this->pTest)
            delete this->pTest;
        this->pTest = NULL;
    }
    this->pIperf = NULL;
    this->pAssoloRcv = NULL;
    this->pAssoloSnd = NULL;
    this->running = false;
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

bool Node::selectServer() {
    //ask the MNodes list and select one MNode with the min latency
    cout << "Selecting server..." << endl;
    vector<Message::node> res;
    int i=0;
    while(res.empty() && i<mNodes.size()) {
        res = this->connections->requestMNodes(mNodes[i]);
        for(int j=0; j<res.size(); j++)
        {
            if(res[j].ip==std::string("::1")||res[j].ip==std::string("127.0.0.1"))
                res[j].ip = mNodes[i].ip;
        }
        i++;
    }
    if(!res.empty())
        mNodes = res;
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

IConnections* Node::getConnections() {
    return (IConnections*)(&(this->connections));
}

IStorage* Node::getStorage() {
    return this->storage;
}

int Node::startIperf() {

    int port = random()%1000 + 5600;

    char command[1024];
    sprintf(command, "%d", port);

    char *args[] = {"/bin/iperf3", "-s","-p",command, NULL };
    ReadProc *proc = new ReadProc(args);
    usleep(50*1000);
    int res = proc->nowaitproc();

    if(res != 0) {
        this->portIperf = port;
        this->pIperf = proc;
        return 0;
    }
    cout << proc->readoutput();
    return -1;
}

int Node::startEstimate() {
    
    
    int port = random()%2000 + 5600;

    char *args1[] = {"./assolo_rcv", NULL };
    ReadProc *proc1 = new ReadProc(args1);

    char command[256];
    sprintf(command, "-U %d", port);

    char *args2[] = {"./assolo_snd", command, NULL };
    ReadProc *proc2 = new ReadProc(args2);

    usleep(50*1000);
    int res1 = proc1->nowaitproc();
    int res2 = proc2->nowaitproc();

    if(res1 != 0 && res2 != 0) {
        this->pAssoloRcv = proc1;
        this->pAssoloSnd = proc2;
        this->portAssolo = port;
        return 0;
    }else {
        proc1->killproc();
        proc2->killproc();
    }
    return -1;
}

float Node::testBandwidthIperf(string ip, int port) {
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

float Node::testBandwidthEstimate(string ip, string myIp, float old) {
    
    if(old < 0) {
        return -1;
    }

    char command[1024];
    sprintf(command, "%d", this->portAssolo);
    old = old/1000; //convert to mbps
    char *args[] = {"./assolo_run","-R",(char*)ip.c_str(),"-S",(char*)myIp.c_str(),"-J", "3", "-t", "30", "-u", (char*)to_string(old*20).c_str(), "-l", (char*)to_string(old/5).c_str(), "-U",command, NULL };
    ReadProc *proc = new ReadProc(args);


    {
        std::unique_lock<std::mutex> lock(this->mTest);
        if(this->pTest)
            return -1;
        this->pTest = proc;
    }
    

    while(proc->nowaitproc() != 0) {
        usleep(100*1000);
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
            output = m.suffix().str();
            
            file = m[1];
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
                //unlink(file.c_str());
                {
                    std::regex reg("([0-9\\.]*) ([0-9\\.]*)\n");

                    std::smatch m;
                    
                    float mean = 0;
                    int num = 0;
                    while (std::regex_search (output,m,reg)) {
                        output = m.suffix().str();
                        
                        mean += stof(m[2]);
                        num++;
                    }
                    if(num>0)  {
                        mean = mean/num;
                        cout << "estimate " << ip << " mbps " << mean << " kbps " << mean * 1000 << endl;
                        ret = mean*1000;
                    }    
                }
            }
        }
    }

    {
        std::unique_lock<std::mutex> lock(this->mTest);
        if(this->pTest)
            delete this->pTest;
        this->pTest = NULL;
    }
    return ret;
}

int Node::testPing(string ip) {
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
            output = m.suffix().str();
            //this->storage->saveLatencyTest(ip, stoi(m[1]));
            cout << ip << " ms " << stoi(m[1]) << endl;
            return stoi(m[1]);
        }
    }
    return -1;
}

void Node::getHardware() {

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
    sleep(1);
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

void Node::timer() {
    int iter=0;
    while(this->running) {
        //generate hardware report and send it
        this->getHardware();

        bool ris = this->connections->sendUpdate(this->nodeS);

        if(ris == false) {
            ris = this->connections->sendUpdate(this->nodeS);
            if(ris == false) {
                //change server
                cout << "Changing server..." << endl;
                if(!selectServer()) {
                    cout << "Failed to find a server!!!!!!!!" << endl;
                }
            }
        }

        //every 10 iteration ask the nodes in case the server cant reach this network
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

        //every 10 iteration update the MNodes
        if(iter%10 == 0) {
            vector<Message::node> res = this->connections->requestMNodes(this->nodeS);
            if(!res.empty()) {
                for(int j=0; j<res.size(); j++)
                {
                    if(res[j].ip==std::string("::1")||res[j].ip==std::string("127.0.0.1"))
                        res[j].ip = this->nodeS.ip;
                }
                mNodes = res;
            }
        }

        sleep(this->timeReport);
        iter++;
    }
}

void Node::testIoT() {
    MicrobitDiscoverer discoverer;
    vector<IThing*> things = discoverer.discover();

    for(int i=0; i<things.size(); i++) {
        this->getStorage()->addIot(things[i]);
    }

    for(int i=0; i<things.size(); i++) {
        delete things[i];
    }
}

void Node::TestTimer() {
    int iter=0;
    while(this->running) {
        //monitor IoT
        if(iter%4 == 0)
            this->testIoT();

        //get list ordered by time for the latency tests
        //test the least recent
        vector<Message::node> ips = this->storage->getLRLatency(this->maxPerLatency, this->timeLatency);

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
        ips = this->storage->getLRBandwidth(this->maxPerBandwidth, this->timeBandwidth);
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

        sleep(this->timeTests);
        iter++;
    }
}


float Node::testBandwidth(Message::node ip, float old, int &state) {
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
                if(result >= 0 && abs(result - old)/old < 0.2) {
                    state = 2;
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
                if(result >= 0 && abs(result - old)/old < 0.2) {
                    state = 0;
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

void Node::setMyId(std::string id) {
    this->myNode.id = id;
}

Message::node Node::getMyNode() {
    return this->myNode;
}

Server* Node::getServer() {
    return this->server;
}

bool Node::setParam(std::string name, int value) {
    if(value <= 0)
        return false;

    if(name == string("time-report")) {
        this->timeReport = value;
    }else if(name == string("time-tests")) {
        this->timeTests = value;
    }else if(name == string("time-latency")) {
        this->timeLatency = value;
    }else if(name == string("time-bandwidth")) {
        this->timeBandwidth = value;
    }else if(name == string("max-per-latency")) {
        this->maxPerLatency = value;
    }else if(name == string("max-per-bandwidth")) {
        this->maxPerBandwidth = value;
    }else{
        return false;
    }
    printf("Param %s: %d\n",name.c_str(),value);
    return true;
}

int Node::getIperfPort() {
    return this->portIperf;
}

int Node::getEstimatePort() {
    return this->portAssolo;
}