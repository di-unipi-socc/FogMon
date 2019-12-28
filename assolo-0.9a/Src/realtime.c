#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include "realtime.h"

void lockMe()
{
    if(geteuid() == 0)
    {
		#ifdef HAVE_SCHED_H
		struct sched_param schedParam;
		/* fprintf(stderr, "setting SCHED_FIFO\n"); */
		schedParam.sched_priority = 1;
		if(sched_setscheduler(0, SCHED_FIFO, &schedParam) != 0)
		{
	    	perror("setscheduler:");
		}
		#endif
		#ifdef HAVE_SYS_MMAN_H
		/* fprintf(stderr, "setting mlockall\n"); */
		if(mlockall(MCL_CURRENT|MCL_FUTURE) != 0)
		{
	    	perror("mlockall:");
		}
		#endif
    }
}
