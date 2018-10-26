#ifndef MASTER_STORAGE_HPP_
#define MASTER_STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class MasterStorage : public IStorage {
private:
    void createTables();
    void addTest(std::string strIpA, std::string strIpB, Report::test_result test, std::string type);
public:
    MasterStorage(std::string path);
    ~MasterStorage();

    std::vector<std::string> getNodes();

    void addNode(std::string strIp, Report::hardware_result);

    void addReportLatency(std::string strIp, std::vector<Report::test_result> latency);
    void addReportBandwidth(std::string strIp, std::vector<Report::test_result> bandwidth);

    void addReport(std::string strIp, Report::hardware_result hardware, std::vector<Report::test_result> latency, std::vector<Report::test_result> bandwidth);

    std::vector<std::string> getLRHardware(int num, int seconds);
    std::vector<std::string> getLRLatency(int num, int seconds);
    std::vector<std::string> getLRBandwidth(int num, int seconds);
};

#endif