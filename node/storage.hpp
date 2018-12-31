#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class Storage : virtual public IStorage {
protected:
    virtual void createTables();

    //start time of the token
    time_t startToken;
    //seconds the token lasts
    int durationToken;
public:
    Storage();
    virtual ~Storage();

    virtual Report::hardware_result getHardware();
    virtual std::vector<Report::test_result> getLatency();
    virtual std::vector<Report::test_result> getBandwidth();

    void saveLatencyTest(std::string ip, int ms);
    void saveBandwidthTest(std::string ip, float kbps, int state);
    void saveHardware(Report::hardware_result hardware);

    void refreshNodes(std::vector<std::string> nodes);
    void updateNodes(std::vector<std::string> add, std::vector<std::string> rem);

    std::vector<std::string> getNodes();
    
    std::vector<std::string> getLRLatency(int num, int seconds);
    std::vector<std::string> getLRBandwidth(int num, int seconds);

    //return -1 on fail
    int getTestBandwidthState(std::string ip, Report::test_result &last);

    void setToken(int duration);
    int hasToken();
};



#endif