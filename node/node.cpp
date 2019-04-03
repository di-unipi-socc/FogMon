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

Node::Node(string ip, string port, int nThreads, bool direct) {
    this->nThreads = nThreads;
    this->running = false;
    this->timerReport = 5;
    this->timeTimerTest = 5;
    this->ipS = ip;
    this->portS = port;
    this->storage = NULL;
    this->connections = NULL;
    this->server = NULL;
    this->direct = direct;
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

    if(!this->direct) {
        //ask the MNodes list and select one MNode with the min latency
        vector<string> MNodes = this->connections->requestMNodes(this->ipS, this->portS);
        MNodes.push_back(this->ipS);
        
        int imin=0;
        unsigned int min = (unsigned int)this->testPing(MNodes[imin]);
        for(int i=1; i<MNodes.size(); i++) {
            unsigned int tmp = (unsigned int)this->testPing(MNodes[imin]);
            if(tmp < min) {
                imin = i;
                min = tmp;
            }
        }
        this->ipS = MNodes[imin];
    }
    printf("ciao");
    this->getHardware();
    if(!this->connections->sendHello(this->ipS, this->portS)) {
        perror("Cannot connect to the main node");
        this->stop();
        exit(0);
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
        char command[256];
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
        std::cout << "Thread finished" << std::endl;
    } else {
        std::cout << "Thread still running" << std::endl;
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

        cout << "bps " << val << " kbps " << val /1000 << endl;
        //this->storage->saveBandwidthTest(ip, val/1000, 0);
        return val/1000;
    }
    return -1;
}

float Node::testBandwidthEstimate(string ip, int port) {
    char command[256];
    if(port > 0) {
        sprintf(command, "./assolo_run -R %s -S %s -J 2 -t 30 -u 100 -l 1 -U %d 2>&1", ip.c_str(), port);
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
        

        return -1;
    }
    return -1;
}

int Node::testPing(string ip) {
    char command[256];
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
            cout<< m[1]<< endl;
            std::cout << std::endl;
            output = m.suffix().str();
            //this->storage->saveLatencyTest(ip, stoi(m[1]));
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
    hardware.free_cpu = ((float)diffIdle)/(totaldiff);
    hardware.memory = mem.total;
    hardware.free_memory = mem.actual_free;
    hardware.disk = disk.total;
    hardware.free_disk = disk.avail;

    this->storage->saveHardware(hardware);

    sigar_cpu_list_destroy(sigar, &cpulist);

    sigar_close(sigar);
}

void Node::timer() {
    while(this->running) {
        //generate hardware report and send it
        this->getHardware();

        this->connections->sendUpdate(this->ipS,this->portS);

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
        free(things[i]);
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
        vector<string> ips = this->storage->getLRLatency(100,30); //param: batch latency dimension

        for(auto ip : ips) {
            if(myIp == ip)
                continue;
            int val = this->testPing(ip);
            if(val >= 0) {
                this->storage->saveLatencyTest(ip, val);
            }
        }
        /*
        int m = this->storage->hasToken();
        if(m > 0) {
            cout << "start test bandwidth:" << endl;
            ips = this->storage->getLRBandwidth(m+1,300);
            cout << "dimension: "<<ips.size() << endl;
            int durationTest = 1;
            //if token then do the same for bandwidth
            int i=0;
            while(this->storage->hasToken() >= durationTest && i<ips.size()) {
                //send open iperf3
                cout << "testing: " << ips[i];
                if(myIp == ips[i]) {
                    i++;
                    continue;
                }
                int port = this->connections->sendStartIperfTest(ips[i]);
                if(port != -1) {
                    cout << "stat test"<<endl;
                    float val = this->testBandwidthIperf(ips[i], port);
                    if(val >= 0)
                        this->storage->saveBandwidthTest(ips[i], val, 0);
                }
                i++;
            }
        }*/
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
                result = this->testBandwidthIperf(ip);
                if(result >= 0)
                    state = 1;
            }
        break;
        case 1: //a test is already done
            port = this->connections->sendStartEstimateTest(ip);
            if(port != -1) {
                result = this->testBandwidthEstimate(ip);
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
                result = this->testBandwidthEstimate(ip);
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
                result = this->testBandwidthIperf(ip);
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