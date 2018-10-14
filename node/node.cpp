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

using namespace std;

Node::Node(int nThreads) : server(this,5555), storage(), connections(this, nThreads) {
    running = false;
    timerReport = 0;
    timerPing = 0;
    timerbandwidth = 0;
    lastReport = 0;
    lastPing = 0;
    lastBandwidth = 0;
    ipS = "localhost:5556";
}

Node::~Node() {
    this->stop();
}

void Node::start() {
    this->server.start();
    this->testPing();
    this->getHardware();
    if(!this->connections.sendHello(this->ipS)) {
        perror("Cannot connect to the main node");
        this->stop();
    }
}

void Node::stop() {
    this->running = false;
    if(this->timerThread.joinable())
    {
        this->timerThread.join();
    }
    this->server.stop();
}

void Node::report() {
    connections.sendReport(this->ipS);
}

Storage* Node::getStorage() {
    return &(this->storage);
}

IConnections* Node::getConnections() {
    return (IConnections*)(&(this->connections));
}

void Node::testBandwidth() {
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

void Node::testPing() {
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

void Node::getHardware() {

    int status, i;
    sigar_t *sigar;
    sigar_cpu_list_t cpulist;

    sigar_open(&sigar);
    sigar_file_system_list_t fsystem;
    sigar_file_system_usage_t disk;
    sigar_file_system_list_get(sigar, &fsystem);
    for(int i =0; i<fsystem.number; i++){
        sigar_file_system_usage_get(sigar,fsystem.data[i].dev_name,&disk);
        cout << "disk " << fsystem.data[i].dev_name << " " << disk.total << " " << disk.avail << " "  << disk.free << endl;
    }
    sigar_file_system_usage_get(sigar,"/",&disk);
    cout << "disk " << "/ " << disk.total << " " << disk.avail << " "  << disk.free << endl;

    status = sigar_cpu_list_get(sigar, &cpulist);

    if (status != SIGAR_OK) {
        printf("cpu_list error: %d (%s)\n",
               status, sigar_strerror(sigar, status));
        exit(1);
    }

    for (i=0; i<cpulist.number; i++) {
        sigar_cpu_t cpu = cpulist.data[i];
        
        cout << "cpu " << i << " " << cpu.idle << endl;
    }
    sigar_mem_t mem;
    sigar_mem_get(sigar,&mem);

    cout << "mem " << mem.actual_free << " " << mem.actual_used << " " << mem.free << " " << mem.free_percent << " " << mem.ram << " " << mem.total << " " << mem.used << " " << mem.used_percent << endl;

    sigar_cpu_list_destroy(sigar, &cpulist);

    sigar_close(sigar);





    stringstream   strStream;
    unsigned long  hdd_size;
    unsigned long  hdd_free;
    ostringstream  strConvert;

    struct sysinfo info;
    sysinfo( &info );   

    struct statvfs fsinfo;
    statvfs("/", &fsinfo);

    unsigned num_cpu = std::thread::hardware_concurrency();

    strStream.str("");
    ifstream stat("/proc/stat");
    strStream << stat.rdbuf();
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;


    std::regex reg("cpu\\s*([0-9]*) ([0-9]*) ([0-9]*) ([0-9]*) [\\s\\S]*");

    std::smatch m;
    string statString = strStream.str();
    std::regex_match (statString,m,reg);
    
    
    if(m.size() == 5) {
        totalUser = stoll(m[1]);
        totalUserLow = stoll(m[2]);
        totalSys = stoll(m[3]);
        totalIdle = stoll(m[4]);
    }
    sleep(1);

    strStream.str("");
    stat.close();
    stat.open("/proc/stat");
    strStream << stat.rdbuf();
    unsigned long long totalUser2, totalUserLow2, totalSys2, totalIdle2;

    statString = strStream.str();
    std::regex_match (statString,m,reg);

    if(m.size() == 5) {
        totalUser2 = stoll(m[1]);
        totalUserLow2 = stoll(m[2]);
        totalSys2 = stoll(m[3]);
        totalIdle2 = stoll(m[4]);
    }

    total = totalUser2-totalUser + totalUserLow2-totalUserLow + totalSys2-totalSys;
    unsigned long long diffIdle = totalIdle2-totalIdle;

    unsigned long   mem_size = (size_t)info.totalram * (size_t)info.mem_unit;
    unsigned long   mem_free = (size_t)info.freeram * (size_t)info.mem_unit;

    hdd_size = fsinfo.f_frsize * fsinfo.f_blocks;
    hdd_free = fsinfo.f_bsize * fsinfo.f_bavail;  
                                            
    std::cout << "CPU core number           = " << num_cpu        << endl;
    std::cout << "CPU free                  = " << (double)diffIdle/(double)(total+diffIdle) << endl;
    std::cout << "CPU used                  = " << (double)total/(double)(total+diffIdle) << endl;

    std::cout << "Memory size               = " << mem_size       << endl;
    std::cout << "Memory free               = " << mem_free       << endl;

    std::cout << "Disk, filesystem size     = " << hdd_size       << endl;
    std::cout << "Disk free space           = " << hdd_free       << endl;
}

void Node::timer() {

}