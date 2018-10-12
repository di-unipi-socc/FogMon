
#include "connections.hpp"
#include "elf.hpp"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

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

Elf::Elf(int nThreads) : connections(this,nThreads), storage() {
    timerReport = 0;
    timerPing = 0;
    timerbandwidth = 0;
    lastReport = 0;
    lastPing = 0;
    lastBandwidth = 0;
    ipS = "";
    portC = 12345;
    efd = eventfd(0,0);
    numThreads = nThreads;
}

Elf::~Elf() {

}

void Elf::start() {
    this->listenerThread = thread(&Elf::listener, this);
    this->connections.start();
    this->testPing();
    this->getHardware();
}

void Elf::stop() {
    this->running = false;
    eventfd_write(efd,1);
    this->connections.stop();
    //signal to poll to terminate
    if(this->timerThread.joinable())
    {
        this->timerThread.join();
    }

    if(this->listenerThread.joinable())
    {
        this->listenerThread.join();
    }
}

void Elf::report() {

}

void Elf::testBandwidth() {
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

void Elf::testPing() {
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

void Elf::getHardware() {

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

void Elf::timer() {

}

void Elf::listener() {
    int error, on = 1;
    int listen_sd = -1, new_sd = -1;
    int compress_array = false;
    struct sockaddr_in6   addr;
    int timeout;
    struct pollfd fds[200]; //TODO: max connection dynamic
    int nfds = 0, current_size = 0, i, j;
    this->running = true;
    //create an AF_INET6 stream socket for listening
    listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket() failed");
        exit(-1);
    }

    //set reusable
    error = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (error < 0)
    {
        perror("setsockopt() failed");
        close(listen_sd);
        exit(-1);
    }

    //set nonblocking
    error = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (error < 0)
    {
        perror("ioctl() failed");
        close(listen_sd);
        exit(-1);
    }

    //bind
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(this->portC);

    error = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (error < 0)
    {
        perror("bind() failed");
        close(listen_sd);
        exit(-1);
    }

    //set queue size for listening
    error = listen(listen_sd, 32);
    if (error < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        exit(-1);
    }

    //timeout of poll
    timeout = (3 * 60 * 1000);

    //initializate poll structure
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = this->efd;
    fds[0].events = POLLIN;
    nfds++;

    fds[1].fd = listen_sd;
    fds[1].events = POLLIN;
    nfds++;

    do
    {
        //printf("Waiting on poll()...\n");
        error = poll(fds, nfds, timeout);

        if (error < 0)
        {
            perror("  poll() failed");
            break;
        }
        if (error == 0)
        {
            //timeout continue
            continue;
        }
        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {
            if(fds[i].revents == 0)
                continue;
            
            if(fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
                running = false;
                break;
            }

            if (fds[i].fd == efd)
            {
                continue;
            }
            else if (fds[i].fd == listen_sd)
            {
                //accept new connections
                //printf("  Listening socket is readable\n");
                do
                {
                    new_sd = accept(listen_sd, NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            running = false;
                        }
                        break;
                    }

                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                } while (new_sd != -1);
            }
            else
            {
                //read incoming packets
                //printf("  Descriptor %d is readable\n", fds[i].fd);

                this->connections.request(fds[i].fd);

                fds[i].fd = -1;
                fds[i].events = 0;
                compress_array = true;
            }
        }
        if (compress_array)
        {
            compress_array = false;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                        fds[j].events = fds[j+1].events;
                    }
                    i--;
                    nfds--;
                }
            }
        }
    }while(this->running);

    //cleanup sockets
    for (i = 0; i < nfds; i++)
    {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
    this->running = false;
}