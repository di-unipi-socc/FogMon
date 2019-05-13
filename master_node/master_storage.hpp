#ifndef MASTER_STORAGE_HPP_
#define MASTER_STORAGE_HPP_

#include "storage.hpp"
#include "imaster_storage.hpp"
#include "message.hpp"
#include "report.hpp"

class MasterStorage : public Storage, virtual public IMasterStorage {
protected:
    virtual void createTables();
    void addTest(std::string strIpA, std::string strIpB, Report::test_result test, std::string type);

    Message::node node;

public:
    MasterStorage(Message::node node);
    virtual ~MasterStorage();

    Message::node getNode();

    std::vector<Message::node> getNodes();
    std::vector<Message::node> getMNodes();
    std::vector<Report::report_result> getReport();
    Report::report_result getReport(Message::node node);

    void addReport(std::vector<Report::report_result> results, Message::node node);
    
    void addNode(Message::node node, Report::hardware_result hardware, Message::node *monitored = NULL);
    void addMNode(Message::node node);
    void addIot(Message::node node, Report::IoT iot);

    void addReportLatency(Message::node node, std::vector<Report::test_result> latency);
    void addReportBandwidth(Message::node node, std::vector<Report::test_result> bandwidth);
    void addReportIot(Message::node node, std::vector<Report::IoT> iots);

    void addReport(Report::report_result result, Message::node *monitored = NULL);

    std::vector<Message::node> getLRHardware(int num, int seconds);
    std::vector<Message::node> getLRLatency(int num, int seconds);
    std::vector<Message::node> getLRBandwidth(int num, int seconds);

    void complete();

    virtual Report::hardware_result getHardware(Message::node node);
    virtual std::vector<Report::test_result> getLatency(Message::node node);
    virtual std::vector<Report::test_result> getBandwidth(Message::node node);
};

#endif