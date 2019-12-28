/* FILE: alloc_rcv.c */

#include "assolo_rcv.h"

/* free arrays for next connection */

void free_arrays()
{
	free(rates);
  	free(qing_delay_cumsum);
  	free(qing_delay);
  	free(inst_bw_estimates_excursion);
  	free(iat);
  	free(av_bw_per_pkt);
  	free(packet_info);
  	free(chirp_info);
  	created_arrays=0;

}

/* create arrays */

void create_arrays()
{

 	/* The first part of the data packet can be accessed
 	    as the log record */
  	udprecord = (struct udprecord *) data;

  	inst_bw_estimates_excursion=(double *)calloc((int)(num_inst_bw),sizeof(double));

  	qing_delay=(double *)calloc((int)(MAXCHIRPSIZE-1+1),sizeof(double));

	qing_delay_cumsum=(double *)calloc((int)(MAXCHIRPSIZE-1+1),sizeof(double));

  	rates=(double *)calloc((int)(MAXCHIRPSIZE-1),sizeof(double));

  	iat=(double *)calloc((int)(MAXCHIRPSIZE-1),sizeof(double));

  	av_bw_per_pkt=(double *)calloc((int)(MAXCHIRPSIZE-1),sizeof(double));

  	update_rates_iat();

  	/*allocating enough memory to store packet information */

  	chirps_per_write=(int)(2*(1+(write_interval/(inter_chirp_time))));

  	pkts_per_write=(int)((MAXCHIRPSIZE)*chirps_per_write);

  	if (debug)
  		fprintf(stderr,"chirps_per_write=%d, pkts_per_write=%d\n",chirps_per_write,pkts_per_write);


  	packet_info=(struct pkt_info *)calloc(pkts_per_write*2,sizeof(struct pkt_info));

  	chirp_info=(struct chirprecord *)calloc((int)((double)chirps_per_write*2.0),sizeof(struct chirprecord));

  	created_arrays=1;

}

	/* update the chirp bit rates and interarrival times*/
void update_rates_iat()
{
  	int count;

  	double thr;

  	int k;

  	if (debug)
  		fprintf(stderr,"low rate=%f, high_rate=%f\n",low_rate,high_rate);


  	thr=(soglia/100)*((high_rate-low_rate)/2);

  	k=(num_interarrival-1)/2;

  	/*caricamento dei vettori*/
  	rates[k] = ((high_rate + low_rate)/2);

  	for (count=0;count < k ;count++)
  	{
		rates[k+count+1]=(rates[k+count]+(thr*pow(spread_factor,count)));
		rates[k-count-1]=(rates[k-count]-(thr*pow(spread_factor,count)));
  	}


  	for (count=0;count<num_interarrival;count++)
  	{
		rates[count]=rates[count]*1000000.0;
  	 	iat[count]=8*((double) pktsize)/rates[count];
		if(debug)
			fprintf(stderr,"count %d iat %.8f rates %f\n",count,iat[count],rates[count]);
 	}

  	return;
}
