

#include "inputParser.hpp"
#include "node.hpp"
#include "message.hpp"

#include <unistd.h>
#include <string>

using namespace std;

//sudo docker run -it --net=host server -C 54.93.78.224
//sudo docker run -it --net=host node -C 54.93.78.224

//sudo docker run -it --net=host server --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10
//sudo docker run -it --net=host server -C 54.93.78.224 --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10
//sudo docker run -it --net=host node -C 54.93.78.224 --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10


// --time-report 5 --time-tests 10 --time-latency 20 --time-bandwidth 60 --heartbeat 10
// --time-report 15 --time-tests 30 --time-latency 40 --time-bandwidth 300 --heartbeat 30
// --time-report 30 --time-tests 60 --time-latency 100 --time-bandwidth 600 --heartbeat 60 

int main(int argc, char *argv[]) {
    cout << argv[1] << endl;
    InputParser input(argc,argv);

    if(input.cmdOptionExists("-h") || input.cmdOptionExists("--help")) {
        cout << "Usage: ./program [OPTIONS]..." << endl<<endl;
        cout << "--leader       for leader" <<endl;
        cout << "-C             ip to connect" <<endl;
        cout << "-P             port to connect" <<endl;
        cout << "--my-port      listening port" <<endl;
        cout <<endl<< "there are other for timing" <<endl;
        return 0;
    }
     

    string ipR = "";
    string portR = "5555";
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
    int leaderCheck = 10;

    bool leader = false;
    
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

    if(input.cmdOptionExists("--leader-check"))
        leaderCheck = stoi(input.getCmdOption("--leader-check"));

    if(input.cmdOptionExists("--leader"))
        leader = true;


    Node node(myPort, leader, threads);

    vector<Message::node> known;

    if(ipR != "")
        known.push_back(Message::node("",ipR,portR));

    node.setMNodes(known);

    node.setParam(string("heartbeat"), time_heartbeat);
    node.setParam(string("time-propagation"), time_propagation);

    node.setParam(string("time-report"), time_report);
    node.setParam(string("time-tests"), time_tests);
    node.setParam(string("time-latency"), time_latency);
    node.setParam(string("max-per-latency"), max_latency);
    node.setParam(string("time-bandwidth"), time_bandwidth);
    node.setParam(string("max-per-bandwidth"), max_bandwidth);
    node.setParam(string("leader-check"), leaderCheck);

    node.start();

    int a = -1;

    do {
        switch(a) {
            case 0:
                node.setParam("start-selection",1);
            break;
            default:
            break;
        }
        scanf("%d",&a);
    }while(a == 0);


    return 0;
}