#ifndef REPORT_HPP_
#define REPORT_HPP_

#include "rapidjson/document.h"
#include <vector> 

class Report {
public:
    Report();
    ~Report();

    typedef struct hardware_result{
        int cores;
        float free_cpu;
        int memory;
        int free_memory;
        int disk;
        int free_disk;
        hardware_result() {cores = 0; free_cpu = 0; memory = 0; free_memory = 0; disk = 0; free_disk = 0;}
        hardware_result(int cores, float free_cpu, int memory, int free_memory, int disk, int free_disk) {
            this->cores = cores;
            this->free_cpu = free_cpu;
            this->memory = memory;
            this->free_memory = free_memory;
            this->disk = disk;
            this->free_disk = free_disk;
        }
    }hardware_result;

    typedef struct test_result{
        std::string target;
        float mean;
        float variance;
        int64_t lasttime;
        test_result() {mean = 0; variance = 0; lasttime = 0;}
        test_result(std::string _target, float _mean, float _variance, long long _lasttime) : target(_target),mean(_mean),variance(_variance), lasttime(_lasttime) {}
    }test_result;

    typedef struct {
        std::string id;
        std::string desc;
        int latency;
    }IoT;

    typedef struct report_result{
        std::string ip;
        hardware_result hardware;
        std::vector<test_result> latency;
        std::vector<test_result> bandwidth;
        std::vector<IoT> iot;

        report_result() {}
        report_result(std::string Ip, hardware_result Hardware, std::vector<test_result> Latency, std::vector<test_result> Bandwidth, std::vector<IoT> Iot)
        : ip(Ip), hardware(Hardware), latency(Latency), bandwidth(Bandwidth), iot(Iot) {}
    }report_result;

    bool parseJson(rapidjson::Value& data);
    rapidjson::Value* getJson();

    void setHardware(hardware_result hardware);
    void setLatency(std::vector<test_result> latency);
    void setBandwidth(std::vector<test_result> bandwidth);
    void setIot(std::vector<IoT> iots);
    void setReport(report_result report);

    void setReports(std::vector<report_result> reports);

    bool getHardware(hardware_result& hardware);
    bool getLatency(std::vector<test_result>& latency);
    bool getBandwidth(std::vector<test_result>& bandwidth);
    bool getIot(std::vector<IoT>& iots);
    bool getReport(report_result &report);
    
    bool getReports(std::vector<report_result> &reports);

private:
    rapidjson::Document doc;
};

#endif