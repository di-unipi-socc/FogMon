#ifndef MASTER_STORAGE_HPP_
#define MASTER_STORAGE_HPP_

#include "istorage.hpp"
#include "report.hpp"

class MasterStorage : public IStorage {
private:
    void createTables();

public:
    MasterStorage(std::string path);
    ~MasterStorage();

    std::vector<std::string> getNodes();

    void addNode(std::string str, Report::hardware_result);
};

#endif