#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"

class Storage : virtual public IStorage {
protected:
    virtual void createTables() override;

    void filterRecv(std::vector<Message::node> &list);
    void filterSend(std::vector<Message::node> &list);
    void filterSend(std::vector<Report::test_result> &list);
    void filterSend(Report::test_result &list);

    std::string ip;

public:
    Storage();
    virtual ~Storage() override;

    void setFilter(std::string ip) override;

    virtual Report::hardware_result getHardware() override;
    virtual std::vector<Report::test_result> getLatency(int sensitivity, int64_t last = 0) override;
    virtual std::vector<Report::test_result> getBandwidth(int sensitivity, int64_t last = 0) override;

    void saveLatencyTest(Message::node node, int ms, int window) override;
    void saveBandwidthTest(Message::node node, float kbps, int state, int window) override;
    void saveHardware(Report::hardware_result hardware, int window) override;

    void refreshNodes(std::vector<Message::node> nodes) override;
    void updateNodes(std::vector<Message::node> add, std::vector<Message::node> rem) override;

    std::vector<Message::node> getNodes() override;
    
    std::vector<Message::node> getLRLatency(int num, int seconds) override;
    std::vector<Message::node> getLRBandwidth(int num, int seconds) override;

    //return -1 on fail
    int getTestBandwidthState(Message::node ip, Report::test_result &last) override;

    std::vector<Report::IoT> getIots() override;

    void addIot(IThing *iot) override;
};



#endif