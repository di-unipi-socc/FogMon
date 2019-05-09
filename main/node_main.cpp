#include <iostream>
#include "inputParser.hpp"
#include "node.hpp"

#include <stdlib.h>
#include <sys/time.h>

#include <unistd.h>
#include "message.hpp"

using namespace std;

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;

#include "readproc.hpp"

int main(int argc, char *argv[]) {
    char *args[] = {"/bin/iperf3","-s", NULL };
    ReadProc proc(args);
sleep(5);
    cout << proc.readoutput();
    proc.killproc();
    cout << proc.readoutput();

    return 0;
    InputParser input(argc,argv);

    string ip = "localhost";
    int threads = 2;
    int time_report = 30;
     int time_tests = 30;
    int time_latency = 30;
    int max_latency = 100;
    int time_bandwidth = 600;
    int max_bandwidth=1;

    if(input.cmdOptionExists("-C"))
        ip = input.getCmdOption("-C");

    if(input.cmdOptionExists("-t"))
        threads = stoi(input.getCmdOption("-t"));
    
    if(input.cmdOptionExists("--time-report"))
        time_report = stoi(input.getCmdOption("--time-report"));

    if(input.cmdOptionExists("--time-tests"))
        time_tests = stoi(input.getCmdOption("--time-tests"));
    
    if(input.cmdOptionExists("--time-latency"))
        time_latency = stoi(input.getCmdOption("--time-latency"));
    
    if(input.cmdOptionExists("--max-per-latency"))
        max_latency = stoi(input.getCmdOption("--max-per-latency"));
    
    if(input.cmdOptionExists("--time-bandwidth"))
        time_bandwidth = stoi(input.getCmdOption("--time-bandwidth"));
    
    if(input.cmdOptionExists("--max-per-bandwidth"))
        time_bandwidth = stoi(input.getCmdOption("--max-per-bandwidth"));


    Node node(ip, threads);

    node.setParam(string("time-report"), time_report);
    node.setParam(string("time-tests"), time_tests);
    node.setParam(string("time-latency"), time_latency);
    node.setParam(string("max-per-latency"), max_latency);
    node.setParam(string("time-bandwidth"), time_bandwidth);
    node.setParam(string("max-per-bandwidth"), max_bandwidth);

    node.initialize();
    node.start();

    int a;
    scanf("%d",&a);

    node.stop();

    return 0;
}
