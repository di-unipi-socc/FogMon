
#include "connections.hpp"

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

using namespace std;

Connections::Connections(INode *parent, int nThread) : IConnections(parent, nThread), storage("node.db") {
}

Connections::~Connections() {
}

Storage* Connections::getStorage() {
    return &(this->storage);
}

void Connections::handler(int fd, Message &m) {

    if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                //build array of nodes
                vector<string> nodes = this->getStorage()->getNodes();
                //send nodes

            }else if(m.getCommand() == Message::Command::SET) {
                //refresh all the nodes with the array of nodes
                vector<string> ips;
                if(!m.getData(ips))
                    return;

                //ips now contains the ip of the nodes
                this->getStorage()->refreshNodes(ips);
            }
        }else if(m.getArgument() == Message::Argument::REPORT) {
            if(m.getCommand() == Message::Command::GET) {
                //build report
                this->getStorage()->generateReport();
                //send report
                
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
                this->getStorage()->updateNodes(ipsNew,ipsRem);
            }
        }
    }   
}

bool Connections::sendHello(string ipS) {
    int Socket = openConnection(ipS);
    
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
    
    bool result = false;

    //send hello message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::REQUEST &&
                res.getCommand() == Message::Command::SET &&
                res.getArgument() == Message::Argument::NODES) {
                
                vector<string> vec;
                if(res.getData(vec)) {
                    this->getStorage()->refreshNodes(vec);
                    result = true;
                }
            }
        }
    }
    close(Socket);
    return result;
}

bool Connections::sendReport(string ipS) {
    int Socket = openConnection(ipS);

    if(Socket < 0) {
        return false;
    }

    //build report
    //send report

    close(Socket);
    return true;
}