#include "master_storage.hpp"
#include "storage.hpp"
#include <gtest/gtest.h>
#include <vector>

#include <stdio.h>

using namespace std;

TEST(StorageTest, RefreshGetNodes) {
    Storage storage("testA.db");

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
    Storage storage("testA.db");

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
    Storage storage("testA.db");
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
    Storage storage("testA.db");
    
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
    Storage storage("testA.db");
    
    storage.saveBandwidthTest("test", 100.0f);

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
}

TEST(StorageTest, SetGetToken) {
    Storage storage("testA.db");

    storage.setToken(100);
    EXPECT_GE(100,storage.hasToken());
}

TEST(StorageTest, GetLRLatency) {
    Storage storage("testA.db");

    vector<string> ris = storage.getLRLatency(2,100);
    int dim = 1;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("testt",ris[0]);
    }else
        FAIL();
}

TEST(StorageTest, GetLRBandwidth) {
    Storage storage("testA.db");

    vector<string> ris = storage.getLRBandwidth(2,100);
    int dim = 1;
    EXPECT_EQ(dim, ris.size());
    if(ris.size() == dim) {
        EXPECT_EQ("testt",ris[0]);
    }else
        FAIL();
}

TEST(StorageMasterTest, SaveGetNode) {
    unlink("testB.db");
    MasterStorage storage("testB.db");
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

TEST(StorageMasterTest, GetLRHardware) {
    MasterStorage storage("testB.db");
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
    MasterStorage storage("testB.db");
    
    Report::test_result test;
    test.mean = 100;
    test.variance = 0;
    test.target = "testt";
    test.lasttime = time(NULL);

    vector<Report::test_result> tests;
    tests.push_back(test);
    test.target = "test";
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
    MasterStorage storage("testB.db");

    Report::test_result test;
    test.mean = 100;
    test.variance = 0;
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

