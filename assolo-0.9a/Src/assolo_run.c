/*
 * assolo_run.c
 * Copyright (c) 2005 Rice University
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
 * Author: Vinay Ribeiro, vinay@rice.edu.  */


/*
This code can be run on a machine separate from the machines running
the sender and receiver modules of assolo. It reads in the
parameters to be given to the receiver module, sends these to the
receiver module running as a daemon remotely, obtaines the results of
the experiment fromt the receiver module and writes these to a file.
*/

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
#include "assolo_run.h"

FILE *fd_instbw;/* file pointers for output files*/
FILE *fd_debug;/* file pointers for debug files*/

struct in_addr src_addr;
struct in_addr dest_addr;
int fromlen=1;/*must be non-zero, used in recvfrom*/
int debug=0;
int jumbo=1;/*number of pkts per jumbo packet */

int num_inst_bw=11;/* number of estimates to smooth over for mean */

int cc=0;
int state=0,ack_not_rec_count=0;
struct control_rcv2snd *pkt;/*control packet pointer*/
char data_snd[MAXMESG];		/* Maxm size packet */
struct itimerval cancel_time,timeout;/* used in setitimer*/
struct sockaddr_in src;	/* socket addr for outgoing packets */
struct sockaddr_in dest;	/* socket addr for incoming packets */

char hostname[MAXHOSTNAMELEN];

char data[MAXMESG];		/* Maxm size packet */
struct udprecord *udprecord;	/* log record for a packet */

double stop_time;/*time to stop experiment */

/* parameters for excursion detection algorithm */
double decrease_factor=1.5;
int busy_period_thresh=5;

int sotcp;/*socket for tcp connection */
int pktsize=1000;

/*rate range in chirp (Mbps)*/
double low_rate=DEFAULT_MIN_RATE,high_rate=DEFAULT_MAX_RATE,avg_rate=DEFAULT_AVG_RATE;

/*default values*/
double soglia=DEFAULT_SGL;
int filter=0;

double spread_factor=1.2; /* decrease in spread of packets within the
                             chirp*/

int num_interarrival;/* number of different interarrivals to keep track of */

int num_pkts_in_info=0;/* how big packet_info currently is*/

int sndPort = SNDPORT;	/* destination UDP port */

int net_option=1; /* network option, Gbps or not */

char rxhostname[MAXHOSTNAMELEN];/*string with local host name*/

double min_timer;/*minimum timer granularity*/

struct itimerval wait_time;/* sigalrm time */





void open_dump_files(char *src,char *dst)
{

	char instbw_file_name[MAXFNAME];
   	char debug_file_name[MAXFNAME];
  	char time_string[40];
  	struct timeval tv;		/* timestamp */

  	(void) gettimeofday (&tv, (struct timezone *) 0);

  	strtok(src,".");
  	strtok(dst,".");

  	strcpy(instbw_file_name,src);
  	strcat(instbw_file_name,"_");
  	strcat(instbw_file_name,dst);
  	strcat(instbw_file_name,"_");


  	sprintf(time_string,"%ld",tv.tv_sec);

  	strcat(instbw_file_name,time_string);
   	strcpy(debug_file_name,instbw_file_name);


  	strcat(instbw_file_name,".instbw");
  	strcat(debug_file_name,".debug");


  	fd_instbw = fopen(instbw_file_name, "w");
  	if (fd_instbw == NULL)
  	{
    	fprintf(stderr, "Error opening dump file\n");
  	}

  	fprintf(stderr, "Opening file: %s\n",instbw_file_name);

  	if(debug)
    {
    	fd_debug = fopen(debug_file_name, "w");
      	if (fd_debug == NULL)
      	{
			fprintf(stderr, "Error opening dump file\n");
      	}
    }

}

in_addr_t gethostaddr(name) char *name;
{
   	in_addr_t addr;
   	register struct	hostent *hp;

   	if ((addr = (in_addr_t)inet_addr (name)) != -1)
    	return (addr);

   	hp = gethostbyname(name);
   	if (hp != NULL)
      	return (*(in_addr_t *)hp->h_addr);
   	else
      return (0);
}



/* usage information */
void usage()
{
  	(void) fprintf (stderr,"usage: assolo_run -R <receiver> -S <sender> -t <duration(secs)> <more options, -h prints all options>\n \t -n <number of estimates to smooth over, default=11>\n \t -d <decrease factor (>1), default=1.5>\n \t -b <busy period length (integer >2), default=5>\n \t -U <receiver port (chirp UDP), default=%d (1024-65535)>\n \t -h Help: produces this output\n",SNDPORT);
   	(void) fprintf (stderr, "\t -R \t receiver host name or IP address \n");
   	(void) fprintf (stderr, "\t -S \t sender host name or IP address \n");
   	(void) fprintf (stderr, "\t -J \t number of packets per Jumbo packet,default=1. In case of packet coalescence use values larger than 1, e.g. -J 6 \n");
   	(void) fprintf (stderr, "\t -l \t lowest rate (Mbps) to probe at within chirp, default=%fMbps. NOTE: by default assolo will find an appropriate probing range.\n",DEFAULT_MIN_RATE);
   	(void) fprintf (stderr, "\t -u \t highest rate (Mbps) to probe at within chirp, default=%fMbps\n",DEFAULT_MAX_RATE);
   	(void) fprintf (stderr, "\t -e \t treshold , default=%fpercent\n",DEFAULT_SGL);
   	(void) fprintf (stderr, "\t -e \t filter , default=%d type\n",0);
   	(void) fprintf (stderr, "\t -p \t packet size <%d-%d>,default=1000 bytes\n",MINPKTSIZE,MAXMESG);
   	(void) fprintf (stderr, "\t -t \t duration of experiment(sec), default=600 sec \n");
   	(void) fprintf (stderr, "\t -s \t spread factor: ratio of consecutive inter-arrivals within a chirp, default=1.2 \n");
   	(void) fprintf (stderr, "\t -a \t average probing rate (Mbps), default=0.3Mbps \n");
   	(void) fprintf (stderr, "\t -v version\n");

  	exit (1);
}


/* get command line parameters */
void parse_cmd_line_run(argc,argv) int	argc; char	*argv[];
{
  	char *ptr;			/* to traverse the arguments */
  	double duration=600.0;
  	struct timeval tp_start;

    argc--; argv++;

  	/* go through the arguments */
   	while (argc > 0)
   	{
      	ptr = *argv;
      	while (*ptr)
      		switch (*ptr++)
      		{
 				case 'l':/* lowest rate to probe at */
       				if (*ptr == 0)
       				{
	 					argc--; argv++;
	 					if (*argv == 0)
	 					{
	   						(void) fprintf (stderr,"assolo_run: no lowest rate '-l'.\n");
	   						(void) exit (1);
	 					}
	 					low_rate = atof (*argv);
       				}
       				else
       				{
	 					low_rate = atof (*argv);
	 					*ptr = 0;
       				}
       				fprintf(stderr,"low rate is %f \n",low_rate);
       			break;

     			case 'u':/* highest rate to probe at */
       				if (*ptr == 0)
       				{
	 					argc--; argv++;
	 					if (*argv == 0)
	 					{
	   						(void) fprintf (stderr,"assolo_run: no highest rate '-u'.\n");
	   						(void) exit (1);
	 					}
	 					high_rate = atof (*argv);
       				}
       				else
       				{
	 					high_rate = atof (*argv);
	 					*ptr = 0;
       				}
       				fprintf(stderr,"high rate is %f \n",high_rate);
       			break;

     			case 'a':
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no average rate '-a'.\n");
	   					(void) exit (1);
	 				}
	 				avg_rate = atof (*argv);
       			}
       			else
       			{
	 				avg_rate = atof (*argv);
	 				*ptr = 0;
       			}
       			fprintf(stderr,"avg_rate is %f \n",avg_rate);

       		break;

			case 't':
				if (*ptr == 0)
				{
					argc--; argv++;
					if (*argv == 0)
					{
						(void) fprintf (stderr,"assolo_run: no duration '-t'.\n");
						(void) exit (1);
					}
					duration = atof (*argv);
				}
				else
				{
					duration = atof (*argv);
					*ptr = 0;
				}

			break;

			case 's':
				if (*ptr == 0)
				{
					argc--; argv++;
					if (*argv == 0)
					{
						(void) fprintf (stderr,"assolo_run: no spread_factor '-s'.\n");
						(void) exit (1);
					}
					spread_factor = atof (*argv);
				}
				else
				{
					spread_factor = atof (*argv);
					*ptr = 0;
				}
				fprintf(stderr,"spread  is %f \n",spread_factor);

			break;

     		case '-':
       		break;

        	case 'p':		/* packet size */
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no packet size given with '-p'.\n");
	   					(void) exit (1);
	 				}
	 				pktsize = atoi (*argv);
       			}
       			else
       			{
	 				pktsize = atoi (*argv);
	 				*ptr = 0;
       			}
       		break;

        	case 'J':		/* Jumbo size in packets*/
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no Jumbo size given with '-J'.\n");
	   					(void) exit (1);
	 				}
	 				jumbo = atoi (*argv);
       			}
       			else
       			{
	 				jumbo = atoi (*argv);
	 				*ptr = 0;
     	  		}
       		break;

     		case 'S':
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no destination host given with '-S'.\n");
	   					(void) exit (1);
	 				}
	 				(void) strcpy (hostname, *argv);
       			}
       			else
       			{
	 				(void) strcpy (hostname, ptr);
	 				*ptr = 0;
       			}
       			fprintf(stderr,"Sender host is:  %s \n",hostname);
       		break;

     		case 'R':
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no receiver host given with '-R'.\n");
	   					(void) exit (1);
	 				}
	 			(void) strcpy (rxhostname, *argv);
       			}
       			else
       			{
	 				(void) strcpy (rxhostname, ptr);
	 				*ptr = 0;
     	  		}
       			fprintf(stderr,"Receiver host is:  %s \n",rxhostname);
       		break;

      		case 'D':
				debug = 1;
			break;

      		case 'v':
				fprintf(stderr,"assolo version %s\n",VERSION);
				exit(0);
			break;

     		case 'b':/* busy period length */
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
					if (*argv == 0)
					{
						(void) fprintf (stderr,"assolo_run: no busy period length '-b'.\n");
					   	(void) exit (1);
					}
					busy_period_thresh = atoi (*argv);
				}
				else
				{
					busy_period_thresh = atoi (*argv);
					*ptr = 0;
				}
				fprintf(stderr,"busy period length is %d \n",busy_period_thresh);
			break;

     		case 'd':/* decrease factor */
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no decrease factor '-d'.\n");
		   				(void) exit (1);
	 				}
	 				decrease_factor = atof (*argv);
       			}
       			else
       			{
	 				decrease_factor = atof (*argv);
	 				*ptr = 0;
       			}
       			fprintf(stderr,"decrease factor is %f \n",decrease_factor);
       		break;

     		case 'n':/* number of inst. bw. estimates to smooth over*/
       			if (*ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no averaging length '-n'.\n");
	   					(void) exit (1);
	 				}
	 				num_inst_bw = atoi (*argv);
       			}
       			else
       			{
	 				num_inst_bw = atoi (*argv);
	 				*ptr = 0;
       			}
       		break;

 	   		case 'U':		/* UDP port */
       			if( *ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no port number given with '-U'.\n");
	   					(void) exit (1);
	 				}
	 				sndPort = atoi (*argv);
       			}
       			else
       			{
	 				sndPort = atoi (ptr);
	 				*ptr = 0;
       			}
       		break;


 			case 'e':		/* treshold */
       			if( *ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no treshold '-e'\n");
	   					(void) exit (1);
	 				}
	 				soglia = atoi (*argv);
       			}
       			else
       			{
	 				soglia = atoi (ptr);
	 				*ptr = 0;
     	 	 	}
       		break;

 			case 'f':		/* filter type */
       			if( *ptr == 0)
       			{
	 				argc--; argv++;
	 				if (*argv == 0)
	 				{
	   					(void) fprintf (stderr,"assolo_run: no filter '-f'\n");
	   					(void) exit (1);
	 				}
			 		filter = atoi (*argv);
       			}
       			else
       			{
	 				filter = atoi (ptr);
	 				*ptr = 0;
     	  		}
       		break;

      		case 'h':
      		case 'H':
	 			usage();

      		default:
	 			(void) fprintf (stderr,"assolo_run: Unknown option '%c'\n", ptr[-1]);

      	}
      	argc--; argv++;
   }
 /* checking  parameters */


	src_addr.s_addr = gethostaddr(hostname);
   	if (src_addr.s_addr == 0)
   	{
   		(void) fprintf (stderr, "assolo_run: %s: unknown sender host\n",hostname);
     	usage();
      	(void) exit (1);
   	}
   	else
   	{
    	/* initializing */
       	bzero((char *)&src, sizeof (src));

   		src.sin_addr = src_addr;
   	}

   	dest_addr.s_addr = gethostaddr(rxhostname);
   	if (dest_addr.s_addr == 0)
   	{
      	(void) fprintf (stderr, "assolo_run: %s: unknown receiver host\n",rxhostname);
     	usage();
      	(void) exit (1);
   	}
   	else
   	{
     	/* initializing */

    	bzero((char *)&dest, sizeof (dest));

     	dest.sin_addr = dest_addr;
   }

  	if (decrease_factor <=1.0)
    {
		perror ("assolo_run: decrease_factor invalid");
	 	(void) exit (1);
    }

  	if (duration <0)
    {
		perror ("assolo_run: time duration invalid");
	 	(void) exit (1);
    }
  	else
    {
      	(void) gettimeofday (&tp_start, (struct timezone *) 0);
      	stop_time=(double)tp_start.tv_sec+(((double)tp_start.tv_usec)/1000000.0)+duration;
    }

  	if (jumbo<1 || jumbo>20)
    {
		perror ("assolo_run: jumbo must be between 1 and 20");
	 	(void) exit (1);
    }

  	if (busy_period_thresh <=2)
    {
		perror ("assolo_run: busy_period_thresh invalid, must be integer >2");
	 	(void) exit (1);
    }

  	if (sndPort<1024 ||  sndPort>65535)
    {
		perror ("assolo_run: Port number must be in [1024-65535]");
	 	(void) exit (1);
    }

 	/*check if parameters ok*/
 	if (pktsize<MINPKTSIZE)
    {
      	fprintf(stderr,"assolo_run: packet size too small, using minimum sized %d byte packet\n",MINPKTSIZE);
      	pktsize=MINPKTSIZE;
    }
  	else
    {
      	if (pktsize>MAXMESG)
		{
	  		fprintf(stderr,"assolo_run: packet size too big, using maximum sized %d byte packet\n",MAXMESG);
	  		pktsize=MAXMESG;
		}
    }

  	if(spread_factor<MINSPREAD)
    	fprintf(stderr,"assolo_run: packet spread too small, using minimum packet spread %f\n",MINSPREAD);

  	if(soglia < MIN_SGL || soglia > MAX_SGL)
	{
    	fprintf(stderr,"assolo_run: treshold not correct using %f\n",DEFAULT_SGL);
    	soglia=DEFAULT_SGL;
	}

  	if( low_rate<0.0 || high_rate<0.0 || avg_rate<0.0 || low_rate>high_rate)
    {
      	perror("assolo_run: probing rates invalid\n");
      	exit(0);
    }


 	if (avg_rate>MAX_AVG_RATE)
    {
 	    fprintf(stderr,"Average rate too high, reducing to %f Mbps\n",MAX_AVG_RATE);
      	avg_rate=MAX_AVG_RATE;
    }

  	if (avg_rate<MIN_AVG_RATE)
    {
    	fprintf(stderr,"Average rate too low, increasing to %f Mbps\n",MIN_AVG_RATE);
      	avg_rate=MIN_AVG_RATE;
    }
  	if (high_rate>2000.0*low_rate)
    {
    	fprintf(stderr,"Ratio of high/low rate very large, increasing low_rate\n");
      	low_rate=high_rate/1000.0;
    }
  	if (high_rate<5.0*low_rate)
    {
    	fprintf(stderr,"Ratio of high/low rate very low, increasing high_rate\n");
      	high_rate=5*low_rate;
    }

  	/* opening files */
  	open_dump_files(hostname,rxhostname);

}


/* close all open files and sockets */

void close_all()
{
   	(void) close (sotcp);
   	fflush(fd_instbw);
   	fclose(fd_instbw);

   	if(debug)
    {
    	fflush(fd_debug);
       	fclose(fd_debug);
    }
}

/*connect to the receiver and send parameters */
void connect_to_rcv(int argc,char **argv_ptr)
{

  	int num_args=0;
 	struct inst_bw_pkt *remote_pkt;
 	double timestamp;
 	double avail_bw;
 	int nc;

   	remote_pkt=(struct inst_bw_pkt *) data;



   	/* create a socket to send result packets by TCP */
   	sotcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   	if (sotcp < 0)
   	{
    	perror("assolo_run: tcp socket");
      	exit(1);
   	}

   	/* initialize the socket address for outgoing connection */
   	dest.sin_family = AF_INET;
   	dest.sin_port = htons(TCPRCVPORT);

   	/* connect to the receiver process */
   	if (connect(sotcp, (struct sockaddr *)&dest, sizeof(dest)) < 0)
   	{
    	perror("assolo_run: connect");
      	exit (1);
   	}

   	num_args=2;
   	strcpy(data_snd,*argv_ptr);

   	/*combine all input parameters into single string*/
   	while(num_args<=argc)
    {
    	/* point to next arguement */
       	argv_ptr++;
       	strcat(data_snd,":");
       	strcat(data_snd,*argv_ptr);
       	num_args++;
    }


	if (write(sotcp,data_snd,100) < 0)
	{
		perror("assolo_rcv_tcp: write");
		exit(1);
	}

	while(read(sotcp,data,MAXMESG)> 0)
	{
		nc=(int)ntohl(remote_pkt->chirp);
		timestamp=(double)ntohl(remote_pkt->timestamp_sec)+(double)ntohl(remote_pkt->timestamp_usec)/100000.0;

		avail_bw=(double)ntohl(remote_pkt->avail_bw);
		avail_bw=avail_bw/1000.0;

		if(timestamp<0.01)
		{
			/* interrupt coalescence */

		    fprintf(stderr,"\nInterrupt Coalescence detected. Current estimates are likely to be less than the actual available bandwidth.\n");
		    fprintf(stderr,"Use the '-J' option at the receiver for improved performance. Example: '-J 4'. On networks allowing large MTU, increase the packet size with the -p option for better results.\n \nassolo will take longer to generate results however.\n\n");
		}
		else
		{
		    fprintf(fd_instbw,"%f %f\n",timestamp,avail_bw);
		    fflush(fd_instbw);
		    fprintf(stderr,"\rChirp Number=%d",nc);
		}
	}

	fprintf(stderr,"\n\nFinished Experiment\n");

}

/*real-time*/
#ifdef RT_PROCESS
   int set_real_time_priority(void)
   {
   		struct sched_param schp;
    	/*
     	 * set the process to real-time privs
     	 */
    	memset(&schp, 0, sizeof(schp));
    	schp.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1;

    	if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
		{
            perror("sched_setscheduler");
            return -1;
    	}

     	return 0;
}
#endif


int main(argc,argv) int	argc; char	*argv[];
{

  	char **store_argv;
  	int store_argc;

  	store_argv=argv;
  	store_argc=argc;

    /*real-time FIFO scheduler*/
  	#ifdef RT_PROCESS
    	set_real_time_priority();
  	#endif

  	parse_cmd_line_run(argc,argv);/*parse options, in parse_cmd_line_run.c*/
  	connect_to_rcv(store_argc,store_argv);

  	close_all();
  	return(0);
}
