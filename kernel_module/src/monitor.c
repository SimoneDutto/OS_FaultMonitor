#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

#include "../include/connt.h"
#include "../include/m_pfile.h"
#include "../include/f_list.h"
#include "../include/monitor.h"

MODULE_AUTHOR("Simone Dutto");
MODULE_DESCRIPTION("module to evaluate faulty proc");
MODULE_LICENSE("GPL");

static void proc_eval_handler(struct work_struct *w);
static void feat_updater(struct work_struct *w);

static struct workqueue_struct *proc_q = 0;
static struct workqueue_struct *tcp_q = 0;
static struct workqueue_struct *ft_q = 0;
DECLARE_DELAYED_WORK(proc_work, proc_eval_handler);
DECLARE_DELAYED_WORK(ft_work, feat_updater);
//static DECLARE_WORK(tcp_work, tcp_sendf_waitr);
static unsigned long onesec;

static void
proc_eval_handler(struct work_struct *w)
{
	struct features_info *feat;
	struct features *f;
        
        pr_info("proc evaluation\n");
	f = f_list_head(); //extract one from queue
	if(f==NULL) add_work_queue(); // test if it's effective
	// access to features vector before sending info to socket handler
	feat = kzalloc(sizeof(*feat), GFP_KERNEL);
	feat->features = f->features;
	feat->pid = f->id;
	// queue a socket work, 
	INIT_WORK(&feat->work, tcp_sendf_waitr);
	queue_work(tcp_q, &feat->work);
	kfree(f);
}


static void feat_updater(struct work_struct *w){
	if(f_list_updater())
		pr_err("Updater failed to update\n");
	pr_info("Updater successful");
	queue_delayed_work(ft_q, &ft_work, onesec/2);
}



int init_module(void)
{
        onesec = msecs_to_jiffies(1000);
        pr_info("Monitor loaded\n");
        if(pfile_init())
        	pr_info("Device not registered\n");
	if(tcp_client_connect())
		pr_info("No connected to server\n");
	f_list_init();	
        proc_q = create_singlethread_workqueue("proc_eval");
        tcp_q = create_singlethread_workqueue("tcp_queue");
        ft_q = create_singlethread_workqueue("f_queue");
        queue_delayed_work(ft_q, &ft_work, onesec/2);

        return 0;
}



void cleanup_module(void)
{
	cancel_delayed_work_sync(&proc_work);
	cancel_delayed_work_sync(&ft_work);
        destroy_workqueue(proc_q);
        destroy_workqueue(tcp_q);
        destroy_workqueue(ft_q);
        tcp_client_disconnect();
	pfile_cleanup();		
        pr_info("monitor exit\n");
}

void add_work_queue(void){
	queue_delayed_work(proc_q, &proc_work, onesec);
}



