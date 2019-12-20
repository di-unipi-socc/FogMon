#include "node.hpp"
#include "leader.hpp"
#include "follower.hpp"

#include <iostream>
#include <unistd.h>

using namespace std;

Node::Node(string port, bool isLeader, int threads) {
    this->isLeader = isLeader;
    this->agent = NULL;
    this->port = port;
    this->threads = threads;
    this->id = this->genId();

    cout << "Generated id: "<< this->id << endl;

    unlink("leader_node.db");

    this->create();

}

Node::~Node() {
    this->stop();
    try{
        if(this->agent)
            delete this->agent;
        this->agent = NULL;
    }catch(...) {}
}

void Node::start() {
    this->agent->start(mNodes);
}

void Node::stop() {
    this->agent->stop();
}

void Node::create() {
    if(isLeader) {
        cout << "Starting Leader" << endl;
        Leader * agent1 = new Leader(Message::node(this->id,"::1",this->port), this->threads);
        agent1->initialize();
        this->agent = agent1;
    }else {
        cout << "Starting Follower" << endl;
        Follower * agent1 = new Follower(Message::node(this->id,"::1",this->port), this->threads);
        agent1->initialize();
        this->agent = agent1;
    }
    this->agent->setParent(this);
}

void Node::promote() {
    if(!isLeader) {

        isLeader = true;

        if(this->restartThread.joinable())
            this->restartThread.join();

        this->restartThread = thread(&Node::restart, this);
    }
}

void Node::demote() {
    if(isLeader) {
        
        isLeader = false;

        if(this->restartThread.joinable())
            this->restartThread.join();
            
        this->restartThread = thread(&Node::restart, this);
    }
}

void Node::restart() {
    this->stop();
    this->create();
    this->start();
}

void Node::setMNodes(std::vector<Message::node> nodes) {
    this->mNodes = nodes;
}

std::vector<Message::node> Node::getMNodes() {
    return this->mNodes;
}

bool Node::isFollower() {
    return !this->isLeader;
}

extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif
}

std::string newUUID()
{
#ifdef WIN32
    UUID uuid;
    UuidCreate ( &uuid );

    unsigned char * str;
    UuidToStringA ( &uuid, &str );

    std::string s( ( char* ) str );

    RpcStringFreeA ( &str );
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
    char s[37];
    uuid_unparse ( uuid, s );
#endif
    return s;
}

string Node::genId() {
    return newUUID();
}

bool Node::setParam(std::string name, int value) {
    if(this->agent->setParam(name,value))
        return true;
    return false;
}