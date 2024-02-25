#include <gtest/gtest.h>

#include "node.hpp"
#include "selector.hpp"
#include "leader_storage.hpp"
#include "follower.hpp"


class MMSelector : public Selector {
public:
    MMSelector(ILeader * leader) : Selector(leader) {}

    void setId(int id) {
        this->id = id;
    }

    void setStatus(Status s) {
        this->status = s;
    }

    Status getStatus() {
        return this->status;
    }

    virtual Message::leader_update selection(int id, int formula) override {
        printf("selection\n");
        if(id == 153) {
            return Selector::selection(id, this->parent->node->leaderFormula);
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
    MStorage2() {}
    ~MStorage2() {}

    void setFilter(string ip) {
        
    }

    Message::node getNode() {
    }

    virtual std::vector<Message::node> getNodes() {}

    virtual Report::hardware_result getHardware() {}
    virtual std::vector<Report::test_result> getLatency(int sensibility, int64_t last) {}
    virtual std::vector<Report::test_result> getBandwidth(int sensibility, int64_t last) {}
    virtual void saveState(int64_t last, int sensitivity) {}

    virtual std::vector<Report::IoT> getIots() {}


    virtual void saveLatencyTest(Message::node ip, int ms, int window) {}
    virtual void saveBandwidthTest(Message::node ip, float kbps, int state, int window) {}
    virtual void saveHardware(Report::hardware_result hardware, int window) {}

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
    virtual Report::report_result getReport(Message::node ip, bool complete = false) {}
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
    virtual vector<Report::report_result> getReport(bool complete = false) {}

    virtual Report::hardware_result getHardware(Message::node ip) {}
    virtual std::vector<Report::test_result> getLatency(Message::node ip, bool complete = false) {}
    virtual std::vector<Report::test_result> getBandwidth(Message::node ip, bool complete = false) {}
    virtual std::string addNode(Message::node strIp, Report::hardware_result hardware, Message::node *monitored = NULL) {}
    virtual void addReport(Report::report_result result, Message::node *monitored = NULL) {}
    virtual void addReport(std::vector<Report::report_result> results, Message::node ip) {}
    virtual vector<Message::node> removeOldLNodes(int seconds, int &leaders, bool force=false) {}
    virtual vector<Message::node> removeOldNodes(int seconds) {}
    virtual void removeChangeRole(std::vector<Message::node> leaders) {}
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

    MMSelector *selectorA = NULL;
    MMSelector *selectorB = NULL;
    bool sent = false;
    bool ended = false;
};

class MParent2 : virtual public ILeader {
public:

    MConnections conn;
    Message::leader_update update;
    ILeaderStorage *store;

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

    ILeaderStorage* getStorage() {return store;}
    void setStorage(ILeaderStorage* store) {this->store = store;}

};

class MMNode : public Node {
public:
    MMNode() : Node("a", false, 0) {}

    void promote(std::vector<Message::node> nodes) override {
        printf("promote\n");
        sent = true;
    }

    void demote(std::vector<Message::node> nodes) override {
        printf("demote\n");
        sent = true;
    }
    bool sent = false;
};


TEST(SelectorTest, calcSelectionTest) {

    MParent2 parent;

    MMSelector sel(&parent);

    ILeaderStorage * stor = new LeaderStorage(Message::node("0","::1",""));
    parent.setStorage(stor);

    // add a node
    Message::node node("a","1","2");
    

    bool e = sel.initSelection(10);
    EXPECT_EQ(e, true);

    e = sel.initSelection(9);
    EXPECT_EQ(e, false);

    e = sel.updateSelection(Message::leader_update());
    EXPECT_EQ(e, false);

    bool r;

    e = sel.calcSelection(Message::node(),10,r);
    EXPECT_EQ(r, true);
    EXPECT_EQ(e, true);
    usleep(1000000);
    EXPECT_EQ(parent.conn.sent, true);


    sel.stopSelection();
    EXPECT_EQ(e, true);

    e = sel.calcSelection(Message::node(),10,r);
    EXPECT_EQ(e, false);
}

TEST(SelectorTest, interactionTest1) {

    MParent2 parent;

    MMNode node;
    parent.setParent(&node);

    ILeaderStorage * stor = new LeaderStorage(Message::node("0","::1",""));
    parent.setStorage(stor);
    stor->open("leader_node.db");
    MMSelector sel(&parent);
    MMSelector sel2(&parent);

    parent.conn.selectorA = &sel;
    parent.conn.selectorB = &sel2;

    sel2.setId(INT_MAX);
    sel2.setStatus(Selector::Status::READY);

    sel.start2();
    usleep(1000000);
    EXPECT_EQ(sel2.getStatus(), Selector::Status::READY);

    parent.conn.selectorB = &sel;
    parent.conn.selectorA = &sel2;
    sel.setId(INT_MIN);

    sel2.updateSelection(Message::leader_update(vector<Message::node>(),1,0,1));

    sel2.setStatus(Selector::Status::FREE);
    sel2.start2();
    EXPECT_EQ(sel2.getStatus(), Selector::Status::STARTED);
    usleep(25000000);
    printf("sel2: %d\n", sel2.getStatus());
    EXPECT_EQ(sel2.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(sel.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(parent.conn.ended, true);
    EXPECT_EQ(parent.conn.sent, true);
}


TEST(SelectorTest, interactionTest2) {
    MParent2 parent;
    MMNode node;
    parent.setParent(&node);
    MMSelector sel(&parent);
    MMSelector sel2(&parent);

    parent.conn.selectorA = &sel;
    parent.conn.selectorB = &sel2;

unlink("leader_node.db");
    ILeaderStorage * stor = new LeaderStorage(Message::node("0","::1",""));
    parent.setStorage(stor);
    stor->open("leader_node.db");

    float table[10][10] {
        {0,0,0,500,500,500,500,500,500,500},
        {0,5,5,500,500,500,500,500,500,500},
        {0,5,5,500,500,500,500,500,500,500},
        {500,500,500,0,0,0,500,500,500,500},
        {500,500,500,0,5,5,500,500,500,500},
        {500,500,500,0,5,5,500,500,500,500},
        {500,500,500,500,500,500,0,0,0,0},
        {500,500,500,500,500,500,0,1,1,1},
        {500,500,500,500,500,500,0,1,1,1},
        {500,500,500,500,500,500,0,1,1,1},
    };
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    hw.lasttime = stor->getTime()-2;
    for(int i=0; i< 10; i++) {
        stor->addNode(Message::node(to_string(i),to_string(i),""),hw);
    }
    hw.lasttime = stor->getTime();
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
            Report::test_result test(Message::node(to_string(j),val,""),table[i][j],0,stor->getTime());
            
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
        report.hardware = hw;
        reports.push_back(report);
    }
    
    stor->addReport(reports,Message::node("0","::1",""));

    EXPECT_EQ(sel.checkSelection(),true);
    EXPECT_EQ(sel.getStatus(), Selector::Status::STARTED);
    usleep(25000000);

    EXPECT_EQ(sel.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(sel2.getStatus(), Selector::Status::FREE);
    EXPECT_EQ(parent.conn.ended, true);
    EXPECT_EQ(parent.conn.sent, true);
}



TEST(SelectorTest, ScriptTest) {
    MParent2 parent;

    MMNode node;
    parent.setParent(&node);
    

    MMSelector sel(&parent);
    MMSelector sel2(&parent);

    parent.conn.selectorA = &sel;
    parent.conn.selectorB = &sel2;

    unlink("leader_node.db");
    ILeaderStorage * stor = new LeaderStorage(Message::node("0","::1",""));
    parent.setStorage(stor);
    stor->open("leader_node.db");

    float table[10][10] {
        {0,0,0,500,500,500,500,500,500,500},
        {0,5,5,500,500,500,500,500,500,500},
        {0,5,5,500,500,500,500,500,500,500},
        {500,500,500,0,0,0,500,500,500,500},
        {500,500,500,0,5,5,500,500,500,500},
        {500,500,500,0,5,5,500,500,500,500},
        {500,500,500,500,500,500,0,0,0,0},
        {500,500,500,500,500,500,0,1,1,1},
        {500,500,500,500,500,500,0,1,1,1},
        {500,500,500,500,500,500,0,1,1,1},
    };
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    hw.lasttime = stor->getTime()-2;
    for(int i=0; i< 10; i++) {
        stor->addNode(Message::node(to_string(i),to_string(i),""),hw);
    }
    hw.lasttime = stor->getTime();
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
            Report::test_result test(Message::node(to_string(j),val,""),table[i][j],0,stor->getTime());
            
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
        report.hardware = hw;
        reports.push_back(report);
    }
    
    stor->addReport(reports,Message::node("0","::1",""));
    auto nodes = stor->getAllNodes();
    printf("nodes[1]: %d\n", nodes.size());
    for (auto n : nodes) {
        printf("node[1]: %s\n", n.id.c_str());
    }
    // stor->flush();
    usleep(10000);
    Message::leader_update up = sel.selection(153, 0);
    int i = 0;
    while(up.cost > 0.0027 || up.cost < 0.0026) {
        up = sel.selection(153, 0);
        if (i > 2) {
            FAIL();
        }
        i++;
    }
    EXPECT_EQ(up.changes,2);
    EXPECT_NEAR(up.cost, 0.0026,0.0001);
    int vect[] = {0,3,7};
    EXPECT_EQ(up.selected.size(), 3);
    for(int i=0; i<up.selected.size(); i++) {
        EXPECT_EQ(to_string(vect[i]),up.selected[i].id);
    }
}

TEST(SelectorTest, ScriptTest2) {
    MParent2 parent;

    MMNode node;
    node.leaderFormula = 5;
    parent.setParent(&node);

    MMSelector sel(&parent);
    MMSelector sel2(&parent);

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
    parent.setStorage(stor);
    stor->open("leader_node.db");
    
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    hw.lasttime = stor->getTime()-2;

    for(int i=0; i< 10; i++) {
        stor->addNode(Message::node(to_string(i),to_string(i),""),hw);
    }
    hw.lasttime = stor->getTime();

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
            Report::test_result test(Message::node(to_string(j),val,""),table[i][j],0,stor->getTime());
            
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
        report.hardware = hw;
        reports.push_back(report);
    }
    
    stor->addReport(reports,Message::node("0","::1",""));
    usleep(10000);
    Message::leader_update up = sel.selection(153, 0);
    int i = 0;
    EXPECT_EQ(up.changes,4);
    
}
class Base;
class Base {
public:
    Base(int a) {};
    ~Base() {};

    void Display() {
        this->vDisplay(vector<Message::node>());
        cout << "Base: Non-virtual display." << endl;
    };
    virtual void vDisplay(std::vector<Message::node> nodes);
};

void Base::vDisplay(std::vector<Message::node> nodes) {
        cout << "Base: Virtual display." << endl;
}

class Derived : public Base {
public:
    Derived() : Base(3) {};
    void Display() {
        this->vDisplay(vector<Message::node>());
        cout << "Derived: Non-virtual display." << endl;
    };
    virtual void vDisplay(std::vector<Message::node> nodes) {
        cout << "Derived: Virtual display." << endl;
    };
};

TEST(SelectorTest, FollowerTest) {

    Derived d;
    Base *b = &d;
    b->Display();

    Follower A(Message::node("a","1","2"),1);

    MMNode node;
    node.promote(vector<Message::node>());
    A.setParent(&node);
    // A.node->promote(vector<Message::node>());

    vector<Message::node> vect;
    vect.push_back(Message::node("a","1","2"));

    A.changeRole(vect);


    EXPECT_EQ(node.sent, true);

}