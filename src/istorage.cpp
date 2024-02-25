#include "istorage.hpp"
#include <string.h>
#include <iostream>

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

void IStorage::flush() {
    // flush database to file
    sqlite3_exec(this->db, "PRAGMA wal_checkpoint;", 0, 0, 0);
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
    printf("lasttime: %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
    if(argv[3] == NULL) {
        test.mean = 0;
    }else {
        test.mean = stof(argv[3]);
    }
    if(argv[4] == NULL) {
        test.variance = 0;
    }else {
        test.variance = stof(argv[4]);
    }
    if(argv[5] == NULL) {
        test.lasttime = 0;
    }else {
        // printf("lasttime: %s\n", argv[5]);
        test.lasttime = stoll(argv[5]);
    }
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
    string query = "select strftime('%s',DATETIME('now'))";

    int64_t ret;

    int err = this->executeQuery(query, {}, getInt64Callback, &ret, &zErrMsg);
    if( err!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg); fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
    return ret;
}

int bindParameters(sqlite3_stmt* stmt, const std::vector<std::variant<int, int64_t, float, std::string>>& params) {
    int rc = SQLITE_OK;
    for (size_t i = 0; i < params.size(); ++i) {
        if (std::holds_alternative<int>(params[i])) {
            rc = sqlite3_bind_int(stmt, i + 1, std::get<int>(params[i]));
        } else if (std::holds_alternative<int64_t>(params[i])) {
            rc = sqlite3_bind_int64(stmt, i + 1, std::get<int64_t>(params[i]));
        } else if (std::holds_alternative<float>(params[i])) {
            rc = sqlite3_bind_double(stmt, i + 1, std::get<float>(params[i]));
        } else if (std::holds_alternative<std::string>(params[i])) {
            rc = sqlite3_bind_text(stmt, i + 1, std::get<std::string>(params[i]).c_str(), -1, SQLITE_TRANSIENT);
        }
        if (rc != SQLITE_OK) {
            return rc;
        }
    }
    return rc;
}

void IStorage::isError(int err, char *zErrMsg, std::string mess) {
    if( err!=SQLITE_OK )
    {
        cerr << "[" << mess << "] SQL error: " << zErrMsg << endl; 
        fflush(stderr);
        sqlite3_free(zErrMsg);
        exit(1);
    }
}

char *IStorage::getErrorMessage() {
    // create the error message and clone
    char* tmp =  (char*)sqlite3_errmsg(this->db);
    char* errMsg = (char*)sqlite3_malloc(strlen(tmp) + 1);
    strcpy(errMsg, tmp);
    return errMsg;
}

// define varint vector
typedef std::vector<std::variant<int, int64_t, float, std::string>> param_type;

int IStorage::executeQuery(const std::string& sql, const param_type &params, Callback callback, void* callbackParam, char** errMsg) {
    sqlite3_stmt* stmt;
    int rc;
    const char * query = sql.c_str();
    const char * tail = nullptr;
    param_type params_query = params;
    // while query is not empty
    while (*query != '\0') {

        rc = sqlite3_prepare_v2(this->db, query, -1, &stmt, &tail);
        if (rc != SQLITE_OK) {
            *errMsg = this->getErrorMessage();
            return SQLITE_ABORT;
        }
        // cut parameters in 2, one long the ? present in the query and the rest
        int numParams = 0;
        for (const char* p = query; *p; ++p) {
            if (p == tail) {
                break;
            }
            if (*p == '?') {
                ++numParams;
            }
        }

        param_type tailParams = param_type(params_query.begin() + numParams, params_query.end());
        param_type headParams = param_type(params_query.begin(), params_query.begin() + numParams);
        rc = bindParameters(stmt, headParams);
        if (rc != SQLITE_OK) {
            *errMsg = this->getErrorMessage();
            return SQLITE_ABORT;
        }

        do  {
            rc = sqlite3_step(stmt);
            if(rc == SQLITE_ROW)
            {
                if (callback == nullptr) {
                    continue;
                }

                int argc = sqlite3_column_count(stmt);
                char** argv = new char*[argc];
                char** azColName = new char*[argc];
                for(int i=0; i<argc; i++) {
                    argv[i] = (char*)sqlite3_column_text(stmt, i);
                    azColName[i] = (char*)sqlite3_column_name(stmt, i);
                }
                int r = callback(callbackParam, argc, argv, azColName);
                delete[] argv;
                delete[] azColName;
                if (r != 0) {
                    *errMsg = this->getErrorMessage();
                    return SQLITE_ABORT;
                }
            } 
        } while(rc != SQLITE_ERROR && rc != SQLITE_DONE && rc != SQLITE_MISUSE);


        if (rc != SQLITE_DONE) {
            *errMsg = this->getErrorMessage();
            return SQLITE_ABORT;
        }
        
        rc = sqlite3_finalize(stmt);
        if (rc != SQLITE_OK) {
            *errMsg = this->getErrorMessage();
            return SQLITE_ABORT;
        }
        query = tail;
        params_query = tailParams;
    }
    errMsg = nullptr;
    return 0;
}