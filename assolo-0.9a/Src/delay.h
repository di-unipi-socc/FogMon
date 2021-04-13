/*
 * delay.h
 * Copyright (c) 2003 Rice University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Rice
 * University not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Rice University makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RICE UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RICE UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Authors: Ryan Christopher King (ryanking@rice.edu),
 *          Niels Kang Hoven (nhoven@rice.edu)
 */



#include<stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define SLEEPTHRESH 20700

//This should be set to the maximum amount of time that the system could
//take to return from a usleep(100) call. It's better to err on the side of
//a higher value here. Basically, smartbusywait(N) will use usleep if N
//is greater than sleepthresh.  For the linux system this was developed on,
//20000 worked very well.  Measured in microseconds

#define SLEEPSTEP (SLEEPTHRESH / 10)
//the amount of time that the system tries to sleep for.
//making this greater than sleepthresh would be a VERY BAD idea
//a setting this low takes advantage of the minimum sleep
//period that linux seems to have



inline void smartwait(unsigned int delg, struct timeval *tpcur)
{
  	unsigned int delsofar;
  	struct timeval tpstart=*tpcur;
  	//  int count=0;
  	gettimeofday(tpcur,(struct timezone *)0);
  	delsofar=(tpcur->tv_sec - tpstart.tv_sec)*1000000 + (tpcur->tv_usec - tpstart.tv_usec);
  	//  fprintf(stderr, "delg= %u, delsofar= %u", delg, delsofar);
  	while ((int)(delg-delsofar) >= SLEEPTHRESH)
  	{
    	//     fprintf(stderr, "BIG SLEEP, %d\n", (int)(delg-delsofar));
    	usleep((unsigned) SLEEPSTEP);
    	gettimeofday(tpcur,(struct timezone *)0);
       	delsofar=(tpcur->tv_sec - tpstart.tv_sec)*1000000 + (tpcur->tv_usec - tpstart.tv_usec);
  	}
  	while (delsofar < delg)
  	{
    	//    count++;
    	gettimeofday(tpcur,(struct timezone *)0);
       	delsofar=(tpcur->tv_sec - tpstart.tv_sec)*1000000 + (tpcur->tv_usec - tpstart.tv_usec);
  	}
};


//a macro that "returns" after delg microseconds, intelligently uses usleep
//if the desired delay is large enough to allow the context switch. This
//greatly lowers the needed processor cycles of the macro.
inline void smartwait2(unsigned int delg)
{
  	struct timeval tpstart, tpcur;
  	//used for keeping track of the start time and current time
  	unsigned int delsofar=0;
  	int count=0;
  	gettimeofday(&tpstart,(struct timezone *)0);
 	while (delsofar < delg)
 	{
    	if ((delg-delsofar) >= SLEEPTHRESH) usleep(SLEEPSTEP);
    	gettimeofday(&tpcur,(struct timezone *)0);
    	delsofar=(tpcur.tv_sec - tpstart.tv_sec)*1000000 + (tpcur.tv_usec - tpstart.tv_usec);
    	count++;
  	}
};


//a macro that "returns" after delg microseconds, never
//sleeps, a true busy wait. Uses processor cycles like crazy
//you probably will never want to use this.

#define busywait(delg) do {\
    gettimeofday(&tpstart,(struct timezone *)0); \
    delsofar = 0;\
    while (delsofar < delg){ \
        gettimeofday(&tpcur,(struct timezone *)0);\
        delsofar=(tpcur.tv_sec - tpstart.tv_sec)*1000000 + (tpcur.tv_usec - tpstart.tv_usec);\
    }\
}while(0)

struct timeval tpstart; //used for keeping track of the start time
struct timeval tpcur;  //used for keeping track of the current time
unsigned int delsofar; //amount of delay we have had so far
int count;
unsigned int msr;

