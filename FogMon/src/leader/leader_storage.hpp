#ifndef LEADER_STORAGE_HPP_
#define LEADER_STORAGE_HPP_

#include "storage.hpp"
#include "ileader_storage.hpp"
#include "message.hpp"
#include "report.hpp"

class LeaderStorage : public Storage, virtual public ILeaderStorage {
protected:
    virtual void createTables() override;
    void addTest(Message::node nodeA, Message::node nodeB, Report::test_result test, std::string type);

    Message::node nodeM;

public:
    LeaderStorage(Message::node node);
    virtual ~LeaderStorage() override;

    Message::node getNode() override;

    std::vector<Message::node> getAllNodes() override;
    std::vector<Message::node> getNodes() override;
    std::vector<Message::node> getMNodes() override;
    std::vector<Report::report_result> getReport(bool complete) override;
    Report::report_result getReport(Message::node node, bool complete) override;

    void addReport(std::vector<Report::report_result> results, Message::node node) override;
    
    std::string addNode(Message::node node, Report::hardware_result hardware, Message::node *monitored = NULL) override;
    void addMNode(Message::node node) override;
    void addIot(Message::node node, Report::IoT iot) override;

    void addReportLatency(Message::node node, std::vector<Report::test_result> latency) override;
    void addReportBandwidth(Message::node node, std::vector<Report::test_result> bandwidth) override;
    void addReportIot(Message::node node, std::vector<Report::IoT> iots) override;

    void addReport(Report::report_result result, Message::node *monitored = NULL) override;

    std::vector<Message::node> getMLRHardware(int num, int seconds) override;
    std::vector<Message::node> getMLRLatency(int num, int seconds) override;
    std::vector<Message::node> getMLRBandwidth(int num, int seconds) override;

    virtual std::vector<Message::node> removeOldLNodes(int seconds, int &leaders_num, bool force=false) override;
    virtual std::vector<Message::node> removeOldNodes(int seconds) override;

    virtual void removeChangeRole(std::vector<Message::node> leaders) override;

    void complete() override;

    virtual Report::hardware_result getHardware(Message::node node) override;
    virtual std::vector<Report::test_result> getLatency(Message::node node, bool complete) override;
    virtual std::vector<Report::test_result> getBandwidth(Message::node node, bool complete) override;
};

#endif