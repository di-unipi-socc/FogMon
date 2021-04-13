#include "istorage.hpp"
#include <string.h>

using namespace std;

IStorage::~IStorage() {

}

void IStorage::start() {
    this->workerThread = thread(&IStorage::worker, this);
    this->running = true;
}

void IStorage::stop() {
    this->running = false;
    if(this->workerThread.joinable())
    {
        this->workerThread.join();
    }
}

void IStorage::worker() {
    while(this->running.load()) {
        function<void()> predicate;
        if(!this->queue.pop(&predicate))
            break;
        predicate();
    }
    this->running = false;
}

void IStorage::open(string path) {
    int err = sqlite3_open(path.c_str(), &(this->db));
    if( err ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char *zErrMsg = 0;

    err = sqlite3_exec(this->db, "PRAGMA journal_mode = WAL", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (pragme journal): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    } 

    err = sqlite3_exec(this->db, "PRAGMA synchronous = NORMAL", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (pragma sync): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    } 

    sqlite3_enable_load_extension(db, 1);
    

    err = sqlite3_exec(this->db, "SELECT load_extension('./libsqlitefunctions')", 0, 0, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error (load extension): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    }   
    

    this->createTables();
}

void IStorage::close() {
    if(this->db != nullptr)
        sqlite3_close(this->db);
    this->db = nullptr;
}

int IStorage::getHardwareCallback(void *R, int argc, char **argv, char **azColName) {
    Report::hardware_result *r = (Report::hardware_result*)R;
    for(int i=0; i<argc; i++) {
        if(strcmp("cores", azColName[i])==0) {
            r->cores = stoi(argv[i]);
        }else if(strcmp("mean_free_cpu", azColName[i])==0) {
            r->mean_free_cpu = stof(argv[i]);
        }else if(strcmp("var_free_cpu", azColName[i])==0) {
            r->var_free_cpu = stof(argv[i]);
        }else if(strcmp("memory", azColName[i])==0) {
            r->memory = stoll(argv[i]);
        }else if(strcmp("mean_free_memory", azColName[i])==0) {
            r->mean_free_memory = stoll(argv[i]);
        }else if(strcmp("var_free_memory", azColName[i])==0) {
            r->var_free_memory = stof(argv[i]);
        }else if(strcmp("disk", azColName[i])==0) {
            r->disk = stoll(argv[i]);
        }else if(strcmp("mean_free_disk", azColName[i])==0) {
            r->mean_free_disk = stoll(argv[i]);
        }else if(strcmp("var_free_disk", azColName[i])==0) {
            r->var_free_disk = stof(argv[i]);
        }else if(strcmp("lasttime", azColName[i])==0) {
            r->lasttime = stoll(argv[i]);
        }
    }
    return 0;
}

int IStorage::getTestCallback(void *R, int argc, char **argv, char **azColName) {
    vector<Report::test_result> *r = (vector<Report::test_result>*)R;
    Report::test_result test;
    test.target.id = string(argv[0]);
    test.target.ip = string(argv[1]);
    test.target.port = string(argv[2]);
    if(argv[3] == NULL)
        test.mean = 0;
    else
        test.mean = stof(argv[3]);
    if(argv[4] == NULL)
        test.variance = 0;
    else
        test.variance = stof(argv[4]);
    if(argv[5] == NULL)
        test.lasttime = 0;
    else
        test.lasttime = stoll(argv[5]);
    r->push_back(test);
    return 0;
}

int IStorage::VectorNodeCallback(void *vec, int argc, char **argv, char **azColName) {
    vector<Message::node> *v = (vector<Message::node>*)vec;
    Message::node test;
    test.id = string(argv[0]);
    test.ip = string(argv[1]);
    test.port = string(argv[2]);
    v->push_back(test);
    return 0;
}

int IStorage::VectorStringCallback(void *vec, int argc, char **argv, char **azColName) {
    vector<string> *v = (vector<string>*)vec;
    v->push_back(string(argv[0]));
    return 0;
}

int IStorage::VectorIntCallback(void *vec, int argc, char **argv, char **azColName) {
    vector<long long> *v = (vector<long long>*)vec;
    v->push_back(stoi(argv[0]));
    return 0;
}

int IStorage::VectorIoTCallback(void *vec, int argc, char **argv, char **azColName) {
    vector<Report::IoT> *v = (vector<Report::IoT>*)vec;
    Report::IoT iot;
    iot.id = string(argv[0]);
    iot.desc = string(argv[1]);
    iot.latency = stoi(argv[2]);
    v->push_back(iot);

    return 0;
}

int getInt64Callback(void *i, int argc, char **argv, char **azColName) {
    int64_t *val = (int64_t*)i;
    *val = stoll(argv[0]);
    return 0;
}

int64_t IStorage::getTime() {
    char *zErrMsg = 0;
    char buf[1024];
    std::sprintf(buf,"select strftime('%%s',DATETIME('now'))");

    int64_t ret;

    int err = sqlite3_exec(this->db, buf, getInt64Callback, &ret, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    return ret;
}