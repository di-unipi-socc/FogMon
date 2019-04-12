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

    std::string ipS;

public:
    Storage();
    virtual ~Storage();

    void setFilter(std::string ipS);

    virtual Report::hardware_result getHardware();
    virtual std::vector<Report::test_result> getLatency();
    virtual std::vector<Report::test_result> getBandwidth();

    void saveLatencyTest(std::string ip, int ms);
    void saveBandwidthTest(std::string ip, float kbps, int state);
    void saveHardware(Report::hardware_result hardware);

    long long getNodeId(std::string ip);
    void refreshNodes(std::vector<std::string> nodes);
    void updateNodes(std::vector<std::string> add, std::vector<std::string> rem);

    std::vector<std::string> getNodes();
    
    std::vector<std::string> getLRLatency(int num, int seconds);
    std::vector<std::string> getLRBandwidth(int num, int seconds);

    //return -1 on fail
    int getTestBandwidthState(std::string ip, Report::test_result &last);

    std::vector<Report::IoT> getIots();

    void addIot(IThing *iot);

    void setToken(int duration);
    int hasToken();
};



#endif