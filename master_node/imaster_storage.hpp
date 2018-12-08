#ifndef IMASTER_STORAGE_HPP_
#define IMASTER_STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class IMasterStorage : virtual public IStorage {
public:
    virtual void addNode(std::string strIp, Report::hardware_result) = 0;

    virtual void addReportLatency(std::string strIp, std::vector<Report::test_result> latency) = 0;
    virtual void addReportBandwidth(std::string strIp, std::vector<Report::test_result> bandwidth) = 0;

    virtual void addReport(std::string strIp, Report::hardware_result hardware, std::vector<Report::test_result> latency, std::vector<Report::test_result> bandwidth) = 0;

    virtual std::vector<std::string> getLRHardware(int num, int seconds) = 0;
    virtual std::vector<std::string> getLRLatency(int num, int seconds) = 0;
    virtual std::vector<std::string> getLRBandwidth(int num, int seconds) = 0;
};

#endif