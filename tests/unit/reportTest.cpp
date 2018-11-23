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
    hw.cores = 4;
    hw.disk = 100*1000*1000;
    hw.free_disk = 10*1000*1000;
    hw.free_cpu = 0.4;
    hw.memory = 10*1000*1000;
    hw.free_memory = 1*1000*1000;
    report.setHardware(hw);

    Report::hardware_result hw2;

    int ris = report.getHardware(hw2);
    EXPECT_EQ(true, ris);

    ris = memcmp(&hw, &hw2, sizeof(Report::hardware_result));
    EXPECT_EQ(0, ris);
}

TEST(ReportTest, SetGetLatency) {
   
}

TEST(ReportTest, SetGetBandwidth) {
    
}