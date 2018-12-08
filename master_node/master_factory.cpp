#include "master_factory.hpp"

IMasterStorage* MasterFactory::newStorage(std::string path) {
    IMasterStorage* ret = new MasterStorage();
    ret->open(path);
    return ret;
}

MasterConnections* MasterFactory::newConnections(int nThread) {
    return new MasterConnections(nThread);
}

Server* MasterFactory::newServer(IConnections* conn, int port) {
    return new Server(conn, port);
}