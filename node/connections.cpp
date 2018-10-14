
#include "connections.hpp"

using namespace std;

Connections::Connections(INode *parent, int nThread) : IConnections(parent, nThread) {
}

Connections::~Connections() {
}


void Connections::handler(int fd, Message &m) {

    if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                //build array of nodes
                vector<string> nodes = this->parent->getStorage()->getNodes();
                //send nodes

            }else if(m.getCommand() == Message::Command::SET) {
                //refresh all the nodes with the array of nodes
                vector<string> ips;
                if(!m.getData(ips))
                    return;

                //ips now contains the ip of the nodes
                this->parent->getStorage()->refreshNodes(ips);
            }
        }else if(m.getArgument() == Message::Argument::REPORT) {
            if(m.getCommand() == Message::Command::GET) {
                //build report
                this->parent->getStorage()->generateReport();
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
                this->parent->getStorage()->updateNodes(ipsNew,ipsRem);
            }
        }
    }
    
}