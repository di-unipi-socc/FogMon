#include "leader_storage.hpp"
#include <string.h>
#include <vector>
#include <sstream>
#include <inttypes.h>
#include "storage.hpp"
#include <chrono>
#include <iostream>

using namespace std;

LeaderStorage::LeaderStorage(Message::node node) : Storage() {
    this->nodeM = node;
}

LeaderStorage::~LeaderStorage() {
}

void LeaderStorage::createTables() {
    Storage::createTables();
    char *zErrMsg = 0;
    
    vector<string> query = {"CREATE TABLE IF NOT EXISTS MMNodes (id STRING PRIMARY KEY, ip STRING NOT NULL, port STRING NOT NULL, UNIQUE(ip, port))",
                            "CREATE TABLE IF NOT EXISTS MNodes (id STRING PRIMARY KEY, ip STRING NOT NULL, port STRING NOT NULL, cores INTEGER, mean_free_cpu REAL, var_free_cpu REAL, memory INTEGER, mean_free_memory FLOAT, var_free_memory FLOAT, disk INTEGER, mean_free_disk FLOAT, var_free_disk FLOAT, lasttime TIMESTAMP, monitoredBy STRING REFERENCES MMNodes(id) NOT NULL, UNIQUE(ip, port))",
                            "CREATE TABLE IF NOT EXISTS MLinks (idA STRING REFERENCES MNodes(id) NOT NULL, idB STRING REFERENCES MNodes(id) NOT NULL, meanL FLOAT, varianceL FLOAT, lasttimeL TIMESTAMP, meanB FLOAT, varianceB FLOAT, lasttimeB TIMESTAMP, PRIMARY KEY(idA,idB))",
                            "CREATE TABLE IF NOT EXISTS MIots (id STRING PRIMARY KEY, desc STRING, ms INTEGER, idNode STRING REFERENCES MNodes(id) NOT NULL)",
                            "DELETE FROM MMNodes"};
    
    for(string str : query) {
        int err = executeQuery(str, {}, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "createTablesLeader");
    }
    std::string str = "INSERT OR IGNORE INTO MMNodes (id, ip, port) VALUES (?,?,?)";
    int err = executeQuery(str, {this->nodeM.id, "::1", this->nodeM.port}, 0, 0, &zErrMsg);
}

Message::node LeaderStorage::getNode() {
    return this->nodeM;
}

// monitored should be nullptr if is the leader itself reporting, and the id if another leader is reporting 
std::string LeaderStorage::addNode(Message::node node, Report::hardware_result hardware, Message::node *monitored) {
    if(hardware.lasttime == 0) { //these are directly measured
        return "";
    }
    char *zErrMsg = 0;
    vector<long long> res;

    int err = executeQuery("SELECT strftime('%s',lasttime) FROM MNodes WHERE (strftime('%s',lasttime)-? > 0) AND (id = ?)", {hardware.lasttime, node.id}, IStorage::VectorIntCallback, &res, &zErrMsg);
    isError(err, zErrMsg, "addNodeLeader0");

    if(!res.size()) {
        std::string monitoredBy = this->nodeM.id;
        if(monitored) {
            // block report about this leader from other nodes
            if (monitored->id == this->nodeM.id)
                return "";
            monitoredBy = monitored->id;
            // printf("report: %s --> %s lasttime: %ld\n",monitored->id.c_str(), node.id.c_str(),hardware.lasttime);
        }
        std::string query = "INSERT OR REPLACE INTO MNodes"
                            " (id, ip, port, cores, mean_free_cpu, var_free_cpu, memory, mean_free_memory, var_free_memory, disk, mean_free_disk, var_free_disk, lasttime, monitoredBy)"
                            " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,DATETIME(?,'unixepoch'),?)";
        err = executeQuery(
            query, 
            {   
                node.id, 
                node.ip, 
                node.port, 
                hardware.cores, 
                hardware.mean_free_cpu, 
                hardware.var_free_cpu, 
                hardware.memory, 
                hardware.mean_free_memory, 
                hardware.var_free_memory, 
                hardware.disk, 
                hardware.mean_free_disk, 
                hardware.var_free_disk, 
                hardware.lasttime, 
                monitoredBy
            }, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "addNodeLeader1");
        return node.id;
    }
    return "";
}

void LeaderStorage::addIot(Message::node node, Report::IoT iot) {
    char *zErrMsg = 0;
    char buf[1024];
    if(node.id == "") {
        return;
    }
    // std::sprintf(buf,"INSERT OR REPLACE INTO MIots (id, desc, ms, idNode) VALUES (\"%s\", \"%s\", %d, \"%s\")",iot.id.c_str(), iot.desc.c_str(), iot.latency, node.id.c_str());

    // int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    int err = executeQuery("INSERT OR REPLACE INTO MIots (id, desc, ms, idNode) VALUES (?,?,?,?)", {iot.id, iot.desc, iot.latency, node.id}, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "addIotLeader");
}

std::vector<Message::node> LeaderStorage::getAllNodes() {
    char *zErrMsg = 0;
    vector<Message::node> nodes;

    int err = executeQuery("SELECT id,ip,port FROM MNodes", {}, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getAllNodesLeader");

    return nodes;
}

vector<Message::node> LeaderStorage::getNodes() {
    char *zErrMsg = 0;
    vector<Message::node> nodes;

    int err = executeQuery("SELECT id,ip,port FROM MNodes WHERE monitoredBy = ?", {this->nodeM.id}, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getNodesLeader");

    return nodes;
}

void LeaderStorage::addTest(Message::node nodeA, Message::node nodeB, Report::test_result test, string type) {
    if(test.lasttime == 0) { //these are directly measured
        return;
    }
    char *zErrMsg = 0;
    std::string query;

    if(type == string("Latency")) {
        query = "INSERT OR IGNORE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                "   VALUES (?,?,?,?,DATETIME(?,'unixepoch'),0,0,NULL)"
                "; "
                "UPDATE MLinks SET meanL = ?, varianceL = ?, lasttimeL = DATETIME(?,'unixepoch') "
                "   WHERE idA = ? AND idB = ? AND (lasttimeL < DATETIME( ?,'unixepoch') OR lasttimeL is Null)";
    }else if(type == string("Bandwidth")) {
        query = "INSERT OR IGNORE INTO MLinks (idA, idB, meanB, varianceB, lasttimeB, meanL, varianceL, lasttimeL) "
                "   VALUES (?,?,?,?,DATETIME(?,'unixepoch'),0,0,NULL)"
                "; "
                "UPDATE MLinks SET meanB = ?, varianceB = ?, lasttimeB = DATETIME(?,'unixepoch') "
                "   WHERE idA = ? AND idB = ? AND (lasttimeB < DATETIME( ?,'unixepoch') OR lasttimeB is Null)";
    }

    int err = executeQuery(
        query, 
        {nodeA.id, nodeB.id, test.mean, test.variance, test.lasttime, test.mean, test.variance, test.lasttime, nodeA.id, nodeB.id, test.lasttime}, nullptr, nullptr, &zErrMsg);
    isError(err, zErrMsg, "addTestLeader");
}

void LeaderStorage::addReportLatency(Message::node node, vector<Report::test_result> latency) {
    for(auto test : latency) {
        this->addTest(node, test.target, test, "Latency");
        this->addTest(test.target, node, test, "Latency"); //because is symmetric
    }
}

void LeaderStorage::addReportBandwidth(Message::node node, vector<Report::test_result> bandwidth) {
    for(auto test : bandwidth) {
        this->addTest(node, test.target, test, "Bandwidth"); //asymmetric
    }
}

 void LeaderStorage::addReportIot(Message::node node, std::vector<Report::IoT> iots) {
    char *zErrMsg = 0;
    char buf[1024];
    if(node.id == "") {
        return;
    }
    int err = executeQuery("DELETE FROM MIots WHERE idNode = ?", {node.id}, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "addReportIotLeader");

    for(auto iot : iots) {
        this->addIot(node, iot);
    }
 }

void LeaderStorage::addReport(Report::report_result result, Message::node *monitored) {
    // printf("adding report: %s\n", result.source.ip.c_str());
    if( this->addNode(result.source, result.hardware, monitored) != "") {
        // printf("added report: %s\n", result.source.ip.c_str());
        this->addReportLatency(result.source, result.latency);        
        this->addReportBandwidth(result.source, result.bandwidth);
        this->addReportIot(result.source, result.iot);
    }
}

void LeaderStorage::addReport(std::vector<Report::report_result> results, Message::node node) {                    
    for(auto &result : results) {
        if(result.source.ip == string("::1"))
            result.source.ip = node.ip;
        for(auto &test : result.latency) {
            if(test.target.ip == string("::1"))
                test.target.ip = node.ip;
        }
        for(auto &test : result.bandwidth) {
            if(test.target.ip == string("::1"))
                test.target.ip = node.ip;
        }
        Message::node leader(result.leader,"","");
        this->addNode(result.source, result.hardware, &leader);
    }

    for(auto result : results) {
        Message::node leader(result.leader,"","");
        this->addReport(result, &leader);
    }
}

// get least monitored latency
std::vector<Message::node> LeaderStorage::getMLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    vector<Message::node> nodes;

    std::string query = "SELECT id,ip,port FROM "
                        " (SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, Null as lasttimeL FROM MNodes as A join MNodes as B"
                        "  WHERE A.id != B.id AND A.monitoredBy = ? AND B.monitoredBy = ? AND NOT EXISTS (SELECT * FROM MLinks WHERE A.id = idA and B.id = idB) "
                        " UNION SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, C.lasttimeL FROM MNodes as A JOIN MNodes as B JOIN MLinks as C"
                        "  WHERE A.id != B.id and A.id = C.idA and B.id = C.idB AND A.monitoredBy = ? AND B.monitoredBy = ? AND strftime('%s',C.lasttimeL)+?-strftime('%s','now') <= 0 "
                        " order by lasttimeL limit ?) "
                        "group by id;";
    int err = executeQuery(query, {this->nodeM.id, this->nodeM.id, this->nodeM.id, this->nodeM.id, seconds, num}, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getMLRLatencyLeader");

    return nodes;
}

std::vector<Message::node> LeaderStorage::getMLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    vector<Message::node> nodes;

    std::string query = "SELECT id,ip,port FROM "
                        " (SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, Null as lasttimeB FROM MNodes as A join MNodes as B"
                        "  WHERE A.id != B.id AND A.monitoredBy = ? AND B.monitoredBy = ? AND NOT EXISTS (SELECT * FROM MLinks WHERE A.id = idA and B.id = idB) "
                        " UNION SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, C.lasttimeB FROM MNodes as A JOIN MNodes as B JOIN MLinks as C"
                        "  WHERE A.id != B.id and A.id = C.idA and B.id = C.idB AND A.monitoredBy = ? AND B.monitoredBy = ? AND strftime('%s',C.lasttimeB)+?-strftime('%s','now') <= 0 "
                        " order by lasttimeB limit ?) "
                        "group by id;";

    int err = executeQuery(query, {this->nodeM.id, this->nodeM.id, this->nodeM.id, this->nodeM.id, seconds, num}, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getMLRBandwidthLeader");

    return nodes;
}

std::vector<Message::node> LeaderStorage::getMLRHardware(int num, int seconds) {
    char *zErrMsg = 0;
    vector<Message::node> nodes;

    std::string query = "SELECT id,ip,port FROM MNodes WHERE monitoredBy = ? AND strftime('%s',lasttime)+ ? -strftime('%s','now') <= 0 ORDER BY lasttime LIMIT ?";
    int err = executeQuery(query, {this->nodeM.id, seconds, num}, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getMLRHardwareLeader");

    return nodes;
}

vector<Message::node> LeaderStorage::getMNodes() {
    char *zErrMsg = 0;
    vector<Message::node> nodes;

    int err = executeQuery("SELECT id,ip,port FROM MMNodes ORDER BY RANDOM()", {}, VectorNodeCallback, &nodes, &zErrMsg);
    isError(err, zErrMsg, "getMNodesLeader");

    return nodes;
}

void LeaderStorage::addMNode(Message::node node) {
    char *zErrMsg = 0;
    if(this->ip == "") {
        return;
    }
    int err = executeQuery("INSERT OR REPLACE INTO MMNodes (id, ip, port) VALUES (?,?,?)", {node.id, node.ip, node.port}, 0, 0, &zErrMsg);
    isError(err, zErrMsg, "addMNodeLeader");
}

vector<Report::report_result> LeaderStorage::getReport(bool complete) {
    //sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    vector<Message::node> nodes = this->getAllNodes();
    vector<Report::report_result> res;

    for(auto node : nodes) {
        res.push_back(this->getReport(node, complete));
    }
    //sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
    return res;
}

Report::hardware_result LeaderStorage::getHardware(Message::node node) {
    char *zErrMsg = 0;
    Report::hardware_result r(-1,0,0,0,0,0,0);

    std::string query = "SELECT cores, mean_free_cpu, var_free_cpu, memory, mean_free_memory, var_free_memory, disk, mean_free_disk, var_free_disk, (CASE WHEN ? = N.monitoredBy THEN strftime('%s','now') ELSE strftime('%s',lasttime) END) as lasttime FROM MNodes as N WHERE id = ? GROUP BY ip";
    int err = executeQuery(query, {this->nodeM.id, node.id}, IStorage::getHardwareCallback, &r, &zErrMsg);
    isError(err, zErrMsg, "getHardwareLeader");

    return r;
}

std::vector<Report::test_result> LeaderStorage::getLatency(Message::node node, bool complete) {
    char *zErrMsg = 0;
    std::string query;
    if (complete) {
        query = "SELECT N2.id, N2.ip, N2.port, M.meanL as mean, M.varianceL as variance, strftime('%s',M.lasttimeL) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.id = ? group by N2.id";
    }else {
        query = "SELECT N2.id, N2.ip, N2.port, M.meanL as mean, M.varianceL as variance, strftime('%s',M.lasttimeL) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.id = ? AND M.lasttimeL <> 0 AND M.lasttimeL <> datetime('1970-01-01 00:00:00') group by N2.id";
    }
    vector<Report::test_result> tests;

    int err = executeQuery(query, {node.id}, IStorage::getTestCallback, &tests, &zErrMsg);
    isError(err, zErrMsg, "getLatencyLeader");

    return tests;
}

std::vector<Report::test_result> LeaderStorage::getBandwidth(Message::node node, bool complete) {
    char *zErrMsg = 0;
    std::string query;

    if (complete) {
        query = "SELECT N2.id, N2.ip, N2.port, M.meanB as mean, M.varianceB as variance, strftime('%s',M.lasttimeB) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.id = ? group by N2.id";
    }else {
        query = "SELECT N2.id, N2.ip, N2.port, M.meanB as mean, M.varianceB as variance, strftime('%s',M.lasttimeB) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.id = ? AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') group by N2.id";
    }

    vector<Report::test_result> tests;

    int err = executeQuery(query, {node.id}, IStorage::getTestCallback, &tests, &zErrMsg);
    isError(err, zErrMsg, "getBandwidthLeader");
 
    return tests;
}

Report::report_result LeaderStorage::getReport(Message::node node, bool complete) {
    Report::report_result r;
    r.source = node;

    r.hardware = this->getHardware(r.source);

    r.latency = this->getLatency(r.source, complete);

    r.bandwidth = this->getBandwidth(r.source, complete);

    vector<string> vec;
    char *zErrMsg = 0;
    int err = executeQuery("SELECT monitoredBy FROM MNodes WHERE id = ?", {node.id}, IStorage::VectorStringCallback, &vec, &zErrMsg);
    isError(err, zErrMsg, "getReport getting leader");
    if (vec.size()!=0)
        r.leader = vec[0];
    else
        r.leader = "";
    return r;
}


vector<Message::node> LeaderStorage::removeOldLNodes(int seconds, int &leaders_num, bool force) {
    char *zErrMsg = 0;
    std::string query;
    //sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    vector<string> leaders;

    if(force) {
        //       SELECT L.id FROM MMNodes as L LEFT JOIN MNodes as N ON L.id==N.monitoredBy GROUP BY monitoredBy having strftime('%s',IFNULL(max(lasttime),0))+100-strftime('%s','now') <= 0
        query = "SELECT L.id FROM MMNodes as L LEFT JOIN MNodes as N ON L.id==N.monitoredBy GROUP BY monitoredBy having strftime('%s',IFNULL(max(lasttime),0))+?-strftime('%s','now') <= 0";
    }else {
        query = "SELECT monitoredBy FROM MNodes GROUP BY monitoredBy having strftime('%s',max(lasttime))+?-strftime('%s','now') <= 0";
    }
    
    int err = executeQuery(query, {seconds, seconds}, IStorage::VectorStringCallback, &leaders, &zErrMsg);
    isError(err, zErrMsg, "removeOldLNodesLeader1");

    query = "SELECT strftime('%s',max(lasttime))-strftime('%s','now') FROM MNodes GROUP BY monitoredBy having strftime('%s',max(lasttime))+?-strftime('%s','now') <= 0";
    vector<int> lasttimes;

    err = executeQuery(query, {seconds, seconds}, IStorage::VectorIntCallback, &lasttimes, &zErrMsg);
    isError(err, zErrMsg, "removeOldLNodesLeader1");

    for(auto lasttime : lasttimes) {
        printf("Lasttime: %d\n", lasttime);
    }

    vector<Message::node> rem;

    leaders_num = leaders.size();

    for(string leader : leaders) {
        if(leader == this->nodeM.id)
            continue;
        printf("Delete leader: %s\n", leader.c_str());
        rem.push_back(Message::node(leader,"",""));
        err = executeQuery("DELETE FROM MMNodes WHERE id = ?", {leader}, nullptr, nullptr, &zErrMsg);
        isError(err, zErrMsg, "removeOldNodesLeader2");

        vector<string> ids;
        err = executeQuery("SELECT id FROM MNodes WHERE monitoredBy = ?", {leader}, IStorage::VectorStringCallback, &ids, &zErrMsg);
        isError(err, zErrMsg, "removeOldLNodesLeader3");

        err = executeQuery("DELETE FROM MNodes WHERE monitoredBy = ?", {leader}, nullptr, nullptr, &zErrMsg);
        isError(err, zErrMsg, "removeOldNodesLeader4");

        for(string id : ids) {
            rem.push_back(Message::node(id,"",""));
            err = executeQuery("DELETE FROM MLinks WHERE idA = ? OR idB = ?", {id, id}, nullptr, nullptr, &zErrMsg);
            isError(err, zErrMsg, "removeOldLNodesLeader5");

            err = executeQuery("DELETE FROM MIots WHERE idNode = ?", {id}, nullptr, nullptr, &zErrMsg);
            isError(err, zErrMsg, "removeOldLNodesLeader6");
        }
    }

    // removing also other leaders node that do not respond anymore
    vector<Message::node> vec;

    err = executeQuery("SELECT id,ip,port FROM MNodes WHERE monitoredBy <> ? AND strftime('%s',lasttime)+?-strftime('%s','now') <= 0", {this->nodeM.id, seconds}, IStorage::VectorNodeCallback, &vec, &zErrMsg);
    isError(err, zErrMsg, "removeOldLNodesLeader7");

    err = executeQuery("DELETE FROM MNodes WHERE monitoredBy <> ? AND strftime('%s',lasttime)+?-strftime('%s','now') <= 0", {this->nodeM.id, seconds}, nullptr, nullptr, &zErrMsg);
    isError(err, zErrMsg, "removeOldLNodesLeader8");

    for(auto node : vec) {
        printf("Delete followerL: %s\n", node.ip.c_str());
        err = executeQuery("DELETE FROM MLinks WHERE idA = ? OR idB = ?", {node.id, node.id}, nullptr, nullptr, &zErrMsg);
        isError(err, zErrMsg, "removeOldLNodesLeader9");

        err = executeQuery("DELETE FROM MIots WHERE idNode = ?", {node.id}, nullptr, nullptr, &zErrMsg);
        isError(err, zErrMsg, "removeOldLNodesLeader10");
    }

    //sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);

    rem.insert(rem.end(), vec.begin(), vec.end());
    return rem;
}

vector<Message::node> LeaderStorage::removeOldNodes(int seconds) {
    char *zErrMsg = 0;
    char buf[1024];

    //sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    std::vector<Message::node> vec = this->getMLRHardware(100, seconds);

    int err = executeQuery("DELETE FROM MNodes WHERE monitoredBy = ? AND strftime('%s',lasttime)+?-strftime('%s','now') <= 0", {this->nodeM.id, seconds}, nullptr, nullptr, &zErrMsg);
    isError(err, zErrMsg, "removeOldNodesLeader1");

    for(auto node : vec) {
        printf("Delete follower: %s\n", node.ip.c_str());
        err = executeQuery("DELETE FROM MLinks WHERE idA = ? OR idB = ?", {node.id, node.id}, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "removeOldNodesLeader2");

        err = executeQuery("DELETE FROM MIots WHERE idNode = ?", {node.id}, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "removeOldNodesLeader3");
    }
    //sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
    return vec;   
}

void LeaderStorage::removeChangeRole(std::vector<Message::node> leaders) {
    std::vector<Message::node> nodes = this->getMNodes();
    std::vector<Message::node> vec;
    for(auto node : nodes) {
        bool found = false;
        for(auto selected : leaders) {
            if(node.id == selected.id) {
                found = true;
            }
        }
        if(!found) {
            vec.push_back(node);
        }
    }
    char *zErrMsg = 0;

    //sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    for(auto node : vec) {
        printf("Delete leader (change): %s\n", node.ip.c_str());
        string leader = node.id;

        int err = executeQuery("DELETE FROM MMNodes WHERE id = ?", {leader}, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "removeOldNodesLeader2");

        vector<string> ids;
        err = executeQuery("SELECT id FROM MNodes WHERE monitoredBy = ?", {leader}, IStorage::VectorStringCallback, &ids, &zErrMsg);
        isError(err, zErrMsg, "removeOldLNodesLeader3");

        err = executeQuery("DELETE FROM MNodes WHERE monitoredBy = ?", {leader}, 0, 0, &zErrMsg);
        isError(err, zErrMsg, "removeOldNodesLeader4");

        for(string id : ids) {
            err = executeQuery("DELETE FROM MLinks WHERE idA = ? OR idB = ?", {id, id}, 0, 0, &zErrMsg);
            isError(err, zErrMsg, "removeOldLNodesLeader5");

            err = executeQuery("DELETE FROM MIots WHERE idNode = ?", {id}, 0, 0, &zErrMsg);
            isError(err, zErrMsg, "removeOldLNodesLeader6");
        }
    }
    //sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);  
}

void LeaderStorage::complete() {
    char *zErrMsg = 0;

    std::string query =
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(meanB) from "
                    "   (SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION "
                    "    SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idB = B.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION "
                    "    SELECT M.meanB      as meanB from MLinks as M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) )"
                    ") AS meanB, "
                    "(SELECT max(varianceB) from "
                    "   (SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.id IN (SELECT id FROM MMNodes) AND B.id NOT IN (SELECT id FROM MMNodes)";

    int err = executeQuery(query, {}, nullptr, nullptr, &zErrMsg);
    isError(err, zErrMsg, "completeLeader1");

    /*  

    select A.ip, B.ip, meanB, lasttimeB from mlinks as M join mnodes as A ON A.id == M.idA join mnodes as B ON B.id == M.idB where lasttimeB<>0;
    

    select A.ip, B.ip, 
    (SELECT min(meanB) from
        (SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION 
                            SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idB = B.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION 
                            SELECT M.meanB      as meanB from MLinks as M WHERE M.idB = B.id AND M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) 
        )
    ) AS
    meanB
    from mlinks as M join mnodes as A ON A.id == M.idA join mnodes as B ON B.id == M.idB where A.ip == "node20" and B.ip == "node11";

    */
    query =
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(meanB) from "
                    "   (SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION "
                    "    SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idB = B.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION "
                    "    SELECT M.meanB      as meanB from MLinks as M WHERE M.idB = B.id AND M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) )"
                    ") AS meanB, "
                    "(SELECT max(varianceB) from "
                    "   (SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.id NOT IN (SELECT id FROM MMNodes) AND B.id IN (SELECT id FROM MMNodes)";


    err = executeQuery(query, {}, nullptr, nullptr, &zErrMsg);
    isError(err, zErrMsg, "completeLeader2");

    query =
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(meanB) from "
                    "   (SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION "
                    "    SELECT max(M.meanB) as meanB from MLinks AS M WHERE M.idB = B.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') UNION "
                    "    SELECT M.meanB      as meanB from MLinks as M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) )"
                    ") AS meanB, "
                    "(SELECT max(varianceB) from "
                    "   (SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.id NOT IN (SELECT id FROM MMNodes) AND B.id NOT IN (SELECT id FROM MMNodes)";

    err = executeQuery(query, {}, nullptr, nullptr, &zErrMsg);
    isError(err, zErrMsg, "completeLeader3");

    this->flush();
}