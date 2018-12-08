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