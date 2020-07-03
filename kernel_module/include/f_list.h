#ifndef _PIDCACHE_H
#define _PIDCACHE_H

struct features
{
        struct list_head list;
        unsigned int id;
	unsigned int bin;
        unsigned int *features;
};

int f_list_add(unsigned int id, unsigned int bin, unsigned int *feat);
void f_list_delete(unsigned int id);
struct features * f_list_head(void);
int f_list_updater(void);
int f_list_init(void);
#endif
