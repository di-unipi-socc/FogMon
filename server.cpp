#include "server.hpp"
#include "iconnections.hpp"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#include <sys/statvfs.h>
#include <sys/sysinfo.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <sys/eventfd.h>
#include <signal.h>

using namespace std;

Server::Server(IConnections *conn, int port) {
    running = false;
    portC = port;
    efd = eventfd(0,0);
    this->connection = conn;
}

Server::~Server() {

}

void Server::start() {
    this->listenerThread = thread(&Server::listener, this);
    this->connection->start();
}

void Server::stop() {
    this->running = false;
    eventfd_write(efd,1);
    this->connection->stop();

    if(this->listenerThread.joinable())
    {
        this->listenerThread.join();
    }
}

void Server::listener() {

    signal(SIGPIPE, SIG_IGN);

    int error, on = 1;
    int listen_sd = -1, new_sd = -1;
    int compress_array = false;
    struct sockaddr_in6   addr;
    int timeout;
    struct pollfd fds[200]; //TODO: max connection dynamic
    int nfds = 0, current_size = 0, i, j;
    this->running = true;
    //create an AF_INET6 stream socket for listening
    listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket() failed");
        exit(-1);
    }

    //set reusable
    error = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (error < 0)
    {
        perror("setsockopt() failed");
        close(listen_sd);
        exit(-1);
    }

    //set nonblocking
    error = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (error < 0)
    {
        perror("ioctl() failed");
        close(listen_sd);
        exit(-1);
    }

    //bind
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(this->portC);

    error = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (error < 0)
    {
        perror("bind() failed");
        close(listen_sd);
        exit(-1);
    }

    //set queue size for listening
    error = listen(listen_sd, 32);
    if (error < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        exit(-1);
    }

    //timeout of poll
    timeout = (3 * 60 * 1000);

    //initializate poll structure
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = this->efd;
    fds[0].events = POLLIN;
    nfds++;

    fds[1].fd = listen_sd;
    fds[1].events = POLLIN;
    nfds++;

    do
    {
        //printf("Waiting on poll()...\n");
        error = poll(fds, nfds, timeout);

        if (error < 0)
        {
            perror("  poll() failed");
            break;
        }
        if (error == 0)
        {
            //timeout continue
            continue;
        }
        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {
            if(fds[i].revents == 0)
                continue;
            
            if(fds[i].revents&POLLIN != POLLIN)
            {
                running = false;
                break;
            }

            if (fds[i].fd == efd)
            {
                continue;
            }
            else if (fds[i].fd == listen_sd)
            {
                //accept new connections
                //printf("  Listening socket is readable\n");
                do
                {
                    new_sd = accept(listen_sd, NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            running = false;
                        }
                        break;
                    }

                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                } while (new_sd != -1);
            }
            else
            {
                //read incoming packets
                //printf("  Descriptor %d is readable\n", fds[i].fd);

                this->connection->request(fds[i].fd);

                fds[i].fd = -1;
                fds[i].events = 0;
                compress_array = true;
            }
        }
        if (compress_array)
        {
            compress_array = false;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                        fds[j].events = fds[j+1].events;
                    }
                    i--;
                    nfds--;
                }
            }
        }
    }while(this->running);

    //cleanup sockets
    for (i = 0; i < nfds; i++)
    {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
    this->running = false;
}


int Server::getPort() {
    return this->portC;
}