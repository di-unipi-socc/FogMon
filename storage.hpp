#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include <sqlite3.h>
#include <string>
#include <vector>

class Storage {
private:
    void open(std::string path);
    void close();
    void createTables();

    static int callback(void *NotUsed, int argc, char **argv, char **azColName);

    sqlite3 *db;

public:
    Storage();
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