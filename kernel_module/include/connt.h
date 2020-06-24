#ifndef _CONNT_H
#define _CONNT_H

int tcp_client_connect(void);
void tcp_client_disconnect(void);
void tcp_sendf_waitr(struct work_struct *w);

struct features_info{
	unsigned int pid;
	unsigned int *features;
	struct work_struct work;
};

#endif
