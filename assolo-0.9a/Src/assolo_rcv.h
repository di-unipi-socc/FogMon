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
#include <netinet/tcp.h>
#include <sched.h>

#include "realtime.h"
#include "assolo.h"

#define RTTUSEC 1000000 /*assuming RTT is 1000 millisec*/

#define PARAMARRAY_SIZE 300

/*extern FILE *fd_instbw;*/ /* file pointers for output files*/
extern FILE *fd_debug;/* file pointers for debug files*/

/*### The following definition was not made on BSDish systems */
#ifndef HAVE_IN_ADDR_T
  typedef unsigned int    in_addr_t;
#endif

#ifdef HAVE_SO_TIMESTAMP
extern struct  msghdr msg;
extern   struct  iovec iov[1];
   union{
     struct cmsghdr cm;
     char control[CMSG_SPACE(sizeof(struct timeval))];
   } control_un;

extern   struct cmsghdr *cmptr;
#endif


extern struct in_addr src_addr;
extern int fromlen;
extern int debug;


extern u_int32_t sender_request_num;
extern u_int32_t request_num;
extern u_int32_t chal_no;
extern int cc;

extern struct control_rcv2snd *pkt;/*control packet pointer*/
extern  char	data_snd[MAXMESG];		/* Maxm size packet */
extern struct itimerval cancel_time,timeout;/* used in setitimer*/
extern   struct	sockaddr_in src;	/* socket addr for incoming packets */
extern   struct	sockaddr_in dst;	/* socket addr for incoming packets */

extern int lowcount,highcount;/*used in sending range updates */

extern int next_ok_due;/* used to send OK packets to sender */

extern u_int32_t cur_num; /* current control packet number */

extern char hostname[MAXHOSTNAMELEN];

extern char	data[MAXMESG];/* size packet */
extern struct	udprecord *udprecord;	/* log record for a packet */

extern double total_inst_bw_excursion;/* sum of chirp estimates over the number of
                            estimates specified */
extern double mx_inst;
extern double ls_inst;
extern double den_vhf;
extern double old_inst_mean;
extern int filter;

extern double perc_bad_chirps;/*in the last write interval how many
                             chirps were affected by context
                             switching*/
extern double stop_time;/*time to stop experiment */

/* parameters for excursion detection algorithm */
extern double decrease_factor;
extern int busy_period_thresh;

/* network speed information */
extern int net_option;

/*context switching threshold */
extern double context_receive_thresh;/*50us*/

extern int soudp;/*socket for udp connection */

extern int jumbo;/*number of packets per jumbo packet*/

extern int pkts_per_write;/* how many packet are expected to arrive at each
                      write interval */

extern int num_inst_bw;/* number of estimates to smooth over for mean */

extern int inst_head;/* pointer to current location in circular buffer */

extern int inst_bw_count;/* total number of chirps used in estimation till
                       now*/

extern double inter_chirp_time;/*time between chirps as stated by the sender */


extern double chirp_duration;/*transmission time of one chirp */

extern double write_interval; /* how often to write to file */
extern int pktsize;
extern double low_rate,high_rate,avg_rate;/*rate range in chirp (Mbps)*/

extern double soglia; /* treshold*/

extern double spread_factor; /* decrease in spread of packets within the
                             chirp*/
extern int created_arrays;


extern int max_good_pkt_this_chirp;/* used in chirps affected by coalescence */

extern double *qing_delay,*qing_delay_cumsum;
extern double *rates,*av_bw_per_pkt,*iat;

extern double *inst_bw_estimates_excursion;/* pointer to interarrivals to look for */

extern int num_interarrival;/* number of different interarrivals to keep track of */

extern int first_chirp,last_chirp;/*keeps track of first and last chirp numbers currently in the records*/

extern int num_pkts_in_info;/* how big packet_info currently is*/

extern int sndPort;	/* destination UDP port */

extern int net_option; /* network option, Gbps or not */
extern int chirps_per_write;/*how many chirps will there be per write timer interrupt*/

extern   char localhost[MAXHOSTNAMELEN];/*string with local host name*/

extern double min_timer;/*minimum timer granularity*/

extern struct itimerval wait_time;/* sigalrm time */

extern struct pkt_info *packet_info;/* keeps track of packet numbers, receive
                                time, send time*/

extern struct chirprecord *chirp_info;

extern int no_chirps_recd;
extern int state,ack_not_rec_count;

extern  int 	new_so1;		/* Actual socket id for incoming msg */

extern  char instbw_remote[PARAMARRAY_SIZE];

extern int remote_host_broken;


extern u_int32_t hash(u_int32_t, u_int32_t);
extern u_int32_t gen_crc_rcv2snd(struct control_rcv2snd *);
extern int check_crc_rcv2snd(struct control_rcv2snd *);
extern u_int32_t gen_crc_snd2rcv(struct udprecord *);
extern int check_crc_snd2rcv(struct udprecord *);

extern void sig_alrm(int);/*defined in signal_alrm_rcv.c */
extern void sig_pipe(int);/*defined in signal_alrm_rcv.c */

extern void  parse_cmd_line(int,char **);
extern void  parse_cmd_line_rcv(int,char **);

extern void send_pkt();
extern Sigfunc *Signal(int,Sigfunc *);/*in signal_alrm_rcv.c*/

extern void  lockMe();/* make sure memory not overwritten, in realtime.c*/

extern void  create_arrays(); /* allocate memory for different arrays, in alloc_rcv.c */
extern void  free_arrays(); /* allocate memory for different arrays, in alloc_rcv.c */

extern void  receive_chirp_pkts();/* receive chirp packets, in chirps_rcv.c */
extern int check_for_new_pars();
extern void  close_all();

extern double timer_gran();
extern void update_rates_iat();

extern void initiate_connection();
extern void compute_parameters();
extern void open_dump_files();
extern void usage();
extern in_addr_t gethostaddr(char *);

