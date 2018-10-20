#include "storage.hpp"
#include <string.h>

using namespace std;

Storage::Storage(string path) {
    open(path);
}

Storage::~Storage() {
}

void Storage::createTables() {
    char *zErrMsg = 0;
    int err = sqlite3_exec(this->db, "CREATE TABLE IF NOT EXISTS Hardware (time TIMESTAMP PRIMARY KEY, cores INTEGER, free_cpu REAL, memory INTEGER, free_memory INTEGER, disk INTEGER, free_disk INTEGER) ", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    err = sqlite3_exec(this->db, "CREATE TABLE IF NOT EXISTS Ping (time TIMESTAMP PRIMARY KEY, ipB STRING, ms INTEGER) ", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    err = sqlite3_exec(this->db, "CREATE TABLE IF NOT EXISTS Band (time TIMESTAMP PRIMARY KEY, ipB STRING, kbps FLOAT) ", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    err = sqlite3_exec(this->db, "CREATE TABLE IF NOT EXISTS Nodes (ip STRING PRIMARY KEY, tested INTEGER) ", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

int Storage::callback(void *vec, int argc, char **argv, char **azColName) {
    vector<string> *v = (vector<string>*)vec;
    v->push_back(string(argv[0]));
}

void Storage::generateReport() {
    
}

void Storage::savePingTest(string ip, int ms) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Ping (ipB, ms) VALUES (\"%s\", %d)", ip.c_str(), ms);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
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
    std::sprintf(buf,"INSERT INTO Band (ipB, kbps) VALUES (\"%s\", %d)", ip.c_str(), kbps);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void Storage::saveHardware(int cores, float free_cpu, int memory, int free_memory, int disk, int free_disk) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Hardware (cores, free_cpu, memory, free_memory, disk, free_disk) VALUES (%d, %f, %d, %d, %d, %d)", cores, free_cpu, memory, free_memory, disk, free_disk);

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
    std::sprintf(buf,"DELETE FROM NODES");

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    for(auto node : nodes) {
        std::sprintf(buf,"INSERT INTO Nodes (ip,tested) VALUES (\"%s\", 0)", node.c_str());
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
        std::sprintf(buf,"INSERT IGNORE INTO Nodes (ip,tested) VALUES (\"%s\", 0)", node.c_str());

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


vector<string> Storage::getNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM Nodes");

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, this->callback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}