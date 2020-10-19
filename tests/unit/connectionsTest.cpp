#include <gtest/gtest.h>
#include "leader_connections.hpp"
#include "ileader.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <sys/socket.h>

using namespace std;

Message::node nodeM("idM","","");
Message::node nodeN("sender","","");

class MStorage : virtual public ILeaderStorage {
protected:
    void createTables() {}

public:

    void setFilter(string ip) {
        
    }

    Message::node getNode() {
        return nodeM;
    }

    virtual std::vector<Message::node> getNodes() {
        vector<Message::node> ret;
        ret.push_back(Message::node("test1","",""));
        ret.push_back(Message::node("test2","",""));
        return ret;
    }

    virtual Report::hardware_result getHardware() {
        return Report::hardware_result();
    }
    virtual std::vector<Report::test_result> getLatency(int64_t last, int sensibility) {
        vector<Report::test_result> ret;
        ret.push_back(Report::test_result());
        return ret;
    }
    virtual std::vector<Report::test_result> getBandwidth(int64_t last, int sensibility) {
        vector<Report::test_result> ret;
        ret.push_back(Report::test_result());
        return ret;
    }

    virtual void saveState() {}

    virtual std::vector<Report::IoT> getIots() {
        std::vector<Report::IoT> iots;
        Report::IoT a(string("123123"),string("aaa"),11);
        iots.push_back(a);
        return iots;
    }


    virtual void saveLatencyTest(Message::node ip, int ms) {}
    virtual void saveBandwidthTest(Message::node ip, float kbps, int state) {}
    virtual void saveHardware(Report::hardware_result hardware) {}

    virtual void refreshNodes(std::vector<Message::node> nodes) {
        EXPECT_EQ(nodes.size(), 1);
        EXPECT_EQ(nodes[0].id, "test"); 
    }
    virtual void updateNodes(std::vector<Message::node> add, std::vector<Message::node> rem) {
        EXPECT_EQ(add.size(), 2);
        EXPECT_EQ(add[0].id, "test"); 
        EXPECT_EQ(add[1].id, "test2");
        EXPECT_EQ(rem.size(), 1);
        EXPECT_EQ(rem[0].id, "test"); 
    }
    
    
    virtual std::vector<Message::node> getLRLatency(int num, int seconds) {return vector<Message::node>();}
    virtual std::vector<Message::node> getLRBandwidth(int num, int seconds) {return vector<Message::node>();}
        
    virtual std::vector<Message::node> getMLRLatency(int num, int seconds) {return vector<Message::node>();}
    virtual std::vector<Message::node> getMLRBandwidth(int num, int seconds) {return vector<Message::node>();}

    virtual int getTestBandwidthState(Message::node ip, Report::test_result &last) { return 0;}

    virtual void setToken(int duration) {
        EXPECT_EQ(duration, 10);
    }
    virtual int hasToken() {return 0;}


    virtual void addNode(Message::node strIp, Report::hardware_result) {}

    virtual void addReportLatency(Message::node strIp, std::vector<Report::test_result> latency) {}
    virtual void addReportBandwidth(Message::node strIp, std::vector<Report::test_result> bandwidth) {}
    virtual void addReportIot(Message::node strIp, std::vector<Report::IoT> iots) {}
    virtual void addReport(Message::node strIp, Report::hardware_result hardware, std::vector<Report::test_result> latency, std::vector<Report::test_result> bandwidth) {}

    virtual std::vector<Message::node> getMLRHardware(int num, int seconds) {return vector<Message::node>();}

    virtual void addMNode(Message::node ip) {}
    virtual Report::report_result getReport(Message::node ip) {}
    virtual std::vector<Message::node> getAllNodes() {}
    virtual vector<Message::node> getMNodes() {}
    virtual vector<Report::report_result> getReport() {}

    virtual Report::hardware_result getHardware(Message::node ip) {}
    virtual std::vector<Report::test_result> getLatency(Message::node ip) {}
    virtual std::vector<Report::test_result> getBandwidth(Message::node ip) {}
    virtual std::string addNode(Message::node strIp, Report::hardware_result hardware, Message::node *monitored = NULL) {}
    virtual void addReport(Report::report_result result, Message::node *monitored = NULL) {}
    virtual void addReport(std::vector<Report::report_result> results, Message::node ip) {}
    virtual void removeOldNodes(int seconds) {}
    virtual void complete() {}

    virtual void addIot(IThing *iot) {};
    virtual void addIot(Message::node strIp, Report::IoT iot) {}
};

class MParent : virtual public ILeader {
public:
    void start() {};
    void stop() {};

    bool setParam(string name, int value) {}
    ILeaderConnections* getConnections() {return NULL;}
    void setMyId(std::string ip) {}
    Message::node getMyNode() {return nodeM;}
    int startIperf() {return 0;}
    int startEstimate() {return 0;}

    int getIperfPort() {return 55555;}
    int getEstimatePort() {return 8365;}
    Server* getServer() {return NULL;}

    virtual bool updateSelection(Message::leader_update) { return true; }
    virtual void changeRoles(Message::leader_update) { }
    virtual bool initSelection(int) { return true; }
    virtual bool calcSelection(Message::node, int, bool&) { return true; }
    virtual void stopSelection() {}

    bool change=false;

    void changeRole(std::vector<Message::node> leaders) {
        EXPECT_EQ(leaders.size(),1);
        cout << "Entering" << endl;
        fflush(stdout);
        for(auto l : leaders) {
            if(l.id == "a") {

            }else {
                FAIL();
            }
        }
        change = true;
        cout << "changed change = True" << endl;
        fflush(stdout);
    }

    ILeaderStorage* getStorage() {return &this->storage;}
    
    MStorage storage;
};

class MConn : public Connections {
public:
    MConn() : Connections(0) {}
    bool sendMessage(int fd, Message &m) {return Connections::sendMessage(fd,m);}

    bool getMessage(int fd, Message &m) {return Connections::getMessage(fd,m);}

    void handler(int,Message &) {}
};

TEST(ConnectionsTest, RStartIperf) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setSender(nodeN);
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
    EXPECT_EQ(port, mNode.getIperfPort());
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RStartEstimate) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM, 0, pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setSender(nodeN);
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::START);
    mess.setArgument(Message::Argument::ESTIMATE);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::START);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    Message::node val;
    EXPECT_EQ(res.getData(val), true);
    EXPECT_EQ(stol(val.port), mNode.getEstimatePort());
    EXPECT_EQ(val.ip, string("::1"));
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RSetNodes) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setSender(nodeN);
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::SET);
    mess.setArgument(Message::Argument::NODES);
    vector<Message::node> strings;
    strings.push_back(Message::node("test","",""));
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

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setSender(nodeN);
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::GET);
    mess.setArgument(Message::Argument::NODES);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    Message res;
    EXPECT_EQ(mConn.getMessage(pipefd[1],res), true);

    EXPECT_EQ(res.getType(), Message::Type::RESPONSE);
    EXPECT_EQ(res.getCommand(), Message::Command::GET);
    EXPECT_EQ(res.getArgument(), Message::Argument::POSITIVE);
    vector<Message::node> strings;
    EXPECT_EQ(res.getData(strings), true);
    EXPECT_EQ(strings, mNode.getStorage()->getNodes());
    
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsTest, RGetReport) {
    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setSender(nodeN);
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

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::NOTIFY);
    mess.setCommand(Message::Command::UPDATE);
    mess.setArgument(Message::Argument::NODES);
    vector<Message::node> stringsA;
    stringsA.push_back(Message::node("test","",""));
    stringsA.push_back(Message::node("test2","",""));
    vector<Message::node> stringsB;
    stringsB.push_back(Message::node("test","",""));
    mess.setData(stringsA,stringsB);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
}

TEST(ConnectionsLeaderTest, sendChangeRoleTest) {

    int pipefd[2];
    MConn mConn;

    EXPECT_EQ(socketpair(PF_LOCAL, SOCK_STREAM,0,pipefd), 0);
    MParent mNode;
    FollowerConnections conn(1);
    conn.initialize(&mNode);
    conn.start();
    conn.request(pipefd[0]);

    Message mess;
    mess.setType(Message::Type::REQUEST);
    mess.setCommand(Message::Command::SET);
    mess.setArgument(Message::Argument::ROLES);

    Message::leader_update update;
    update.selected.push_back(Message::node("a","b","c"));

    mess.setData(update);
    
    EXPECT_EQ(mConn.sendMessage(pipefd[1],mess), true);
    usleep(10000);
    EXPECT_EQ(close(pipefd[1]), 0);
    conn.stop();
    EXPECT_EQ(mNode.change, true);
}

TEST(ConnectionsLeaderTest, Aaaa) {
    EXPECT_EQ(1,1);
}