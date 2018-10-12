#include <stdio.h>
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
#include <netdb.h>

using namespace std;


int readS(long fd, char *data, int len) {
	int n;
	int pos =0;	
	while(pos<len &&(n = read(fd, &(data[pos]), len-pos)) > 0 )
	{
		pos+=n;
	}
	if(pos<len)
		return -1;
	return 0;
}

int writeS(long fd, const char *data, int len) {
	int n;
	int pos =0;
	while(pos < len && (n = write(fd, &(data[pos]), len-pos)) < len)
	{
		if(n < 0)
		{
			if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				;
			else
			{
				perror("send");
				return -1;
			}
		}else
		{
			pos+=n;
		}
	}
	return 0;
}

int openConnection(string ip, string port, double &timeMillisec) {
    struct addrinfo *result,hints;

    memset( &hints,0, sizeof(hints));
	
	result = NULL;

	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *ptr = NULL;
    int iResult;
    int Socket;
    int state=1;

    // Resolve the server address and port
	iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
    if ( iResult != 0 )
	{
        printf("getaddrinfo failed with error: %d\n", iResult);
        return false;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
	{

        // Create a SOCKET for connecting to server
        Socket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (Socket == -1)
		{
			state = 0;
            printf("socket failed");
			freeaddrinfo(result);
            return false;
        }
		if (state == 0)
			return false;
        // Connect to server.

        iResult = connect( Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == -1)
		{
			state = 0;
			close(Socket);
            Socket = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (Socket == -1)
	{
		state = 0;
        printf("Unable to connect to server!\n");
        return false;
    }
    printf("ready");
    fflush(stdout);
    char buffer[10];

    if(readS(Socket, buffer ,strlen("ready"))<0)
    {
        printf("Error reading\n");
        return false;
    }

    if(strncmp(buffer,"ready",strlen("ready"))!=0) {
        return false;
    }

    printf("start");

    struct timespec tstart={0,0}, tend={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    

    if(writeS(Socket, "ciao",5)<0)
    {
        printf("Error sending\n");
        return false;
    }

    if(readS(Socket, buffer ,5)<0)
    {
        printf("Error reading\n");
        return false;
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);

    struct timespec diff={tend.tv_sec-tstart.tv_sec,tend.tv_nsec-tstart.tv_nsec};
    timeMillisec = (double)((diff.tv_sec*1000 + diff.tv_nsec*1.0e-6));
    printf("Elapsed time: %lf milliseconds\n",timeMillisec);
    printf("read: %s\n",buffer);


    close(Socket);
    return true;
}

int main() {
    double sum = 0;
    double max = 0;
    double min = 10000000;
    long i = 0;
    while(true) {
        double diff = 0;
        if(openConnection("localhost","12345",diff)==false)
        {
            printf("error\n");
            return 1;
        }
        if(diff>max)
            max = diff;
        if(diff<min)
            min = diff;
        sum += diff;
        i++;
        usleep(1000000);
        printf("mean: %lf num: %d min: %lf max: %lf\n",sum/i,i,min,max);
    }
}