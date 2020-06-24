#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <asm/errno.h>

#include "../include/f_list.h"
#include "../include/monitor.h"

/*
	Code take by github/jinb-park
*/

struct features
{
        struct list_head list;
        unsigned int id;
        unsigned int *features;
};

/* Protects the f_list, f_list_num, and the featuress within it */
spinlock_t list_lock;
static LIST_HEAD(f_list);
static unsigned int f_list_num = 0;
#define MAX_f_list_SIZE 10


int list_change_feature(unsigned int pid, unsigned int *new_feat){
	struct features *f = NULL;
	struct features *new_f = NULL;
	struct features *old_f = NULL;
	/**
	 * updater
	 *
	 * (updater) require that alloc new node & copy, update new node & reclaim old node
	 * list_replace_rcu() is used to do that.
	*/
	rcu_read_lock();

	list_for_each_entry(f, &f_list, list) {
		if(f->id == pid) {
			old_f = f;
			break;
		}
	}

	if(!old_f) {
		rcu_read_unlock();
		return -1;
	}
	new_f = kzalloc(sizeof(struct features), GFP_ATOMIC);
	if(!new_f) {
		rcu_read_unlock();
		return -1;
	}
	memcpy(new_f, old_f, sizeof(struct features));
	new_f->features = new_feat;
	spin_lock(&list_lock);
	list_replace_rcu(&old_f->list, &new_f->list);
	spin_unlock(&list_lock);

	rcu_read_unlock();
	synchronize_rcu();
	kfree(old_f);
	
	return 0;

}

int f_list_add(unsigned int id, unsigned int *feat)
{
        struct features *obj;

        if ((obj = kzalloc(sizeof(*obj), GFP_KERNEL)) == NULL)
                return -ENOMEM;

        obj->id = id;
        obj->features = feat;
        
        spin_lock(&list_lock);
        list_add_tail_rcu(&obj->list, &f_list);
        spin_unlock(&list_lock);
        add_work_queue();
        return 0;
}


void f_list_delete(unsigned int id)
{
        struct features *f;

	spin_lock(&list_lock);
	list_for_each_entry(f, &f_list, list) {
		if(f->id == id) {
			list_del_rcu(&f->list);
			spin_unlock(&list_lock);
			synchronize_rcu();
			kfree(f);
			return;
		}
	}
	spin_unlock(&list_lock);

	pr_err("not exist book\n");
}

unsigned int f_list_head(void){
	unsigned int PID=0;
	
        return PID;
	
}

int f_list_init(void){
	spin_lock_init(&list_lock);
	return 0;
}


