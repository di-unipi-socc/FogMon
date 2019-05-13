#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"

class Storage : virtual public IStorage {
protected:
    virtual void createTables();

    void filterRecv(std::vector<std::string> &list);
    void filterSend(std::vector<std::string> &list);
    void filterSend(std::vector<Report::test_result> &list);
    void filterSend(Report::test_result &list);
    //start time of the token
    time_t startToken;
    //seconds the token lasts
    int durationToken;

    Message::node node;

public:
    Storage();
    virtual ~Storage();

    void setFilter(Message::node node);

    virtual Report::hardware_result getHardware();
    virtual std::vector<Report::test_result> getLatency();
    virtual std::vector<Report::test_result> getBandwidth();

    void saveLatencyTest(Message::node node, int ms);
    void saveBandwidthTest(Message::node node, float kbps, int state);
    void saveHardware(Report::hardware_result hardware);

    long long getNodeId(std::string ip);
    void refreshNodes(std::vector<Message::node> nodes);
    void updateNodes(std::vector<Message::node> add, std::vector<Message::node> rem);

    std::vector<Message::node> getNodes();
    
    std::vector<Message::node> getLRLatency(int num, int seconds);
    std::vector<Message::node> getLRBandwidth(int num, int seconds);

    //return -1 on fail
    int getTestBandwidthState(Message::node ip, Report::test_result &last);

    std::vector<Report::IoT> getIots();

    void addIot(IThing *iot);

    void setToken(int duration);
    int hasToken();
};



#endif