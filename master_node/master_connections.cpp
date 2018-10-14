
#include "master_connections.hpp"
#include "inode.hpp"

MasterConnections::MasterConnections(INode *parent, int nThread) : IConnections(parent, nThread) {
}

MasterConnections::~MasterConnections() {
}

void MasterConnections::handler(int fd, Message &m) {

    if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                //build array of nodes
                vector<string> nodes = this->parent->getStorage()->getNodes();
                //send nodes

            }
        }else if(m.getArgument() == Message::Argument::REPORT) {
            if(m.getCommand() == Message::Command::SET) {
                //read report

                //save report
            }
        }
    }else if(m.getType() == Message::Type::NOTIFY) {
        if(m.getCommand() == Message::Command::HELLO) {
            
        }else if(m.getCommand() == Message::Command::UPDATE) {
            if(m.getArgument() == Message::Argument::REPORT) {
                //get the report
                //the report should be only a part of it
            }
        }
    }
    
}