#ifndef FOLLOWER_HPP_
#define FOLLOWER_HPP_

#include "iagent.hpp"
#include "follower_connections.hpp"
#include "server.hpp"
#include "factory.hpp"
#include "iiotdiscoverer.hpp"
#include "readproc.hpp"

class Follower : virtual public IAgent {
public:
    Follower(Message::node node, int nThreads);
    ~Follower();

    bool setParam(std::string name, int value);

    virtual void initialize(Factory* factory = NULL);

    //start listener for incoming ping and directions
    virtual void start(vector<Message::node> mNodes);
    //stop everything
    virtual void stop();

    virtual IConnections* getConnections();
    virtual IStorage* getStorage();

    virtual void setMyId(std::string id);
    virtual Message::node getMyNode();

    //start iperf command line server and return the port it is open in
    virtual int getIperfPort();
    //start estimate tool (assolo) server and return the port it is open in
    virtual int getEstimatePort();

    virtual Server* getServer();

    virtual void changeRole(std::vector<Message::node> leaders);

protected:

    //id and port saved here
    Message::node myNode;

    //remember when the last update happened and the old server
    //so that new updates can be selective on new information only
    std::pair<int64_t,Message::node> update;

    //ports for tests
    int portIperf;
    int portAssolo;

    //processes for tests
    ReadProc *pIperf;
    ReadProc *pAssoloRcv;
    ReadProc *pAssoloSnd;
    ReadProc *pTest;
    std::mutex mTest;

    //threads
    std::thread timerThread;
    std::thread timerTestThread;

    Server* server;

    FollowerConnections * connections;
    IStorage* storage;

    Factory tFactory;
    Factory * factory;

    int nThreads;


    //start iperf command line server
    virtual int startIperf();
    //start estimate tool (assolo) server
    virtual int startEstimate();

    //timer for heartbeat
    void timer();

    //timer for latency and bandwidth tests
    void TestTimer();

    void testIoT();

    //test the bandwidth with another ip
    float testBandwidth(Message::node node, float old, int &state);

    //test the bandwidth using iperf3
    float testBandwidthIperf(std::string ip, int port = -1);
    //test the bandwidth using an estimation tool (assolo)
    float testBandwidthEstimate(std::string ip, std::string myIp, float old);

    //test the latency with another node
    int testPing(std::string ip);

    //from ipS and mNodes select the closest server
    bool selectServer(vector<Message::node> mNodes);

    //check if the best leader is selected
    bool checkServer(vector<Message::node> mNodes);

    //get the hardware of this node
    void getHardware();
};

#endif















































































