
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
    string strIp = this->getSource(fd,m);

    bool handled = false;

    if(m.getType() == Message::Type::MREQUEST) {
        if(m.getCommand() == Message::Command::SET) {
            if(m.getArgument() == Message::Argument::REPORT) {
                handled = true;
                Report r;
                if(m.getData(r)) {
                    vector<Report::report_result> results;
                    if(r.getReports(results)) {
                        this->parent->getStorage()->addReport(results, m.getSender());
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
            Message res;
            res.setType(Message::Type::MRESPONSE);
            res.setCommand(Message::Command::MHELLO);

            if(m.getSender().id == this->parent->getMyNode().id)
            {
                res.setArgument(Message::Argument::NEGATIVE);
            }else {
                this->parent->getStorage()->addMNode(m.getSender());
                
                res.setArgument(Message::Argument::POSITIVE);

                vector<Message::node> nodes = this->parent->getStorage()->getMNodes();

                res.setData(nodes);
            }
            sendMessage(fd, res);
        }
    }else if(m.getType() == Message::Type::REQUEST) {
        if(m.getArgument() == Message::Argument::NODES) {
            if(m.getCommand() == Message::Command::GET) {
                handled = true;
                //build array of nodes
                vector<Message::node> nodes = this->parent->getStorage()->getNodes();
                
                //local node needs to monitor also the other mnodes
                if(strIp == "::1") {
                    vector<Message::node> mnodes = this->parent->getStorage()->getMNodes();
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
                vector<Message::node> nodes = this->parent->getStorage()->getMNodes();
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

            //get report on hardware
            Report r;
            if(m.getData(r)) {
                Report::hardware_result hardware;
                r.getHardware(hardware);

                Message::node sender = m.getSender();
                sender.id = "";
                if(sender.ip == "::1" && sender.port == this->parent->getMyNode().port) {
                    sender.id = this->parent->getMyNode().id;
                }

                //set new node online                
                sender.id = this->parent->getStorage()->addNode(sender, hardware);
                

                vector<Message::node> vec = this->parent->getStorage()->getNodes();

                //get nodelist
                Message res;
                res.setType(Message::Type::RESPONSE);
                res.setCommand(Message::Command::HELLO);
                res.setArgument(Message::Argument::POSITIVE);

                res.setData(sender, vec);
                
                sendMessage(fd, res);

                //inform all the other nodes about it
                //
                Message broadcast;
                broadcast.setType(Message::Type::NOTIFY);
                broadcast.setCommand(Message::Command::UPDATE);
                broadcast.setArgument(Message::Argument::NODES);
                vector<Message::node> v;
                v.push_back(sender);
                vector<Message::node> v2;
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
                        this->parent->getStorage()->addNode(m.getSender(), hardware);
                    }
                    if(r.getLatency(latency)) {
                        this->parent->getStorage()->addReportLatency(m.getSender(), latency);
                    }
                    if(r.getBandwidth(bandwidth)) {
                        this->parent->getStorage()->addReportBandwidth(m.getSender(), bandwidth);
                    }
                    if(r.getIot(iot)) {
                        this->parent->getStorage()->addReportIot(m.getSender(), iot);
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

bool MasterConnections::sendRemoveNodes(std::vector<Message::node> ips) {
    Message broadcast;
    broadcast.setSender(this->parent->getMyNode());
    broadcast.setType(Message::Type::NOTIFY);
    broadcast.setCommand(Message::Command::UPDATE);
    broadcast.setArgument(Message::Argument::NODES);

    vector<Message::node> v;
    broadcast.setData(v ,ips);

    return this->notifyAll(broadcast);
}

bool MasterConnections::sendRequestReport(Message::node ip) {
    int Socket = openConnection(ip.ip,ip.port);
    
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setSender(this->parent->getMyNode());
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

bool MasterConnections::sendMReport(Message::node ip, vector<Report::report_result> report) {
    int Socket = this->openConnection(ip.ip, ip.port);
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setSender(this->parent->getMyNode());
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

bool MasterConnections::sendMHello(Message::node ip) {
    int Socket = this->openConnection(ip.ip, ip.port);
    if(Socket < 0) {
        return false;
    }

    fflush(stdout);
    char buffer[10];

    //build message
    Message m;
    m.setSender(this->parent->getMyNode());
    m.setType(Message::Type::MREQUEST);//TODO
    m.setCommand(Message::Command::MHELLO);
    m.setArgument(Message::Argument::REPORT);
    m.setData(this->parent->getStorage()->getNode());

    bool ret = false;

    //send message
    if(this->sendMessage(Socket, m)) {
        Message res;
        if(this->getMessage(Socket, res)) {
            if( res.getType()==Message::Type::MRESPONSE &&
                res.getCommand() == Message::Command::MHELLO &&
                res.getArgument() == Message::Argument::POSITIVE) {
                
                vector<Message::node> vec;
                if(res.getData(vec)) {
                    for(auto node : vec) {
                        if(node.ip == "::1")
                            node.ip = ip.ip;
                        this->parent->getStorage()->addMNode(node);
                    }
                    ret = true;
                }
            }
        }
    }
    close(Socket);
    return ret;
}