#include <iostream>
#include "inputParser.hpp"
#include "follower.hpp"

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
    
    if((argc-1) % 2 != 0 || argc < 3) {
        cout << "Usage: ./program [OPTIONS]... remote_ip remote_port" << endl;
        return 0;
    }
    InputParser input(argc-2,argv);

    string ipR = string(argv[argc-2]);
    string portR = string(argv[argc-1]);
    string myPort = "5555";
    int threads = 2;
    int time_report = 30;
     int time_tests = 30;
    int time_latency = 30;
    int max_latency = 100;
    int time_bandwidth = 600;
    int max_bandwidth=1;

    if(input.cmdOptionExists("--my-port"))
        myPort = input.getCmdOption("--my-port");

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

    Follower node(Message::node("","::1",myPort), threads);

    node.setParam(string("time-report"), time_report);
    node.setParam(string("time-tests"), time_tests);
    node.setParam(string("time-latency"), time_latency);
    node.setParam(string("max-per-latency"), max_latency);
    node.setParam(string("time-bandwidth"), time_bandwidth);
    node.setParam(string("max-per-bandwidth"), max_bandwidth);

    node.initialize();
    vector<Message::node> vec;
    vec.push_back(Message::node("",ipR,portR));
    node.start(vec);

    int a;
    scanf("%d",&a);

    node.stop();

    return 0;
}
