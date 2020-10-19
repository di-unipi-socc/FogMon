#include <gtest/gtest.h>

#include "selector.hpp"
#include "leader_storage.hpp"
#include "follower.hpp"

class MSelector : public Selector {
public:
    MSelector(ILeader * leader) : Selector(leader) {}

    void setId(int id) {
        this->id = id;
    }

    void setStatus(Status s) {
        this->status = s;
    }

    Status getStatus() {
        return this->status;
    }

    virtual Message::leader_update selection(int id) {
        if(id == 153) {
            return Selector::selection(id);
        }
        vector<Message::node> vect;
        vect.push_back(Message::node("a","",""));
        
        return Message::leader_update(vect,0.5,0,id);
    }

    void start2() {
        this->startSelection();
    }
};

class MStorage2 : virtual public ILeaderStorage {
protected:
    void createTables() {}

public:

    void setFilter(string ip) {
        
    }

    Message::node getNode() {
    }

    virtual std::vector<Message::node> getNodes() {}

    virtual Report::hardware_result getHardware() {}
    virtual std::vector<Report::test_result> getLatency(int64_t last, int sensibility) {}
    virtual std::vector<Report::test_result> getBandwidth(int64_t last, int sensibility) {}
    virtual void saveState() {}

    virtual std::vector<Report::IoT> getIots() {}


    virtual void saveLatencyTest(Message::node ip, int ms) {}
    virtual void saveBandwidthTest(Message::node ip, float kbps, int state) {}
    virtual void saveHardware(Report::hardware_result hardware) {}

    virtual void refreshNodes(std::vector<Message::node> nodes) {}
    virtual void updateNodes(std::vector<Message::node> add, std::vector<Message::node> rem) {}
    
    
    virtual std::vector<Message::node> getLRLatency(int num, int seconds) {return vector<Message::node>();}
    virtual std::vector<Message::node> getLRBandwidth(int num, int seconds) {return vector<Message::node>();}
        
    virtual std::vector<Message::node> getMLRLatency(int num, int seconds) {return vector<Message::node>();}
    virtual std::vector<Message::node> getMLRBandwidth(int num, int seconds) {return vector<Message::node>();}

    virtual int getTestBandwidthState(Message::node ip, Report::test_result &last) { return 0;}

    virtual void setToken(int duration) { }
    virtual int hasToken() {}


    virtual void addNode(Message::node strIp, Report::hardware_result) {}

    virtual void addReportLatency(Message::node strIp, std::vector<Report::test_result> latency) {}
    virtual void addReportBandwidth(Message::node strIp, std::vector<Report::test_result> bandwidth) {}
    virtual void addReportIot(Message::node strIp, std::vector<Report::IoT> iots) {}
    virtual void addReport(Message::node strIp, Report::hardware_result hardware, std::vector<Report::test_result> latency, std::vector<Report::test_result> bandwidth) {}

    virtual std::vector<Message::node> getMLRHardware(int num, int seconds) {return vector<Message::node>();}

    virtual void addMNode(Message::node ip) {}
    virtual Report::report_result getReport(Message::node ip) {}
    virtual std::vector<Message::node> getAllNodes() {
        vector<Message::node> vect;
        for(int i=0; i<10; i++) {
            vect.push_back(Message::node());
        }
        return vect;
    }
    virtual vector<Message::node> getMNodes() {
        return vector<Message::node>();
    }
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

class MConnections : virtual public ILeaderConnections {
public:
    MConnections() {}
    ~MConnections() {}

    virtual void start() {}
    virtual void stop() {}
    virtual void request(int fd) {}

    virtual void initialize(ILeader* parent) {}
    virtual void initialize(IAgent* parent) {}
    
    virtual bool sendMHello(Message::node ip) {}

    virtual bool sendRemoveNodes(std::vector<Message::node> ips) {}
    virtual bool sendRequestReport(Message::node ip) {}
    virtual bool sendMReport(Message::node ip, vector<Report::report_result> report) {}

    virtual bool sendInitiateSelection(int id) {
        return selectorB->initSelection(id);
    }

    virtual bool sendStartSelection(int id) {
        bool r;
        return selectorB->calcSelection(Message::node("a","1","2"),id,r);
    }
    virtual bool sendSelection(Message::leader_update update,Message::node node) { 
        sent = true;
        if(selectorA) {
            EXPECT_EQ(true, node == Message::node("a","1","2"));
            selectorA->updateSelection(update);
        }
    }
    virtual bool sendEndSelection(Message::leader_update update, bool result) {
        ended = true;
    }

    virtual bool sendChangeRoles(Message::leader_update update) {}

    MSelector *selectorA = NULL;
    MSelector *selectorB = NULL;
    bool sent = false;
    bool ended = false;
};

class MParent2 : virtual public ILeader {
public:

    MConnections conn;
    Message::leader_update update;
    MStorage2 store;

    void start() {};
    void stop() {};

    bool setParam(string name, int value) {}
    ILeaderConnections* getConnections() {return &conn;}
    void setMyId(std::string ip) {}
    Message::node getMyNode() {return Message::node();}
    int startIperf() {return 0;}
    int startEstimate() {return 0;}

    int getIperfPort() {return 55555;}
    int getEstimatePort() {return 8365;}
    Server* getServer() {return NULL;}

    virtual bool updateSelection(Message::leader_update) { return true; }
    virtual void changeRoles(Message::leader_update u) {
        this->update = u;
    }
    virtual bool initSelection(int) { return true; }
    virtual bool calcSelection(Message::node, int, bool&) { return true; }
    virtual void stopSelection() {}

    void changeRole(std::vector<Message::node> leaders) {}

    ILeaderStorage* getStorage() {return &store;}

};

class MNode : public Node {
public:

    MNode() : Node("a",false,0) {}

    void promote() {
        sent=true;
    }
    bool sent = false;
};

TEST(SelectorTest, calcSelectionTest) {

    MParent2 parent;

    MSelector sel(&parent);

    bool e = sel.initSelection(10);
    EXPECT_EQ(e, true);

    e = sel.initSelection(9);
    EXPECT_EQ(e, true);

    e = sel.updateSelection(Message::leader_update());
    EXPECT_EQ(e, false);

    bool r;

    e = sel.calcSelection(Message::node(),10,r);
    EXPECT_EQ(r, true);
    EXPECT_EQ(e, true);
    usleep(10000);
    EXPECT_EQ(parent.conn.sent, true);


    sel.stopSelection();
    EXPECT_EQ(e, true);

    e = sel.calcSelection(Message::node(),10,r);
    EXPECT_EQ(e, false);
}

TEST(SelectorTest, interactionTest1) {

    MParent2 parent;

    MSelector sel(&parent);
    MSelector sel2(&parent);

    parent.conn.selectorA = &sel;
    parent.conn.selectorB = &sel2;

    sel2.setId(INT_MAX);
    sel2.setStatus(Selector::Status::READY);

    sel.start2();
    usleep(1000);
    EXPECT_EQ(sel2.getStatus(), Selector::Status::READY);

    parent.conn.selectorB = &sel;
    parent.conn.selectorA = &sel2;
    sel.setId(INT_MIN);

    sel2.updateSelection(Message::leader_update(vector<Message::node>(),1,0,1));

    sel2.setStatus(Selector::Status::FREE);
    sel2.start2();
    EXPECT_EQ(sel2.getStatus(), Selector::Status::STARTED);
    usleep(10000);

    EXPECT_EQ(sel2.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(sel.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(parent.conn.ended, true);
    EXPECT_EQ(parent.conn.sent, true);
}


TEST(SelectorTest, interactionTest2) {
    MParent2 parent;

    MSelector sel(&parent);
    MSelector sel2(&parent);

    parent.conn.selectorA = &sel;
    parent.conn.selectorB = &sel2;

    EXPECT_EQ(sel.checkSelection(),true);
    EXPECT_EQ(sel.getStatus(), Selector::Status::STARTED);
    usleep(10000);

    EXPECT_EQ(sel.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(sel2.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(parent.conn.ended, true);
    EXPECT_EQ(parent.conn.sent, true);
}


TEST(SelectorTest, ScriptTest) {
    MParent2 parent;

    MSelector sel(&parent);
    MSelector sel2(&parent);

    parent.conn.selectorA = &sel;
    parent.conn.selectorB = &sel2;

    float table[10][10] {
        {0,0,0,50,50,50,500,500,500,500},
        {0,5,5,50,50,50,500,500,500,500},
        {0,5,5,50,50,50,500,500,500,500},
        {50,50,50,0,0,0,500,500,500,500},
        {50,50,50,0,5,5,500,500,500,500},
        {50,50,50,0,5,5,500,500,500,500},
        {500,500,500,500,500,500,0,0,0,0},
        {500,500,500,500,500,500,0,1,1,1},
        {500,500,500,500,500,500,0,1,1,1},
        {500,500,500,500,500,500,0,1,1,1},
    };
    unlink("leader_node.db");
    ILeaderStorage * stor = new LeaderStorage(Message::node("0","::1",""));
    stor->open("leader_node.db");
    
    for(int i=0; i< 10; i++) {
        stor->addNode(Message::node(to_string(i),to_string(i),""),Report::hardware_result());
    }

    vector<Report::report_result> reports;
    for(int i=0; i<10; i++) {
        Report::report_result report;
        vector<Report::test_result> lat;
        for(int j=0; j<10; j++) {
            string val;
            if(i==j) {
                continue;
            }
            if(j==0) {
                val = "::1";
            }
            else 
                val = to_string(j);
            Report::test_result test(Message::node(to_string(j),val,""),table[i][j],0,0);
            
            lat.push_back(test);
        }
        report.latency = lat;
        string val;
        if(i==0) {
            val = "::1";
        }
        else 
            val = to_string(i);
        report.source = Message::node(to_string(i),val,"");
        reports.push_back(report);
    }
    
    stor->addReport(reports,Message::node("0","::1",""));
    usleep(10000);
    Message::leader_update up = sel.selection(153);
    int i = 0;
    while(up.cost > 0.0141 || up.cost < 0.014) {
        up = sel.selection(153);
        if (i > 2) {
            FAIL();
            return;
        }
        i++;
    }
    EXPECT_EQ(up.changes,2);
    EXPECT_NEAR(up.cost, 0.014,0.001);
    int vect[] = {0,3,6};
    EXPECT_EQ(up.selected.size(), 3);
    for(int i=0; i<up.selected.size(); i++) {
        EXPECT_EQ(to_string(vect[i]),up.selected[i].id);
    }
}

TEST(SelectorTest, FollowerTest) {

    Follower A(Message::node("a","1","2"),1);

    MNode node;
    A.setParent(&node);

    vector<Message::node> vect;
    vect.push_back(Message::node("a","1","2"));

    A.changeRole(vect);


    EXPECT_EQ(node.sent, true);

}