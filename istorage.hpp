#ifndef ISTORAGE_HPP_
#define ISTORAGE_HPP_

#include <sqlite3.h>
#include <string>
#include <vector>

class IStorage {
protected:
    void open(std::string path);
    void close();
    virtual void createTables() = 0;

    sqlite3 *db;
    
public:
    virtual std::vector<std::string> getNodes() = 0;
};



#endif