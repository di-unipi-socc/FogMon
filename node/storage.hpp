#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "istorage.hpp"

class Storage : public IStorage {
private:
    void createTables();

    static int callback(void *NotUsed, int argc, char **argv, char **azColName);

public:
    Storage(std::string path);
    ~Storage();

    void generateReport();
    void savePingTest(std::string ip, int ms);
    void saveBandwidthTest(std::string ip, float kbps);
    void saveHardware(int cores, float free_cpu, int memory, int free_memory, int disk, int free_disk);

    void refreshNodes(std::vector<std::string> nodes);
    void updateNodes(std::vector<std::string> add, std::vector<std::string> rem);

    std::vector<std::string> getNodes();

};



#endif