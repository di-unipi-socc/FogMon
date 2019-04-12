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

#include <sigar.h>
#include "rapidjson/document.h"

#include "microbit/microbit_discoverer.hpp"

#include "storage.hpp"

using namespace std;
using namespace rapidjson;

Node::Node(string ip, int nThreads) {
    this->nThreads = nThreads;
    this->running = false;
    this->timerReport = 5;
    this->timeTimerTest = 5;
    this->ipS = ip;
    this->storage = NULL;
    this->connections = NULL;
    this->server = NULL;
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
    
    mNodes.push_back(this->ipS);

    this->startEstimate();
    this->getHardware();

    if(!selectServer()) {
        fprintf(stderr,"Cannot connect to the main node\n");
        this->stop();
        exit(1);
    }

    this->timerThread = thread(&Node::timer, this);
    this->timerTestThread = thread(&Node::TestTimer, this);
}

void Node::stop() {
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
    vector<string> res;
    int i=0;
    while(res.empty() && i<mNodes.size()) {
        res = this->connections->requestMNodes(mNodes[i]);
        for(int j=0; j<res.size(); j++)
        {
            if(res[j]==std::string("::1")||res[j]==std::string("127.0.0.1"))
                res[j] = mNodes[i];
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
        unsigned int min = (unsigned int)this->testPing(res[imin]);

        for(int i=1; i<res.size(); i++) {
            unsigned int tmp = (unsigned int)this->testPing(res[i]);
            if(tmp < min) {
                imin = i;
                min = tmp;
            }
        }
        this->ipS = res[imin];
        if(!this->connections->sendHello(this->ipS)) {
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
    
    using namespace std::chrono_literals;

    int ret = -1;

    int port = random()%3000 + 5600;
    std::packaged_task<void(int)> task([](int port) {
        char command[1024];
        sprintf(command, "iperf3 -s -p %d -1 2>&1", port);
        string mode = "r";

        // Run Popen
        FILE *in;

        // Test output
        if(!(in = popen(command, mode.c_str()))){
            return -1;
        }

        // Close
        int exit_code = pclose(in);
        return -1;
    });
    auto f1 = task.get_future();
    std::thread(std::move(task),port).detach();

    // Use wait_for() with zero milliseconds to check thread status.
    auto status = f1.wait_for(50ms);

    // Print status.
    if (status == std::future_status::ready) {
        
    } else {
        ret = port;
    }
    
    return ret;
}

int Node::startEstimate() {
    using namespace std::chrono_literals;

    int ret = -1;

    int port = 8365;
    std::packaged_task<void(int)> task1([](int port) {
        char command[256];
        sprintf(command, "./assolo_rcv 2>&1");
        string mode = "r";

        // Run Popen
        FILE *in;

        // Test output
        if(!(in = popen(command, mode.c_str()))){
            return -1;
        }

        // Close
        int exit_code = pclose(in);
        return -1;
    });
    std::packaged_task<void(int)> task2([](int port) {
        char command[256];
        sprintf(command, "./assolo_snd 2>&1");
        string mode = "r";

        // Run Popen
        FILE *in;

        // Test output
        if(!(in = popen(command, mode.c_str()))){
            return -1;
        }

        // Close
        int exit_code = pclose(in);
        return -1;
    });

    auto f1 = task1.get_future();
    auto thread1 = std::thread(std::move(task1),port);
    auto th1 = thread1.native_handle();
    thread1.detach();
    auto f2 = task2.get_future();
    auto thread2 = std::thread(std::move(task2),port);
    auto th2 = thread2.native_handle();
    thread2.detach();
    
    return ret;;
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

float Node::testBandwidthEstimate(string ip, int port) {
    char command[1024];
    if(port > 0) {
        sprintf(command, "./assolo_run -R %s -S %s -J 2 -t 30 -u 100 -l 1 -U %d 2>&1", ip.c_str(),this->myIp.c_str(), port);
    }else
        sprintf(command, "./assolo_run -R %s -S %s -J 2 -t 30 -u 100 -l 1 2>&1", ip.c_str(), this->myIp.c_str());
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
        std::regex reg("Opening file: ([0-9a-zA-Z_\\.]*)\n");

        std::smatch m;
        
        string file = "";

        while (std::regex_search (output,m,reg)) {
            output = m.suffix().str();
            
            file = m[1];
        }
        if(file.empty())
            return -1;
        FILE *in = fopen(file.c_str(),"r");
        if(in == NULL)
            return -1;
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
                output = m.suffix().str();
                
                mean += stof(m[2]);
                num++;
            }
            if(num>0)  {
                mean = mean/num;
                cout << "estimate " << ip << " mbps " << mean << " kbps " << mean * 1000 << endl;
                return mean*1000;
            }    
        }
        return -1;
    }
    return -1;
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
    while(this->running) {
        //generate hardware report and send it
        this->getHardware();

        bool ris = this->connections->sendUpdate(this->ipS);

        if(ris == false) {
            ris = this->connections->sendUpdate(this->ipS);
            if(ris == false) {
                //change server
                cout << "Changing server..." << endl;
                if(!selectServer()) {
                    cout << "Failed to find a server!!!!!!!!" << endl;
                }
            }
        }

        sleep(this->timerReport);
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
        vector<string> ips = this->storage->getLRLatency(100,30);

        for(auto ip : ips) {
            if(myIp == ip)
                continue;
            int val = this->testPing(ip);
            if(val >= 0) {
                this->storage->saveLatencyTest(ip, val);
            }
        }
        //test bandwidth
        //get 10 nodes tested more than 300 seconds in the past
        ips = this->storage->getLRBandwidth(10,300);
        int i=0;
        int tested=0;
        while(i < ips.size() && tested < 1) {
            if(this->myIp == ips[i]) {
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

        //every 10 iteration ask the nodes in case the server cant reach this network
        if(iter%10 == 0) {
            ips = this->connections->requestNodes(this->ipS);
            vector<string> tmp = this->getStorage()->getNodes();
            vector<string> rem;

            for(auto ip : tmp) {
                bool found = false;
                int i=0;
                while(!found && i<ips.size()) {
                    if(ip == ips[i])
                        found = true;
                    i++;
                }
                if(!found) {
                    rem.push_back(ip);
                }
            }

            this->getStorage()->updateNodes(ips,rem);
        }

        if(iter%10 == 0) {
            vector<string> res = this->connections->requestMNodes(this->ipS);
            if(!res.empty()) {
                for(int j=0; j<res.size(); j++)
                {
                    if(res[j]==std::string("::1")||res[j]==std::string("127.0.0.1"))
                        res[j] = this->ipS;
                }
                mNodes = res;
            }
        }

        sleep(this->timeTimerTest);
        iter++;
    }
}


float Node::testBandwidth(std::string ip, float old, int &state) {
    float result = -1;
    int port;
    switch(state) {
        case 0: //base state
            port = this->connections->sendStartIperfTest(ip);
            if(port != -1) {
                result = this->testBandwidthIperf(ip,port);
                if(result >= 0)
                    state = 1;
            }
        break;
        case 1: //a test is already done
            port = this->connections->sendStartEstimateTest(ip);
            if(port != -1) {
                result = this->testBandwidthEstimate(ip,port);
                if(result >= 0 && abs(result - old)/old < 0.1) {
                    state = 2;
                }else {
                    state = 3;
                    result = -1;
                }
            }
        break;
        case 2: //estimate succeeded
            port = this->connections->sendStartEstimateTest(ip);
            if(port != -1) {
                result = this->testBandwidthEstimate(ip,port);
                if(result >= 0 && abs(result - old)/old < 0.1) {
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
                result = this->testBandwidthIperf(ip,port);
                if(result >= 0)
                    state = 0;
            }
    }
    return result;
}

void Node::setMyIp(std::string ip) {
    this->myIp = ip;
}

std::string Node::getMyIp() {
    return this->myIp;
}

Server* Node::getServer() {
    return this->server;
}