/*
 * assolo.h
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


#define SNDPORT	        8365		/* port on which sender waits */
#define TCPRCVPORT	7365		/* port on which receiver waits */
#define MAXRCVBUF	32768		/* Maximum receive buffer */
#define	NO		0
#define	YES		1
#define MAX_HIGH_RATE  1500.0 /* maximum probing rate within chirp in Mbps */
#define MAX_AVG_RATE  3.0 /* maximum avg. probing rate in Mbps */
#define MIN_AVG_RATE  0.005 /* minimum avg. probing rate in Mbps */
#define	MAXFNAME	140		/* Maximum filename */
#define	MAXMESG		8200		/* Maximum packet size */
#define	MAXCHIRPSIZE	200		/* Maximum number of packets in a chirp */
#define MINSPREAD 1.1
#define MINPKTSIZE 40 
#define MAX_RECV_OK_COUNT 30 
#define DEFAULT_MIN_RATE 10.0 
#define DEFAULT_MAX_RATE 200.0 
#define DEFAULT_AVG_RATE 0.3 

//Modifica
#define DEFAULT_SGL 5.0
#define MIN_SGL 2.0
#define MAX_SGL 10.0

#define CONSEC_BAD_PKTS 2
#define NEG_THRESH -1000.0

#define	REQ_CONN 1
#define	STOP 0
#define	CHALL_REPLY 2
#define	UPDATE_RATES 3
#define CHIRPS_STARTED 4
#define RECV_OK 5
#define VERSION "2.4.1"

#ifndef HAVE_U_INT32_T
typedef unsigned int u_int32_t;
#endif

typedef void Sigfunc(int);

struct udprecord {
  u_int32_t num;			/* sender's pkt number */
  u_int32_t chirp_num;		/* chirp's pkt number */
  u_int32_t request_num;            /* which request number does the chirp rates correspond to */
  u_int32_t timesec;         /* seconds time stamp*/
  u_int32_t timeusec;         /* usec */
  u_int32_t chal_no;              /* challenge number */
  u_int32_t checksum;              /* checksum for security */
};



struct control_rcv2snd {
  u_int32_t  request_type;	/* start=1, stop=0, update=2*/
  u_int32_t  request_num;/*all packets for each request have this common*/
  u_int32_t  num;		/* unique packet number*/
  u_int32_t  timesec;
  u_int32_t  timeusec;
  u_int32_t  chal_no;
  /* following used only in update */
  u_int32_t num_interarrival; /*one less than the number of packets per chirp*/
  u_int32_t inter_chirp_time;/* time between chirps */
  u_int32_t low_rate;/* lowest rate (Mbps) to probe at */

  //modifica
  u_int32_t high_rate;/* highest rate (Mbps) to probe at */
  u_int32_t soglia;/* highest rate (Mbps) to probe at */  
  u_int32_t filter;/* highest rate (Mbps) to probe at */  

  u_int32_t spread_factor; /* decrease in spread of packets within the chirp */
  u_int32_t pktsize;/* pkt size in bytes used for probing */
  u_int32_t jumbo;/* number of back to back packets to transmit instead of one chirp packet */
  u_int32_t  checksum;/*check sum for security*/

};


struct pkt_info {
	double snd_time;		/* sender's timestamp */
	double rcv_time;		/* receiver timestamp */
        u_int32_t 	num;			/* sender's pkt number */
        u_int32_t	chirp_num;			/* chirp number */
        u_int32_t	request_num;			/* sender's request num*/
  int good_jumbo_pkt;/*at least one non-coalesced packet in this jumbo pkt*/
	int context_switch;             /* if context switch =1, else 0 */
};


struct chirprecord {
  long	num;/*number of packets received */
  long 	chirp_num;		/* chirp's number */
  int reorder;/* reorder=2 if there was packet reordering in this chirp*/
  long last_pkt;
  int first_pkt_loc;/*location in packet_info of first pkt in chirp */
};

struct inst_bw_pkt {
  u_int32_t chirp;/*chirp number*/
  u_int32_t timestamp_sec;/*timestamp sec field*/
  u_int32_t timestamp_usec;/*timestamp sec field*/
  u_int32_t avail_bw;/* available bw in kbps*/
};


