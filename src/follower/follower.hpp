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
    virtual ~Follower() override;

    bool setParam(std::string name, int value) override;

    virtual void initialize(Factory* factory = NULL);

    //start listener for incoming ping and directions
    virtual void start(vector<Message::node> mNodes) override;
    //stop everything
    virtual void stop() override;

    virtual IConnections* getConnections() override;
    virtual IStorage* getStorage() override;

    virtual void setMyId(std::string id) override;
    virtual Message::node getMyNode() override;

    //start iperf command line server and return the port it is open in
    virtual int getIperfPort() override;
    //start estimate tool (assolo) server and return the port it is open in
    virtual int getEstimatePort() override;

    virtual Server* getServer() override;

    virtual void changeRole(std::vector<Message::node> leaders) override;

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
    virtual int startIperf() override;
    //start estimate tool (assolo) server
    virtual int startEstimate() override;

    //timer for heartbeat
    void timer();

    //timer for latency and bandwidth tests
    void TestTimer();

    void testIoT();

    //test the bandwidth with another ip
    float testBandwidth(Message::node node, float old, int &state);

    //test the bandwidth using iperf3
    virtual float testBandwidthIperf(std::string ip, int port = -1);
    //test the bandwidth using an estimation tool (assolo)
    virtual float testBandwidthEstimate(std::string ip, std::string myIp, float old);

    //test the latency with another node
    virtual int testPing(std::string ip);

    //from ipS and mNodes select the closest server
    bool selectServer(vector<Message::node> mNodes);

    //check if the best leader is selected
    bool checkServer(vector<Message::node> mNodes);

    //get the hardware of this node
    void getHardware();
};

#endif















































































