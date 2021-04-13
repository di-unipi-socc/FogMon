#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"

class Storage : virtual public IStorage {
protected:
    virtual void createTables();

    void filterRecv(std::vector<Message::node> &list);
    void filterSend(std::vector<Message::node> &list);
    void filterSend(std::vector<Report::test_result> &list);
    void filterSend(Report::test_result &list);

    std::string ip;

    void isError(int err, char *zErrMsg, std::string mess);

public:
    Storage();
    virtual ~Storage();

    void setFilter(std::string ip);

    virtual Report::hardware_result getHardware();
    virtual std::vector<Report::test_result> getLatency(int sensitivity, int64_t last = 0);
    virtual std::vector<Report::test_result> getBandwidth(int sensitivity, int64_t last = 0);

    void saveLatencyTest(Message::node node, int ms, int window);
    void saveBandwidthTest(Message::node node, float kbps, int state, int window);
    void saveHardware(Report::hardware_result hardware, int window);

    void saveState(int64_t last, int sensitivity);

    void refreshNodes(std::vector<Message::node> nodes);
    void updateNodes(std::vector<Message::node> add, std::vector<Message::node> rem);

    std::vector<Message::node> getNodes();
    
    std::vector<Message::node> getLRLatency(int num, int seconds);
    std::vector<Message::node> getLRBandwidth(int num, int seconds);

    //return -1 on fail
    int getTestBandwidthState(Message::node ip, Report::test_result &last);

    std::vector<Report::IoT> getIots();

    void addIot(IThing *iot);
};



#endif