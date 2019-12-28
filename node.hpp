#ifndef NODE_HPP_
#define NODE_HPP_



#include <string>

#include <thread>
#include "message.hpp"
class IAgent;


class Node {
public:

    Node(std::string port, bool isLeader, int threads);
    ~Node();
    //start listener for incoming ping and directions
    void start();
    //stop everything
    void stop();

    bool setParam(std::string name, int value);

    //promote to leader if follower
    virtual void promote();

    //demote to follower if leader
    void demote();

    void setMNodes(std::vector<Message::node> nodes);
    std::vector<Message::node> getMNodes();

    bool isFollower();

    //configs
    int timeReport;
    int timeTests;
    int timeLatency;
    int timeBandwidth;
    int maxPerLatency;
    int maxPerBandwidth;
    int leaderCheck;

    int timeheartbeat;
    int timePropagation;

protected:
    IAgent * agent;
    bool isLeader;

    //list of known leaders for future reconnections
    std::vector<Message::node> mNodes;

    //thread to promote/demote
    std::thread restartThread;

    std::string id;

    int threads;
    std::string port;

    void restart();

    void create();

    static std::string genId();
};

#endif