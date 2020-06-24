#ifndef _PIDCACHE_H
#define _PIDCACHE_H

int cache_add(unsigned int id);
void cache_delete(unsigned int id);
int cache_find(unsigned int id);
unsigned int cache_head(void);

#endif
