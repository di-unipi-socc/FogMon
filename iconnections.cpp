
#include "iconnections.hpp"
#include "inode.hpp"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/un.h>
#include <string>
#include <iostream>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;
using namespace std;

IConnections::IConnections(INode *parent, int nThread) {
    this->parent = parent;
    num = nThread;
    this->running = false;
    
    this->workers = new thread[num];
}

IConnections::~IConnections() {
    this->stop();
    delete [] this->workers;
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
        if(this->getMessage(fd,&m)) {
            this->handler(fd, m);    
        }
        close(fd);    
    }
    this->running = false;
}

//return 1 if all ok
//return 0 if end of stream
//return -1 if error
int readS(long fd, void *data, int len) {
	int n;
	int pos =0;
	while(pos<len && (n = recv(fd, &(((char*)data)[pos]), len-pos,0)) > 0)
	{
        if(n == 0)
        {
            //end of stream
            return -1;
        }
		pos+=n;
	}
	if(pos<len)
		return -1;
	return 1;
}

//return 1 if all ok
//return -1 if error
int writeS(long fd, const char *data, int len) {
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

bool IConnections::getMessage(int fd, Message * m) {
    if(fd < 0 || m == NULL)
        return false;

    int error;
    int32_t len;

    error = readS(fd, &len, sizeof(len));
    if (error < 0)
    {
        perror("   recv() failed at len");
    }else if(len > 0) {
        char * data = new char[len];
        error = readS(fd, data, len);
        if(error < 0) {
            delete data;
            perror("   recv() failed at data");
        }else {
            m->Clear();
            if(m->ParseJson(data))
                return true;
        }
    }
    return false;
}
