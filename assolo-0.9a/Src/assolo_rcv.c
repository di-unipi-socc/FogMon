/*
 * assolo_rcv.c
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
 * Author: Vinay Ribeiro, vinay@rice.edu.  */

/*
 * "assolo_rcv" receives chirp packets from "assolo_snd", estimates
 *  available bandwidth and writes this to a file.
 *
 *  assolo_rcv.c and related code is based on udpread.c of the NetDyn tool.
 *
 */

/*
 * udpread.c
 * Copyright (c) 1991 University of Maryland
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of U.M. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  U.M. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * U.M. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL U.M.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Dheeraj Sanghi, Department of Computer Science.
 */


#include "assolo_rcv.h"


/*FILE *fd_instbw;*//* file pointers for output files*/
FILE *fd_debug;/* file pointers for debug files*/

FILE *fd_tmp;/*for debugging*/
#ifdef HAVE_SO_TIMESTAMP
struct  msghdr msg;
   struct  iovec iov[1];

   struct cmsghdr *cmptr;
#endif

struct in_addr src_addr;
int fromlen=1;/*must be non-zero, used in recvfrom*/
int debug=0;
int jumbo=1;/*number of pkts per jumbo packet */
u_int32_t request_num=0;/* current request number */
u_int32_t sender_request_num=0;/* current request number */
 u_int32_t chal_no=0;
int cc=0;
int state=0,ack_not_rec_count=0;
struct control_rcv2snd *pkt;/*control packet pointer*/
 char	data_snd[MAXMESG];		/* Maxm size packet */
struct itimerval cancel_time,timeout;/* used in setitimer*/
  struct	sockaddr_in src;	/* socket addr for outgoing packets */
  struct	sockaddr_in dst;	/* socket addr for incoming packets */

int lowcount=0,highcount=0;/*used in sending range updates */

int next_ok_due=0;/* used to send OK packets to sender */

int created_arrays=0;

int max_good_pkt_this_chirp;/* used in chirps affected by coalescence */

u_int32_t cur_num=0; /* current control packet number */

char hostname[MAXHOSTNAMELEN];

char	data[MAXMESG];		/* Maxm size packet */
struct	udprecord *udprecord;	/* log record for a packet */

double total_inst_bw_excursion=0.0;/* sum of chirp estimates over the number of
                            estimates specified */

double mx_inst=0.0;
double ls_inst=0.0;
double den_vhf=0.0;
double old_inst_mean=0.0;
int filter=0;

double perc_bad_chirps=0.0;/*in the last write interval how many
                             chirps were affected by context
                             switching*/
double stop_time;/*time to stop experiment */

/* parameters for excursion detection algorithm */
double decrease_factor=1.5;
int busy_period_thresh=5;


/*context switching threshold */
double context_receive_thresh=0.000010;/*10us*/

int soudp;/*socket for udp connection */


int pkts_per_write;/* how many packet are expected to arrive at each
                      write interval */

int num_inst_bw=11;/* number of estimates to smooth over for mean */

int inst_head=0;/* pointer to current location in circular buffer */

int inst_bw_count=0;/* total number of chirps used in estimation till
                       now*/

double inter_chirp_time;/*time between chirps as stated by the sender */

double chirp_duration;/*transmission time of one chirp */

double write_interval; /* how often to write to file */
int pktsize=1000;

/*rate range in chirp (Mbps)*/
double low_rate=DEFAULT_MIN_RATE,high_rate=DEFAULT_MAX_RATE,avg_rate=DEFAULT_AVG_RATE;


double soglia=DEFAULT_SGL; /*treshold*/

double spread_factor=1.2; /* decrease in spread of packets within the
                             chirp*/

double *qing_delay,*qing_delay_cumsum;
double *rates,*av_bw_per_pkt,*iat;

double *inst_bw_estimates_excursion;/* pointer to interarrivals to look for */

int num_interarrival;/* number of different interarrivals to keep track of */

int first_chirp,last_chirp;/*keeps track of first and last chirp numbers currently in the records*/

int num_pkts_in_info=0;/* how big packet_info currently is*/

int sndPort = SNDPORT;	/* destination UDP port */

int net_option=1; /* network option, Gbps or not */
int chirps_per_write;/*how many chirps will there be per write timer interrupt*/


char localhost[MAXHOSTNAMELEN];/*string with local host name*/

double min_timer;/*minimum timer granularity*/

struct itimerval wait_time;/* sigalrm time */

struct pkt_info *packet_info;/* keeps track of packet numbers, receive
                                time, send time*/

struct chirprecord *chirp_info;

int no_chirps_recd=0;


/* variables form assolo_rcv_tcp.c*/

  struct	sockaddr_in receiver;	/* Own address/port etc. */
  struct	sockaddr_in remoteaddr;	/* remote's address/port */

  int	so1;			/* socket id for incoming pkts */
  int	rcv_size = MAXRCVBUF;	/* socket receive buffer size */

  /* Extra variables. -- Suman */
  int 	new_so1;		/* Actual socket id for incoming msg */

  char paramarray[PARAMARRAY_SIZE];
  char instbw_remote[PARAMARRAY_SIZE];
  char *params;

  char *argv_array[20];/*store an array of pointers to the parameters*/
  int flag_on_recv;

  int argc_val=0;

int remote_host_broken=0;

void reset_pars()
{

 	fromlen=1;/*must be non-zero, used in recvfrom*/
 	debug=0;
 	jumbo=1;/*number of pkts per jumbo packet */
 	request_num=0;/* current request number */
 	sender_request_num=0;/* current request number */
 	chal_no=0;
 	cc=0;
 	state=0;
 	ack_not_rec_count=0;
 	lowcount=0;
 	highcount=0;/*used in sending range updates */

 	next_ok_due=0;/* used to send OK packets to sender */

 	created_arrays=0;

 	cur_num=0; /* current control packet number */

 	total_inst_bw_excursion=0.0;/* sum of chirp estimates over the number of
                            estimates specified */

 	perc_bad_chirps=0.0;/*in the last write interval how many
                             chirps were affected by context
                             switching*/

	/* parameters for excursion detection algorithm */
 	decrease_factor=1.5;
 	busy_period_thresh=5;


	/*context switching threshold */
 	context_receive_thresh=0.000010;/*10us*/

 	num_inst_bw=11;/* number of estimates to smooth over for mean */

 	inst_head=0;/* pointer to current location in circular buffer */

 	inst_bw_count=0;/* total number of chirps used in estimation till
                       now*/

 	pktsize=1000;

	/*rate range in chirp (Mbps)*/
 	low_rate=DEFAULT_MIN_RATE;
 	high_rate=DEFAULT_MAX_RATE;
 	avg_rate=DEFAULT_AVG_RATE;

	soglia=DEFAULT_SGL; /*treshold*/
 	filter=0; /*filter type*/

	spread_factor=1.2; /* decrease in spread of packets within the chirp*/
	num_pkts_in_info=0;/* how big packet_info currently is*/

	sndPort = SNDPORT;	/* destination UDP port */

	net_option=1; /* network option, Gbps or not */
	argc_val=0;
	remote_host_broken=0;
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
   	(void) fprintf (stderr,"usage: assolo_rcv\n");
   	(void) fprintf (stderr, "\t -h Help. Produces this output\n");
   	(void) fprintf (stderr, "\t -v version\n");
   	(void) fprintf (stderr, "\t -D print debug information \n");

  	exit (1);
}


/* close all open files and sockets */

void close_all()
{

  	/* cancelling timer*/
  	setitimer(ITIMER_REAL, &cancel_time, 0);
  	if (debug) fprintf(stderr,"Cancelled timer\n");


   	(void) close (soudp);
   	(void) close (new_so1);

   	/*
	fflush(fd_instbw);
	fclose(fd_instbw);
   */

   	if (debug)
    {
       	fflush(fd_debug);
       	fclose(fd_debug);
    }

}



void create_listen_socket()
{

  	/* create a socket for conenction with sender */
  	so1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  	if (so1 < 0)
  	{
    	perror("assolo_rcv_tcp: socket");
    	exit(1);
  	}

  	/* initialize address/port etc for incoming connection */
  	bzero((char *)&receiver, sizeof (receiver));
  	receiver.sin_family = AF_INET;
  	receiver.sin_port = htons(TCPRCVPORT);

 	/*the following is to use socket even if already bound */
 	flag_on_recv = YES;

  	if (setsockopt(so1, SOL_SOCKET, SO_REUSEADDR,(char *) &flag_on_recv,sizeof(flag_on_recv))<0)
  	{
    	perror ("assolo_rcv_tcp: setsockopt failed");
      	(void) exit (1);
  	}


  	/* bind the address/port number to the socket */
  	if (bind (so1, (struct sockaddr *) &receiver, sizeof(receiver)) < 0)
  	{
    	perror ("assolo_rcv_tcp: bind");
    	exit (1);
  	}

  	/* set receive buffers to maximum so that packets are not lost if
  	 * for some reason this program does not get CPU for some time. */
  	if (setsockopt(so1, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_size,sizeof (rcv_size)) < 0)
  	{
    	perror ("udpecho: receive buffer option");
    	exit (1);
  	}



  	/* First we get a connect from the sender. Then we setup */
  	/* a connection to the receiver. -- Suman */
  	if (listen (so1,1) < 0)
  	{
    	perror("assolo_rcv_tcp: listen");
    	exit(1);
  	}

  	return;
}


/* wait for remote host to connect, get parameters from remote  */

void remote_connection()
{

   	int tcp_no_delay = 1;	/* Variable to use to set the TCP Push */
				/* bit, in the setsockopt system call */

    if ((new_so1 = accept (so1, (struct sockaddr *)&remoteaddr, &fromlen))< 0)
    {
    	perror("assolo_rcv_tcp: accept");
      	remote_host_broken=1;
      	return;
    }

    if (setsockopt (new_so1, IPPROTO_TCP, TCP_NODELAY, (char *)&tcp_no_delay,sizeof(tcp_no_delay)) < 0)
	{
	 	perror ("tcpsend: TCP_NODELAY_options");
    }


    if (read(new_so1,paramarray,PARAMARRAY_SIZE)== 0)
    {
		perror("assolo_rcv_tcp: read error");
      	remote_host_broken=1;
      	return;
    }

    /* split received parameters into array of strings  */

    params=strtok(paramarray,":");

    argc_val=0;
    /* setup argc_val and argv_array  */
    while(params!=NULL)
    {
		argv_array[argc_val]=params;/*store location in array*/
		argc_val++;
		params=strtok(NULL,":");
    }

    argv_array[argc_val]=params;/*store location in array*/

}

/*real time*/

#ifdef RT_PROCESS
  int set_real_time_priority(void)
  {
 		struct sched_param schp;
    	/*
     	 * set the process to real-time privs
     	*/
    	memset(&schp, 0, sizeof(schp));
		schp.sched_priority = sched_get_priority_max(SCHED_FIFO);

    	if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
		{
        	perror("sched_setscheduler");
            return -1;
    	}

     	return 0;
}
#endif


/* main function executing all others */

int main(argc,argv)int	argc; char	*argv[];
{
  	/*create a TCP socket for listening */
  	create_listen_socket();

  	min_timer=timer_gran();/* find minimum timer granularity */

  	/*
    Start the signal handler for SIGALRM.
    The timer is started whenever a packet from a
    new chirp is received.
    Compute available bandwidth after each timer expires.
  	*/

  	Signal(SIGALRM, sig_alrm);/*in signal_alrm_rcv.c*/
  	Signal(SIGPIPE, sig_pipe);/*in signal_alrm_rcv.c*/


  	lockMe(); /* make sure memory not overwritten, in realtime.c*/

    /*real-time FIFO scheduler*/
  	#ifdef RT_PROCESS
    	set_real_time_priority();
  	#endif

  	parse_cmd_line_rcv(argc,argv);/*parse options, in parse_cmd_line_rcv.c*/


  	while(1)
  	{

    	fprintf(stderr,"Waiting for remote host\n");


  		/*wait for remote host to make connection*/
    	remote_connection();

      	parse_cmd_line(argc_val,argv_array);/*parse options from assolo_run, in parse_cmd_line_rcv.c*/

    	if (remote_host_broken!=1)
      		create_arrays(); /* allocate memory for different arrays, in alloc_rcv.c */


    	/* contact sender and reply to challenge packet, in control_rcv.c */
    	if (remote_host_broken!=1)
      		initiate_connection();

    	/*start receiving chirp packets*/
    	if (remote_host_broken!=1)
      		receive_chirp_pkts();

    	/* free all arrays that were allocated memory */
    	free_arrays();
    	close_all();
    	reset_pars();

  	}

  	return(0);
}
