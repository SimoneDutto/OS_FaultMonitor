#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

static void mykmod_work_handler(struct work_struct *w);

static struct workqueue_struct *wq = 0;
static DECLARE_DELAYED_WORK(mykmod_work, mykmod_work_handler);
static unsigned long onesec;

static void
mykmod_work_handler(struct work_struct *w)
{
        pr_info("mykmod work %u jiffies\n", (unsigned)onesec);
}


int init_module(void)
{
        onesec = msecs_to_jiffies(1000);
        pr_info("mykmod loaded %u jiffies\n", (unsigned)onesec);

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


MODULE_DESCRIPTION("mykmod");
MODULE_LICENSE("GPL");
