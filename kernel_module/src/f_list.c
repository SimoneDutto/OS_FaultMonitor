#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/random.h>
#include <linux/pmctrack.h>
#include <asm/errno.h>

#include "../include/f_list.h"
#include "../include/monitor.h"

/*
	Code take by github/jinb-park
*/
#define NUM 20


/* Protects the f_list, f_list_num, and the featuress within it */
spinlock_t list_lock;
static LIST_HEAD(f_list);
//static unsigned int f_list_num = 0;
#define MAX_f_list_SIZE 10



static unsigned int* get_randoms(unsigned int id){
	int i = 0;
	unsigned int *fts = NULL;
	uint64_t value;
	
	fts = kzalloc(NUM*sizeof(unsigned int), GFP_KERNEL);
	if(fts == NULL) return NULL;
	
	
	for(i=0;i<NUM;i++){
		//get_random_bytes(&fts[i], sizeof(unsigned int));		
		pmcs_get_current_metric_value(get_pid_task(find_get_pid(id),PIDTYPE_PID),\
		 				0, &value);
		fts[i] = (unsigned int) value;
		
	}
	
	return fts;
}

/**
*  Routine to update features: exploit RCU list to being able to update features without
*  affecting the evaluation thread
*
**/
int f_list_updater(void){
	struct features *f = NULL;
	struct features *new_f = NULL;
	struct features *old_f = NULL;
	unsigned int * new_feat;
	int finish = 0, init=0;
	/**
	 * updater
	 *
	 * (updater) require that alloc new node & copy, update new node & reclaim old node
	 * list_replace_rcu() is used to do that.
	*/
	while(!finish){
		new_feat = get_randoms(f->id);
		if(!new_feat) return -1;
		
		rcu_read_lock();
		if(!init) 
			old_f = list_first_or_null_rcu(&f_list, struct features, list);
		else 
			old_f = list_next_or_null_rcu(&f_list, &new_f->list, \
							struct features, list);
		if(old_f == NULL){
			finish = 1;
			rcu_read_unlock();
			continue;
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
	}
	
	return 0;

}

int f_list_add(unsigned int id, unsigned int bin, unsigned int *feat)
{
        struct features *obj;

        if ((obj = kzalloc(sizeof(*obj), GFP_KERNEL)) == NULL)
                return -ENOMEM;

        obj->id = id;
	obj->bin = bin;
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

struct features * f_list_head(void){
	struct features* old_f=NULL, *new_f=NULL;
	rcu_read_lock();
	old_f = list_first_or_null_rcu(&f_list, struct features, list);
	if(old_f == NULL) return NULL;
	new_f = kzalloc(sizeof(*new_f), GFP_ATOMIC);
	if(!new_f) {
		rcu_read_unlock();
		return NULL;
	}
	memcpy(new_f, old_f, sizeof(struct features));
	spin_lock(&list_lock);
	list_del_rcu(&old_f->list);
	spin_unlock(&list_lock);
	rcu_read_unlock();
	synchronize_rcu();
	kfree(old_f);
        return new_f;
	
}

int f_list_init(void){
	spin_lock_init(&list_lock);
	return 0;
}


