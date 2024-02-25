
#include "../config.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sched.h>
#include <unistd.h>
#include <sys/select.h>
#include <math.h>
#include <sched.h>

#include "assolo.h"
#include "realtime.h"

extern u_int32_t hash(u_int32_t, u_int32_t);
extern u_int32_t gen_crc_rcv2snd(struct control_rcv2snd *);
extern int check_crc_rcv2snd(struct control_rcv2snd *);
extern u_int32_t gen_crc_snd2rcv(struct udprecord *);
extern int check_crc_snd2rcv(struct udprecord *);
extern void handle_request(u_int32_t);

extern struct control_rcv2snd *pkt;
