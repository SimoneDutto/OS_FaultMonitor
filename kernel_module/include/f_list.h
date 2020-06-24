#ifndef _PIDCACHE_H
#define _PIDCACHE_H

int f_list_add(unsigned int id, unsigned int *feat);
void f_list_delete(unsigned int id);
unsigned int f_list_head(void);
int f_list_change_feature(unsigned int pid, unsigned int *new_feat);
int f_list_init(void);
#endif
