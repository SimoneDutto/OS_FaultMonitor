#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <linux/uaccess.h>

#include "../include/m_pfile.h"
#include "../include/pid_cache.h"


#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME 		"signalpid"

#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h> /* seq_read, seq_lseek, single_open, single_release */
#include <uapi/linux/stat.h> /* S_IRUSR */

#include "../include/m_pfile.h"

static const char *filename = "monitor";

static int show(struct seq_file *m, void *v)
{
	seq_printf(m, "abcd\n");
	return 0;
}

static int open(struct inode *inode, struct  file *file)
{
	return single_open(file, show, NULL);
}
// sudo bash -c "/usr/bin/echo 1234 > /proc/monitor" to try

static ssize_t write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	long ret;
	unsigned int PID;
	ret = kstrtouint_from_user(buffer, count, 10, &PID);
	if (ret)
		return ret;
	pr_info("PID to signal: %u", PID);
	cache_add(PID);
	return count;
}

static const struct file_operations fops = {
	.llseek = seq_lseek,
	.open = open,
	.write = write,
	.owner = THIS_MODULE,
	.read = seq_read,
	.release = single_release,
};

int pfile_init(void)
{
	proc_create(filename, 0, NULL, &fops);
	return 0;
}

void pfile_cleanup(void)
{
	remove_proc_entry(filename, NULL);
}








