
#include "master_connections.hpp"
#include "inode.hpp"

MasterConnections::MasterConnections(INode *parent, int nThread) : IConnections(parent, nThread), storage("MasterNode.db") {
}

MasterConnections::~MasterConnections() {
}

MasterStorage* MasterConnections::getStorage() {
    return &(this->storage);
}

void MasterConnections::handler(int fd, Message &m) {

    if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                //build array of nodes
                vector<string> nodes = this->getStorage()->getNodes();
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
            //get nodelist
            Message res;
            res.setType(Message::Type::REQUEST);
            res.setCommand(Message::Command::SET);
            res.setArgument(Message::Argument::NODES);
            vector<string> vec;
            vec.push_back("localhost:5556");
            vec.push_back("127.0.0.1:5555");
            res.setData(vec);
            
            sendMessage(fd, res);

        }else if(m.getCommand() == Message::Command::UPDATE) {
            if(m.getArgument() == Message::Argument::REPORT) {
                //get the report
                //the report should be only a part of it
            }
        }
    }
    
}