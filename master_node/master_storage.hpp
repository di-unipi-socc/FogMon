#ifndef MASTER_STORAGE_HPP_
#define MASTER_STORAGE_HPP_

#include "storage.hpp"
#include "imaster_storage.hpp"
#include "report.hpp"

class MasterStorage : public Storage, virtual public IMasterStorage {
protected:
    virtual void createTables();
    void addTest(std::string strIpA, std::string strIpB, Report::test_result test, std::string type);

public:
    MasterStorage();
    virtual ~MasterStorage();

    std::vector<std::string> getNodes();
    std::vector<std::string> getMNodes();
    std::vector<Report::report_result> getReport();
    Report::report_result getReport(std::string strIp);

    void addReport(std::vector<Report::report_result> results, std::string ip);
    
    void addNode(std::string strIp, Report::hardware_result hardware, std::string monitored = "::1");
    void addMNode(std::string strIp);
    void addIot(std::string strIp, Report::IoT iot);

    void addReportLatency(std::string strIp, std::vector<Report::test_result> latency);
    void addReportBandwidth(std::string strIp, std::vector<Report::test_result> bandwidth);
    void addReportIot(std::string strIp, std::vector<Report::IoT> iots);

    void addReport(Report::report_result result, std::string monitored = "::1");

    std::vector<std::string> getLRHardware(int num, int seconds);
    std::vector<std::string> getLRLatency(int num, int seconds);
    std::vector<std::string> getLRBandwidth(int num, int seconds);

    void complete();

    virtual Report::hardware_result getHardware(std::string ip);
    virtual std::vector<Report::test_result> getLatency(std::string ip);
    virtual std::vector<Report::test_result> getBandwidth(std::string ip);
};

#endif