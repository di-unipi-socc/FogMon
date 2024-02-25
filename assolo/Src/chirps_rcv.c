#include "assolo_rcv.h"
#include <math.h>

extern void sig_alrm(int signo);


/* recording contents of packet. Using inline to make code run faster */
inline void update_info(u_int32_t nc,u_int32_t np,double snd_time,double rcv_time,u_int32_t pkt_req_num,int good_jumbo_pkt)
{
  	packet_info[num_pkts_in_info].chirp_num=nc;
  	packet_info[num_pkts_in_info].num=np;
  	packet_info[num_pkts_in_info].snd_time=snd_time;
  	packet_info[num_pkts_in_info].rcv_time=rcv_time;
  	packet_info[num_pkts_in_info].request_num=pkt_req_num;
  	packet_info[num_pkts_in_info].good_jumbo_pkt=good_jumbo_pkt;
  	num_pkts_in_info++;
  	if(num_pkts_in_info>=(int)(1.5*(double)pkts_per_write))
    {
      	fprintf(stderr,"Too many packets being stored,nc=%d,np=%d\n",nc,np);
      	exit(0);
    }

  	/*  if (debug)
    		fprintf(stderr,"\n Added packet info info np=%d \n",np);
  	*/
  	return;
}

/* receive the chirps and perform available bandwidth estimations */
void receive_chirp_pkts()
{
  	double cur_pkt_rcv_time;/* receiver time stamp */
  	double cur_pkt_snd_time;/* sender time stamp */


  	/* NOTE: here a packet is non-coalesced if the next packet is received
     	at a time greater than context_rcv_thresh after it*/
  	double prev_pkt_rcv_time=0.0;/* receiver time stamp */
  	double prev_pkt_snd_time=0.0;/* receiver time stamp */
  	double good_pkt_rcv_time=0.0;/* non-coalesced receiver time stamp */
  	double good_pkt_snd_time=0.0;/* non-coalesced sender time stamp */

  	u_int32_t	np,nc,good_pkt_num=-1,good_chirp_num=-1;			/* packet, chirp number received */
  	u_int32_t prev_chirp=0,prev_pkt_num=0,prev_pkt_chirp=0;/* previous chirp number */

  	struct timeval *tp;		/* timestamp */


	/*assuming write interval in secs*/
	wait_time.it_value.tv_sec = (long)floor(write_interval);
  	wait_time.it_value.tv_usec = (long)((double)((double)write_interval-(double)floor(write_interval))*1000000.0);
  	if (debug)
    	fprintf(stderr,"\nWait time usec=%ld\n", wait_time.it_value.tv_usec);

  	wait_time.it_interval.tv_sec=0;
  	wait_time.it_interval.tv_usec=0;

  	/* this is a zero timer value used to cancel timers */
  	cancel_time.it_value.tv_sec = 0;
  	cancel_time.it_value.tv_usec =0;
  	cancel_time.it_interval.tv_sec=0;
  	cancel_time.it_interval.tv_usec=0;

  	/* timeout time */
  	if (write_interval>1)
    	timeout.it_value.tv_sec =(long)floor(write_interval)*5;
  	else
    	timeout.it_value.tv_sec =1;

  	timeout.it_value.tv_usec = 0;
  	timeout.it_interval.tv_sec=0;
  	timeout.it_interval.tv_usec=0;

 	/* this timer is set just in case the sender goes down and we are not
     	stuck forever waiting*/
  	setitimer(ITIMER_REAL,&timeout,0);

  	/* alloc space for timestamp */
  	tp = (struct timeval *)(malloc (sizeof (struct timeval)));

  	/* fix msg for recvmsg*/

	#ifdef HAVE_SO_TIMESTAMP
  		msg.msg_control=control_un.control;
  		msg.msg_controllen=sizeof(control_un.control);
  		msg.msg_name=NULL;
  		msg.msg_namelen=0;

  		msg.msg_iov=iov;
  		iov[0].iov_base=(void *)data;
  		iov[0].iov_len=MAXMESG;
  		msg.msg_iovlen=1;
	#endif

   	/*keep receiving packets*/

  	while (1)
  	{
    	static 	int cc;		/* char count for recv/send */

    	/* if remote host goes down quit this while loop and function*/
    	if (remote_host_broken==1)
      		break;

    	/* read one packet */

		#ifdef HAVE_SO_TIMESTAMP
    		cc=recvmsg(soudp,&msg,0);
		#else
    		cc = read(soudp, data, MAXMESG);
		#endif


    	if (cc < 0)
    	{
	 		perror ("assolo_rcv: read");
	 		break;
     	}

      	/* get the timestamp (kernel timestamp if possible) */

		#ifdef HAVE_TIMESTAMP_IOCTL
      		if(ioctl(soudp, SIOCGSTAMP, (void *)tp) < 0)
      		{
	 			(void) gettimeofday (tp, (struct timezone *) 0);
      		}
      		/*if(debug) fprintf(stderr,"getting IOCTL timestamp packet time stamp\n");*/

		#elif HAVE_SO_TIMESTAMP
      		if ((cmptr = CMSG_FIRSTHDR(&msg))!=NULL && cmptr->cmsg_len==CMSG_LEN(sizeof(struct timeval)))
	  		{
	  	  		if (cmptr->cmsg_level != SOL_SOCKET)
	      		{
	      			perror("assolo_rcv: control not SOL_SOCKET\n");
	      			exit(0);
	      		}

	  			if (cmptr->cmsg_type != SCM_TIMESTAMP)
	    		{
	      			perror("assolo_rcv: type not SCM_TIMESTAMP\n");
	      			exit(0);
	    		}
	  			tp=(struct timeval *)CMSG_DATA(cmptr);
			}
			else
		  	{
	    		fprintf(stderr,"TIMESTAMP PROBLEM \n");
          		exit(0);
	  		}
		#else
      		(void) gettimeofday (tp, (struct timezone *) 0);
		#endif


	    /* get the packet number */
      	np = ntohl ((u_long) udprecord->num);
      	nc = ntohl ((u_long) udprecord->chirp_num);


      	/*
       	* to end the connection, sender program will send a
       	* few packets in the end with packet number 0. It sends
       	* more than 1, so that at least one of them makes it to
       	* the receiver and the receiver process can stop gracefully.
       	*/

      	if (!np)
      	{
	 		(void) fprintf (stderr,"\nassolo_rcv: Finished servicing this client\n");
	 		break;
      	}

      	cur_pkt_snd_time =  (double) (ntohl (udprecord->timesec))
		+ ((double)(ntohl (udprecord->timeusec))/1000000.0);
      	cur_pkt_rcv_time= (double) tp->tv_sec + ((double)tp->tv_usec/1000000.0);

      	/* record packet info if it corresponds to latest request of rates, if it is last non-coalesced packet of a particular rate*/
      	if (ntohl(udprecord->request_num)==request_num-1)
		{
	  		if(jumbo>1)
	    	{

	      		if (cur_pkt_rcv_time-prev_pkt_rcv_time>context_receive_thresh)
				{
		  			good_pkt_rcv_time=prev_pkt_rcv_time;
		  			good_pkt_snd_time=prev_pkt_snd_time;
		  			good_chirp_num=prev_pkt_chirp;
		  			good_pkt_num=prev_pkt_num;
				}

	      		if (np>prev_pkt_num && prev_pkt_num>0)
				{
		  			/*if the good packet has the previous packet number record it */
		  			if (good_pkt_num==np-1)
		  			{
						update_info(good_chirp_num,good_pkt_num,good_pkt_snd_time,good_pkt_rcv_time,ntohl(udprecord->request_num),1);
		  			}
		  			else
		  	  			if (prev_pkt_num==np-1)/*record last packet of previous jumbo*/
		      			{
							update_info(prev_pkt_chirp,prev_pkt_num,prev_pkt_snd_time,prev_pkt_rcv_time,ntohl(udprecord->request_num),0);
							/* since there was no good packet only consider packets till this one*/
		      			}
		    			else /*no packets from np-1 jumbo so record good packet of last jumbo*/
		      				update_info(good_chirp_num,good_pkt_num,good_pkt_snd_time,good_pkt_rcv_time,ntohl(udprecord->request_num),1);
				}

	      		/*record first packet of last jumbo, not optimal but should not cause much trouble*/
	      		if (np==num_interarrival+1 && prev_pkt_num<num_interarrival+1)
					update_info(nc,np,cur_pkt_snd_time,cur_pkt_rcv_time,ntohl(udprecord->request_num),1);

	      		prev_pkt_rcv_time=cur_pkt_rcv_time;
	      		prev_pkt_snd_time=cur_pkt_snd_time;
	      		prev_pkt_num=np;
	      		prev_pkt_chirp=nc;

	 		}
	  		else /*if single packet jumbo*/
	    	{
	      		update_info(nc,np,cur_pkt_snd_time,cur_pkt_rcv_time,ntohl(udprecord->request_num),1);
	    	}
		}

    	if (nc>prev_chirp)
		{

	  		/* use latest wait time if chirp corresponds to latest parameters */
	  		if (ntohl(udprecord->request_num)==request_num-1)
	  		{
	      		wait_time.it_value.tv_sec = (long)floor(write_interval);
	      		wait_time.it_value.tv_usec = (long)((double)((double)write_interval-(double)floor(write_interval))*1000000.0);

	      		if (debug)
	      			fprintf(stderr,"Wait time=%ld us\n",wait_time.it_value.tv_usec);
	      		if (write_interval>1)
					timeout.it_value.tv_sec =(long)floor(write_interval)*5;
	      		else
					timeout.it_value.tv_sec =1;

	    	  		setitimer(ITIMER_REAL, &cancel_time, 0);
	      		if (debug) fprintf(stderr,"Cancelled timer\n");

	      		setitimer(ITIMER_REAL, &wait_time, 0);
	    	}
		}
    	prev_chirp=nc;

  	}/*end of while*/

  	/* freeing the timestamp pointer*/
  	free(tp);

}

