
/* cryptographic hash function for authentication */

/* SECURITY: In this implementation assolo_snd will be on
continuously and wait for assolo_rcv to initiate the connection.
In addition assolo_rcv acts as a master telling assolo_snd
at what rates to send packets etc.

As the first step towards making assolo more secure we
add a crc to each control packet to ensure that a  malicious
receiver does not ask deployed senders to send out UDP storms.

NOTE: This is a cooked up, insecure, crc function for testing.
Feel free to replace hash with a better function (SHA-1, MD5 etc.).
 */

/*
TO USE THE SECURITY FEATURE
===========================

Set the values of XORMASK1/2/3 to different values of your choice.
They must be the same at sender and receiver and unknown to anyone else.
*/

#include "assolo_snd.h"


#define XORMASK1 0x7ab04793L
#define XORMASK2 0x02ef4c83L
#define XORMASK3 0x1e158321L

/* hash function, input two 32 bit words, output one 32 bit key*/
u_int32_t hash(u_int32_t crc, u_int32_t element)
{
	u_int32_t  tmp,tmp1;
	crc=((~crc) ^ XORMASK1);

	tmp=(((element & 0x0000ffff)<<16) ^ XORMASK2);
	tmp=tmp ^ (((element & 0xffff0000)>>16) ^ XORMASK3);
	tmp=tmp ^ (((element & 0x00ffff00)>>8) ^ XORMASK1);
  	tmp=tmp ^ (((element & 0x00ffff00)<<8) ^ XORMASK1);
	tmp = tmp ^ element;

	tmp1=(((crc & 0x0000ffff)<<16) ^ XORMASK2);
	tmp1=tmp1 ^ (((crc & 0xffff0000)>>16) ^ XORMASK3);
	tmp1=tmp1 ^ (((crc & 0x00ffff00)>>8) ^ XORMASK1);
	tmp=tmp ^ (((crc & 0x00ffff00)<<8) ^ XORMASK1);
	tmp1 = tmp1 ^ crc;

	crc= tmp ^ tmp1;

	return crc;

}

/* given a rcv2snd control packet compute the checksum*/
u_int32_t gen_crc_rcv2snd(struct control_rcv2snd *pkt)
{
  	u_int32_t cur_element,crc;

  	crc=ntohl(pkt->request_type);
  	cur_element=ntohl(pkt->request_num);
  	crc=hash(crc,cur_element);

  	cur_element=ntohl(pkt->num);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->timesec);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->timeusec);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->chal_no);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->num_interarrival);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->inter_chirp_time);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->low_rate);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->high_rate);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->soglia);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->filter);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->spread_factor);
  	crc=hash(crc,cur_element);
  	cur_element=ntohl(pkt->pktsize);
  	crc=hash(crc,cur_element);

  	return(crc);
}


int check_crc_rcv2snd(struct control_rcv2snd *pkt)
{

  	u_int32_t crc;

  	crc=gen_crc_rcv2snd(pkt);

  	if (crc==ntohl(pkt->checksum))
    	return 1;
  	else
  		return 0;
}

/* given a snd2rcv packet compute the checksum*/

u_int32_t gen_crc_snd2rcv(struct udprecord *pkt)
{
  	u_int32_t cur_element,crc;

  	crc=ntohl(pkt->num);

  	cur_element=ntohl(pkt->request_num);
  	crc=hash(crc,cur_element);

  	cur_element=ntohl(pkt->chirp_num);
  	crc=hash(crc,cur_element);

  	cur_element=ntohl(pkt->timesec);
  	crc=hash(crc,cur_element);

  	cur_element=ntohl(pkt->timeusec);
  	crc=hash(crc,cur_element);

  	cur_element=ntohl(pkt->chal_no);
  	crc=hash(crc,cur_element);

  	return(crc);
}

int check_crc_snd2rcv(struct udprecord *pkt)
{

  	u_int32_t crc;

  	crc=gen_crc_snd2rcv(pkt);
  	if (crc==ntohl(pkt->checksum))
    	return 1;
  	else
    	return 0;
}

