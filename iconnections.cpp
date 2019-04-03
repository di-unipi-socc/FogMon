
#include "iconnections.hpp"
#include "inode.hpp"

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

#include <sys/types.h>
#include <sys/un.h>
#include <string>
#include <netdb.h>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;
using namespace std;

IConnections::IConnections(int nThread) {
    num = nThread;
    this->running = false;
    
    this->workers = new thread[num];
}

IConnections::~IConnections() {
    this->stop();
    delete [] this->workers;
}

void IConnections::initialize(INode *parent) {
    this->parent = parent;
}

//start the workers and the queue
void IConnections::start() {
    
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
        this->workers[i] = thread(&IConnections::worker, this);
    }
}

//stop the workers and the queue
void IConnections::stop() {
    this->running = false;
    this->queue.stopqueue();

    for(int i=0; i<num; i++) {
        if(this->workers[i].joinable())
        {
            this->workers[i].join();
        }
    }

}

void IConnections::request(int fd) {
    this->queue.push(fd);
}

void IConnections::worker() {
    while(this->running.load()) {
        int fd;
        if(this->queue.pop(&fd)!=0)
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
int IConnections::readS(long fd, void *data, int len) {
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
int IConnections::writeS(long fd, const char *data, int len) {
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

bool IConnections::getMessage(int fd, Message &m) {
    if(fd < 0)
        return false;

    int error;
    int32_t len;

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
            delete data;
            perror("   recv() failed at data");
        }else {
            m.Clear();
            //printf("ricevo: %s\n",data);
            if(m.parseJson(data))
                return true;
        }
    }
    return false;
}

bool IConnections::sendMessage(int fd, Message &m) {
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
        perror("   recv() failed at len");
    }else if(len > 0) {

        error = writeS(fd, json.c_str(), len);
        if(error < 0) {
            perror("   recv() failed at data");
        }else {
            return true;
        }
    }
    return false;
}

bool IConnections::notifyAll(Message &m) {
    vector<string> nodes = this->parent->getStorage()->getNodes();
    for(auto ip : nodes) {
        int fd = this->openConnection(ip);
        if(fd >= 0 ) {
            this->sendMessage(fd,m);
            close(fd);
        }
    }
    return true;
}

int IConnections::openConnection(string ip, string port) {

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
                    tv.tv_sec = 2; 
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
                            fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt)); 
                            break; 
                        } 
                        ris = true;
                        break;
                    } 
                    else { 
                        fprintf(stderr, "Timeout in select() - Cancelling!\n");
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
                fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
            } 
        }
        if(ris == false) {
            close(Socket);
            Socket = -1;
            fprintf(stdout, "retry connection %s", ip.c_str());
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