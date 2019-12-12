#include "factory.hpp"

IStorage* Factory::newStorage(std::string path) {
    IStorage* ret = new Storage();
    ret->open(path);
    return ret;
}

Connections* Factory::newConnections(int nThread) {
    return new Connections(nThread);
}

Server* Factory::newServer(IConnections* conn, int port) {
    return new Server(conn, port);
}