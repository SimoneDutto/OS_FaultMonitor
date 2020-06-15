#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <thread.h>
#include <synch.h>
#include <monitor.h>
#include <vnode.h>
#include <vfs.h>
#include <uio.h>


#define MAX_PROC 100

struct dsfile{
	struct vnode *v;
	off_t offset;
};

struct dsfile ds_files[3];
char ds_names[3][30] = {
"dataset/bitcount.csv",
"dataset/qsort.csv",
"dataset/basicmath.csv"
};

static struct _monitor{
        int active;
        struct proc *proc[MAX_PROC+1];  
        int last_i;
        struct spinlock lk;
} monitor;

static
void monitor_check(void *ptr, unsigned long nargs){
	ptr = (void *) ptr;
	nargs = (unsigned long) nargs;
	
	while(1){
		int i = 0;
		while(i < monitor.last_i){
			spinlock_acquire(&monitor.lk);
			struct proc* p = monitor.proc[i];
			spinlock_release(&monitor.lk);
			if(p == NULL){
				i++;
				continue;
			}
			evaluate_features();
			uint32_t n = random()%10000;	
			if(n == 1){
				spinlock_acquire(&p->p_lock);
				p->faulty = 1;
				spinlock_release(&p->p_lock);
				n = 0;
			}
			i++;
			//kprintf("Ciao sono il thread di controllo\n");
		}
		thread_yield();		
	}
}

int evaluate_features(void){
	struct iovec iov;
  	struct uio ku;
  	int result;
  	struct vnode *vn;
  	struct dsfile *of;
  	void *kbuf;
	unsigned int features[20];
	int i=0, j=0, flag=1;
	
	of = &ds_files[0];
	vn = of->v;
	char feature[10];

	while(flag){
		kbuf = kmalloc(1);
  		uio_kinit(&iov, &ku, kbuf, 1, of->offset, UIO_READ);
  		result = VOP_READ(vn, &ku);
  		if (result) {
   			return 0;
  		}
		char* lett = (char *) kbuf;
		feature[j]=lett[0];
		j++;
		if(strcmp(lett,",")==0){
			feature[j]='\0';
			features[i] = atoi(feature);
			j=0;
			i++;
		}
		of->offset++;
		kfree(kbuf);
	}
	kprintf("%u first feature\n",features[0]);	
	return 0;
}

int monitor_start(void){
	struct vnode *v;
	spinlock_init(&monitor.lk);
	monitor.active=1;
	monitor.last_i=0;
	char args[20]="dummy";
	
	int result = vfs_open((char *) ds_names[0], O_RDONLY, 0, &v);
  	if (result) {
    		return -1;
  	}
	ds_files[0].v = v;
	ds_files[0].offset = 0;

	result = thread_fork("monitor" /* thread name */,
			NULL /* new process */,
			monitor_check /* thread function */,
			args /* thread arg */, 0 /* thread arg */);
	if (result) {
		kprintf("thread_fork failed: %s\n", strerror(result));
		return result;
	}
	return 0;
}

int monitor_addproc(struct proc* proc){
	int i;
	spinlock_acquire(&monitor.lk);
	i = monitor.last_i;
  	if (i>MAX_PROC) i=0;
  	while (i<MAX_PROC) {
    		if (monitor.proc[i] == NULL) {
      			monitor.proc[i] = proc;
	      		monitor.last_i = i+1;
      			break;
    		}
    		i++;
  	}
  	spinlock_release(&monitor.lk);
	return 0;
}

int monitor_removeproc(struct proc* p){
	int i;
	for(i=0;i<MAX_PROC+1;i++){
		if(monitor.proc[i] == p){
			monitor.proc[i] = NULL;
			break;
		}
	
	}
	if(i == MAX_PROC+1) return -1;
	return 0;
}


