#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

#include "../include/connt.h"

MODULE_AUTHOR("Simone Dutto");
MODULE_DESCRIPTION("module to evaluate faulty proc");
MODULE_LICENSE("GPL");

static void proc_eval_handler(struct work_struct *w);

static struct workqueue_struct *proc_q = 0;
static struct workqueue_struct *tcp_q = 0;
static DECLARE_DELAYED_WORK(proc_work, proc_eval_handler);
//static DECLARE_WORK(tcp_work, tcp_sendf_waitr);
static unsigned long onesec;
static unsigned int *features = {0};

static void
proc_eval_handler(struct work_struct *w)
{
	struct task_struct *tsk;
	struct features_info *feat;
        int pid = 2031;
        pr_info("proc evaluation\n");
	
	rcu_read_lock();
	tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
	rcu_read_unlock();
	if(tsk == NULL)
		pr_info("No process with this PID");
	pr_info("Found the process");
	
	// access to feature vectore
	feat = kzalloc(sizeof(*feat), GFP_KERNEL);
	feat->features = features;
	feat->pid = 1;
	
	INIT_WORK(&feat->work, tcp_sendf_waitr);
	queue_work(tcp_q, &feat->work); 
	
	queue_delayed_work(proc_q, &proc_work, onesec);
}

int init_module(void)
{
        onesec = msecs_to_jiffies(1000);
        pr_info("Monitor loaded\n");
	if(tcp_client_connect())
		pr_info("No connected to server\n");
		
        proc_q = create_singlethread_workqueue("proc_eval");
        tcp_q = create_singlethread_workqueue("tcp_queue");
        queue_delayed_work(proc_q, &proc_work, onesec);

        return 0;
}

void cleanup_module(void)
{
        
	cancel_delayed_work_sync(&proc_work);
        destroy_workqueue(proc_q);
        destroy_workqueue(tcp_q);
			
        pr_info("monitor exit\n");
}


