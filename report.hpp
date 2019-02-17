#ifndef REPORT_HPP_
#define REPORT_HPP_

#include "rapidjson/document.h"
#include <vector> 

class Report {
public:
    Report();
    ~Report();

    typedef struct {
        int cores;
        float free_cpu;
        int memory;
        int free_memory;
        int disk;
        int free_disk;
    }hardware_result;

    typedef struct {
        std::string target;
        float mean;
        float variance;
        long lasttime;
    }test_result;

    typedef struct report_result{
        std::string ip;
        hardware_result hardware;
        std::vector<test_result> latency;
        std::vector<test_result> bandwidth;

        report_result() {}
        report_result(std::string Ip, hardware_result Hardware, std::vector<test_result> Latency, std::vector<test_result> Bandwidth)
        : ip(Ip), hardware(Hardware), latency(Latency), bandwidth(Bandwidth) {}
    }report_result;

    bool parseJson(rapidjson::Value& data);
    rapidjson::Value* getJson();

    void setHardware(hardware_result hardware);
    void setLatency(std::vector<test_result> latency);
    void setBandwidth(std::vector<test_result> bandwidth);
    void setReport(report_result report);

    void setReports(std::vector<report_result> reports);

    bool getHardware(hardware_result& hardware);
    bool getLatency(std::vector<test_result>& latency);
    bool getBandwidth(std::vector<test_result>& bandwidth);
    bool getReport(report_result &report);
    
    bool getReports(std::vector<report_result> &reports);

private:
    rapidjson::Document doc;
};

#endif