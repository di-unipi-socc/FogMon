#include "factory.hpp"

IStorage* Factory::newStorage(std::string path) {
    IStorage* ret = new Storage();
    ret->open(path);
    return ret;
}

FollowerConnections* Factory::newConnections(int nThread) {
    return new FollowerConnections(nThread);
}

Server* Factory::newServer(IConnections* conn, int port) {
    return new Server(conn, port);
}