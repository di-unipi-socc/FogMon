
#include "master_connections.hpp"
#include "inode.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

MasterConnections::MasterConnections(INode *parent, int nThread) : IConnections(parent, nThread), storage("masterNode.db") {
}

MasterConnections::~MasterConnections() {
}

MasterStorage* MasterConnections::getStorage() {
    return &(this->storage);
}

void MasterConnections::handler(int fd, Message &m) {

    socklen_t len;
    struct sockaddr_storage addr;
    char ip[INET6_ADDRSTRLEN];
                
    len = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &len);
                
    if(addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
    }else if(addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
    }
    string strIp = string(ip);

    if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                //build array of nodes
                vector<string> nodes = storage.getNodes();
                //send nodes
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::NODELIST);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(nodes);
            
                sendMessage(fd, res);
            }
        }else if(m.getArgument() == Message::Argument::REPORT) {
            if(m.getCommand() == Message::Command::SET) {
                //read report
                Report r;
                if(m.getData(r)) {
                    Report::hardware_result hardware;
                    vector<Report::test_result> latency;
                    vector<Report::test_result> bandwidth;

                    if(r.getHardware(hardware) && r.getLatency(latency) && r.getBandwidth(bandwidth)) {
                        storage.addReport(strIp, hardware, latency, bandwidth);
                    }
                }
            }
        }
    }else if(m.getType() == Message::Type::NOTIFY) {
        if(m.getCommand() == Message::Command::HELLO) {
            //get report on hardware
            Report r;
            if(m.getData(r)) {
                Report::hardware_result hardware;
                r.getHardware(hardware);

                //set new node online                
                storage.addNode(strIp, hardware);
                
                //inform all the other nodes about it
                //
                //

                vector<string> vec = storage.getNodes();

                //get nodelist
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::HELLO);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(strIp, vec);
                
                sendMessage(fd, res);
            }
        }else if(m.getCommand() == Message::Command::UPDATE) {
            if(m.getArgument() == Message::Argument::REPORT) {
                //get the report
                //the report should be only a part of it
                Report r;
                if(m.getData(r)) {
                    Report::hardware_result hardware;
                    vector<Report::test_result> latency;
                    vector<Report::test_result> bandwidth;

                    if(r.getHardware(hardware)) {
                        storage.addNode(strIp, hardware);
                    }
                    if(r.getLatency(latency)) {
                        storage.addReportLatency(strIp, latency);
                    }
                    if(r.getBandwidth(bandwidth)) {
                        storage.addReportBandwidth(strIp, bandwidth);
                    }
                }
            }
        }
    }
    
}