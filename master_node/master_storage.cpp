#include "master_storage.hpp"
#include <string.h>
#include <vector>

using namespace std;

MasterStorage::MasterStorage(string path) {
    open(path);
}

MasterStorage::~MasterStorage() {
}

void MasterStorage::createTables() {
    char *zErrMsg = 0;
    
    vector<string> query = {"CREATE TABLE IF NOT EXISTS Nodes (ip STRING PRIMARY KEY, cores INTEGER, free_cpu REAL, memory INTEGER, free_memory INTEGER, disk INTEGER, free_disk INTEGER, lasttime TIMESTAMP)",
                            "CREATE TABLE IF NOT EXISTS Bandwidth (ipA STRING, ipB STRING, mean FLOAT, variance FLOAT, lasttime TIMESTAMP, PRIMARY KEY(ipA,ipB))",
                            "CREATE TABLE IF NOT EXISTS Latency (ipA STRING, ipB STRING, mean FLOAT, variance FLOAT, lasttime TIMESTAMP, PRIMARY KEY(ipA,ipB))",
                            "CREATE INDEX IF NOT EXISTS lastNodes ON Nodes(lasttime)",
                            "CREATE INDEX IF NOT EXISTS lastBandwidth ON Bandwidth(lasttime)",
                            "CREATE INDEX IF NOT EXISTS lastLatency ON Latency(lasttime)"};
    
    for(string str : query) {
        int err = sqlite3_exec(this->db, str.c_str(), 0, 0, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }        
    }
}

vector<string> MasterStorage::getNodes() {
    return vector<string>();
}