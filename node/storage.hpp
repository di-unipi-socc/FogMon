#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class Storage : public IStorage {
private:
    void createTables();

public:
    Storage(std::string path);
    ~Storage();

    void generateReport();
    Report::hardware_result getHardware();
    void savePingTest(std::string ip, int ms);
    void saveBandwidthTest(std::string ip, float kbps);
    void saveHardware(Report::hardware_result hardware);

    void refreshNodes(std::vector<std::string> nodes);
    void updateNodes(std::vector<std::string> add, std::vector<std::string> rem);

    std::vector<std::string> getNodes();

};



#endif