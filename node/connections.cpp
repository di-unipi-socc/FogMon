#include "connections.hpp"

#include "node.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/un.h>
#include <string>
#include <netdb.h>
#include <iostream>

using namespace std;

Connections::Connections(Node *parent, int nThread) : IConnections((INode*)parent, nThread), storage("node.db") {
    this->parent = parent;
}

Connections::~Connections() {
}

Storage* Connections::getStorage() {
    return &(this->storage);
}

void Connections::handler(int fd, Message &m) {

    if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::IPERF) {
            if(m.getCommand() == Message::Command::START) {
                
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::START);
                int port = this->parent->startIperf();
                if(port > 0) {
                    res.setArgument(Message::Argument::POSITIVE);
                    res.setData(port);
                }else {
                    res.setArgument(Message::Argument::NEGATIVE);
                }
                //send response
                if(this->sendMessage(fd, res)) {
                    
                }
            }
        }else if(m.getArgument() == Message::Argument::TOKEN) {
            if(m.getCommand() == Message::Command::SET) {
                
                //send reponse
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::SET);

                int d;
                if(m.getData(d)) {
                    this->storage.setToken(d);
                    res.setArgument(Message::Argument::POSITIVE);
                }else {
                    res.setArgument(Message::Argument::NEGATIVE);
                }

                if(this->sendMessage(fd, res)) {
                    
                }
            }
        }else if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                //build array of nodes
                vector<string> nodes = storage.getNodes();

                //send nodes
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::GET);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(nodes);
            
                if(this->sendMessage(fd, res)) {
                    
                }
            }else if(m.getCommand() == Message::Command::SET) {
                //refresh all the nodes with the array of nodes
                vector<string> ips;
                if(!m.getData(ips))
                    return;

                //ips now contains the ip of the nodes
                storage.refreshNodes(ips);
            }
        }else if(m.getArgument() == Message::Argument::REPORT) {
            if(m.getCommand() == Message::Command::GET) {
                //build report
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::GET);
                res.setArgument(Message::Argument::POSITIVE);
                Report r;
                
                r.setHardware(storage.getHardware());
                r.setLatency(storage.getLatency());
                r.setBandwidth(storage.getBandwidth());
                res.setData(r);

                //send report
                if(this->sendMessage(fd, res)) {
                    
                }
                
            }
        }
    }else if(m.getType() == Message::Type::NOTIFY) {
        if(m.getCommand() == Message::Command::UPDATE) {
            if(m.getArgument() == Message::Argument::NODES) {
                //data contains 2 array: new and deleted nodes
                vector<string> ipsNew;
                vector<string> ipsRem;
                if(!m.getData(ipsNew,ipsRem))
                    return;
                //update the nodes
                storage.updateNodes(ipsNew,ipsRem);
            }
        }
    }   
}

bool Connections::sendHello(string ipS, string portS) {
    int Socket = openConnection(ipS, portS);
    
    if(Socket < 0) {
        return false;
    }

    printf("ready");
    fflush(stdout);
    char buffer[10];

    //build hello message
    Message m;
    m.setType(Message::Type::NOTIFY);
    m.setCommand(Message::Command::HELLO);
    Report r;
    
    r.setHardware(storage.getHardware());
    m.setData(r);

    bool result = false;

    //send hello message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::RESPONSE &&
                res.getCommand() == Message::Command::HELLO &&
                res.getArgument() == Message::Argument::POSITIVE) {
                string ip;
                vector<string> vec;
                if(res.getData(ip, vec)) {
                    cout << ip << endl;
                    this->parent->setMyIp(ip);
                    storage.refreshNodes(vec);
                    result = true;
                }
            }
        }
    }
    close(Socket);
    return result;
}

bool Connections::sendUpdate(string ipS, string portS) {
    int Socket = openConnection(ipS, portS);
    
    if(Socket < 0) {
        return false;
    }

    printf("ready");
    fflush(stdout);
    char buffer[10];

    //build update message
    Message m;
    m.setType(Message::Type::NOTIFY);
    m.setCommand(Message::Command::UPDATE);
    m.setArgument(Message::Argument::REPORT);
    Report r;
    
    r.setHardware(storage.getHardware());
    r.setLatency(storage.getLatency());
    r.setBandwidth(storage.getBandwidth());
    m.setData(r);

    bool result = false;

    //send update message
    if(this->sendMessage(Socket, m)) {
        result = true;
    }
    close(Socket);
    return result;
}

int Connections::sendStartBandwidthTest(string ip) {
    int Socket = openConnection(ip, to_string(this->parent->getServer()->getPort()));
    
    if(Socket < 0) {
        return -1;
    }

    printf("ready");
    fflush(stdout);
    char buffer[10];

    //build update message
    Message m;
    m.setType(Message::Type::REQUEST);
    m.setCommand(Message::Command::START);
    m.setArgument(Message::Argument::IPERF);

    int port = -1;

    //send update message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::RESPONSE &&
                res.getCommand() == Message::Command::START &&
                res.getArgument() == Message::Argument::POSITIVE) {
                
                m.getData(port);
            }
        }
    }
    close(Socket);
    return port;
}