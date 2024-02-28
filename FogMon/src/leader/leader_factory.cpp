#include "leader_factory.hpp"

ILeaderStorage* LeaderFactory::newStorage(std::string path, Message::node node) {
    ILeaderStorage* ret = new LeaderStorage(node);
    ret->open(path);
    return ret;
}

LeaderConnections* LeaderFactory::newConnections(int nThread) {
    return new LeaderConnections(nThread);
}

Server* LeaderFactory::newServer(IConnections* conn, int port) {
    return new Server(conn, port);
}