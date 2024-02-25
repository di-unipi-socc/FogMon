
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <sched.h> 
#include "realtime.h"
#include "assolo.h"

#define RTTUSEC 1000000 /*assuming RTT is 1000 millisec*/


