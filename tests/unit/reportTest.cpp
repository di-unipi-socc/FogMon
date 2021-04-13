#include <gtest/gtest.h>
#include "report.hpp"

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <stdio.h>
using namespace std;

TEST(ReportTest, ParseGetJson) {
    Report report;
    string json = "{\"ciao\":\"hello\"}";
    rapidjson::Document document;
    document.Parse(json.c_str());
    report.parseJson(document);

    rapidjson::Value * val = report.getJson();

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer (s);
    val->Accept (writer);
    std::string str (s.GetString());
    EXPECT_EQ(json, str);
}

TEST(ReportTest, SetGetHardware) {
    Report report;
    Report::hardware_result hw;
    memset(&hw,0,sizeof(Report::hardware_result));
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;
    report.setHardware(hw);

    Report::hardware_result hw2;
    memset(&hw2,0,sizeof(Report::hardware_result));
    int ris = report.getHardware(hw2);
    EXPECT_EQ(true, ris);

    ris = memcmp(&hw, &hw2, sizeof(Report::hardware_result));

    EXPECT_EQ(0, ris);
}

TEST(ReportTest, SetGetLatency) {
    Report report;
    Report::test_result test;
    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 100.0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> tests;
    tests.push_back(test);
    report.setLatency(tests);

    vector<Report::test_result> tests2;

    int ris = report.getLatency(tests2);
    EXPECT_EQ(true, ris);
    for(int i=0; i<tests.size(); i++) {
        EXPECT_EQ(tests[i].mean, tests2[i].mean); 
        EXPECT_EQ(tests[i].variance, tests2[i].variance); 
        EXPECT_EQ(tests[i].lasttime, tests2[i].lasttime); 
        EXPECT_EQ(tests[i].target, tests2[i].target);
    }
}

TEST(ReportTest, SetGetBandwidth) {
    Report report;
    Report::test_result test;
    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 30.0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> tests;
    tests.push_back(test);
    report.setBandwidth(tests);

    vector<Report::test_result> tests2;

    int ris = report.getBandwidth(tests2);
    EXPECT_EQ(true, ris);
    for(int i=0; i<tests.size(); i++) {
        EXPECT_EQ(tests[i].mean, tests2[i].mean); 
        EXPECT_EQ(tests[i].variance, tests2[i].variance); 
        EXPECT_EQ(tests[i].lasttime, tests2[i].lasttime); 
        EXPECT_EQ(tests[i].target, tests2[i].target);
    }
}

TEST(ReportTest, SetGetReport) {
    Report report;
    Report::test_result test;
    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> testsL,tests2L;
    testsL.push_back(test);

    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 56789.0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> testsB,tests2B;
    testsB.push_back(test);

    Report::hardware_result hw,hw2;
    memset(&hw,0,sizeof(Report::hardware_result));
    memset(&hw2,0,sizeof(Report::hardware_result));
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;

    Report::IoT iot("aaa","aaa",1);
    Report::IoT iot1("aaa43r","aaa3asd",55);

    vector<Report::IoT> iots;
    vector<Report::IoT> iots2;
    iots.push_back(iot);
    iots.push_back(iot1);

    report.setLatency(testsL);
    report.setBandwidth(testsB);
    report.setHardware(hw);
    report.setIot(iots);

    rapidjson::Value * val = report.getJson();

    Report report2;
    report2.parseJson(*val);

    EXPECT_EQ(true, report2.getHardware(hw2));
    EXPECT_EQ(true, report2.getLatency(tests2L));
    EXPECT_EQ(true, report2.getBandwidth(tests2B));
    EXPECT_EQ(true, report2.getIot(iots2));
    bool ris;
    ris = memcmp(&hw, &hw2, sizeof(Report::hardware_result));
    EXPECT_EQ(0, ris);
    
    for(int i=0; i<testsL.size(); i++) {
        EXPECT_EQ(testsL[i].mean, tests2L[i].mean); 
        EXPECT_EQ(testsL[i].variance, tests2L[i].variance); 
        EXPECT_EQ(testsL[i].lasttime, tests2L[i].lasttime); 
        EXPECT_EQ(testsL[i].target, tests2L[i].target);
    }
    for(int i=0; i<testsB.size(); i++) {
        EXPECT_EQ(testsB[i].mean, tests2B[i].mean); 
        EXPECT_EQ(testsB[i].variance, tests2B[i].variance); 
        EXPECT_EQ(testsB[i].lasttime, tests2B[i].lasttime); 
        EXPECT_EQ(testsB[i].target, tests2B[i].target);
    }

    for(int i=0; i<iots.size(); i++) {
        EXPECT_EQ(iots[i].desc, iots2[i].desc); 
        EXPECT_EQ(iots[i].id, iots2[i].id); 
        EXPECT_EQ(iots[i].latency, iots2[i].latency); 
    }
}

TEST(ReportTest, SetGetReport2) {
    Report report;
    Report::test_result test;
    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> testsL,tests2L;
    testsL.push_back(test);

    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> testsB,tests2B;
    testsB.push_back(test);

    Report::hardware_result hw;
    memset(&hw,0,sizeof(Report::hardware_result));
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;

    Report::report_result result;

    result.source = Message::node("ciao","::1","1234");
    result.hardware = hw;
    result.latency = testsL;
    result.bandwidth = testsB;

    report.setReport(result);

    rapidjson::Value * val = report.getJson();

    Report report2;
    report2.parseJson(*val);

    Report::report_result result2;
    memset(&result2.hardware,0,sizeof(Report::hardware_result));

    EXPECT_EQ(true, report2.getReport(result2));
    int ris;
    ris = memcmp(&result.hardware, &result2.hardware, sizeof(Report::hardware_result));
    EXPECT_EQ(0, ris);

    for(int i=0; i<result.latency.size(); i++) {
        EXPECT_EQ(result.latency[i].mean, result2.latency[i].mean); 
        EXPECT_EQ(result.latency[i].variance, result2.latency[i].variance); 
        EXPECT_EQ(result.latency[i].lasttime, result2.latency[i].lasttime); 
        EXPECT_EQ(result.latency[i].target, result2.latency[i].target);
    }
    for(int i=0; i<result.bandwidth.size(); i++) {
        EXPECT_EQ(result.bandwidth[i].mean, result2.bandwidth[i].mean); 
        EXPECT_EQ(result.bandwidth[i].variance, result2.bandwidth[i].variance); 
        EXPECT_EQ(result.bandwidth[i].lasttime, result2.bandwidth[i].lasttime); 
        EXPECT_EQ(result.bandwidth[i].target, result2.bandwidth[i].target);
    }
}

TEST(ReportTest, SetGetReports) {
    
    vector<Report::report_result> results;

    Report report;
    Report::test_result test;
    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> testsL,tests2L;
    testsL.push_back(test);

    test.lasttime = time(NULL);
    test.mean = 10;
    test.variance = 0;
    test.target = Message::node("ciao","::1","1234");
    vector<Report::test_result> testsB,tests2B;
    testsB.push_back(test);

    Report::hardware_result hw,hw2;
    memset(&hw,0,sizeof(Report::hardware_result));
    
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.mean_free_disk = 10*1000*1000;
    hw.mean_free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.mean_free_memory = 1*1000*1000;

    Report::report_result result;

    result.source = Message::node("ciao","::1","1234");
    result.hardware = hw;
    result.latency = testsL;
    result.bandwidth = testsB;

    results.push_back(result);


    report.setReports(results);

    rapidjson::Value * val = report.getJson();

    Report report2;
    report2.parseJson(*val);

    vector<Report::report_result> results2;

    EXPECT_EQ(true, report2.getReports(results2));
    bool ris;
    Report::report_result result2 = results2[0];
    ris = memcmp(&result.hardware, &result2.hardware, sizeof(Report::hardware_result));
    EXPECT_EQ(0, ris);

    for(int i=0; i<result.latency.size(); i++) {
        EXPECT_EQ(result.latency[i].mean, result2.latency[i].mean); 
        EXPECT_EQ(result.latency[i].variance, result2.latency[i].variance); 
        EXPECT_EQ(result.latency[i].lasttime, result2.latency[i].lasttime); 
        EXPECT_EQ(result.latency[i].target, result2.latency[i].target);
    }
    for(int i=0; i<result.bandwidth.size(); i++) {
        EXPECT_EQ(result.bandwidth[i].mean, result2.bandwidth[i].mean); 
        EXPECT_EQ(result.bandwidth[i].variance, result2.bandwidth[i].variance); 
        EXPECT_EQ(result.bandwidth[i].lasttime, result2.bandwidth[i].lasttime); 
        EXPECT_EQ(result.bandwidth[i].target, result2.bandwidth[i].target);
    }
}