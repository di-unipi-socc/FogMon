#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class Storage : public IStorage {
protected:
    void createTables();

    //start time of the token
    time_t startToken;
    //seconds the token lasts
    int durationToken;

public:
    Storage(std::string path);
    ~Storage();

    Report::hardware_result getHardware();
    std::vector<Report::test_result> getLatency();
    std::vector<Report::test_result> getBandwidth();

    void saveLatencyTest(std::string ip, int ms);
    void saveBandwidthTest(std::string ip, float kbps);
    void saveHardware(Report::hardware_result hardware);

    void refreshNodes(std::vector<std::string> nodes);
    void updateNodes(std::vector<std::string> add, std::vector<std::string> rem);

    std::vector<std::string> getNodes();
    
    std::vector<std::string> getLRLatency(int num, int seconds);
    std::vector<std::string> getLRBandwidth(int num, int seconds);

    void setToken(int duration);
    int hasToken();
};



#endif