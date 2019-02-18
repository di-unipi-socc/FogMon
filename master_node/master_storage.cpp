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
    
    vector<string> query = {"CREATE TABLE IF NOT EXISTS MMNodes (ip STRING PRIMARY KEY)",
                            "CREATE TABLE IF NOT EXISTS MNodes (ip STRING PRIMARY KEY, cores INTEGER, free_cpu REAL, memory INTEGER, free_memory INTEGER, disk INTEGER, free_disk INTEGER, lasttime TIMESTAMP, monitoredBy STRING)",
                            "CREATE TABLE IF NOT EXISTS MLinks (ipA STRING, ipB STRING, meanL FLOAT, varianceL FLOAT, lasttimeL TIMESTAMP, meanB FLOAT, varianceB FLOAT, lasttimeB TIMESTAMP, PRIMARY KEY(ipA,ipB))",
                            "CREATE INDEX IF NOT EXISTS MlastNodes ON MNodes(lasttime)",
                            "CREATE INDEX IF NOT EXISTS MlastBandwidth ON MLinks(lasttimeB)",
                            "CREATE INDEX IF NOT EXISTS MlastLatency ON MLinks(lasttimeL)"};
    
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
    std::sprintf(buf,"INSERT OR REPLACE INTO MNodes (ip, cores, free_cpu, memory, free_memory, disk, free_disk, lasttime, monitored) VALUES (\"%s\", %d, %f, %d, %d, %d, %d, DATETIME('now'), %d)", ip.c_str(), hardware.cores, hardware.free_cpu, hardware.memory, hardware.free_memory, hardware.disk, hardware.free_disk, monitored.c_str());

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
    std::sprintf(buf,"SELECT ip FROM MNodes");

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
    char buf[1024];
    if(type == string("Latency")) {
        std::sprintf(buf,
                        "INSERT OR REPLACE INTO MLinks (ipA, ipB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                        "VALUES (\"%s\", \"%s\", %f, %f, DATETIME(%d,\"unixepoch\"), "
                        "(SELECT meanB FROM MLinks WHERE ipA = \"%s\" AND ipA = \"%s\"), "
                        "(SELECT varianceB FROM MLinks WHERE ipA = \"%s\" AND ipA = \"%s\"), "
                        "(SELECT lasttimeB FROM MLinks WHERE ipA = \"%s\" AND ipA = \"%s\") ",
                        strIpA.c_str(), strIpB.c_str() , test.mean, test.variance, test.lasttime, strIpA.c_str(), strIpB.c_str(), strIpA.c_str(), strIpB.c_str(), strIpA.c_str(), strIpB.c_str());
    }else if(type == string("Bandwidth")) {
        std::sprintf(buf,
                        "INSERT OR REPLACE INTO MLinks (ipA, ipB, meanB, varianceB, lasttimeB, meanL, varianceL, lasttimeL) "
                        "VALUES (\"%s\", \"%s\", %f, %f, DATETIME(%d,\"unixepoch\"), "
                        "(SELECT meanL FROM MLinks WHERE ipA = \"%s\" AND ipA = \"%s\"), "
                        "(SELECT varianceL FROM MLinks WHERE ipA = \"%s\" AND ipA = \"%s\"), "
                        "(SELECT lasttimeL FROM MLinks WHERE ipA = \"%s\" AND ipA = \"%s\") ",
                        strIpA.c_str(), strIpB.c_str() , test.mean, test.variance, test.lasttime, strIpA.c_str(), strIpB.c_str(), strIpA.c_str(), strIpB.c_str(), strIpA.c_str(), strIpB.c_str());

    }
    
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

void MasterStorage::addReport(Report::report_result result, string monitored) {
    this->addNode(result.ip, result.hardware, monitored);
    this->addReportLatency(result.ip, result.latency);
    this->addReportBandwidth(result.ip, result.bandwidth);
}

void MasterStorage::addReport(std::vector<Report::report_result> results, string ip) {
    for(auto result : results) {
        this->addReport(result, ip);
    }
}

std::vector<std::string> MasterStorage::getLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "SELECT ip FROM "
                        " (SELECT A.ip, B.ip, Null as lasttimeL FROM MNodes as A join MNodes as B"
                        "  WHERE A.ip != B.ip and not exists (SELECT * FROM MLinks WHERE A.ip = ipA and B.ip = ipB) "
                        " UNION SELECT A.ip, B.ip, C.lasttimeL FROM MNodes as A join MNodes as B left join MLinks as C"
                        "  WHERE A.ip != B.ip and A.ip = C.ipA and B.ip = C.ipB and strftime('%%s',C.lasttimeL)+%d-strftime('%%s','now') <= 0 "
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
                        " (SELECT A.ip, B.ip, Null as lasttimeB FROM MNodes as A join MNodes as B"
                        "  WHERE A.ip != B.ip and not exists (SELECT * FROM MLinks WHERE A.ip = ipA and B.ip = ipB) "
                        " UNION SELECT A.ip, B.ip, C.lasttimeB FROM MNodes as A join MNodes as B left join MLinks as C"
                        "  WHERE A.ip != B.ip and A.ip = C.ipA and B.ip = C.ipB and strftime('%%s',C.lasttimeB)+%d-strftime('%%s','now') <= 0 "
                        " order by lasttimeB limit %d) "
                        "group by ip;",
                seconds, num);
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
    std::sprintf(buf,"SELECT ip FROM MNodes WHERE strftime('%%s',lasttime)+%d-strftime('%%s','now') <= 0 ORDER BY lasttime LIMIT %d", seconds, num);

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

vector<string> MasterStorage::getMNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ip FROM MMNodes");

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
    std::sprintf(buf,"INSERT OR REPLACE INTO MMNodes (ip) VALUES (\"%s\")", ip.c_str());

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
    std::sprintf(buf,"SELECT * FROM MHardware WHERE ip = \"%s\" ORDER BY time LIMIT 1", ip.c_str());

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

std::vector<Report::test_result> MasterStorage::getLatency(std::string ip) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipB, meanL as mean, varianceL as variance, lasttimeL as time FROM MLinks WHERE ipA = \"%s\" group by ipB", ip.c_str());

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

std::vector<Report::test_result> MasterStorage::getBandwidth(std::string ip) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT ipB, meanB as mean, varianceB as variance, lasttime as time FROM MBandwidth WHERE ipA = \"%s\" group by ipB", ip.c_str());

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
    char buf[1024];
    std::sprintf(buf,
                "INSERT OR REPLACE INTO MLinks (ipA, ipB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.ip as ipA, B.ip as ipB, "
                    "(SELECT meanL from MLinks WHERE ipA = A.monitoredBy AND ipB = B.monitoredBy) + "
                    "(SELECT meanL from MLinks WHERE ipA = A.ip AND ipB = A.monitoredBy) + "
                    "(SELECT meanL from MLinks WHERE ipA = B.monitoredBy AND ipB = B.ip) "
                    "as meanL, "
                    "(SELECT varianceL from MLinks WHERE ipA = A.monitoredBy AND ipB = B.monitoredBy) + "
                    "(SELECT varianceL from MLinks WHERE ipA = A.ip AND ipB = A.monitoredBy) + "
                    "(SELECT varianceL from MLinks WHERE ipA = B.monitoredBy AND ipB = B.ip) "
                    "as varianceL, NULL as lasttimeL, "
                    "(SELECT min(meanB) from MLinks WHERE (ipA = A.ip AND ipB <> B.ip) OR (ipA <> A.ip AND ipB = B.ip) "
                    "as meanB, "
                    "(SELECT max(varianceB) from MLinks WHERE (ipA = A.ip AND ipB <> B.ip) OR (ipA <> A.ip AND ipB = B.ip) "
                    "as varianceB, NULL as lasttimeB "
                "  FROM Nodes as A JOIN Nodes as B WHERE A.ip <> B.ip");

    vector<Report::test_result> tests;

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}