#ifndef ISTORAGE_HPP_
#define ISTORAGE_HPP_

#include <sqlite3.h>
#include <string>
#include <vector>
#include "report.hpp"

class IStorage {
protected:
    virtual void createTables() = 0;

    sqlite3 *db;
    
public:
    virtual ~IStorage();

    void open(std::string path);
    void close();
    virtual std::vector<std::string> getNodes() = 0;

    virtual Report::hardware_result getHardware() = 0;
    virtual std::vector<Report::test_result> getLatency() = 0;
    virtual std::vector<Report::test_result> getBandwidth() = 0;

    virtual void saveLatencyTest(std::string ip, int ms) = 0;
    virtual void saveBandwidthTest(std::string ip, float kbps) = 0;
    virtual void saveHardware(Report::hardware_result hardware) = 0;

    virtual void refreshNodes(std::vector<std::string> nodes) = 0;
    virtual void updateNodes(std::vector<std::string> add, std::vector<std::string> rem) = 0;
    
    virtual std::vector<std::string> getLRLatency(int num, int seconds) = 0;
    virtual std::vector<std::string> getLRBandwidth(int num, int seconds) = 0;

    virtual void setToken(int duration) = 0;
    virtual int hasToken() = 0;
};



#endif