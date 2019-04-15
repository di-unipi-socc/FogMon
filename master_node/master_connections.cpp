
#include "master_connections.hpp"
#include "master_node.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

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

MasterConnections::MasterConnections(int nThread) : Connections(nThread) {
}

MasterConnections::~MasterConnections() {
}

void MasterConnections::initialize(IMasterNode* parent) {
    Connections::initialize(parent);
    this->parent = parent;
}

void MasterConnections::handler(int fd, Message &m) {
    socklen_t len;
    struct sockaddr_storage addr;
    char ip[INET6_ADDRSTRLEN];
    memset(ip, 0, INET6_ADDRSTRLEN);
    strcat(ip, "::1");

    len = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &len);
                
    if(addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
    }else if(addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
        if (IN6_IS_ADDR_V4MAPPED(&s->sin6_addr)) {
            inet_ntop(AF_INET, &(((in_addr*)(s->sin6_addr.s6_addr+12))->s_addr), ip, sizeof(ip));
        }
        else
            inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
    }else {
        cerr << "error socket family" << endl;
#ifndef ENABLE_TESTS
        return;
#endif
    }
    if(strcmp(ip,"127.0.0.1")==0)
        strcpy(ip,"::1");
    string strIp = string(ip);

    bool handled = false;

    if(m.getType() == Message::Type::MREQUEST) {
        if(m.getCommand() == Message::Command::SET) {
            if(m.getArgument() == Message::Argument::REPORT) {
                handled = true;
                Report r;
                if(m.getData(r)) {
                    vector<Report::report_result> results;
                    if(r.getReports(results)) {
                        this->parent->getStorage()->addReport(results, strIp);
                    }
                    Message res;
                    res.setType(Message::Type::MRESPONSE);
                    res.setCommand(Message::Command::SET);
                    res.setArgument(Message::Argument::POSITIVE);

                    sendMessage(fd, res);
                }
            }
        }else if(m.getCommand() == Message::Command::MHELLO) {
            handled = true;
            this->parent->getStorage()->addMNode(strIp);
            Message res;
            res.setType(Message::Type::MRESPONSE);
            res.setCommand(Message::Command::MHELLO);
            res.setArgument(Message::Argument::POSITIVE);

            vector<string> nodes = this->parent->getStorage()->getMNodes();

            res.setData(nodes);

            sendMessage(fd, res);
        }
    }else if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                handled = true;
                //build array of nodes
                vector<string> nodes = this->parent->getStorage()->getNodes();
                
                //local node needs to monitor also the other mnodes
                if(strIp == "::1") {
                    vector<string> mnodes = this->parent->getStorage()->getMNodes();
                    nodes.insert(nodes.end(), mnodes.begin(), mnodes.end());
                }
                
                //send nodes
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::NODELIST);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(nodes);
            
                sendMessage(fd, res);
            }
        }else if(m.getArgument() == Message::Argument::MNODES) {
            if(m.getCommand() == Message::Command::GET) {
                handled = true;
                //build array of nodes
                vector<string> nodes = this->parent->getStorage()->getMNodes();
                //send nodes
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::MNODELIST);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(nodes);
            
                sendMessage(fd, res);
            }
        }else if(m.getArgument() == Message::Argument::REPORT) {
            if(m.getCommand() == Message::Command::SET) {
                handled = true;
                //read report ---------------------------
                Report r;
                if(m.getData(r)) {
                    Report::report_result report;
                    if(r.getReport(report)) {
                        this->parent->getStorage()->addReport(report);
                    }
                }
            }
        }
    }else if(m.getType() == Message::Type::NOTIFY) {
        if(m.getCommand() == Message::Command::HELLO) {
            handled = true;

            int max_group = 30;
            if(this->parent->getStorage()->getNodes().size() >= max_group) {
                //refuse, max number of nodes exceded
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::HELLO);
                res.setArgument(Message::Argument::NEGATIVE);
                
                sendMessage(fd, res);
            }

            //get report on hardware
            Report r;
            if(m.getData(r)) {
                Report::hardware_result hardware;
                r.getHardware(hardware);

                //set new node online                
                this->parent->getStorage()->addNode(strIp, hardware);
                

                vector<string> vec = this->parent->getStorage()->getNodes();

                //get nodelist
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::HELLO);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(strIp, vec);
                
                sendMessage(fd, res);

                //inform all the other nodes about it
                //
                Message broadcast;
                broadcast.setType(Message::Type::NOTIFY);
                broadcast.setCommand(Message::Command::UPDATE);
                broadcast.setArgument(Message::Argument::NODES);
                vector<string> v;
                v.push_back(strIp);
                vector<string> v2;
                broadcast.setData(v ,v2);

                this->notifyAll(broadcast);
            }
        }else if(m.getCommand() == Message::Command::UPDATE) {
            if(m.getArgument() == Message::Argument::REPORT) {
                handled = true;
                //get the report
                //the report should be only a part of it
                Report r;
                if(m.getData(r)) {
                    Report::hardware_result hardware;
                    vector<Report::test_result> latency;
                    vector<Report::test_result> bandwidth;
                    vector<Report::IoT> iot;
                    if(r.getHardware(hardware)) {
                        this->parent->getStorage()->addNode(strIp, hardware);
                    }
                    if(r.getLatency(latency)) {
                        this->parent->getStorage()->addReportLatency(strIp, latency);
                    }
                    if(r.getBandwidth(bandwidth)) {
                        this->parent->getStorage()->addReportBandwidth(strIp, bandwidth);
                    }
                    if(r.getIot(iot)) {
                        this->parent->getStorage()->addReportIot(strIp, iot);
                    }
                    Message res;
                    res.setType(Message::Type::RESPONSE);
                    res.setCommand(Message::Command::UPDATE);
                    res.setArgument(Message::Argument::POSITIVE);
                    
                    sendMessage(fd, res);
                }
            }
        }
    }
    if(!handled)
        Connections::handler(fd, m);
}

bool MasterConnections::sendRemoveNodes(std::vector<std::string> ips) {
    Message broadcast;
    broadcast.setType(Message::Type::NOTIFY);
    broadcast.setCommand(Message::Command::UPDATE);
    broadcast.setArgument(Message::Argument::NODES);

    vector<string> v;
    broadcast.setData(v ,ips);

    return this->notifyAll(broadcast);
}

bool MasterConnections::sendRequestReport(std::string ip) {
    int Socket = openConnection(ip);
    
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setType(Message::Type::REQUEST);
    m.setCommand(Message::Command::GET);
    m.setArgument(Message::Argument::REPORT);

    bool ret = false;

    //send message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::RESPONSE &&
                res.getCommand() == Message::Command::GET &&
                res.getArgument() == Message::Argument::POSITIVE) {
                //get report and save it
                Report r;
                if(m.getData(r)) {
                    Report::hardware_result hardware;
                    vector<Report::test_result> latency;
                    vector<Report::test_result> bandwidth;
                    vector<Report::IoT> iot;
                    if(r.getHardware(hardware)) {
                        this->parent->getStorage()->addNode(ip, hardware);
                    }
                    if(r.getLatency(latency)) {
                        this->parent->getStorage()->addReportLatency(ip, latency);
                    }
                    if(r.getBandwidth(bandwidth)) {
                        this->parent->getStorage()->addReportBandwidth(ip, bandwidth);
                    }
                    if(r.getIot(iot)) {
                        this->parent->getStorage()->addReportIot(ip, iot);
                    }
                    ret = true;
                }
            }
        }
    }
    close(Socket);
    return ret;
}

bool MasterConnections::sendSetToken(std::string ip, int time) {
    int Socket = openConnection(ip);
    
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setType(Message::Type::REQUEST);
    m.setCommand(Message::Command::SET);
    m.setArgument(Message::Argument::TOKEN);

    m.setData(time);
    bool ret = false;

    //send message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::RESPONSE &&
                res.getCommand() == Message::Command::SET &&
                res.getArgument() == Message::Argument::POSITIVE) {
                ret = true;
            }
        }
    }
    close(Socket);
    return ret;
}

bool MasterConnections::sendMReport(std::string ip, vector<Report::report_result> report) {
    int Socket = this->openConnection(ip);
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setType(Message::Type::MREQUEST);//TODO
    m.setCommand(Message::Command::SET);
    m.setArgument(Message::Argument::REPORT);

    Report r;
    r.setReports(report);

    m.setData(r);
    bool ret = false;

    //send message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::MRESPONSE &&
                res.getCommand() == Message::Command::SET &&
                res.getArgument() == Message::Argument::POSITIVE) {
                ret = true;
            }
        }
    }
    close(Socket);
    return ret;
}

bool MasterConnections::sendMHello(std::string ip) {
    int Socket = this->openConnection(ip);
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setType(Message::Type::MREQUEST);//TODO
    m.setCommand(Message::Command::MHELLO);
    m.setArgument(Message::Argument::REPORT);

    bool ret = false;

    //send message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::MRESPONSE &&
                res.getCommand() == Message::Command::MHELLO &&
                res.getArgument() == Message::Argument::POSITIVE) {
                
                vector<string> vec;
                if(res.getData(vec)) {
                    for(auto node : vec) {
                        this->parent->getStorage()->addMNode(node);
                    }
                    this->parent->getStorage()->addMNode(ip);
                    ret = true;
                }
            }
        }
    }
    close(Socket);
    return ret;
}