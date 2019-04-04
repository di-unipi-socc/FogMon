#include "master_storage.hpp"
#include "storage.hpp"
#include <gtest/gtest.h>
#include <vector>

#include <stdio.h>

using namespace std;

TEST(StorageTest, RefreshGetNodes) {
    unlink("testA.db");
    Storage storage;
    storage.open("testA.db");

    vector<string> strs;
    strs.push_back("test");
    strs.push_back("testtt");
    storage.refreshNodes(strs);

    vector<string> ris = storage.getNodes();

    int dim = 2;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("test",ris[0]);
        EXPECT_EQ("testtt",ris[1]);
    }else
        FAIL();
}

TEST(StorageTest, UpdateGetNodes) {
    Storage storage;
    storage.open("testA.db");

    vector<string> add;
    add.push_back("test");
    add.push_back("testt");
    vector<string> rem;
    rem.push_back("testtt");
    storage.updateNodes(add,rem);

    vector<string> ris = storage.getNodes();

    int dim = 2;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("test",ris[0]);
        EXPECT_EQ("testt",ris[1]);
    }else
        FAIL();
}

TEST(StorageTest, SaveGetHardware) {
    Storage storage;
    storage.open("testA.db");

    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.free_disk = 10*1000*1000;
    hw.free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.free_memory = 1*1000*1000;
    storage.saveHardware(hw);

    Report::hardware_result hw2 = storage.getHardware();

    int ris = memcmp(&hw, &hw2, sizeof(Report::hardware_result));
    EXPECT_EQ(0, ris);
}

TEST(StorageTest, SaveGetLatency) {
    Storage storage;
    storage.open("testA.db");
    
    storage.saveLatencyTest("test", 10);

    vector<Report::test_result> test = storage.getLatency();
    int dim = 1;
    EXPECT_EQ(dim, test.size());
    if(test.size() == dim) {
        EXPECT_FLOAT_EQ(10, test[0].mean);
        EXPECT_FLOAT_EQ(0, test[0].variance);
        EXPECT_EQ("test", test[0].target);
        EXPECT_GE(time(NULL), test[0].lasttime);
    }else 
        FAIL();
}

TEST(StorageTest, SaveGetBandwidth) {
    Storage storage;
    storage.open("testA.db");
    
    storage.saveBandwidthTest("test", 100.0f, 2);

    vector<Report::test_result> test = storage.getBandwidth();
    int dim = 1;
    EXPECT_EQ(dim, test.size());
    if(test.size() == dim) {
        EXPECT_FLOAT_EQ(100.0f, test[0].mean);
        EXPECT_FLOAT_EQ(0, test[0].variance);
        EXPECT_EQ("test", test[0].target);
        EXPECT_GE(time(NULL), test[0].lasttime);
    }else 
        FAIL();
    Report::test_result last;
    int state = storage.getTestBandwidthState("test", last);
    EXPECT_EQ(state, 2);
    EXPECT_FLOAT_EQ(100.0f, last.mean);
    EXPECT_FLOAT_EQ(0, last.variance);
    EXPECT_EQ("test", last.target);
    EXPECT_GE(time(NULL), last.lasttime);
}

TEST(StorageTest, SetGetToken) {
    Storage storage;
    storage.open("testA.db");

    storage.setToken(100);
    EXPECT_GE(100,storage.hasToken());
}

TEST(StorageTest, GetLRLatency) {
    Storage storage;
    storage.open("testA.db");

    vector<string> ris = storage.getLRLatency(2,100);
    int dim = 1;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("testt",ris[0]);
    }else
        FAIL();
}

TEST(StorageTest, GetLRBandwidth) {
    Storage storage;
    storage.open("testA.db");

    vector<string> ris = storage.getLRBandwidth(2,100);
    int dim = 1;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("testt",ris[0]);
    }else
        FAIL();
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

TEST(StorageMasterTest, AddGetNode) {
    unlink("testB.db");
    MasterStorage storage;
    storage.open("testB.db");
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.free_disk = 10*1000*1000;
    hw.free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.free_memory = 1*1000*1000;
    storage.addNode("test",hw);

    std::vector<std::string> res = storage.getNodes();
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0]);
    else
        FAIL();
}

TEST(StorageMasterTest, FailNullRef) {
    unlink("testB.db");
    MasterStorage storage;
    storage.open("testB.db");
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.free_disk = 10*1000*1000;
    hw.free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.free_memory = 1*1000*1000;
    storage.addNode("test",hw);

    std::vector<std::string> res = storage.getNodes();
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0]);
    else
        FAIL();
    
    Report::test_result test;
    test.mean = 100;
    test.variance = 0;
    test.target = "testt";
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = "test";
    test.mean = 50;
    tests.push_back(test);
    storage.addReportLatency("testtt",tests);

    //missing test-testt

    res = storage.getLRLatency(3, 10);
    dim = 0;
    EXPECT_EQ(dim, res.size());

    storage.addReportBandwidth("testtt",tests);

    tests.clear();
    test.target = "testtt";
    tests.push_back(test);
    test.target = "test";
    tests.push_back(test);
    storage.addReportBandwidth("testt",tests);

    res = storage.getLRBandwidth(4, 10);
    dim = 0;
    EXPECT_EQ(dim, res.size()); 
}

TEST(StorageMasterTest, AddGetMNode) {
    MasterStorage storage;
    storage.open("testB.db");
    //defualt ::1 as MNode
    storage.addMNode("testMNode");

    std::vector<std::string> res = storage.getMNodes();
    int dim = 2;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("testMNode", res[1]);
    else
        FAIL();
}

TEST(StorageMasterTest, GetHardware) {
    MasterStorage storage;
    storage.open("testB.db");


    Report::hardware_result hw;
    hw.cores = 5;
    hw.disk = 100*1000*1000;
    hw.free_disk = 10*1000*1000;
    hw.free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.free_memory = 1*1000*1000;
    storage.addNode("testt",hw);
    storage.addNode("testtAAA",hw,"testMNode"); //node monitored by another MNode

    Report::hardware_result hw1 = storage.getHardware("testt");

    EXPECT_EQ(hw1.cores, hw.cores);
    EXPECT_EQ(hw1.disk, hw.disk);
    EXPECT_EQ(hw1.memory, hw.memory);
    EXPECT_EQ(hw1.free_cpu, hw.free_cpu);
    EXPECT_EQ(hw1.free_disk, hw.free_disk);
    EXPECT_EQ(hw1.free_memory, hw.free_memory);

    hw1 = storage.getHardware("testtAAA");

    EXPECT_EQ(hw1.cores, hw.cores);
    EXPECT_EQ(hw1.disk, hw.disk);
    EXPECT_EQ(hw1.memory, hw.memory);
    EXPECT_EQ(hw1.free_cpu, hw.free_cpu);
    EXPECT_EQ(hw1.free_disk, hw.free_disk);
    EXPECT_EQ(hw1.free_memory, hw.free_memory);
}

TEST(StorageMasterTest, GetLRHardware) {
    MasterStorage storage;
    storage.open("testB.db");
    sleep(2);
    Report::hardware_result hw;
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.free_disk = 10*1000*1000;
    hw.free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.free_memory = 1*1000*1000;
    storage.addNode("testt",hw);
    storage.addNode("testtt",hw);

    vector<string> res = storage.getLRHardware(2, 2);
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0]);
    else
        FAIL();
}

TEST(StorageMasterTest, ReportGetLRLatency) {
    MasterStorage storage;
    storage.open("testB.db");
    
    Report::test_result test;
    test.mean = 100;
    test.variance = 0;
    test.target = "testt";
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = "test";
    test.mean = 50;
    tests.push_back(test);
    storage.addReportLatency("testtt",tests);

    //missing test-testt

    vector<string> res = storage.getLRLatency(3, 10);
    int dim = 2;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim) {
        EXPECT_EQ("test", res[0]);
        EXPECT_EQ("testt", res[1]);
    }
    else
        FAIL();
}

TEST(StorageMasterTest, ReportGetLRBandwidth) {
    MasterStorage storage;
    storage.open("testB.db");

    Report::test_result test;
    test.mean = 100;
    test.variance = 9999.0;
    test.target = "test";
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = "testt";
    tests.push_back(test);
    storage.addReportBandwidth("testtt",tests);

    tests.clear();
    test.target = "testtt";
    tests.push_back(test);
    test.target = "test";
    tests.push_back(test);
    storage.addReportBandwidth("testt",tests);

    vector<string> res = storage.getLRBandwidth(4, 10);
    int dim = 1;
    EXPECT_EQ(dim, res.size());
    if(res.size() == dim)
        EXPECT_EQ("test", res[0]);
    else
        FAIL();
}

TEST(StorageMasterTest, GetLatency) {
    MasterStorage storage;
    storage.open("testB.db");

    vector<Report::test_result> tests = storage.getLatency("testtt");

    int dim = 2;
    EXPECT_EQ(tests.size(),dim);
    
    for(auto test:tests) {
        EXPECT_EQ(test.variance,0);
    }
}

TEST(StorageMasterTest, GetBandiwdth) {
    MasterStorage storage;
    storage.open("testB.db");
    vector<Report::test_result> tests = storage.getBandwidth("testtt");

    int dim = 2;
    EXPECT_EQ(tests.size(),dim);
    
    for(auto test:tests) {
        EXPECT_EQ(test.variance,9999.0);
    }
}

TEST(StorageMasterTest, Complete) {

    FAIL();
}