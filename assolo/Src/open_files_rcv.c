
#include "assolo_rcv.h"

/*
Creates files for writing. The file name is <src name>_<dst
name>_<starting timestamp>.{instbw,par}

".instbw" contains <time> <mean>

".par" contains the parameters of the experiment for
future reference
*/

void open_dump_files(char *src,char *dst)
{
	char instbw_file_name[MAXFNAME];
   	char debug_file_name[MAXFNAME];
  	char time_string[40];
  	struct  timeval tv;		/* timestamp */

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


  	/*  fd_instbw = fopen(instbw_file_name, "w");
  	if (fd_instbw == NULL)
  	{
    	fprintf(stderr, "Error opening dump file\n");
  	}

  	fprintf(stderr, "Opening file: %s\n",instbw_file_name);
  	*/
  	if(debug)
    {
      	fd_debug = fopen(debug_file_name, "w");
      	if (fd_debug == NULL)
      	{
			fprintf(stderr, "Error opening dump file\n");
      	}
    }

}
