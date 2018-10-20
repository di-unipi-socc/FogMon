#include "istorage.hpp"
#include <string.h>

using namespace std;

void IStorage::open(string path) {
    int err = sqlite3_open(path.c_str(), &(this->db));
    if( err ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    this->createTables();
}

void IStorage::close() {
    sqlite3_close(this->db);
}