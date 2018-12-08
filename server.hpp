#ifndef SERVER_HPP_
#define SERVER_HPP_
#include <string>
#include <thread>

class INode;
class IConnections;

#include <thread>


class Server {
private:
    bool running;

    uint16_t portC;

    std::thread listenerThread;

    IConnections* connection;

    //fd to wake up the listener thread during poll
    int efd;
public:
    Server(IConnections *conn, int port);
    ~Server();

    void start();
    void stop();

    void listener();

    int getPort();
};

#endif