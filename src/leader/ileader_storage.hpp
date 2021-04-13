#ifndef ILEADER_STORAGE_HPP_
#define ILEADER_STORAGE_HPP_

#include "istorage.hpp"
#include "message.hpp"
#include "report.hpp"

class ILeaderStorage : virtual public IStorage {
public:
    virtual Message::node getNode() = 0;

    virtual std::string addNode(Message::node node, Report::hardware_result hardware, Message::node *monitored = NULL) = 0;
    virtual void addMNode(Message::node node) = 0;
    virtual void addIot(Message::node node, Report::IoT iot) = 0;

    virtual void addReportLatency(Message::node node, std::vector<Report::test_result> latency) = 0;
    virtual void addReportBandwidth(Message::node node, std::vector<Report::test_result> bandwidth) = 0;
    virtual void addReportIot(Message::node node, std::vector<Report::IoT> iots) = 0;
    virtual void addReport(Report::report_result result, Message::node *monitored = NULL) = 0;

    virtual Report::report_result getReport(Message::node node) = 0;

    virtual std::vector<Message::node> getMLRHardware(int num, int seconds) = 0;
    virtual std::vector<Message::node> getMLRLatency(int num, int seconds) = 0;
    virtual std::vector<Message::node> getMLRBandwidth(int num, int seconds) = 0;

    virtual std::vector<Message::node> getAllNodes() = 0;
    virtual std::vector<Message::node> getMNodes() = 0;
    virtual std::vector<Report::report_result> getReport() = 0;

    virtual void addReport(std::vector<Report::report_result> results, Message::node ip) = 0;

    virtual Report::hardware_result getHardware(Message::node ip) = 0;
    virtual std::vector<Report::test_result> getLatency(Message::node ip) = 0;
    virtual std::vector<Report::test_result> getBandwidth(Message::node ip) = 0;

    virtual std::vector<Message::node> removeOldLNodes(int seconds, bool force=false) = 0;
    virtual std::vector<Message::node> removeOldNodes(int seconds) = 0;

    virtual void removeChangeRole(std::vector<Message::node> leaders) = 0;

    virtual void complete() = 0;
};

#endif