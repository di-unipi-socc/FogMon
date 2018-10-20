#include "master_storage.hpp"
#include <string.h>
#include <vector>

using namespace std;

MasterStorage::MasterStorage(string path) {
    open(path);
}

MasterStorage::~MasterStorage() {
}

void MasterStorage::createTables() {

}

vector<string> MasterStorage::getNodes() {
    return vector<string>();
}