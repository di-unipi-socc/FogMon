#ifndef IMASTER_STORAGE_HPP_
#define IMASTER_STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class IMasterStorage : virtual public IStorage {
public:
    virtual void addNode(std::string strIp, Report::hardware_result hardware, std::string monitored = "::1") = 0;
    virtual void addMNode(std::string strIp) = 0;

    virtual void addReportLatency(std::string strIp, std::vector<Report::test_result> latency) = 0;
    virtual void addReportBandwidth(std::string strIp, std::vector<Report::test_result> bandwidth) = 0;

    virtual void addReport(Report::report_result result, std::string monitored = "::1") = 0;

    virtual Report::report_result getReport(std::string strIp) = 0;

    virtual std::vector<std::string> getLRHardware(int num, int seconds) = 0;
    virtual std::vector<std::string> getLRLatency(int num, int seconds) = 0;
    virtual std::vector<std::string> getLRBandwidth(int num, int seconds) = 0;

    virtual std::vector<std::string> getMNodes() = 0;
    virtual std::vector<Report::report_result> getReport() = 0;

    virtual void addReport(std::vector<Report::report_result> results, std::string ip) = 0;

    virtual Report::hardware_result getHardware(std::string ip) = 0;
    virtual std::vector<Report::test_result> getLatency(std::string ip) = 0;
    virtual std::vector<Report::test_result> getBandwidth(std::string ip) = 0;

    virtual void complete() = 0;
};

#endif