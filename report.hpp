#ifndef REPORT_HPP_
#define REPORT_HPP_

#include "rapidjson/document.h"

#include "message.hpp"
#include <vector> 

/**
 * define a report class that holds the json format for the communication
*/
class Report {
public:
    Report();
    ~Report();

    /**
     * hold a hardware test
    */
    typedef struct hardware_result{
        /**
         * number of cores
        */
        int cores = 0;
        /**
         * cpu percentage available
        */
        float mean_free_cpu = 0;
        float var_free_cpu = 0;
        /**
         * total RAM memory in bytes
        */
        int64_t memory = 0;
        /**
         * available RAM
        */
        float mean_free_memory = 0;
        float var_free_memory = 0;
        /**
         * total disk memory in kilobyte
        */
        int64_t disk = 0;
        /**
         * disk memory available
        */
        float mean_free_disk = 0;
        float var_free_disk = 0;
        hardware_result() {}
        hardware_result(int cores, float free_cpu, int64_t memory, float free_memory, int64_t disk, float free_disk) {
            this->cores = cores;
            this->mean_free_cpu = free_cpu;
            this->memory = memory;
            this->mean_free_memory = free_memory;
            this->disk = disk;
            this->mean_free_disk = free_disk;
        }
        hardware_result(int cores, float mean_free_cpu, float var_free_cpu, int64_t memory, float mean_free_memory, float var_free_memory,  int64_t disk,  float mean_free_disk, float var_free_disk) {
            this->cores = cores;
            this->mean_free_cpu = mean_free_cpu;
            this->var_free_cpu = var_free_cpu;
            this->memory = memory;
            this->mean_free_memory = mean_free_memory;
            this->var_free_memory = var_free_memory;
            this->disk = disk;
            this->mean_free_disk = mean_free_disk;
            this->var_free_disk = var_free_disk;
        }
    }hardware_result;
    /**
     * holds an aggregation of network tests
    */
    typedef struct test_result{
        /**
         * the other node the tests are done against
        */
        Message::node target;
        /**
         * the mean of the tests
        */
        float mean;
        /**
         * the variance of the tests
        */
        float variance;
        /**
         * the time of the last test
        */
        int64_t lasttime;
        test_result() {mean = 0; variance = 0; lasttime = 0;}
        test_result(Message::node _target, float _mean, float _variance, long long _lasttime) : target(_target),mean(_mean),variance(_variance), lasttime(_lasttime) {}
    }test_result;
    /**
     * holds a Thing measurement
    */
    typedef struct IoT{
        /**
         * id of the Thing
        */
        std::string id;
        /**
         * description of the Thing
        */
        std::string desc;
        /**
         * the latency of the Thing
        */
        int latency;
        IoT() {id =""; desc =""; latency = 0;}
        IoT(std::string _id, std::string _desc, int _latency): id(_id),desc(_desc),latency(_latency) {}
    }IoT;
    /**
     * the structure of a complete report
    */
    typedef struct report_result{
        /**
         * the tester
        */
        Message::node source;
        /**
         * an hardware test
        */
        hardware_result hardware;
        /**
         * a vector of latency tests
        */
        std::vector<test_result> latency;
        /**
         * a vector of bandwidth tests
        */
        std::vector<test_result> bandwidth;
        /**
         * a vector of Things
        */
        std::vector<IoT> iot;

        report_result() {}
        report_result(Message::node Source, hardware_result Hardware, std::vector<test_result> Latency, std::vector<test_result> Bandwidth, std::vector<IoT> Iot)
        : source(Source), hardware(Hardware), latency(Latency), bandwidth(Bandwidth), iot(Iot) {}
    }report_result;

    /**
     * parse a rapidjson value to generate the report
     * @param data the rapidjson value
    */
    bool parseJson(rapidjson::Value& data);

    /**
     * @return the rapidjson from this report
    */
    rapidjson::Value* getJson();

    /**
     * set the hardware test
     * @param hardware the hardware test
    */
    void setHardware(hardware_result hardware);

    /**
     * set the latency tests
     * @param latency the vector containing the tests
    */
    void setLatency(std::vector<test_result> latency);

    /**
     * set the bandwidth tests
     * @param bandwidth the vector containing the tests
    */
    void setBandwidth(std::vector<test_result> bandwidth);

    /**
     * set the Things
     * @param iots the vector containing the Things
    */
    void setIot(std::vector<IoT> iots);

    /**
     * set the report given a report structure
     * @param report
    */
    void setReport(report_result report);

    /**
     * set this class to comunicate multiple report not just one
     * @param reports the vector of reports
    */
    void setReports(std::vector<report_result> reports);

    /**
     * get the hardware test
     * @param hardware the variable that in case of success is changed
     * @return true in case of success and set the variable
    */
    bool getHardware(hardware_result& hardware);

    /**
     * get the latency tests
     * @param latency the variable that in case of success is changed
     * @return true in case of success and set the variable
    */
    bool getLatency(std::vector<test_result>& latency);

    /**
     * get the bandwidth tests
     * @param bandwidth the variable that in case of success is changed
     * @return true in case of success and set the variable
    */
    bool getBandwidth(std::vector<test_result>& bandwidth);
    
    /**
     * get the Things
     * @param reports the variable that in case of success is changed
     * @return true in case of success and set the variable
    */
    bool getIot(std::vector<IoT>& iots);
    
    /**
     * get the report
     * @param reports the variable that in case of success is changed
     * @return true in case of success and set the variable
    */
    bool getReport(report_result &report);
    
    /**
     * get the reports
     * @param reports the variable that in case of success is changed
     * @return true in case of success and set the variable
    */
    bool getReports(std::vector<report_result> &reports);

private:

    /**
     * contains the rapidjson document to build the json
    */
    rapidjson::Document doc;
};

#endif