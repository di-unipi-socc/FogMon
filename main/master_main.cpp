#include "inputParser.hpp"
#include "master_node.hpp"

#include <unistd.h>


int main(int argc, char *argv[]) {
    InputParser input(argc,argv);

    string ip = "::1";
    int threads = 2;
    int time_heartbeat = 120;
    int time_propagation = 20;

    if(input.cmdOptionExists("-C"))
        ip = input.getCmdOption("-C");

    if(input.cmdOptionExists("-t"))
        threads = stoi(input.getCmdOption("-t"));

    if(input.cmdOptionExists("--heartbeat"))
        time_heartbeat = stoi(input.getCmdOption("--heartbeat"));

    if(input.cmdOptionExists("--time-propagation"))
        time_propagation = stoi(input.getCmdOption("--time-propagation"));

    int time_report = 30;
    int time_tests = 60;
    int time_latency = 30;
    int max_latency = 100;
    int time_bandwidth = 600;
    int max_bandwidth=1;
    
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


    MasterNode node(threads);

    node.setParam(string("heartbeat"), time_heartbeat);
    node.setParam(string("time-propagation"), time_propagation);

    node.setParam(string("time-report"), time_report);
    node.setParam(string("time-tests"), time_tests);
    node.setParam(string("time-latency"), time_latency);
    node.setParam(string("max-per-latency"), max_latency);
    node.setParam(string("time-bandwidth"), time_bandwidth);
    node.setParam(string("max-per-bandwidth"), max_bandwidth);

    node.initialize();
    node.start(ip);

    int a;
    scanf("%d",&a);
    scanf("%d",&a);

    node.stop();
    return 0;
}