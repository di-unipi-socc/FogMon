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

#include "storage.hpp"

using namespace std;
using namespace rapidjson;

Node::Node(string ip, string port, int nThreads) : server(this,5555), connections(this, nThreads) {
    running = false;
    timerReport = 5;
    timeTimerTest = 5;
    ipS = ip;
    portS = port;
}

Node::~Node() {
    this->stop();
}

void Node::start() {
    this->running = true;
    this->server.start();
    srandom(time(nullptr));
    this->getHardware();
    if(!this->connections.sendHello(this->ipS,this->portS)) {
        perror("Cannot connect to the main node");
        this->stop();
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
    this->server.stop();
}

IConnections* Node::getConnections() {
    return (IConnections*)(&(this->connections));
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

void Node::testBandwidth(string ip, int port) {
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
        return;
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
            return;
        
        if( !doc.HasMember("end") && !doc["end"].IsObject() &&
            !doc["end"].HasMember("sum_received") && !doc["end"]["sum_received"].IsObject() &&
            !doc["end"]["sum_received"].HasMember("bits_per_second") && !doc["end"]["sum_received"]["bits_per_second"].IsFloat())
            return;

        float val = doc["end"]["sum_received"]["bits_per_second"].GetFloat();

        cout << "bps " << val << " kbps " << val /1000 << endl;
        this->connections.getStorage()->saveBandwidthTest(ip, val/1000);
    }
}

void Node::testPing(string ip) {
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
        return;
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
            this->connections.getStorage()->saveLatencyTest(ip, stoi(m[1]));
            cout<< m[1]<< endl;
            std::cout << std::endl;
            output = m.suffix().str();
        }
    }
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

    connections.getStorage()->saveHardware(hardware);

    sigar_cpu_list_destroy(sigar, &cpulist);

    sigar_close(sigar);
}

void Node::timer() {
    while(this->running) {
        //generate hardware report and send it
        this->getHardware();

        this->connections.sendUpdate(this->ipS,this->portS);

        sleep(this->timerReport);
    }
}

void Node::TestTimer() {
    while(this->running) {
        //get list ordered by time for the latency tests
        //test the least recent
        vector<string> ips = this->connections.getStorage()->getLRLatency(100,30); //param: batch latency dimension

        for(auto ip : ips) {
            if(myIp == ip)
                continue;
            this->testPing(ip);
        }
        int m = this->connections.getStorage()->hasToken();
        if(m > 0) {
            cout << "start test bandwidth:" << endl;
            ips = this->connections.getStorage()->getLRBandwidth(m+1,300);
            cout << "dimension: "<<ips.size() << endl;
            int durationTest = 1;
            //if token then do the same for bandwidth
            int i=0;
            while(this->connections.getStorage()->hasToken() >= durationTest && i<ips.size()) {
                //send open iperf3
                cout << "testing: " << ips[i];
                if(myIp == ips[i]) {
                    i++;
                    continue;
                }
                int port = this->connections.sendStartBandwidthTest(ips[i]);
                if(port != -1) {
                    cout << "stat test"<<endl;
                    this->testBandwidth(ips[i], port);
                }
                i++;
            }
        }
        sleep(this->timeTimerTest);
    }
}

void Node::setMyIp(std::string ip) {
    this->myIp = ip;
}

std::string Node::getMyIp() {
    return this->myIp;
}

Server* Node::getServer() {
    return &this->server;
}