#include "storage.hpp"
#include "iagent.hpp"
#include <string.h>
#include <sstream>
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

int getFloatCallback(void *i, int argc, char **argv, char **azColName) {
    float *val = (float*)i;
    *val = stof(argv[0]);
    return 0;
}

int getIntCallback(void *i, int argc, char **argv, char **azColName) {
    int *val = (int*)i;
    *val = stoi(argv[0]);
    return 0;
}

void Storage::createTables() {
    char *zErrMsg = 0;

    vector<string> query = {"CREATE TABLE IF NOT EXISTS Hardware (time TIMESTAMP PRIMARY KEY, cores INTEGER, free_cpu REAL, memory INTEGER, free_memory REAL, disk INTEGER, free_disk REAL)",
                            "CREATE TABLE IF NOT EXISTS Nodes (id STRING PRIMARY KEY NOT NULL, ip STRING NOT NULL, port STRING NOT NULL, latencyTime TIMESTAMP, lastMeanL FLOAT, lastVarianceL FLOAT, bandwidthTime TIMESTAMP, bandwidthState INTEGER, lastMeanB FLOAT, lastVarianceB FLOAT, UNIQUE(ip, port))",
                            "CREATE TABLE IF NOT EXISTS Latency (time TIMESTAMP, idNodeB STRING REFERENCES Nodes(id) NOT NULL, ms INTEGER)",
                            "CREATE TABLE IF NOT EXISTS Bandwidth (time TIMESTAMP, idNodeB STRING REFERENCES Nodes(id) NOT NULL, kbps FLOAT)",
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
                        "   avg(free_disk) AS mean_free_disk, variance(free_disk) AS var_free_disk, strftime('%%s','now') as lasttime  FROM Hardware");

    Report::hardware_result r;
    memset(&r,0,sizeof(Report::hardware_result));

    int err = sqlite3_exec(this->db, buf, IStorage::getHardwareCallback, &r, &zErrMsg);
    isError(err, zErrMsg, "getHardware");

    return r;
}

std::vector<Report::test_result> Storage::getLatency(int sensitivity, int64_t last) {
    char *zErrMsg = 0;
    char buf[1024];
    stringstream query;
    query << "FROM Latency AS L JOIN Nodes AS N WHERE L.idNodeB = N.id GROUP BY N.id HAVING ((abs(N.lastMeanL-avg(L.ms))/abs(N.lastMeanL) >= ("<< sensitivity <<"/100) OR abs(N.lastVarianceL-variance(L.ms))/abs(N.lastVarianceL) >= ("<< sensitivity <<"/100)) AND strftime('%s',max(L.time))>" << last << ") OR strftime('%s','now')-N.latencyTime > 180";
    std::sprintf(buf,"SELECT N.id, N.ip, N.port, avg(L.ms) AS mean, variance(L.ms) AS var, strftime('%%s',max(L.time)) as time %s", query.str().c_str());

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
    isError(err, zErrMsg, "getLatency");
    filterSend(tests);

    char buf2[2048];
    std::sprintf(buf2,  "INSERT OR REPLACE INTO Nodes (id,ip,port, latencyTime, lastMeanL, lastVarianceL, bandwidthTime, bandwidthState, lastMeanB, lastVarianceB) "
                        " SELECT A.id AS id, A.ip AS ip, A.port AS port, R.time AS latencyTime, "
                        "  R.mean AS lastMeanL, R.var AS lastVarianceL,"
                        "  A.bandwidthTime AS bandwidthTime, A.bandwidthState AS bandwidthState,"
                        "  A.lastMeanB AS lastMeanB, A.lastVarianceB AS lastVarianceB"
                        " FROM Nodes AS A"
                        "  JOIN (SELECT N.id, max(L.time) as time, avg(L.ms) AS mean, variance(L.ms) AS var %s) AS R "
                        " WHERE A.id == R.id ",query.str().c_str());

    //std::sprintf(buf2,"update Nodes SET latencyTime = R.time, lastMeanL = R.mean, lastVarianceL = R.var FROM Nodes JOIN (SELECT N.id, max(L.time) as time, avg(L.ms) AS mean, variance(L.ms) AS var %s) as R WHERE id == R.id", query.str().c_str());
    err = sqlite3_exec(this->db, buf2, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "getLatency2");

    return tests;
}

std::vector<Report::test_result> Storage::getBandwidth(int sensitivity, int64_t last) {
    char *zErrMsg = 0;
    char buf[1024];
    //std::sprintf(buf,"SELECT N.id, N.ip, N.port, avg(B.kbps) AS mean, variance(B.kbps) AS var, strftime('%%s',max(B.time)) as time FROM Bandwidth AS B JOIN Nodes AS N WHERE B.idNodeB = N.id GROUP BY N.id HAVING (( abs(N.lastMeanB-avg(B.kbps))/abs(N.lastMeanB) > (%d/100) OR abs(N.lastVarianceB-variance(B.kbps))/abs(N.lastVarianceB) > (%d/100)) AND strftime('%%s',max(B.time))>%" PRId64 ") OR strftime('%%s','now')-%" PRId64 ">300", sensitivity, sensitivity, last,last);

    stringstream query;
    query << "FROM Bandwidth AS B JOIN Nodes AS N WHERE B.idNodeB = N.id GROUP BY N.id HAVING ((abs(N.lastMeanB-avg(B.kbps))/abs(N.lastMeanB) >= ("<< sensitivity <<"/100) OR abs(N.lastVarianceB-variance(B.kbps))/abs(N.lastVarianceB) >= ("<< sensitivity <<"/100)) AND strftime('%s',max(B.time))>" << last << ") OR strftime('%s','now')-N.bandwidthTime > 180";
    std::sprintf(buf,"SELECT N.id, N.ip, N.port, avg(B.kbps) AS mean, variance(B.kbps) AS var, strftime('%%s',max(B.time)) as time %s", query.str().c_str());


    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
    isError(err, zErrMsg, "getBandwidth");
    filterSend(tests);

    char buf2[2048];
    std::sprintf(buf2,  "INSERT OR REPLACE INTO Nodes (id,ip,port, latencyTime, lastMeanL, lastVarianceL, bandwidthTime, bandwidthState, lastMeanB, lastVarianceB) "
                        " SELECT A.id AS id, A.ip AS ip, A.port AS port, A.bandwidthTime AS latencyTime, "
                        "  A.lastMeanl AS lastMeanL, A.lastVariancel AS lastVarianceL,"
                        "  R.time AS bandwidthTime, A.bandwidthState AS bandwidthState,"
                        "  R.mean AS lastMeanB, R.var AS lastVarianceB"
                        " FROM Nodes AS A"
                        "  JOIN (SELECT N.id, max(B.time) as time, avg(B.kbps) AS mean, variance(B.kbps) AS var %s) AS R "
                        " WHERE A.id == R.id ",query.str().c_str());
    //std::sprintf(buf2,"update Nodes SET bandwidthTime = R.time, lastMeanB = R.mean, lastVarianceB = R.var FROM Nodes JOIN (SELECT N.id, max(B.time) as time, avg(B.kbps) AS mean, variance(B.kbps) AS var %s) as R WHERE id == R.id", query.str().c_str());
    err = sqlite3_exec(this->db, buf2, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "getBandwidth2");

    return tests;
}

void Storage::saveState(int64_t last, int sensitivity) {
    char *zErrMsg = 0;
    char buf[2048];
    // std::sprintf(buf,   "INSERT OR REPLACE INTO Nodes (id,ip,port, latencyTime, lastMeanL, lastVarianceL, bandwidthTime, bandwidthState, lastMeanB, lastVarianceB) "
    //                     " SELECT A.id AS id, A.ip AS ip, A.port AS port, A.latencyTime AS latencyTime, "
    //                     " L.mean AS lastMeanL, L.var AS lastVarianceL,"
    //                     " A.bandwidthTime AS bandwidthTime, A.bandwidthState AS bandwidthState,"
    //                     " B.mean AS lastMeanB, B.var AS lastVarianceB"
    //                     " from Nodes AS A"
    //                     " join (SELECT N.id as id, avg(L1.ms) AS mean, variance(L1.ms) AS var %s) AS L "
    //                     " join (SELECT N.id as id, avg(B1.kbps) AS mean, variance(B1.kbps) AS var %s) AS B "
    //                     " WHERE A.id == L.id AND A.id == L.id ",
    //                     sensitivity, sensitivity, last, sensitivity, sensitivity, last);


    //int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    //isError(err, zErrMsg, "saveState");

}

void Storage::saveLatencyTest(Message::node node, int ms, int window) {
    if(node.id == "")
        return;

    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Latency (time ,idNodeB, ms) VALUES (DATETIME('now'), \"%s\", %d)", node.id.c_str(),ms);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveLatencyTest1");

    float mean = -1;

    std::sprintf(buf,"SELECT lastMeanL from Nodes WHERE id = \"%s\" LIMIT 1",node.id.c_str());
    err = sqlite3_exec(this->db, buf, getFloatCallback, &mean, &zErrMsg);
    isError(err, zErrMsg, "saveLatencyTest3");

    if ( (ms-mean)/mean > 5) {
        cout << "meanL: " << mean << " ms: " << ms << endl;
        std::sprintf(buf,"DELETE FROM Latency WHERE time < (SELECT time FROM Latency WHERE idNodeB = \"%s\" ORDER BY time DESC LIMIT 1) and idNodeB = \"%s\"",node.id.c_str(), node.id.c_str());
        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "saveLatencyTest4");

        //signal to recalc bandwidth
        std::sprintf(buf,"DELETE FROM Bandwidth WHERE idNodeB = \"%s\"",node.id.c_str());
        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "saveLatencyTest5");

        std::sprintf(buf,"UPDATE Nodes SET bandwidthState = %d WHERE id = \"%s\"", 0, node.id.c_str());
        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "saveLatencyTest6");
    }

    std::sprintf(buf,"DELETE FROM Latency WHERE time <= (SELECT time FROM Latency WHERE idNodeB = \"%s\" ORDER BY time DESC LIMIT 1 OFFSET %d) and idNodeB = \"%s\"",node.id.c_str(), window,node.id.c_str());

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveLatencyTest3");
}

void Storage::saveBandwidthTest(Message::node node, float kbps, int state, int window) {
    if(node.id == "")
        return;

    char *zErrMsg = 0;
    char buf[1024];
    int err;

    if (kbps <= 0) {
        std::sprintf(buf,"UPDATE Nodes SET bandwidthState = %d WHERE id = \"%s\"", state, node.id.c_str());
        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "saveBandwidthTest2");
        return;
    }

    std::sprintf(buf,"INSERT INTO Bandwidth (time, idNodeB, kbps) VALUES (DATETIME('now'), \"%s\", %f)", node.id.c_str(), kbps);
    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest1");

    std::sprintf(buf,"UPDATE Nodes SET bandwidthState = %d WHERE id = \"%s\"", state, node.id.c_str());
    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest2");

    float mean = -1;

    std::sprintf(buf,"SELECT lastMeanB from Nodes WHERE id = \"%s\" LIMIT 1",node.id.c_str());
    err = sqlite3_exec(this->db, buf, getFloatCallback, &mean, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest3");

    if ( (mean-kbps)/mean > 0.3) {
        cout << "meanB: " << mean << " kbps: " << kbps << endl;
        std::sprintf(buf,"DELETE FROM Bandwidth WHERE time < (SELECT time FROM Bandwidth WHERE idNodeB = \"%s\" ORDER BY time DESC LIMIT 1) and idNodeB = \"%s\"",node.id.c_str(), node.id.c_str());
        err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "saveBandwidthTest4");
    }


    std::sprintf(buf,"DELETE FROM Bandwidth WHERE time <= (SELECT time FROM Bandwidth WHERE idNodeB = \"%s\" ORDER BY time DESC LIMIT 1 OFFSET %d) and idNodeB = \"%s\"",node.id.c_str(), window, node.id.c_str());
    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveBandwidthTest4");
}

void Storage::saveHardware(Report::hardware_result hardware, int window) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"INSERT INTO Hardware (time, cores, free_cpu, memory, free_memory, disk, free_disk) VALUES (DATETIME('now'), %d, %f, %ld, %f, %" PRId64", %f)", hardware.cores, hardware.mean_free_cpu, hardware.memory, hardware.mean_free_memory, hardware.disk, hardware.mean_free_disk);

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "saveHardware1");

    std::sprintf(buf,"DELETE FROM Hardware WHERE time <= (SELECT time FROM Hardware ORDER BY time DESC LIMIT 1 OFFSET %d)", window);

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
    std::sprintf(buf,"SELECT N.id,N.ip,N.port FROM Nodes as N LEFT JOIN latency as L ON N.id == L.idNodeB GROUP BY N.id HAVING (strftime('%%s',IFNULL(max(L.time),0))+%d-strftime('%%s','now') <= 0) ORDER BY IFNULL(max(L.time),0) LIMIT %d",seconds, num);

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getLRLatency");

    return nodes;
}

std::vector<Message::node> Storage::getLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    //                SELECT N.id,N.ip,N.port FROM Nodes as N LEFT JOIN bandwidth as B ON N.id == B.idNodeB GROUP BY N.id HAVING (strftime('%s',IFNULL(max(B.time),0))+600-strftime('%s','now') <= 0) ORDER BY IFNULL(max(B.time),0) LIMIT 100
    std::sprintf(buf,"SELECT N.id,N.ip,N.port FROM Nodes as N LEFT JOIN bandwidth as B ON N.id == B.idNodeB GROUP BY N.id HAVING (strftime('%%s',IFNULL(max(B.time),0))+%d-strftime('%%s','now') <= 0) ORDER BY IFNULL(max(B.time),0) LIMIT %d",seconds, num);

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getLRBandwidth");

    return nodes;
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