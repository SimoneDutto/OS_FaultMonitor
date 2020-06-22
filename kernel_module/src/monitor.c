#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/rcupdate.h>

#include "../include/connt.h"

MODULE_AUTHOR("Simone Dutto");
MODULE_DESCRIPTION("module to evaluate faultry proc");
MODULE_LICENSE("GPL");

static void mykmod_work_handler(struct work_struct *w);

static struct workqueue_struct *wq = 0;
static DECLARE_DELAYED_WORK(mykmod_work, mykmod_work_handler);
static unsigned long onesec;
static unsigned int *features = {0,0};

static void
mykmod_work_handler(struct work_struct *w)
{
	struct task_struct *tsk;
        pr_info("mykmod work %u jiffies\n", (unsigned)onesec);
	
	rcu_read_lock();
	tsk = pid_task(find_vpid(2031), PIDTYPE_PID);
	if(tsk == NULL)
		pr_info("No process with this PID");
	pr_info("Found the process");
	tcp_sendf_waitr(features);
	
	rcu_read_unlock();
}

int init_module(void)
{
        onesec = msecs_to_jiffies(1000);
        pr_info("mykmod loaded %u jiffies\n", (unsigned)onesec);
	tcp_client_connect();
        if (!wq)
                wq = create_singlethread_workqueue("mykmod");
        if (wq)
                queue_delayed_work(wq, &mykmod_work, onesec);

        return 0;
}

void cleanup_module(void)
{
        if (wq){
		cancel_delayed_work_sync(&mykmod_work);
                destroy_workqueue(wq);
			
	}
        pr_info("mykmod exit\n");
}


