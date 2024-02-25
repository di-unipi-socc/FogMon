
#include "assolo_rcv.h"

/* check for packet reordering and loss */
void check_reorder_loss()
{
 	int count=0,too_many_chirps=0;
 	unsigned long nc,np;

  	/* initializing */
  	memset(chirp_info, 0, (int) chirps_per_write*2 * sizeof(struct chirprecord));

  	first_chirp=packet_info[0].chirp_num;/*first chirp stored*/
  	last_chirp=first_chirp;

  	chirp_info[0].chirp_num=first_chirp;
  	chirp_info[0].num=1;
  	chirp_info[0].last_pkt=packet_info[0].num;

    if(debug)
	{
		fprintf(fd_debug,"%d %d %f %f\n",packet_info[count].chirp_num,packet_info[count].num,packet_info[count].snd_time,packet_info[count].rcv_time);
	}

  	for (count=1;count<num_pkts_in_info;count++)
    {
      	if(debug)
		{
	  		fprintf(fd_debug,"%d %d %f %f\n",packet_info[count].chirp_num,packet_info[count].num,packet_info[count].snd_time,packet_info[count].rcv_time);
		}

      	nc=packet_info[count].chirp_num;

 		/*Overflow check, don't record if out of area*/
        if ((nc-first_chirp)>=(2*chirps_per_write))
        {
        	if (too_many_chirps==0)
            {
            	fprintf(stderr,"\nNo space left in chirp_info, skipping packets\n");
                too_many_chirps=1;
            }
            continue;
        }

      	if (nc>last_chirp)
		{
	  		last_chirp=nc;
	  		chirp_info[nc-first_chirp].first_pkt_loc=count;/*location of first packet*/
	  		/*Note: For chirps with reordered packets "count" need not represent packet number 1. However we discard chirps with reordered packets and so wrongly marking the above is not a problem.*/
		}

      	np=packet_info[count].num;
      	/* if late packet of earlier chirp, mark as reordered and discard*/
      	if (nc>=first_chirp)
		{
	  		/* discarding all chirps that do not have the new rates */
	  		if (packet_info[count].request_num<request_num-1)
	  	  		chirp_info[nc-first_chirp].reorder=2;

	  		if (packet_info[count].request_num>sender_request_num)
	    		sender_request_num=packet_info[count].request_num;

	  		chirp_info[nc-first_chirp].num++;
	  		if(np<chirp_info[nc-first_chirp].last_pkt)
	    		chirp_info[nc-first_chirp].reorder=2;/*2 is the value for reordering */

	  		chirp_info[nc-first_chirp].last_pkt=np;
		}
      	else
		{
	  		packet_info[count].context_switch=2;/*2 is the value for reordering, will discard packet */
	  		/*We mark packet_info because there is no entry in chirp_info for packets from "earlier" chirps that have arrived late*/
		}
    }

}





