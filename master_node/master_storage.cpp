#include "master_storage.hpp"
#include <string.h>
#include <vector>
#include <sstream>
#include "storage.hpp"

using namespace std;

MasterStorage::MasterStorage(Message::node node) : Storage() {
    this->nodeM = node;
}

MasterStorage::~MasterStorage() {
}

void MasterStorage::createTables() {
    Storage::createTables();
    char *zErrMsg = 0;
    
    vector<string> query = {"CREATE TABLE IF NOT EXISTS MMNodes (id STRING PRIMARY KEY, ip STRING NOT NULL, port STRING NOT NULL, UNIQUE(ip, port))",
                            "CREATE TABLE IF NOT EXISTS MNodes (id STRING PRIMARY KEY, ip STRING NOT NULL, port STRING NOT NULL, cores INTEGER, mean_free_cpu REAL, var_free_cpu REAL, memory INTEGER, mean_free_memory FLOAT, var_free_memory FLOAT, disk INTEGER, mean_free_disk FLOAT, var_free_disk FLOAT, lasttime TIMESTAMP, monitoredBy STRING REFERENCES MMNodes(id) NOT NULL, UNIQUE(ip, port))",
                            "CREATE TABLE IF NOT EXISTS MLinks (idA STRING REFERENCES MNodes(id) NOT NULL, idB STRING REFERENCES MNodes(id) NOT NULL, meanL FLOAT, varianceL FLOAT, lasttimeL TIMESTAMP, meanB FLOAT, varianceB FLOAT, lasttimeB TIMESTAMP, PRIMARY KEY(idA,idB))",
                            "CREATE TABLE IF NOT EXISTS MIots (id STRING PRIMARY KEY, desc STRING, ms INTEGER, idNode STRING REFERENCES MNodes(id) NOT NULL)",
                            string("INSERT OR IGNORE INTO MMNodes (id, ip, port) VALUES (\"")+ this->nodeM.id+ string("\", \"::1\", \""+ this->nodeM.port +"\")")};
    
    for(string str : query) {
        int err = sqlite3_exec(this->db, str.c_str(), 0, 0, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error (creating tables %s): %s\n", str.c_str(), zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }
    }
}

Message::node MasterStorage::getNode() {
    return this->nodeM;
}

std::string MasterStorage::addNode(Message::node node, Report::hardware_result hardware, Message::node *monitored) {
    char *zErrMsg = 0;
    
    if(node.id == "") 
    {
        //get old id if present, else generate the new id

        stringstream query;
        vector<Message::node> nodes;

        query << "SELECT id,ip,port FROM MNodes WHERE ip = \""+ node.ip+"\" AND port = \""+node.port+"\" AND monitoredBy = \""<< this->nodeM.id <<"\" LIMIT 1";
        int err = sqlite3_exec(this->db, query.str().c_str(), VectorNodeCallback, &nodes, &zErrMsg);
        if( err!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error (get node %s): %s\n", query.str().c_str(),zErrMsg);
            sqlite3_free(zErrMsg);
            exit(1);
        }
        if(nodes.size() > 0) {
            //exist alread
            node.id = nodes[0].id;
        }else {
            stringstream query;
            vector<Message::node> nodes;

            query << "SELECT id,ip,port FROM MNodes WHERE monitoredBy = \""<< this->nodeM.id <<"\" ORDER BY id DESC LIMIT 1";
            int err = sqlite3_exec(this->db, query.str().c_str(), VectorNodeCallback, &nodes, &zErrMsg);
            if( err!=SQLITE_OK )
            {
                fprintf(stderr, "SQL error (get node %s): %s\n", query.str().c_str(),zErrMsg);
                sqlite3_free(zErrMsg);
                exit(1);
            }

            int base = 0;

            if(nodes.size() > 0) {
                try {
                    int num = stol(nodes[0].id.substr(this->nodeM.id.size()+1));
                    base = num;
                }catch (...) {

                }
            }
            char id[256];
            snprintf(id,256,"%s-%016d",this->nodeM.id.c_str(),base+1);
            node.id = string(id);
        }
    }

    
    stringstream query;
    query <<    "INSERT OR REPLACE INTO MNodes"
                " (id, ip, port, cores, mean_free_cpu, var_free_cpu, memory, mean_free_memory, var_free_memory, disk, mean_free_disk, var_free_disk, lasttime, monitoredBy)"
                " VALUES (\""<< node.id <<"\", \""<< node.ip <<"\", \""<< node.port <<"\", "<<
                    hardware.cores <<", "<< hardware.mean_free_cpu <<", "<< hardware.var_free_cpu <<", "<<
                    hardware.memory <<", "<< hardware.mean_free_memory <<", "<< hardware.var_free_memory <<", "<<
                    hardware.disk <<", "<< hardware.mean_free_disk <<", "<< hardware.var_free_disk <<", DATETIME('now'),";
    if(!monitored) {
        query << " \"" << this->nodeM.id <<"\")";
    }else {
        query << " \""<< monitored->id <<"\")";
    }

    int err = sqlite3_exec(this->db, query.str().c_str(), 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node %s): %s\n", query.str().c_str(),zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    return node.id;
}

void MasterStorage::addIot(Message::node node, Report::IoT iot) {
    char *zErrMsg = 0;
    char buf[1024];
    if(node.id == "") {
        return;
    }
    std::sprintf(buf,"INSERT OR REPLACE INTO MIots (id, desc, ms, idNode) VALUES (\"%s\", \"%s\", %d, \"%s\")",iot.id.c_str(), iot.desc.c_str(), iot.latency, node.id.c_str());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

vector<Message::node> MasterStorage::getNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT id,ip,port FROM MNodes WHERE monitoredBy = \"%s\"",this->nodeM.id.c_str());

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (SELECT nodes): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

void MasterStorage::addTest(Message::node nodeA, Message::node nodeB, Report::test_result test, string type) {
    char *zErrMsg = 0;
    char buf[2048];
    if(type == string("Latency")) {
        std::sprintf(buf,
                        "INSERT OR IGNORE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                        "   VALUES (\"%s\", \"%s\", %f, %f, DATETIME(%lld,\"unixepoch\"), 0, 0, DATETIME('now', '-1 month'))"
                        "; "
                        "UPDATE MLinks SET meanL = %f, varianceL = %f, lasttimeL = DATETIME(%lld,\"unixepoch\") "
                        "   WHERE idA = \"%s\" AND idB = \"%s\"",
                        nodeA.id.c_str(), nodeB.id.c_str(), test.mean, test.variance, test.lasttime, test.mean, test.variance, test.lasttime, nodeA.id.c_str(), nodeB.id.c_str());
    }else if(type == string("Bandwidth")) {
        std::sprintf(buf,
                        "INSERT OR IGNORE INTO MLinks (idA, idB, meanB, varianceB, lasttimeB, meanL, varianceL, lasttimeL) "
                        "   VALUES (\"%s\", \"%s\", %f, %f, DATETIME(%lld,\"unixepoch\"), 0, 0, DATETIME('now', '-1 month'))"
                        "; "
                        "UPDATE MLinks SET meanB = %f, varianceB = %f, lasttimeB = DATETIME(%lld,\"unixepoch\") "
                        "   WHERE idA = \"%s\" AND idB = \"%s\"",
                        nodeA.id.c_str(), nodeB.id.c_str(), test.mean, test.variance, test.lasttime, test.mean, test.variance, test.lasttime, nodeA.id.c_str(), nodeB.id.c_str());
    }


    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert test): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

void MasterStorage::addReportLatency(Message::node node, vector<Report::test_result> latency) {
    for(auto test : latency) {
        this->addTest(node, test.target, test, "Latency");
        this->addTest(test.target, node, test, "Latency"); //because is symmetric
    }
}

void MasterStorage::addReportBandwidth(Message::node node, vector<Report::test_result> bandwidth) {
    for(auto test : bandwidth) {
        this->addTest(node, test.target, test, "Bandwidth"); //asymmetric
    }
}

 void MasterStorage::addReportIot(Message::node node, std::vector<Report::IoT> iots) {
    for(auto iot : iots) {
        this->addIot(node, iot);
    }
 }

void MasterStorage::addReport(Report::report_result result, Message::node *monitored) {
    this->addNode(result.source, result.hardware, monitored);
    this->addReportLatency(result.source, result.latency);
    this->addReportBandwidth(result.source, result.bandwidth);
    this->addReportIot(result.source, result.iot);
}

void MasterStorage::addReport(std::vector<Report::report_result> results, Message::node node) {
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
        this->addNode(result.source, result.hardware, &node);
    }
    for(auto result : results) {
        this->addReport(result, &node);
    }
}

std::vector<Message::node> MasterStorage::getMLRLatency(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "SELECT id,ip,port FROM "
                        " (SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, Null as lasttimeL FROM MNodes as A join MNodes as B"
                        "  WHERE A.id != B.id AND A.monitoredBy = \"%s\" AND B.monitoredBy = \"%s\" AND NOT EXISTS (SELECT * FROM MLinks WHERE A.id = idA and B.id = idB) "
                        " UNION SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, C.lasttimeL FROM MNodes as A JOIN MNodes as B JOIN MLinks as C"
                        "  WHERE A.id != B.id and A.id = C.idA and B.id = C.idB AND A.monitoredBy = \"%s\" AND B.monitoredBy = \"%s\" AND strftime('%%s',C.lasttimeL)+%ld-strftime('%%s','now') <= 0 "
                        " order by lasttimeL limit %d) "
                        "group by id;",
                this->nodeM.id.c_str(),this->nodeM.id.c_str(),this->nodeM.id.c_str(),this->nodeM.id.c_str(),seconds, num);
    
    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stdout, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<Message::node> MasterStorage::getMLRBandwidth(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,   "SELECT id,ip,port FROM "
                        " (SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, Null as lasttimeB FROM MNodes as A join MNodes as B"
                        "  WHERE A.id != B.id AND A.monitoredBy = \"%s\" AND B.monitoredBy = \"%s\" AND not exists (SELECT * FROM MLinks WHERE A.id = idA and B.id = idB) "
                        " UNION SELECT A.ip AS ip, A.port AS port, A.id AS id, B.id, C.lasttimeB FROM MNodes as A join MNodes as B left join MLinks as C"
                        "  WHERE A.id != B.id and A.id = C.idA and B.id = C.idB AND A.monitoredBy = \"%s\" AND B.monitoredBy = \"%s\" AND strftime('%%s',C.lasttimeB)+%ld-strftime('%%s','now') <= 0 "
                        " order by lasttimeB limit %d) "
                        "group by id;",
                this->nodeM.id.c_str(),this->nodeM.id.c_str(),this->nodeM.id.c_str(),this->nodeM.id.c_str(),seconds, num);
    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

std::vector<Message::node> MasterStorage::getMLRHardware(int num, int seconds) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT id,ip,port FROM MNodes WHERE monitoredBy = \"%s\" AND strftime('%%s',lasttime)+%ld-strftime('%%s','now') <= 0 ORDER BY lasttime LIMIT %d", this->nodeM.id.c_str(), seconds, num);

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

vector<Message::node> MasterStorage::getMNodes() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT id,ip,port FROM MMNodes ORDER BY RANDOM()");

    vector<Message::node> nodes;

    int err = sqlite3_exec(this->db, buf, VectorNodeCallback, &nodes, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (SELECT nodes): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }

    return nodes;
}

void MasterStorage::addMNode(Message::node node) {
    char *zErrMsg = 0;
    char buf[1024];
    if(ip == "") {
        return;
    }
    std::sprintf(buf,"INSERT OR REPLACE INTO MMNodes (id, ip, port) VALUES (\"%s\",\"%s\",\"%s\")", node.id.c_str(),node.ip.c_str(),node.port.c_str());

    int err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (insert node): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

vector<Report::report_result> MasterStorage::getReport() {
    vector<Message::node> ips = this->getNodes();
    vector<Report::report_result> res;

    for(auto ip : ips) {
        res.push_back(this->getReport(ip));
    }

    return res;
}

Report::hardware_result MasterStorage::getHardware(Message::node node) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT * FROM MNodes WHERE id = \"%s\" GROUP BY ip", node.id.c_str());

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

std::vector<Report::test_result> MasterStorage::getLatency(Message::node node) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT N2.id, N2.ip, N2.port, M.meanL as mean, M.varianceL as variance, strftime('%%s',M.lasttimeL) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.id = \"%s\" group by N2.id", node.id.c_str());

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

std::vector<Report::test_result> MasterStorage::getBandwidth(Message::node node) {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"SELECT N2.id, N2.ip, N2.port, M.meanB as mean, M.varianceB as variance, strftime('%%s',M.lasttimeB) as time FROM MLinks AS M JOIN MNodes AS N1 JOIN MNodes AS N2 WHERE N1.id=M.idA AND N2.id=M.idB AND N1.id = \"%s\" group by N2.id", node.id.c_str());

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

Report::report_result MasterStorage::getReport(Message::node node) {
    Report::report_result r;
    r.source = node;

    r.hardware = this->getHardware(r.source);

    r.latency = this->getLatency(r.source);

    r.bandwidth = this->getBandwidth(r.source);

    return r;
}

void MasterStorage::complete() {
    char *zErrMsg = 0;
    char buf[2048];
    std::sprintf(buf,
                "INSERT OR REPLACE INTO MLinks (idA, idB, meanL, varianceL, lasttimeL, meanB, varianceB, lasttimeB) "
                " SELECT A.id AS idA, B.id AS idB, "
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(meanB) from "
                    "   (SELECT max(M.meanB) as meanB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT max(M.meanB) as meanB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS meanB, "
                    "(SELECT max(varianceB) from "
                    "   (SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.id IN (SELECT id FROM MMNodes) AND B.id NOT IN (SELECT id FROM MMNodes)");

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
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(meanB) from "
                    "   (SELECT max(M.meanB) as meanB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT max(M.meanB) as meanB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS meanB, "
                    "(SELECT max(varianceB) from "
                    "   (SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.id NOT IN (SELECT id FROM MMNodes) AND B.id IN (SELECT id FROM MMNodes)");


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
                    "((SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.meanL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS meanL, "
                    "((SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy) AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = A.id AND M.idB = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = A.monitoredBy)) + "
                    "(SELECT M.varianceL from MLinks AS M WHERE M.idA = (SELECT MMN.id FROM MMNodes AS MMN WHERE MMN.id = B.monitoredBy) AND M.idB = B.id)) "
                    "AS varianceL, NULL AS lasttimeL, "
                    "(SELECT min(meanB) from "
                    "   (SELECT max(M.meanB) as meanB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT max(M.meanB) as meanB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS meanB, "
                    "(SELECT max(varianceB) from "
                    "   (SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.idA = A.id AND M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00')) UNION "
                    "   SELECT avg(M.varianceB) as varianceB from MLinks AS M WHERE (M.lasttimeB <> 0 AND M.lasttimeB <> datetime('1970-01-01 00:00:00') AND M.idB = B.id)))"
                    "AS varianceB, NULL AS lasttimeB "
                "  FROM MNodes AS A JOIN MNodes AS B WHERE A.id <> B.id AND A.monitoredBy <> B.monitoredBy AND A.id NOT IN (SELECT id FROM MMNodes) AND B.id NOT IN (SELECT id FROM MMNodes)");

    err = sqlite3_exec(this->db, buf, 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error3: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}