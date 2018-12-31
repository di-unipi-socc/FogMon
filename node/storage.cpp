#include "storage.hpp"
#include <string.h>

#include <ctime>

using namespace std;

Storage::Storage() {
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
                            "CREATE TABLE IF NOT EXISTS Nodes (ip STRING PRIMARY KEY, latencyTime TIMESTAMP, bandwidthTime TIMESTAMP, bandwidthState INTEGER)"};
    
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

Report::hardware_result Storage::getHardware() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT * FROM Hardware ORDER BY time LIMIT 1");

    Report::hardware_result r;

    int err = sqlite3_exec(this->db, buf, IStorage::getHardwareCallback, &r, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return r;
}

std::vector<Report::test_result> Storage::getLatency() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipB, avg(ms) AS mean, variance(ms) AS var, strftime('%%s',max(time)) as time FROM Latency group by ipB");

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
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

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
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

void Storage::saveBandwidthTest(string ip, float kbps, int state) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Bandwidth (time, ipB, kbps) VALUES (DATETIME('now'), \"%s\", %f)", ip.c_str(), kbps);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    std::sprintf(buf,"UPDATE Nodes SET bandwidthTime = DATETIME('now'), bandwidthState = %d WHERE ip = \"%s\"", state, ip.c_str());

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
        std::sprintf(buf,"INSERT OR IGNORE INTO Nodes (ip, latencyTime, bandwidthTime, bandwidthState) VALUES (\"%s\", datetime('now', '-1 month'), datetime('now', '-1 month'), 0)", node.c_str());
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
        std::sprintf(buf,"INSERT OR IGNORE INTO Nodes (ip, latencyTime, bandwidthTime, bandwidthState) VALUES (\"%s\", datetime('now', '-1 month'), datetime('now', '-1 month'), 0)", node.c_str());

        int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }
    }

    for(auto node : rem) {
        std::sprintf(buf,"DELETE FROM Nodes WHERE ip = \"%s\"", node.c_str());

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
    std::sprintf(buf,"SELECT ip FROM Nodes WHERE strftime('%%s',latencyTime)+%d-strftime('%%s','now') <= 0 ORDER BY latencyTime LIMIT %d",seconds, num);

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
    std::sprintf(buf,"SELECT ip FROM Nodes WHERE strftime('%%s',bandwidthTime)+%d-strftime('%%s','now') <= 0 ORDER BY RANDOM() LIMIT %d",seconds, num);

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

int getIntCallback(void *i, int argc, char **argv, char **azColName) {
    int *val = (int*)i;
    *val = stoi(argv[0]);
    return 0;
}

int getTest2Callback(void *T, int argc, char **argv, char **azColName) {
    Report::test_result *test =(Report::test_result*)T;
    test->target = string(argv[0]);
    test->mean = stof(argv[1]);
    test->variance = stof(argv[2]);
    test->lasttime = stol(argv[3]);
    return 0;
}

int Storage::getTestBandwidthState(std::string ip, Report::test_result &last) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT bandwidthState FROM Nodes WHERE ip = \"%s\" LIMIT 1", ip.c_str());

    int val = -1;

    int err = sqlite3_exec(this->db, buf, getIntCallback, &val, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    std::sprintf(buf,"SELECT ipB as ip, avg(kbps) AS mean, variance(kbps) AS var, strftime('%%s',max(time)) as time FROM Bandwidth where ipB = \"%s\" group by ipB", ip.c_str());

    vector<Report::test_result> tests;

    err = sqlite3_exec(this->db, buf, getTest2Callback, &last, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return val;
}

void Storage::setToken(int duration) {
    startToken = time(nullptr);
    durationToken = duration;
}

int Storage::hasToken() {
    return startToken+durationToken - time(nullptr);
}