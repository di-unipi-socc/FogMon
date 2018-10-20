#ifndef MASTER_STORAGE_HPP_
#define MASTER_STORAGE_HPP_

#include "istorage.hpp"

class MasterStorage : public IStorage {
private:
    void createTables();

public:
    MasterStorage(std::string path);
    ~MasterStorage();

    std::vector<std::string> getNodes();
};

#endif