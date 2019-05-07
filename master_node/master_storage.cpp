#include "master_storage.hpp"
#include <string.h>
#include <vector>
#include "storage.hpp"

using namespace std;

MasterStorage::MasterStorage() : Storage() {
}

MasterStorage::~MasterStorage() {
}

void MasterStorage::createTables() {
    Storage::createTables();
    char *zErrMsg = 0;
    
    vector<string> query = {"CREATE TABLE IF NOT EXISTS MMNodes (id INTEGER PRIMARY KEY AUTOINCREMENT, ip STRING UNIQUE)",
                            "CREATE TABLE IF NOT EXISTS MNodes (id INTEGER PRIMARY KEY AUTOINCREMENT, ip STRING UNIQUE, cores INTEGER, mean_free_cpu REAL, var_free_cpu REAL, memory INTEGER, mean_free_memory FLOAT, var_free_memory FLOAT, disk INTEGER, mean_free_disk FLOAT, var_free_disk FLOAT, lasttime TIMESTAMP, monitoredBy INTEGER REFERENCES MMNodes(id) NOT NULL)",
                            "CREATE TABLE IF NOT EXISTS MLinks (idA INTEGER REFERENCES MNodes(id) NOT NULL, idB INTEGER REFERENCES MNodes(id) NOT NULL, meanL FLOAT, varianceL FLOAT, lasttimeL TIMESTAMP, meanB FLOAT, varianceB FLOAT, lasttimeB TIMESTAMP, PRIMARY KEY(idA,idB))",
                            "CREATE TABLE IF NOT EXISTS MIots (id STRING PRIMARY KEY, desc STRING, ms INTEGER, idNode INTEGER REFERENCES MNodes(id) NOT NULL)",
                            "INSERT OR IGNORE INTO MMNodes (id, ip) VALUES (1, \"::1\")"};
    
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

void MasterStorage::addNode(std::string ip, Report::hardware_result hardware, string monitored) {
    char *zErrMsg = 0;
    char buf[1024];
    if(ip == "") {
        return;
    }
    std::sprintf(buf,"INSERT OR REPLACE INTO MNodes (id, ip, cores, mean_free_cpu, var_free_cpu, memory, mean_free_memory, var_free_memory, disk, mean_free_disk, var_free_disk, lasttime, monitoredBy) VALUES ((SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\") ,\"%s\", %ld, %f,%f, %lld, %f,%f, %lld, %f,%f , DATETIME('now'), (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.ip = \"%s\"))",
            ip.c_str(), ip.c_str(), hardware.cores, hardware.mean_free_cpu, hardware.var_free_cpu, hardware.memory, hardware.mean_free_memory, hardware.var_free_memory, hardware.disk, hardware.mean_free_disk, hardware.var_free_disk, monitored.c_str());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void MasterStorage::addIot(std::string ip, Report::IoT iot) {
    char *zErrMsg = 0;
    char buf[1024];
    if(ip == "") {
        return;
    }
    std::sprintf(buf,"INSERT OR REPLACE INTO MIots (id, desc, ms, idNode) VALUES (\"%s\", \"%s\", %d, (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\"))",iot.id.c_str(), iot.desc.c_str(), iot.latency, ip.c_str());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

vector<string> MasterStorage::getNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM MNodes WHERE monitoredBy = 1");

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (SELECT nodes): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

void MasterStorage::addTest(string strIpA, string strIpB, Report::test_result test, string type) {
    char *zErrMsg = 0;
    char buf[2048];
    if(type == string("Latency")) {
        std::sprintf(buf,
                        "INSERT OR IGNORE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                        "   VALUES ((SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\"), (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\"), %f, %f, DATETIME(%lld,\"unixepoch\"), 0, 0, DATETIME('now', '-1 month'))"
                        "; "
                        "UPDATE MLinks SET meanL = %f, varianceL = %f, lasttimeL = DATETIME(%lld,\"unixepoch\") "
                        "   WHERE idA = (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\") AND idB = (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\")",
                        strIpA.c_str(), strIpB.c_str(), test.mean, test.variance, test.lasttime, test.mean, test.variance, test.lasttime, strIpA.c_str(), strIpB.c_str());
    }else if(type == string("Bandwidth")) {
        std::sprintf(buf,
                        "INSERT OR IGNORE INTO MLinks (idA, idB, meanB, varianceB, lasttimeB, meanL, varianceL, lasttimeL) "
                        "   VALUES ((SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\"), (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\"), %f, %f, DATETIME(%lld,\"unixepoch\"), 0, 0, DATETIME('now', '-1 month'))"
                        "; "
                        "UPDATE MLinks SET meanB = %f, varianceB = %f, lasttimeB = DATETIME(%lld,\"unixepoch\") "
                        "   WHERE idA = (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\") AND idB = (SELECT MN.id FROM MNodes AS MN WHERE MN.ip = \"%s\")",
                        strIpA.c_str(), strIpB.c_str(), test.mean, test.variance, test.lasttime, test.mean, test.variance, test.lasttime, strIpA.c_str(), strIpB.c_str());
    }


    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert test): %s\n", zErrMsg);
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

 void MasterStorage::addReportIot(std::string strIp, std::vector<Report::IoT> iots) {
    for(auto iot : iots) {
        this->addIot(strIp, iot);
    }
 }

void MasterStorage::addReport(Report::report_result result, string monitored) {
    this->addNode(result.ip, result.hardware, monitored);
    this->addReportLatency(result.ip, result.latency);
    this->addReportBandwidth(result.ip, result.bandwidth);
    this->addReportIot(result.ip, result.iot);
}

void MasterStorage::addReport(std::vector<Report::report_result> results, string ip) {
    for(auto &result : results) {
        if(result.ip == string("::1"))
            result.ip = ip;
        for(auto &test : result.latency) {
            if(test.target == string("::1"))
                test.target = ip;
        }
        for(auto &test : result.bandwidth) {
            if(test.target == string("::1"))
                test.target = ip;
        }
        this->addNode(result.ip, result.hardware, ip);
    }
    for(auto result : results) {
        this->addReport(result, ip);
    }
}

std::vector<std::string> MasterStorage::getLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "SELECT ip FROM "
                        " (SELECT A.ip AS ip, A.id, B.id, Null as lasttimeL FROM MNodes as A join MNodes as B"
                        "  WHERE A.id != B.id AND A.monitoredBy = 1 AND B.monitoredBy = 1 AND NOT EXISTS (SELECT * FROM MLinks WHERE A.id = idA and B.id = idB) "
                        " UNION SELECT A.ip AS ip, A.id, B.id, C.lasttimeL FROM MNodes as A JOIN MNodes as B JOIN MLinks as C"
                        "  WHERE A.id != B.id and A.id = C.idA and B.id = C.idB AND A.monitoredBy = 1 AND B.monitoredBy = 1 AND strftime('%%s',C.lasttimeL)+%ld-strftime('%%s','now') <= 0 "
                        " order by lasttimeL limit %d) "
                        "group by ip;",
                seconds, num);
    
    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stdout, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<std::string> MasterStorage::getLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "SELECT ip FROM "
                        " (SELECT A.ip AS ip, A.id, B.id, Null as lasttimeB FROM MNodes as A join MNodes as B"
                        "  WHERE A.id != B.id AND A.monitoredBy = 1 AND B.monitoredBy = 1 AND not exists (SELECT * FROM MLinks WHERE A.id = idA and B.id = idB) "
                        " UNION SELECT A.ip AS ip, A.id, B.id, C.lasttimeB FROM MNodes as A join MNodes as B left join MLinks as C"
                        "  WHERE A.id != B.id and A.id = C.idA and B.id = C.idB AND A.monitoredBy = 1 AND B.monitoredBy = 1 AND strftime('%%s',C.lasttimeB)+%ld-strftime('%%s','now') <= 0 "
                        " order by lasttimeB limit %d) "
                        "group by ip;",
                seconds, num);
    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<std::string> MasterStorage::getLRHardware(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM MNodes WHERE monitoredBy = 1 AND strftime('%%s',lasttime)+%ld-strftime('%%s','now') <= 0 ORDER BY lasttime LIMIT %d", seconds, num);

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

vector<string> MasterStorage::getMNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM MMNodes ORDER BY RANDOM()");

    vector<string> nodes;

    int err = sqlite3_exec(this->db, buf, VectorStringCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (SELECT nodes): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

void MasterStorage::addMNode(std::string ip) {
    char *zErrMsg = 0;
    char buf[1024];
    if(ip == "") {
        return;
    }
    std::sprintf(buf,"INSERT OR REPLACE INTO MMNodes (id, ip) VALUES ((SELECT A.id FROM MMNodes AS A WHERE A.ip = \"%s\"),\"%s\")", ip.c_str(),ip.c_str());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

vector<Report::report_result> MasterStorage::getReport() {
    vector<string> ips = this->getNodes();
    vector<Report::report_result> res;

    for(auto ip : ips) {
        res.push_back(this->getReport(ip));
    }

    return res;
}

Report::hardware_result MasterStorage::getHardware(std::string ip) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT * FROM MNodes WHERE ip = \"%s\" GROUP BY ip", ip.c_str());

    Report::hardware_result r(-1,0,0,0,0,0);

    int err = sqlite3_exec(this->db, buf, IStorage::getHardwareCallback, &r, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return r;
}

std::vector<Report::test_result> MasterStorage::getLatency(std::string ip) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT N2.ip, M.meanL as mean, M.varianceL as variance, strftime('%%s',M.lasttimeL) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.ip = \"%s\" group by N2.ip", ip.c_str());

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return tests;
}

std::vector<Report::test_result> MasterStorage::getBandwidth(std::string ip) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT N2.ip, M.meanB as mean, M.varianceB as variance, strftime('%%s',M.lasttimeB) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.ip = \"%s\" group by N2.ip", ip.c_str());

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, IStorage::getTestCallback, &tests, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
 
    return tests;
}

Report::report_result MasterStorage::getReport(string strIp) {
    Report::report_result r;
    r.ip = strIp;

    r.hardware = this->getHardware(r.ip);

    r.latency = this->getLatency(r.ip);

    r.bandwidth = this->getBandwidth(r.ip);

    return r;
}

void MasterStorage::complete() {
    char *zErrMsg = 0;
    char buf[2048];
    std::sprintf(buf,
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(M.meanB) from MLinks AS M WHERE (M.idA = A.id AND M.idB <> B.id) OR (M.idA <> A.id AND M.idB = B.id)) "
                    "AS meanB, "
                    "(SELECT max(M.varianceB) from MLinks AS M WHERE (M.idA = A.id AND M.idB <> B.id) OR (M.idA <> A.id AND M.idB = B.id)) "
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.ip IN (SELECT ip FROM MMNodes) AND A.ip NOT IN (SELECT ip FROM MMNodes)");

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error1: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

        std::sprintf(buf,
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(M.meanB) from MLinks AS M WHERE (M.idA = A.id AND M.idB <> B.id) OR (M.idA <> A.id AND M.idB = B.id)) "
                    "AS meanB, "
                    "(SELECT max(M.varianceB) from MLinks AS M WHERE (M.idA = A.id AND M.idB <> B.id) OR (M.idA <> A.id AND M.idB = B.id)) "
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.ip NOT IN (SELECT ip FROM MMNodes) AND A.ip IN (SELECT ip FROM MMNodes)");


    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error2: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

        std::sprintf(buf,
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy) AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy) AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = A.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MN.id FROM MMNodes AS MMN JOIN MNodes AS MN WHERE MN.ip=MMN.ip AND MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(M.meanB) from MLinks AS M WHERE (M.idA = A.id AND M.idB <> B.id) OR (M.idA <> A.id AND M.idB = B.id)) "
                    "AS meanB, "
                    "(SELECT max(M.varianceB) from MLinks AS M WHERE (M.idA = A.id AND M.idB <> B.id) OR (M.idA <> A.id AND M.idB = B.id)) "
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.ip NOT IN (SELECT ip FROM MMNodes) AND A.ip NOT IN (SELECT ip FROM MMNodes)");

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error3: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}