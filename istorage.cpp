#include "istorage.hpp"
#include <string.h>

using namespace std;

IStorage::~IStorage() {

}

void IStorage::open(string path) {
    int err = sqlite3_open(path.c_str(), &(this->db));
    if( err ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    sqlite3_enable_load_extension(db, 1);
    char *zErrMsg = 0;

    err = sqlite3_exec(this->db, "SELECT load_extension('./libsqlitefunctions.so')", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (load extension): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }   
    

    this->createTables();
}

void IStorage::close() {
    sqlite3_close(this->db);
}

int IStorage::getHardwareCallback(void *R, int argc, char **argv, char **azColName) {
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

int IStorage::getTestCallback(void *R, int argc, char **argv, char **azColName) {
    vector<Report::test_result> *r = (vector<Report::test_result>*)R;
    Report::test_result test;
    test.target = string(argv[0]);
    test.mean = stof(argv[1]);
    test.variance = stof(argv[2]);
    test.lasttime = stol(argv[3]);
    r->push_back(test);
    return 0;
}