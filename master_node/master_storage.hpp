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

    void addReport(std::vector<Report::report_result> results);
    
    void addNode(std::string strIp, Report::hardware_result hardware, bool monitored = true);
    void addMNode(std::string strIp);

    void addReportLatency(std::string strIp, std::vector<Report::test_result> latency);
    void addReportBandwidth(std::string strIp, std::vector<Report::test_result> bandwidth);

    void addReport(Report::report_result result, bool monitored = true);

    std::vector<std::string> getLRHardware(int num, int seconds);
    std::vector<std::string> getLRLatency(int num, int seconds);
    std::vector<std::string> getLRBandwidth(int num, int seconds);

    virtual Report::hardware_result getHardware(std::string ip);
    virtual std::vector<Report::test_result> getLatency(std::string ip);
    virtual std::vector<Report::test_result> getBandwidth(std::string ip);
};

#endif