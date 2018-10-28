#include "storage.hpp"
#include <string.h>

#include <ctime>

using namespace std;

Storage::Storage(string path) {
    open(path);

    startToken = time(nullptr);
    durationToken = 0;
}

Storage::~Storage() {
}

void Storage::createTables() {
    char *zErrMsg = 0;

    vector<string> query = {"CREATE TABLE IF NOT EXISTS Hardware (time TIMESTAMP PRIMARY KEY, cores INTEGER, free_cpu REAL, memory INTEGER, free_memory INTEGER, disk INTEGER, free_disk INTEGER)",
                            "CREATE TABLE IF NOT EXISTS Latency (time TIMESTAMP, ipB STRING, ms INTEGER)",
                            "CREATE TABLE IF NOT EXISTS Bandwidth (time TIMESTAMP PRIMARY KEY, ipB STRING, kbps FLOAT)",
                            "CREATE TABLE IF NOT EXISTS Nodes (ip STRING PRIMARY KEY, latencyTime TIMESTAMP, bandwidthTime TIMESTAMP)"};
    
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

int getHardwareCallback(void *R, int argc, char **argv, char **azColName) {
    Report::hardware_result *r = (Report::hardware_result*)R;
    for(int i=0; i<argc; i++) {
        if(strcmp("cores", azColName[i])==0) {
            r->cores = stoi(argv[i]);
        }else if(strcmp("free_cpu", azColName[i])==0) {
            r->free_cpu = stof(argv[i]);
        }else if(strcmp("memory", azColName[i])==0) {
            r->memory = stoi(argv[i]);
        }else if(strcmp("free_memory", azColName[i])==0) {
            r->free_memory = stoi(argv[i]);
        }else if(strcmp("disk", azColName[i])==0) {
            r->disk = stoi(argv[i]);
        }else if(strcmp("free_disk", azColName[i])==0) {
            r->free_disk = stoi(argv[i]);
        }
    }
    return 0;
}

Report::hardware_result Storage::getHardware() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT * FROM Hardware ORDER BY time LIMIT 1");

    Report::hardware_result r;

    int err = sqlite3_exec(this->db, buf, getHardwareCallback, &r, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return r;
}

int getTestCallback(void *R, int argc, char **argv, char **azColName) {
    vector<Report::test_result> *r = (vector<Report::test_result>*)R;
    Report::test_result test;
    test.target = string(argv[0]);
    test.mean = stof(argv[1]);
    test.variance = stof(argv[2]);
    test.lasttime = stol(argv[3]);
    r->push_back(test);
    return 0;
}

std::vector<Report::test_result> Storage::getLatency() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipB, avg(ms) AS mean, variance(ms) AS var, strftime('%%s',max(time)) as time FROM Latency group by ipB");

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, getTestCallback, &tests, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return tests;
}

std::vector<Report::test_result> Storage::getBandwidth() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipB as ip, avg(kbps) AS mean, variance(kbps) AS var, strftime('%%s',max(time)) as time FROM Bandwidth group by ipB");

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, getTestCallback, &tests, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
 
    return tests;
}

void Storage::saveLatencyTest(string ip, int ms) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Latency (time ,ipB, ms) VALUES (DATETIME('now'),\"%s\", %d)", ip.c_str(), ms);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    std::sprintf(buf,"UPDATE Nodes SET latencyTime = DATETIME('now') WHERE ip = \"%s\"", ip.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void Storage::saveBandwidthTest(string ip, float kbps) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Bandwidth (time, ipB, kbps) VALUES (DATETIME('now'), \"%s\", %d)", ip.c_str(), kbps);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    std::sprintf(buf,"UPDATE Nodes SET bandwidthTime = DATETIME('now') WHERE ip = \"%s\"", ip.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void Storage::saveHardware(Report::hardware_result hardware) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Hardware (time, cores, free_cpu, memory, free_memory, disk, free_disk) VALUES (DATETIME('now'), %d, %f, %d, %d, %d, %d)", hardware.cores, hardware.free_cpu, hardware.memory, hardware.free_memory, hardware.disk, hardware.free_disk);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void Storage::refreshNodes(vector<string> nodes) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"DELETE FROM Nodes");

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    for(auto node : nodes) {
        std::sprintf(buf,"INSERT OR IGNORE INTO Nodes (ip, latencyTime, bandwidthTime) VALUES (\"%s\", datetime('now', '-1 month'), datetime('now', '-1 month'))", node.c_str());
        printf("%s\n", buf);
        int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }
    }
}

void Storage::updateNodes(vector<string> add, vector<string> rem) {
    char *zErrMsg = 0;
    char buf[1024];

    for(auto node : add) {
        std::sprintf(buf,"INSERT OR IGNORE INTO Nodes (ip, latencyTime, bandwidthTime) VALUES (\"%s\", datetime('now', '-1 month'), datetime('now', '-1 month'))", node.c_str());

        int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }
    }

    for(auto node : rem) {
        std::sprintf(buf,"DELETE Nodes WHERE ip = \"%s\"", node.c_str());

        int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }
    }
}

int getNodesCallback(void *vec, int argc, char **argv, char **azColName) {
    vector<string> *v = (vector<string>*)vec;
    v->push_back(string(argv[0]));
    return 0;
}


vector<string> Storage::getNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM Nodes");

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, getNodesCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<std::string> Storage::getLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM Nodes WHERE strftime('%%s',latencyTime)+%d-strftime('%%s','now') < 0 ORDER BY latencyTime LIMIT %d",seconds, num);

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, getNodesCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<std::string> Storage::getLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM Nodes WHERE strftime('%%s',bandwidthTime)+%d-strftime('%%s','now') < 0 ORDER BY bandwidthTime LIMIT %d",seconds, num);

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, getNodesCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

void Storage::setToken(int duration) {
    startToken = time(nullptr);
    durationToken = duration;
}

int Storage::hasToken() {
    return startToken+durationToken - time(nullptr);
}