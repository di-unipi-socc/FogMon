

#include "inputParser.hpp"
#include "leader.hpp"

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


L1-0000000000000001|131.114.3.77|5555|4|0.970996|0.00140436|971063296|728149000.0|730112000000.0|14943708|5749280.0|20.7473|2019-05-31 05:26:17|L1
L1-0000000000000004|131.114.2.66|5555|4|0.93777|0.00235206|971063296|459651000.0|831497000000.0|14943708|5358150.0|1.23077|2019-05-31 05:26:17|L1
L1-0000000000000002|151.15.218.15|5555|4|0.985093|0.000408489|971063296|739633000.0|230640000000.0|14943708|5713330.0|2.9011|2019-05-31 05:26:18|L1
L1-0000000000000005|131.114.2.41|5555|4|0.955168|0.00189386|971063296|729341000.0|913505000000.0|14943708|5752560.0|45.4506|2019-05-31 05:26:18|L1
L1-0000000000000003|95.237.3.57|5555|10|0.830932|0.00370859|9189412864|3374000000.0|674276000000000.0|51343840|32463100.0|2.9011|2019-05-31 05:26:19|L1
L1|131.114.72.76|5555|1|0.757898|0.0289526|3726606336|775358000.0|1333070000000.0|29324176|17077300.0|202.458|2019-05-31 05:26:19|L1
L2-0000000000000002|52.58.99.191|5555|1|0.942722|0.0497109|1031446528|804254000.0|29950000000.0|8376300|4533090.0|101222.0|2019-05-31 05:26:35|L2
L2|52.57.90.19|5555|1|0.965903|0.00136034|1031446528|838607000.0|417573000000.0|8376300|2976080.0|113.642|2019-05-31 05:26:35|L2
L2-0000000000000001|37.116.14.93|5555|4|0.960544|0.00752691|971063296|705050000.0|133085000000.0|14943708|5204270.0|8.21053|2019-05-31 05:26:36|L2
L3|::1|5555|1|0.898271|0.00930774|3588915200|3231910000.0|186738000000000.0|30831524|23826700.0|276.042|2019-05-31 05:26:48|L3
L3-0000000000000002|13.81.62.203|5555|1|0.991089|0.00025695|3582115840|3184390000.0|298553000000.0|30308240|27034000.0|10.7789|2019-05-31 05:26:48|L3
L3-0000000000000001|13.69.122.42|5555|1|0.976221|0.00114333|3582115840|3177290000.0|451487000000.0|30308240|27032700.0|11.7895|2019-05-31 05:26:53|L3

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
    

    string ipR = "";
    string portR = "";
    string myPort = "5555";
    int threads = 2;
    int time_heartbeat = 120;
    int time_propagation = 20;

    if(input.cmdOptionExists("-C"))
        ipR = input.getCmdOption("-C");

    if(input.cmdOptionExists("-P"))
        portR = input.getCmdOption("-P");

    if(input.cmdOptionExists("--my-port"))
        myPort = input.getCmdOption("--my-port");

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


    Leader node(Message::node(name,"::1",myPort), threads);

    node.setParam(string("heartbeat"), time_heartbeat);
    node.setParam(string("time-propagation"), time_propagation);

    node.setParam(string("time-report"), time_report);
    node.setParam(string("time-tests"), time_tests);
    node.setParam(string("time-latency"), time_latency);
    node.setParam(string("max-per-latency"), max_latency);
    node.setParam(string("time-bandwidth"), time_bandwidth);
    node.setParam(string("max-per-bandwidth"), max_bandwidth);

    node.initialize();
    vector<Message::node> vec;
    if(!ipR.empty()) {
        vec.push_back(Message::node("",ipR,portR));
    }

    node.start(vec);

    int a;
    scanf("%d",&a);
    scanf("%d",&a);

    node.stop();
    return 0;
}