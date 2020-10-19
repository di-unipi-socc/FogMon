#include "storage.hpp"
#include "iagent.hpp"
#include <string.h>
#include <iostream>
#include <inttypes.h>

using namespace std;

Storage::Storage() {
    this->ip = "::1";
}


void Storage::filterRecv(std::vector<Message::node> &list) {
    for(int i=0; i<list.size(); i++)
    {
        if(list[i].ip==std::string("::1")||list[i].ip==std::string("127.0.0.1"))
            list[i].ip = this->ip;
    }
}

void Storage::filterSend(std::vector<Message::node> &list) {
    for(int i=0; i<list.size(); i++)
    {
        if(list[i].ip==this->ip)
            list[i].ip = "::1";
    }
}

void Storage::filterSend(std::vector<Report::test_result> &list) {
    for(int i=0; i<list.size(); i++)
    {
        if(list[i].target.ip==this->ip)
            list[i].target.ip = "::1";
    }
}

void Storage::filterSend(Report::test_result &list) {
    if(list.target.ip == this->ip)
        list.target.ip = "::1";
}

void Storage::setFilter(std::string ip) {
    this->ip = ip;
}


Storage::~Storage() {

}

void Storage::isError(int err, char *zErrMsg, std::string mess) {
    if( err!=SQLITE_OK )
    {
        cerr << "[" << mess << "] SQL error: " << zErrMsg << endl; 
        fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void Storage::createTables() {
    char *zErrMsg = 0;

    vector<string> query = {"CREATE TABLE IF NOT EXISTS Hardware (time TIMESTAMP PRIMARY KEY, cores INTEGER, free_cpu REAL, memory INTEGER, free_memory REAL, disk INTEGER, free_disk REAL)",
                            "CREATE TABLE IF NOT EXISTS Nodes (id STRING PRIMARY KEY NOT NULL, ip STRING NOT NULL, port STRING NOT NULL, latencyTime TIMESTAMP, lastMeanL FLOAT, lastVarianceL FLOAT, bandwidthTime TIMESTAMP, bandwidthState INTEGER, lastMeanB FLOAT, lastVarianceB FLOAT, UNIQUE(ip, port))",
                            "CREATE TABLE IF NOT EXISTS Latency (time TIMESTAMP, idNodeB STRING REFERENCES Nodes(id) NOT NULL, ms INTEGER)",
                            "CREATE TABLE IF NOT EXISTS Bandwidth (time TIMESTAMP PRIMARY KEY, idNodeB STRING REFERENCES Nodes(id) NOT NULL, kbps FLOAT)",
                            "CREATE TABLE IF NOT EXISTS IoTs (id STRING PRIMARY KEY, desc STRING, ms INTEGER)"};
    
    for(string str : query) {
        int err = sqlite3_exec(this->db, str.c_str(), 0, 0, &zErrMsg);
        isError(err, zErrMsg, "createTables");
    }
}

Report::hardware_result Storage::getHardware() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "SELECT max(cores) as cores, avg(free_cpu) AS mean_free_cpu, variance(free_cpu) AS var_free_cpu, max(memory) as memory,"
                        "   avg(free_memory) AS mean_free_memory, variance(free_memory) AS var_free_memory, max(disk) as disk,"
                        "   avg(free_disk) AS mean_free_disk, variance(free_disk) AS var_free_disk FROM Hardware");

    Report::hardware_result r;
    memset(&r,0,sizeof(Report::hardware_result));

    int err = sqlite3_exec(this->db, buf, IStorage::getHardwareCallback, &r, &zErrMsg);
    isError(err, zErrMsg, "getHardware");

    return r;
}

std::vector<Report::test_result> Storage::getLatency(int64_t last, int sensitivity) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT N.id, N.ip, N.port, avg(L.ms) AS mean, variance(L.ms) AS var, strftime('%%s',max(L.time)) as time FROM Latency AS L JOIN Nodes AS N WHERE L.idNodeB = N.id GROUP BY N.id HAVING ((N.lastMeanL-avg(L.ms))/N.lastMeanL > %d OR (N.lastVarianceL-variance(L.ms))/N.lastVarianceL > %d) AND strftime('%%s',max(L.time))>%" PRId64, sensitivity, sensitivity,last);

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
    isError(err, zErrMsg, "getLatency");

    filterSend(tests);

    return tests;
}

std::vector<Report::test_result> Storage::getBandwidth(int64_t last, int sensitivity) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT N.id, N.ip, N.port, avg(B.kbps) AS mean, variance(B.kbps) AS var, strftime('%%s',max(B.time)) as time FROM Bandwidth AS B JOIN Nodes AS N WHERE B.idNodeB = N.id GROUP BY N.id HAVING ((N.lastMeanL-avg(B.kbps))/N.lastMeanL > %d OR (N.lastVarianceL-variance(B.kbps))/N.lastVarianceL > %d) AND strftime('%%s',max(B.time))>%" PRId64, sensitivity, sensitivity, last);

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
    isError(err, zErrMsg, "getBandwidth");
    filterSend(tests);
    return tests;
}

void Storage::saveState() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "INSERT OR REPLACE INTO Nodes (id,ip,port, latencyTime, lastMeanL, lastVarianceL, bandwidthTime, bandwidthState, lastMeanB, lastVarianceB) "
                        " SELECT A.id AS id, A.ip AS ip, A.port AS port, A.latencyTime AS latencyTime, "
                        " L.mean AS lastMeanL, L.var AS lastVarianceL,"
                        " A.bandwidthTime AS bandwidthTime, A.bandwidthState AS bandwidthState,"
                        " B.mean AS lastMeanB, B.var AS lastVarianceB"
                        " from Nodes AS A"
                        " join (SELECT N.id as id, avg(L1.ms) AS mean, variance(L1.ms) AS var FROM Latency AS L1 JOIN Nodes AS N WHERE L1.idNodeB = N.id group by N.id ) AS L "
                        " join (SELECT N.id as id, avg(B1.kbps) AS mean, variance(B1.kbps) AS var FROM Bandwidth AS B1 JOIN Nodes AS N WHERE B1.idNodeB = N.id group by N.id ) AS B "
                        " WHERE A.id == L.id AND A.id == L.id ");


    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveState");

}

void Storage::saveLatencyTest(Message::node node, int ms) {
    if(node.id == "")
        return;

    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Latency (time ,idNodeB, ms) VALUES (DATETIME('now'), \"%s\", %d)", node.id.c_str(),ms);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveLatencyTest1");

    std::sprintf(buf,"UPDATE Nodes SET latencyTime = DATETIME('now') WHERE id = \"%s\"", node.id.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveLatencyTest2");

    std::sprintf(buf,"DELETE FROM Latency WHERE time <= (SELECT time FROM Latency WHERE idNodeB = \"%s\" ORDER BY time DESC LIMIT 1 OFFSET 20)",node.id.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveLatencyTest3");
}

void Storage::saveBandwidthTest(Message::node node, float kbps, int state) {
    if(node.id == "")
        return;

    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Bandwidth (time, idNodeB, kbps) VALUES (DATETIME('now'), \"%s\", %f)", node.id.c_str(), kbps);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest1");

    std::sprintf(buf,"UPDATE Nodes SET bandwidthTime = DATETIME('now'), bandwidthState = %d WHERE id = \"%s\"", state, node.id.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest2");

    std::sprintf(buf,"DELETE FROM Bandwidth WHERE time <= (SELECT time FROM Bandwidth WHERE idNodeB = \"%s\" ORDER BY time DESC LIMIT 1 OFFSET 5)",node.id.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest3");
}

void Storage::saveHardware(Report::hardware_result hardware) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Hardware (time, cores, free_cpu, memory, free_memory, disk, free_disk) VALUES (DATETIME('now'), %d, %f, %ld, %f, %" PRId64", %f)", hardware.cores, hardware.mean_free_cpu, hardware.memory, hardware.mean_free_memory, hardware.disk, hardware.mean_free_disk);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveHardware1");

    std::sprintf(buf,"DELETE FROM Hardware WHERE time <= (SELECT time FROM Hardware ORDER BY time DESC LIMIT 1 OFFSET 20)");

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveHardware2");
}

void Storage::refreshNodes(vector<Message::node> nodes) {
    char *zErrMsg = 0;
    char buf[1024];

    filterRecv(nodes);

    std::sprintf(buf,"DELETE FROM Nodes");
    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "refreshNodes1");  

    std::sprintf(buf,"DELETE FROM Latency WHERE idNodeB NOT IN (SELECT id FROM Nodes)");
    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "refreshNodes2");

    std::sprintf(buf,"DELETE FROM Bandwidth WHERE idNodeB NOT IN (SELECT id FROM Nodes)");
    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "refreshNodes3"); 

    for(auto node : nodes) {
        if(node.id != "") {
            //does not exists then insert
            std::sprintf(buf,"INSERT OR REPLACE INTO Nodes (id,ip,port, latencyTime, lastMeanL, lastVarianceL, bandwidthTime, bandwidthState, lastMeanB, lastVarianceB) VALUES (\"%s\", \"%s\", \"%s\", datetime('now', '-1 month'), -1, -1, datetime('now', '-1 month'), 0, -1, -1)", node.id.c_str(),node.ip.c_str(),node.port.c_str());
            err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
            isError(err, zErrMsg, "refreshNodes4");
        }
    }
}

void Storage::updateNodes(vector<Message::node> add, vector<Message::node> rem) {
    char *zErrMsg = 0;
    char buf[1024];

    filterRecv(add);
    filterRecv(rem);

    for(auto node : add) {
        if(node.id != "") {
            //does not exists then insert
            std::sprintf(buf,"INSERT OR IGNORE INTO Nodes (id,ip,port, latencyTime, lastMeanL, lastVarianceL, bandwidthTime, bandwidthState, lastMeanB, lastVarianceB) VALUES (\"%s\", \"%s\", \"%s\", datetime('now', '-1 month'), -1, -1, datetime('now', '-1 month'), 0, -1, -1)", node.id.c_str(),node.ip.c_str(),node.port.c_str());
            int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
            isError(err, zErrMsg, "updateNodes1"); 
        }
    }

    for(auto node : rem) {
        if(node.id == "") {
            continue;
        }

        std::sprintf(buf,"DELETE FROM Nodes WHERE id = \"%s\"", node.id.c_str());

        int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "updateNodes2");

        std::sprintf(buf,"DELETE FROM Latency WHERE idNodeB = \"%s\"", node.id.c_str());

        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "updateNodes3");

        std::sprintf(buf,"DELETE FROM Bandwidth WHERE idNodeB = \"%s\"", node.id.c_str());

        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "updateNodes4");
    }
}

vector<Message::node> Storage::getNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT id,ip,port FROM Nodes");

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getNodes");

    filterSend(nodes);
    return nodes;
}

std::vector<Message::node> Storage::getLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT id,ip,port FROM Nodes WHERE strftime('%%s',latencyTime)+%d-strftime('%%s','now') <= 0 ORDER BY latencyTime LIMIT %d",seconds, num);

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getLRLatency");

    return nodes;
}

std::vector<Message::node> Storage::getLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT id,ip,port FROM Nodes WHERE strftime('%%s',bandwidthTime)+%d-strftime('%%s','now') <= 0 ORDER BY RANDOM() LIMIT %d",seconds, num);

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getLRBandwidth");

    return nodes;
}

int getIntCallback(void *i, int argc, char **argv, char **azColName) {
    int *val = (int*)i;
    *val = stoi(argv[0]);
    return 0;
}

int getTest2Callback(void *T, int argc, char **argv, char **azColName) {
    Report::test_result *test =(Report::test_result*)T;
    test->target.id = string(argv[0]);
    test->target.ip = string(argv[1]);
    test->target.port = string(argv[2]);
    if(argv[3] == NULL)
        test->mean = 0;
    else
        test->mean = stof(argv[3]);
    if(argv[4] == NULL)
        test->variance = 0;
    else
        test->variance = stof(argv[4]);
    if(argv[5] == NULL)
        test->lasttime = 0;
    else
        test->lasttime = stof(argv[5]);
    return 0;
}

int Storage::getTestBandwidthState(Message::node node, Report::test_result &last) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT bandwidthState FROM Nodes WHERE id = \"%s\" LIMIT 1", node.id.c_str());

    int val = -1;

    int err = sqlite3_exec(this->db, buf, getIntCallback, &val, &zErrMsg);
    isError(err, zErrMsg, "getTestBandwidthState1");

    std::sprintf(buf,"SELECT N.id as id, N.ip as ip, N.port as port, avg(B.kbps) AS mean, variance(B.kbps) AS var, strftime('%%s',max(B.time)) as time FROM Bandwidth AS B JOIN Nodes AS N where B.idNodeB = N.id AND N.id = \"%s\" group by N.id", node.id.c_str());

    err = sqlite3_exec(this->db, buf, getTest2Callback, &last, &zErrMsg);
    isError(err, zErrMsg, "getTestBandwidthState2");

    filterSend(last);

    return val;
}

vector<Report::IoT> Storage::getIots() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT * FROM IoTs");

    vector<Report::IoT> vect;

    int err = sqlite3_exec(this->db, buf, VectorIoTCallback, &vect, &zErrMsg);
    isError(err, zErrMsg, "getIots");

    return vect;
}

void Storage::addIot(IThing *iot) {
    if(iot == NULL)
        return;
        
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT OR REPLACE INTO IoTs (id, desc, ms) VALUES (\"%s\",\"%s\",%d)", iot->getId().c_str(), iot->getDesc().c_str(), iot->getLatency());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "addIot");
}