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
    std::sprintf(buf,"INSERT OR REPLACE INTO Nodes (ip, cores, free_cpu, memory, free_memory, disk, free_disk, lasttime) VALUES (\"%s\", %d, %f, %d, %d, %d, %d, DATETIME('now'))", ip.c_str(), hardware.cores, hardware.free_cpu, hardware.memory, hardware.free_memory, hardware.disk, hardware.free_disk);

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

void MasterStorage::addTest(string strIpA, string strIpB, Report::test_result test, string type) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT OR REPLACE INTO %s (ipA, ipB, mean, variance, lasttime) VALUES (\"%s\", \"%s\", %f, %f, DATETIME('now'))", strIpA.c_str(), strIpB.c_str() , test.mean, test.variance, type.c_str());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void MasterStorage::addReportLatency(string strIp, vector<Report::test_result> latency) {
    for(auto test : latency) {
        this->addTest(strIp, test.target, test, "Latency");
        this->addTest(test.target, strIp, test, "Latency"); //because is symmetric
    }
}

void MasterStorage::addReportBandwidth(string strIp, vector<Report::test_result> bandwidth) {
    for(auto test : bandwidth) {
        this->addTest(strIp, test.target, test, "Bandwidth"); //asymmetric
    }
}

void MasterStorage::addReport(string strIp, Report::hardware_result hardware, vector<Report::test_result> latency, vector<Report::test_result> bandwidth) {
    this->addNode(strIp, hardware);
    this->addReportLatency(strIp, latency);
    this->addReportBandwidth(strIp, bandwidth);
}

std::vector<std::string> MasterStorage::getLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipA FROM Latency WHERE strftime('%%s',lasttime)+%d < strftime('%%s','now') ORDER BY lasttime LIMIT %d", seconds, num);

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<std::string> MasterStorage::getLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipA FROM Bandwidth WHERE strftime('%%s',lasttime)+%d < strftime('%%s','now') ORDER BY lasttime LIMIT %d", seconds, num);

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<std::string> MasterStorage::getLRHardware(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM Nodes WHERE strftime('%%s',lasttime)+%d < strftime('%%s','now') ORDER BY lasttime LIMIT %d", seconds, num);

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}