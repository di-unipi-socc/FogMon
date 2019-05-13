

#include "inputParser.hpp"
#include "master_node.hpp"

#include <unistd.h>

/*
2|1|0.0|0.0|2019-04-25 19:09:19|362876.625|0.0|2019-04-25 19:08:05
1|2|0.0|0.0|2019-04-25 19:09:19|459420.6875|2101014144.0|2019-04-25 19:08:04
3|1|20.0|0.0|2019-04-25 19:09:17|68123.203125|178906720.0|2019-04-25 19:08:51
1|3|20.0|0.0|2019-04-25 19:09:17|0.0|0.0|2019-03-25 19:07:35
3|2|19.6|0.3|2019-04-25 19:09:19|86748.164062|118201680.0|2019-04-25 19:08:35
2|3|19.6|0.3|2019-04-25 19:09:19|210226.4375|648463040.0|2019-04-25 19:09:07
1|4|0.75|0.25|2019-04-25 19:09:11|0.0|2101014144.0|
2|4|0.0|0.0||0.0|0.0|
3|4|0.0|0.0||0.0|0.0|
4|1|0.75|0.25|2019-04-25 19:09:11|671381.0625|465071616.0|2019-04-25 19:08:48
4|2|0.0|0.0||79060.453125|2101014144.0|
4|3|0.0|0.0||0.0|0.0|
5|4|2.25|0.25|2019-04-25 19:09:13|556968.75|0.0|2019-04-25 19:08:48
4|5|2.25|0.25|2019-04-25 19:09:13|0.0|0.0|2019-03-25 19:09:19
5|3|0.0|0.0||0.0|0.0|
3|5|0.0|0.0||0.0|0.0|2019-03-25 19:09:19
5|2|0.0|0.0||0.0|2101014144.0|
2|5|0.0|0.0||0.0|0.0|2019-03-25 19:09:19
5|1|2.0|0.0|2019-04-25 19:09:11|578297.0|571315072.0|2019-04-25 19:08:59
1|5|2.0|0.0|2019-04-25 19:09:11|0.0|0.0|2019-03-25 19:09:19

*/

//sudo docker run -it --net=host server -C 54.93.78.224
//sudo docker run -it --net=host node -C 54.93.78.224

//sudo docker run -it --net=host server --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10
//sudo docker run -it --net=host server -C 54.93.78.224 --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10
//sudo docker run -it --net=host node -C 54.93.78.224 --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10


// --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10
// --time-report 15 --time-tests 30 --time-latency 40 --time-bandwidth 300 --heartbeat 30
// --time-report 30 --time-tests 60 --time-latency 100 --time-bandwidth 600 --heartbeat 60 

int main(int argc, char *argv[]) {
    if(argc % 2 != 0) {
        cout << "Usage: ./program [OPTIONS]... node_name" << endl;
        return 0;
    }
    InputParser input(argc-1,argv);

    string name = string(argv[argc-1]);
    

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
    int time_tests = 30;
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


    MasterNode node(name, threads);

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