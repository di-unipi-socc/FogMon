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
            fprintf(stderr, "SQL error (creating tables): %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }        
    }
}

void MasterStorage::addNode(string ip, Report::hardware_result hardware) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT OR REPLACE INTO Nodes (ip, cores, free_cpu, memory, free_memory, disk, free_disk, lasttime) VALUES (\"%s\", %d, %f, %d, %d, %d, %d, datetime('now', 'unixepoch'))", ip.c_str(), hardware.cores, hardware.free_cpu, hardware.memory, hardware.free_memory, hardware.disk, hardware.free_disk);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}


int VectorStringCallback(void *vec, int argc, char **argv, char **azColName) {
    vector<string> *v = (vector<string>*)vec;
    v->push_back(string(argv[0]));
    return 0;
}

vector<string> MasterStorage::getNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM Nodes");

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (select nodes): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}