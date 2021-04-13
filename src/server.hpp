#ifndef SERVER_HPP_
#define SERVER_HPP_
#include <string>
#include <thread>

class IAgent;
class IConnections;

#include <thread>

/** 
 * Define a server that accept connections an IConnections handle it 
*/
class Server {
private:
    /**
     * the state of the server true=running else is closing
    */
    bool running;

    /**
     * Holds the port to listen to
    */
    uint16_t portC;

    /**
     * thread of the listener to terminate it
    */
    std::thread listenerThread;

    /**
     * Holds the handler of the new connections
    */
    IConnections* connection;

    /** 
     * fd to wake up the listener thread during poll
    */
    int efd;

    /** 
     * The listener function used to create the socket and wait for new connections
    */
    void listener();
public:

    /**
     * @param conn the handler of the connections
     * @param port the port to listen to
    */
    Server(IConnections *conn, int port);
    ~Server();

    /**
     * Start the listener thread
    */
    void start();

    /**
     * Stop the thread
    */
    void stop();

    /**
     * @return the port of the listener
    */
    int getPort();
};

#endif