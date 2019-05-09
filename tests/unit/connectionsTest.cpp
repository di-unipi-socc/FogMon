#include <gtest/gtest.h>
#include "master_connections.hpp"
#include "imaster_node.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <sys/socket.h>

using namespace std;

class MStorage : virtual public IMasterStorage {
protected:
    void createTables() {}

public:

    void setFilter(string ip) {
        
    }

    virtual std::vector<std::string> getNodes() {
        vector<string> ret;
        ret.push_back("test1");
        ret.push_back("test2");
        return ret;
    }

    virtual Report::hardware_result getHardware() {
        return Report::hardware_result();
    }
    virtual std::vector<Report::test_result> getLatency() {
        vector<Report::test_result> ret;
        ret.push_back(Report::test_result());
        return ret;
    }
    virtual std::vector<Report::test_result> getBandwidth() {
        vector<Report::test_result> ret;
        ret.push_back(Report::test_result());
        return ret;
    }

    virtual std::vector<Report::IoT> getIots() {
        std::vector<Report::IoT> iots;
        Report::IoT a(string("123123"),string("aaa"),11);
        iots.push_back(a);
        return iots;
    }


    virtual void saveLatencyTest(std::string ip, int ms) {}
    virtual void saveBandwidthTest(std::string ip, float kbps, int state) {}
    virtual void saveHardware(Report::hardware_result hardware) {}

    virtual void refreshNodes(std::vector<std::string> nodes) {
        EXPECT_EQ(nodes.size(), 1);
        EXPECT_EQ(nodes[0], "test"); 
    }
    virtual void updateNodes(std::vector<std::string> add, std::vector<std::string> rem) {
        EXPECT_EQ(add.size(), 2);
        EXPECT_EQ(add[0], "test"); 
        EXPECT_EQ(add[1], "test2");
        EXPECT_EQ(rem.size(), 1);
        EXPECT_EQ(rem[0], "test"); 
    }
    
    
    virtual std::vector<std::string> getLRLatency(int num, int seconds) {return vector<string>();}
    virtual std::vector<std::string> getLRBandwidth(int num, int seconds) {return vector<string>();}

    virtual int getTestBandwidthState(std::string ip, Report::test_result &last) { return 0;}

    virtual void setToken(int duration) {
        EXPECT_EQ(duration, 10);
    }
    virtual int hasToken() {return 0;}


    virtual void addNode(std::string strIp, Report::hardware_result) {}

    virtual void addReportLatency(std::string strIp, std::vector<Report::test_result> latency) {}
    virtual void addReportBandwidth(std::string strIp, std::vector<Report::test_result> bandwidth) {}
    virtual void addReportIot(std::string strIp, std::vector<Report::IoT> iots) {}
    virtual void addReport(std::string strIp, Report::hardware_result hardware, std::vector<Report::test_result> latency, std::vector<Report::test_result> bandwidth) {}

    virtual std::vector<std::string> getLRHardware(int num, int seconds) {return vector<string>();}

    virtual void addMNode(string ip) {}
    virtual Report::report_result getReport(string ip) {}
    virtual vector<string> getMNodes() {}
    virtual vector<Report::report_result> getReport() {}

    virtual Report::hardware_result getHardware(string ip) {}
    virtual std::vector<Report::test_result> getLatency(string ip) {}
    virtual std::vector<Report::test_result> getBandwidth(string ip) {}
    virtual void addNode(std::string strIp, Report::hardware_result hardware, std::string monitored = "::1") {}
    virtual void addReport(Report::report_result result, std::string monitored = "::1") {}
    virtual void addReport(std::vector<Report::report_result> results, std::string ip) {}
    virtual void complete() {}

    virtual void addIot(IThing *iot) {};
    virtual void addIot(std::string strIp, Report::IoT iot) {}
};

class MParent : virtual public IMasterNode {
public:
    void start() {};
    void stop() {};

    bool setParam(string name, int value) {}
    IConnections* getConnections() {return NULL;}
    void setMyIp(std::string ip) {}
    std::string getMyIp() {return "";}
    int startIperf() {return 0;}
    int startEstimate() {return 0;}

    int getIperfPort() {return 55555;}
    int getEstimatePort() {return 8365;}
    Server* getServer() {return NULL;}

    IMasterStorage* getStorage() {return &this->storage;}

    MStorage storage;
};

class MConn : public IConnections {
public:
    MConn() : IConnections(0) {}
    bool sendMessage(int fd, Message &m) {return IConnections::sendMessage(fd,m);}

    bool getMessage(int fd, Message &m) {return IConnections::getMessage(fd,m);}

    void handler(int,Message &) {}
};


TEST(ConnectionsTest, RSetToken) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::SET);
    mess.setArgument(Message::Argument::TOKEN);
    mess.setData(10);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::SET);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RStartIperf) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::START);
    mess.setArgument(Message::Argument::IPERF);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::START);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    int port = -1;
    EXPECT_EQ(res.getData(port), true);
    EXPECT_EQ(port, mNode.startIperf());
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RStartEstimate) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::START);
    mess.setArgument(Message::Argument::ESTIMATE);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::START);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    int port = -1;
    EXPECT_EQ(res.getData(port), true);
    EXPECT_EQ(port, mNode.startEstimate());
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RSetNodes) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::SET);
    mess.setArgument(Message::Argument::NODES);
    vector<string> strings;
    strings.push_back("test");
    mess.setData(strings);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::SET);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RGetNodes) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::GET);
    mess.setArgument(Message::Argument::NODES);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::GET);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    vector<string> strings;
    EXPECT_EQ(res.getData(strings), true);
    EXPECT_EQ(strings, mNode.getStorage()->getNodes());
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RGetReport) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::GET);
    mess.setArgument(Message::Argument::REPORT);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::GET);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    Report r;
    string strss;
    res.buildString();
    strss = res.getString();
    EXPECT_EQ(res.getData(r), true);

    Report r2; 
    r2.setHardware(((IStorage*)mNode.getStorage())->getHardware());
    r2.setLatency(((IStorage*)mNode.getStorage())->getLatency());
    r2.setBandwidth(((IStorage*)mNode.getStorage())->getBandwidth());
    r2.setIot(((IStorage*)mNode.getStorage())->getIots());

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer (s);
    r.getJson()->Accept (writer);
    std::string str (s.GetString());

    rapidjson::StringBuffer s2;
    rapidjson::Writer<rapidjson::StringBuffer> writer2 (s2);
    r2.getJson()->Accept (writer2);
    std::string str2 (s2.GetString());
    EXPECT_EQ(str2, str);
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, NUpdateNodes) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(AF_LOCAL,SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    Connections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::NOTIFY);
    mess.setCommand(Message::Command::UPDATE);
    mess.setArgument(Message::Argument::NODES);
    vector<string> stringsA;
    stringsA.push_back("test");
    stringsA.push_back("test2");
    vector<string> stringsB;
    stringsB.push_back("test");
    mess.setData(stringsA,stringsB);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsMasterTest, Aaaa) {
    FAIL();
}