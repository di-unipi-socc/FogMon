#ifndef LEADER_STORAGE_HPP_
#define LEADER_STORAGE_HPP_

#include "storage.hpp"
#include "ileader_storage.hpp"
#include "message.hpp"
#include "report.hpp"

class LeaderStorage : public Storage, virtual public ILeaderStorage {
protected:
    virtual void createTables();
    void addTest(Message::node nodeA, Message::node nodeB, Report::test_result test, std::string type);

    Message::node nodeM;

public:
    LeaderStorage(Message::node node);
    virtual ~LeaderStorage();

    Message::node getNode();

    std::vector<Message::node> getAllNodes();
    std::vector<Message::node> getNodes();
    std::vector<Message::node> getMNodes();
    std::vector<Report::report_result> getReport();
    Report::report_result getReport(Message::node node);

    void addReport(std::vector<Report::report_result> results, Message::node node);
    
    std::string addNode(Message::node node, Report::hardware_result hardware, Message::node *monitored = NULL);
    void addMNode(Message::node node);
    void addIot(Message::node node, Report::IoT iot);

    void addReportLatency(Message::node node, std::vector<Report::test_result> latency);
    void addReportBandwidth(Message::node node, std::vector<Report::test_result> bandwidth);
    void addReportIot(Message::node node, std::vector<Report::IoT> iots);

    void addReport(Report::report_result result, Message::node *monitored = NULL);

    std::vector<Message::node> getMLRHardware(int num, int seconds);
    std::vector<Message::node> getMLRLatency(int num, int seconds);
    std::vector<Message::node> getMLRBandwidth(int num, int seconds);

    virtual std::vector<Message::node> removeOldLNodes(int seconds, bool force=false);
    virtual std::vector<Message::node> removeOldNodes(int seconds);

    virtual void removeChangeRole(std::vector<Message::node> leaders);

    void complete();

    virtual Report::hardware_result getHardware(Message::node node);
    virtual std::vector<Report::test_result> getLatency(Message::node node);
    virtual std::vector<Report::test_result> getBandwidth(Message::node node);
};

#endif