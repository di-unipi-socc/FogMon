
#include "connections.hpp"
#include "iagent.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <iostream>

#include <sys/types.h>
#include <sys/un.h>
#include <string>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;
using namespace std;

Connections::Connections(int nThread) {
    num = nThread;
    this->running = false;
    
    this->workers = new thread[num];
}

Connections::~Connections() {
    this->stop();
    delete [] this->workers;
}

void Connections::initialize(IAgent *parent) {
    this->parent = parent;
}

//start the workers and the queue
void Connections::start() {
    
    //check if workers are already running
    for(int i=0; i<num; i++) {
        if(this->workers[i].joinable())
        {
            return;
        }
    }

    //start
    this->running = true;
    this->queue.startqueue();
    for(int i=0; i<num; i++) {
        this->workers[i] = thread(&Connections::worker, this);
    }
}

//stop the workers and the queue
void Connections::stop() {
    this->running = false;
    this->queue.stopqueue();

    for(int i=0; i<num; i++) {
        if(this->workers[i].joinable())
        {
            this->workers[i].join();
        }
    }

}

void Connections::request(int fd) {
    this->queue.push(fd);
}

void Connections::worker() {
    while(this->running.load()) {
        int fd;
        if(!this->queue.pop(&fd))
            break;

        int error;
        Message m;
        if(this->getMessage(fd, m)) {
            this->handler(fd, m);    
        }
        close(fd);    
    }
    this->running = false;
}

//return 1 if all ok
//return 0 if end of stream
//return -1 if error
int Connections::readS(long fd, void *data, int len) {
	int n;
	int pos =0;
	while(pos<len && (n = read(fd, &(((char*)data)[pos]), len-pos)) < len)
	{
        if(n < 0 && !(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
		{
			perror("send");
			return -1;
		}else if(n == 0)
        {
            //end of stream
            return -1;
        }
		pos+=n;
	}
	return 1;
}

//return 1 if all ok
//return -1 if error
int Connections::writeS(long fd, const char *data, int len) {
    int n;
	int pos =0;
	while(pos < len && (n = write(fd, &(data[pos]), len-pos)) < len)
	{
		if(n < 0 && !(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
		{
			perror("send");
			return -1;
		}else
		{
			pos+=n;
		}
	}
	return 1;
}

bool Connections::getMessage(int fd, Message &m) {
    if(fd < 0)
        return false;

    int error;
    int32_t len;
    bool ret = false;
    error = readS(fd, &len, sizeof(len));
    if (error < 0)
    {
        perror("   recv() failed at len");
    }else if(len > 0) {
        char * data;
        try {
            data = new char[len+1];
        }catch(...) {
            return false;
        }
        data[len] = 0;

        error = readS(fd, data, len);
        if(error < 0) {
            perror("   recv() failed at data");
        }else {
            m.Clear();
            //printf("ricevo: %s\n",data);
            if(m.parseJson(data)) {
                ret = true;
            }
        }
        delete[] data;
    }
    return ret;
}

bool Connections::sendMessage(int fd, Message &m) {
    if(fd < 0)
        return false;

    m.buildString();
    string json = m.getString();
    //printf("invio: %s\n", json.c_str());
    int error;
    int32_t len = json.length();

    error = writeS(fd, (const char*)&len, sizeof(len));
    if (error < 0)
    {
        perror("   write() failed at len");
    }else if(len > 0) {

        error = writeS(fd, json.c_str(), len);
        if(error < 0) {
            perror("   write() failed at data");
        }else {
            return true;
        }
    }
    return false;
}

bool Connections::notifyAll(Message &m) {
    vector<Message::node> nodes = this->parent->getStorage()->getNodes();
    for(auto node : nodes) {
        int fd = this->openConnection(node.ip);
        if(fd >= 0 ) {
            this->sendMessage(fd,m);
            close(fd);
        }
    }
    return true;
}

int Connections::openConnection(string ip, string port) {

    struct addrinfo *result,hints;

    memset( &hints,0, sizeof(hints));
	
	result = NULL;

	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *ptr = NULL;
    int iResult;
    int Socket = -1;
    
    // Resolve the server address and port
	iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
    if ( iResult != 0 )
	{
        fprintf(stderr, "getaddrinfo failed with error (%s:%s): %d\n",ip.c_str(),port.c_str(), iResult);
        return -1;
    }
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
	{
        // Create a SOCKET for connecting to server
        Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (Socket == -1)
		{
            fprintf(stderr, "socket failed");
			freeaddrinfo(result);
            return -1;
        }
        // Connect to server.
        int arg;
        if( (arg = fcntl(Socket, F_GETFL, NULL)) < 0) { 
            fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
            return -1;
        } 
        arg |= O_NONBLOCK; 
        if( fcntl(Socket, F_SETFL, arg) < 0) { 
            fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
            return -1;
        } 
        fd_set myset;
        bool ris = false;
        // Trying to connect with timeout 
        iResult = connect(Socket, ptr->ai_addr, (int)ptr->ai_addrlen); 
        if (iResult < 0) { 
            if (errno == EINPROGRESS) { 
                int num = 3;
                do {
                    struct timeval tv; 
                    tv.tv_sec = 5; 
                    tv.tv_usec = 0; 
                    FD_ZERO(&myset); 
                    FD_SET(Socket, &myset); 
                    iResult = select(Socket+1, NULL, &myset, NULL, &tv); 
                    if (iResult < 0 && errno != EINTR) { 
                        fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
                        break;
                    } 
                    else if (iResult > 0) {
                        int valopt;
                        socklen_t lon;
                        // Socket selected for write 
                        lon = sizeof(int); 
                        if (getsockopt(Socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
                            fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
                            break;
                        } 
                        // Check the value returned... 
                        if (valopt) { 
                            //error with the connection
                            break; 
                        } 
                        ris = true;
                        break;
                    } 
                    else { 
                        //timeout
                        if(num>0) {
                            num--;
                            continue;
                        }
                        else
                            break;
                    } 
                } while (1); 
            } 
            else { 
                //fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
            } 
        }
        if(ris == false) {
            close(Socket);
            Socket = -1;
            //fprintf(stdout, "retry connection %s", ip.c_str());
            continue;
        }

        // Set to blocking mode again... 
        if( (arg = fcntl(Socket, F_GETFL, NULL)) < 0) { 
            fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
            return -1;
        } 
        arg &= (~O_NONBLOCK); 
        if( fcntl(Socket, F_SETFL, arg) < 0) { 
            fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
            return -1;
        } 
        break;
    }

    freeaddrinfo(result);

    if (Socket == -1)
	{
        fprintf(stderr, "Unable to connect to server! (%s : %s)\n",ip.c_str(), port.c_str());
        return -1;
    }
    return Socket;
}

std::string Connections::getSource(int fd, Message &m) {
    socklen_t len;
    struct sockaddr_storage addr;
    char ip[INET6_ADDRSTRLEN];
    memset(ip, 0, INET6_ADDRSTRLEN);
    strcat(ip, "::1");

    len = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &len);
                
    if(addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
    }else if(addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
        if (IN6_IS_ADDR_V4MAPPED(&s->sin6_addr)) {
            inet_ntop(AF_INET, &(((in_addr*)(s->sin6_addr.s6_addr+12))->s_addr), ip, sizeof(ip));
        }
        else
            inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
    }else {
#ifndef ENABLE_TESTS
        cerr << "error socket family" << endl;
        return string("");
#endif
        strcpy(ip, "::1");
    }
    if(strcmp(ip,"127.0.0.1")==0)
        strcpy(ip,"::1");
    string strIp = string(ip);
    
    //change the ip of the sender
    {
        Message::node node = m.getSender();
        node.ip = ip;
        m.setSender(node);
    }
    return strIp;
}