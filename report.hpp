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
    }test_result;

    bool parseJson(rapidjson::Value& data);
    rapidjson::Value* getJson();

    void setHardware(hardware_result hardware);
    void setLatency(std::vector<test_result> latency);
    void setBandwidth(std::vector<test_result> bandwidth);

    bool getHardware(hardware_result& hardware);
    bool getLatency(std::vector<test_result>& latency);
    bool getBandwidth(std::vector<test_result>& bandwidth);

private:
    rapidjson::Document doc;
};

#endif