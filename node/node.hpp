#ifndef NODE_HPP_
#define NODE_HPP_

#include "inode.hpp"
#include "connections.hpp"
#include "server.hpp"
#include "factory.hpp"
#include "iiotdiscoverer.hpp"
#include "readproc.hpp"

class Node : virtual public INode {
public:
    Node(std::string ip, int nThreads);
    ~Node();

    bool setParam(std::string name, int value);

    virtual void initialize(Factory* factory = NULL);

    //start listener for incoming ping and directions
    void start();
    //stop everything
    void stop();

    virtual IConnections* getConnections();
    virtual IStorage* getStorage();

    virtual void setMyIp(std::string ip);
    virtual std::string getMyIp();

    //start iperf command line server and return the port it is open in
    virtual int getIperfPort();
    //start estimate tool (assolo) server and return the port it is open in
    virtual int getEstimatePort();

    virtual Server* getServer();
protected:

    std::vector<std::string> mNodes;

    //ip to differentiate between other nodes in the list and self
    std::string myIp;

    int timeReport;
    int timeTests;
    int timeLatency;
    int timeBandwidth;
    int maxPerLatency;
    int maxPerBandwidth;

    int portIperf;
    int portAssolo;
    ReadProc *pIperf;
    ReadProc *pAssoloRcv;
    ReadProc *pAssoloSnd;

    ReadProc *pTest;
    std::mutex mTest;

    bool running;

    std::thread timerThread;
    std::thread timerTestThread;
    Server* server;

    Connections * connections;
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
    float testBandwidth(std::string ip, float old, int &state);

    //test the bandwidth using iperf3
    float testBandwidthIperf(std::string ip, int port = -1);
    //test the bandwidth using an estimation tool (assolo)
    float testBandwidthEstimate(std::string ip, int port = -1);

    //test the latency with another node
    int testPing(std::string ip);

    //from ipS and mNodes select the closest server
    bool selectServer();

    //get the hardware of this node
    void getHardware();
};

#endif















































































