#include "node.hpp"
#include <sys/statvfs.h>
#include <sys/sysinfo.h>

#include <sstream>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>

#include <unistd.h>

#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <sys/eventfd.h>

#include <sigar.h>

#include "storage.hpp"

using namespace std;

Node::Node(string ip, int nThreads) : server(this,5555), connections(this, nThreads) {
    running = false;
    timerReport = 5;
    timeTimerTest = 5;
    ipS = ip;
}

Node::~Node() {
    this->stop();
}

void Node::start() {
    this->running = true;
    this->server.start();
    this->testPing("192.168.1.1");
    this->getHardware();
    if(!this->connections.sendHello(this->ipS)) {
        perror("Cannot connect to the main node");
        this->stop();
    }
    this->timerThread = thread(&Node::timer, this);
}

void Node::stop() {
    this->running = false;
    if(this->timerThread.joinable())
    {
        this->timerThread.join();
    }
    this->server.stop();
}

IConnections* Node::getConnections() {
    return (IConnections*)(&(this->connections));
}

void Node::testBandwidth(string ip) {
    string command = "ping -c 3 192.168.1.1 2>&1";
    string mode = "r";
    string output;

    std::stringstream sout;

    // Run Popen
    FILE *in;
    char buff[512];

    // Test output
    if(!(in = popen(command.c_str(), mode.c_str()))){
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
            cout<< m[1]<< endl;
            std::cout << std::endl;
            output = m.suffix().str();
        }
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

        this->connections.sendUpdate(this->ipS);

        sleep(this->timerReport);
    }
}

void Node::TestTimer() {
    while(this->running) {
        //get list ordered by time for the latency tests
        //test the least recent
        vector<string> ips = this->connections.getStorage()->getLRLatency(10); //param: batch latency dimension

        for(auto ip : ips) {
            this->testPing(ip);
        }

        //if token then do the same for bandwidth

        sleep(this->timeTimerTest);
    }
}