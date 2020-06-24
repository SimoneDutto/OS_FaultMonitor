#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <asm/errno.h>

#include "../include/pid_cache.h"
#include "../include/monitor.h"

/*
	Code take by kernel.org
*/

struct object
{
        struct list_head list;
        unsigned int id;
};

/* Protects the cache, cache_num, and the objects within it */
static DEFINE_MUTEX(cache_lock);
static LIST_HEAD(cache);
static unsigned int cache_num = 0;
#define MAX_CACHE_SIZE 10

/* Must be holding cache_lock */
static struct object *__cache_find(int id)
{
        struct object *i;

        list_for_each_entry(i, &cache, list)
                if (i->id == id) {
                        return i;
                }
        return NULL;
}

/* Must be holding cache_lock */
static void __cache_delete(struct object *obj)
{
        BUG_ON(!obj);
        list_del(&obj->list);
        kfree(obj);
        cache_num--;
}

/* Must be holding cache_lock */
static unsigned int __cache_head(void)
{
	struct object *obj = NULL;
	unsigned int PID;
        obj = list_first_entry(&cache, struct object, list);
        BUG_ON(!obj);
        PID = obj->id;
        list_del(&obj->list);
        kfree(obj);
        cache_num--;
        return PID;
}

/* Must be holding cache_lock */
static void __cache_add(struct object *obj)
{
        list_add(&obj->list, &cache);
        /* handle too many PID to check: unlikely
        if (++cache_num > MAX_CACHE_SIZE) {
                struct object *i, *outcast = NULL;
                list_for_each_entry(i, &cache, list) {
                        if (!outcast || i->popularity < outcast->popularity)
                                outcast = i;
                }
                __cache_delete(outcast);
        }
        */
        
}

int cache_add(unsigned int id)
{
        struct object *obj;

        if ((obj = kmalloc(sizeof(*obj), GFP_KERNEL)) == NULL)
                return -ENOMEM;

        obj->id = id;
        
        mutex_lock(&cache_lock);
        __cache_add(obj);
        mutex_unlock(&cache_lock);
        add_work_queue();
        return 0;
}

void cache_delete(unsigned int id)
{
        mutex_lock(&cache_lock);
        __cache_delete(__cache_find(id));
        mutex_unlock(&cache_lock);
}

int cache_find(unsigned int id)
{
        struct object *obj;
        int ret = -ENOENT;

        mutex_lock(&cache_lock);
        obj = __cache_find(id);
        if (obj) {
                ret = 0;
        }
        mutex_unlock(&cache_lock);
        return ret;
}

unsigned int cache_head(void){
	unsigned int PID;
	mutex_lock(&cache_lock);
        PID = __cache_head();
        mutex_unlock(&cache_lock);
        return PID;
	
}
