#include "leader_storage.hpp"
#include "storage.hpp"
#include <gtest/gtest.h>
#include <vector>

#include <stdio.h>

using namespace std;

Message::node node_test("test","::1","1234");
Message::node node_testt("testt","::1","12345");
Message::node node_testtt("testtt","::1","123456");

TEST(StorageTest, RefreshGetNodes) {
    unlink("testA.db");
    Storage storage;
    storage.open("testA.db");

    vector<Message::node> strs;
    strs.push_back(node_test);
    strs.push_back(node_testtt);
    storage.refreshNodes(strs);

    vector<Message::node> ris = storage.getNodes();

    int dim = 2;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("test",ris[0].id);
        EXPECT_EQ("testtt",ris[1].id);
    }else
        FAIL();
}

TEST(StorageTest, UpdateGetNodes) {
    Storage storage;
    storage.open("testA.db");

    vector<Message::node> add;
    add.push_back(node_test);
    add.push_back(node_testt);
    vector<Message::node> rem;
    rem.push_back(node_testtt);
    storage.updateNodes(add,rem);

    vector<Message::node> ris = storage.getNodes();

    int dim = 2;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("test",ris[0].id);
        EXPECT_EQ("testt",ris[1].id);
    }else
        FAIL();
}

TEST(StorageTest, SaveGetHardware) {
    Storage storage;
    storage.open("testA.db");

    Report::hardware_result hw;
    memset(&hw,0,sizeof(Report::hardware_result));
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    storage.saveHardware(hw);

    Report::hardware_result hw2 = storage.getHardware();

    int ris = memcmp(&hw, &hw2, sizeof(Report::hardware_result));
    EXPECT_EQ(0, ris);
}

TEST(StorageTest, SaveGetLatency) {
    Storage storage;
    storage.open("testA.db");
    
    storage.saveLatencyTest(node_test, 10);

    vector<Report::test_result> test = storage.getLatency();
    int dim = 1;
    EXPECT_EQ(dim, test.size());
    if(test.size() == dim) {
        EXPECT_FLOAT_EQ(10, test[0].mean);
        EXPECT_FLOAT_EQ(0, test[0].variance);
        EXPECT_EQ("test", test[0].target.id);
        EXPECT_GE(time(NULL), test[0].lasttime);
    }else 
        FAIL();
}

TEST(StorageTest, SaveGetBandwidth) {
    Storage storage;
    storage.open("testA.db");
    
    storage.saveBandwidthTest(node_test, 100.0f, 2);

    vector<Report::test_result> test = storage.getBandwidth();
    int dim = 1;
    EXPECT_EQ(dim, test.size());
    if(test.size() == dim) {
        EXPECT_FLOAT_EQ(100.0f, test[0].mean);
        EXPECT_FLOAT_EQ(0, test[0].variance);
        EXPECT_EQ("test", test[0].target.id);
        EXPECT_GE(time(NULL), test[0].lasttime);
    }else 
        FAIL();
    Report::test_result last;
    int state = storage.getTestBandwidthState(node_test, last);
    EXPECT_EQ(state, 2);
    EXPECT_FLOAT_EQ(100.0f, last.mean);
    EXPECT_FLOAT_EQ(0, last.variance);
    EXPECT_EQ("test", last.target.id);
    EXPECT_GE(time(NULL)+60, last.lasttime); //+60 because time(NULL) != DATETIME('now')
}

TEST(StorageTest, GetLRLatency) {
    Storage storage;
    storage.open("testA.db");

    vector<Message::node> ris = storage.getLRLatency(2,100);
    int dim = 1;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("testt",ris[0].id);
    }else
        FAIL();
}

TEST(StorageTest, GetLRBandwidth) {
    Storage storage;
    storage.open("testA.db");

    vector<Message::node> ris = storage.getLRBandwidth(2,100);
    int dim = 1;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("testt",ris[0].id);
    }else
        FAIL();
}

TEST(StorageTest, SaveState) {
    Storage storage;
    storage.open("testA.db");

    storage.saveState();
    //TODO test if correct
}

class TestIoT : public IThing {
public:
    string id;
    string desc;
    int latency;

    TestIoT(string id, string desc, int latency) {
        this->id = id;
        this->desc = desc;
        this->latency = latency;
    }

    string getId() {
        return id;
    }

    string getDesc() {
        return desc;
    }

    int getLatency() {
        return latency;
    }

    void monitor() {}

};

TEST(StorageTest, AddGetIots) {
    Storage storage;
    storage.open("testA.db");
    TestIoT iot("testid","testdesc",10);
    storage.addIot(&iot);

    vector<Report::IoT> vect = storage.getIots();

    if(!vect.empty()) {
        EXPECT_EQ(vect[0].id, "testid");
        EXPECT_EQ(vect[0].desc, "testdesc");
        EXPECT_EQ(vect[0].latency, 10);
    }
}

Message::node nodeA("idM","::1","1234567");
Message::node nodeB("testMNode","::1","12345678");
Message::node nodeB1("testAAA","::1","1273691");

TEST(StorageLeaderTest, AddGetNode) {
    unlink("testB.db");
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    storage.addNode(Message::node("1234321","::1","1234"),hw);
    std::vector<Message::node> res = storage.getNodes();
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("1234321", res[0].id);
    else
        FAIL();
}

TEST(StorageLeaderTest, FailNullRef) {
    unlink("testB.db");
    LeaderStorage storage(nodeA);
    storage.open("testB.db");

    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    storage.addNode(node_test,hw);
    storage.addNode(node_testt,hw);
    storage.addNode(node_testtt,hw);

    std::vector<Message::node> res = storage.getNodes();
    int dim = 3;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0].id);
    else
        FAIL();
    
    Report::test_result test;
    test.mean = 100;
    test.variance = 0;
    test.target = node_testt;
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = node_test;
    test.mean = 50;
    tests.push_back(test);
    storage.addReportLatency(node_testtt,tests);

    //missing test-testt and testt-testtt

    res = storage.getMLRLatency(3, 10);
    dim = 2;
    EXPECT_EQ(dim, res.size());

    storage.addReportBandwidth(node_testtt,tests);

    tests.clear();
    test.target = node_testtt;
    tests.push_back(test);
    test.target = node_test;
    tests.push_back(test);
    storage.addReportBandwidth(node_testt,tests);

    res = storage.getMLRBandwidth(4, 10);
    dim = 1;
    EXPECT_EQ(dim, res.size()); 
}

TEST(StorageLeaderTest, AddGetMNode) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    //defualt ::1 as MNode
    storage.addMNode(nodeB);

    std::vector<Message::node> res = storage.getMNodes();
    int dim = 2;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        if(res[1].id != nodeB.id) {
            EXPECT_EQ(nodeB.id, res[0].id);
        }else
        {
            EXPECT_EQ(nodeB.id, res[1].id);
        }
        
    else
        FAIL();
}

TEST(StorageLeaderTest, GetHardware) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");


    Report::hardware_result hw;
    hw.cores = 5;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    storage.addNode(node_testt,hw);
    storage.addNode(nodeB1,hw,&nodeB); //node monitored by another MNode

    Report::hardware_result hw1 = storage.getHardware(node_testt);

    EXPECT_EQ(hw1.cores, hw.cores);
    EXPECT_EQ(hw1.disk, hw.disk);
    EXPECT_EQ(hw1.memory, hw.memory);
    EXPECT_EQ(hw1.mean_free_cpu, hw.mean_free_cpu);
    EXPECT_EQ(hw1.mean_free_disk, hw.mean_free_disk);
    EXPECT_EQ(hw1.mean_free_memory, hw.mean_free_memory);

    hw1 = storage.getHardware(nodeB1);

    EXPECT_EQ(hw1.cores, hw.cores);
    EXPECT_EQ(hw1.disk, hw.disk);
    EXPECT_EQ(hw1.memory, hw.memory);
    EXPECT_EQ(hw1.mean_free_cpu, hw.mean_free_cpu);
    EXPECT_EQ(hw1.mean_free_disk, hw.mean_free_disk);
    EXPECT_EQ(hw1.mean_free_memory, hw.mean_free_memory);
}

TEST(StorageLeaderTest, GetMLRHardware) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    sleep(2);
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    storage.addNode(node_testt,hw);
    storage.addNode(node_testtt,hw);

    vector<Message::node> res = storage.getMLRHardware(2, 2);
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0].id);
    else
        FAIL();
}

TEST(StorageLeaderTest, ReportGetMLRLatency) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    
    Report::test_result test;
    test.mean = 100;
    test.variance = 0;
    test.target = node_testt;
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = node_test;
    test.mean = 50;
    tests.push_back(test);
    storage.addReportLatency(node_testtt,tests);

    //missing test-testt

    vector<Message::node> res = storage.getMLRLatency(3, 10);
    int dim = 2;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim) {
        EXPECT_EQ("test", res[0].id);
        EXPECT_EQ("testt", res[1].id);
    }
    else
        FAIL();
}

TEST(StorageLeaderTest, ReportGetMLRBandwidth) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");

    Report::test_result test;
    test.mean = 100;
    test.variance = 9999.0;
    test.target = node_test;
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = node_testt;
    tests.push_back(test);
    storage.addReportBandwidth(node_testtt,tests);

    tests.clear();
    test.target = node_testtt;
    tests.push_back(test);
    test.target = node_test;
    tests.push_back(test);
    storage.addReportBandwidth(node_testt,tests);

    vector<Message::node> res = storage.getMLRBandwidth(4, 10);
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0].id);
    else
        FAIL();
}

TEST(StorageLeaderTest, GetLatency) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");

    vector<Report::test_result> tests = storage.getLatency(node_testtt);

    int dim = 2;
    EXPECT_EQ(tests.size(),dim);
    
    for(auto test:tests) {
        EXPECT_EQ(test.variance,0);
    }
}

TEST(StorageLeaderTest, GetBandiwdth) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    vector<Report::test_result> tests = storage.getBandwidth(node_testtt);

    int dim = 2;
    EXPECT_EQ(tests.size(),dim);
    
    for(auto test:tests) {
        EXPECT_EQ(test.variance,9999.0);
    }
}

TEST(StorageLeaderTest, Complete) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    storage.addMNode(nodeB);
    storage.addNode(nodeB,hw,&nodeB);
    storage.addNode(nodeA,hw,&nodeA);

    Report::test_result test;
    test.mean = 100;
    test.variance = 9999.0;
    test.target = nodeB1;
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    storage.addReportBandwidth(nodeB,tests);
    storage.addReportLatency(nodeB,tests);

    tests.clear();
    test.target = nodeB;
    tests.push_back(test);
    storage.addReportBandwidth(nodeB1,tests);
    storage.addReportLatency(nodeB1,tests);

    tests.clear();
    test.target = nodeA;
    tests.push_back(test);
    storage.addReportBandwidth(nodeB,tests);
    storage.addReportLatency(nodeB,tests);

    tests.clear();
    test.target = nodeB;
    tests.push_back(test);
    storage.addReportBandwidth(nodeA,tests);
    storage.addReportLatency(nodeA,tests);

    tests.clear();
    test.target = nodeA;
    tests.push_back(test);
    storage.addReportBandwidth(node_test,tests);
    storage.addReportLatency(node_test,tests);

    tests.clear();
    test.target = nodeA;
    tests.push_back(test);
    storage.addReportBandwidth(node_testt,tests);
    storage.addReportLatency(node_testt,tests);

    tests.clear();
    test.target = nodeA;
    tests.push_back(test);
    storage.addReportBandwidth(node_testtt,tests);
    storage.addReportLatency(node_testtt,tests);

    tests.clear();
    test.target = node_test;
    tests.push_back(test);
    test.target = node_testt;
    tests.push_back(test);
    test.target = node_testtt;
    tests.push_back(test);
    storage.addReportBandwidth(nodeA,tests);
    storage.addReportLatency(nodeA,tests);

    storage.complete();

    vector<Report::test_result> res = storage.getLatency(node_test);
    int dim = 4;
    EXPECT_EQ(res.size(),dim);
    vector<Message::node> nodes = {node_testtt, nodeA, nodeB1, nodeB};
    for(auto &node : nodes) {
        bool found = false;
        for(auto &test : res) {
            if(test.target == node) {
                found = true;
            }
        }
        if(!found) {
            FAIL() << "Not found " << node.id << endl;
        }
    }

    for(auto &test : res) {
        if(test.target == nodeB1) {
            EXPECT_EQ(test.mean, 300);
        }else if(test.target == nodeB) {
            EXPECT_EQ(test.mean, 200);
        }
    }
}

TEST(StorageLeaderTest, RemoveOldNodes) {
    LeaderStorage storage(nodeA);
    storage.open("testB.db");
    storage.removeOldNodes(0);
}