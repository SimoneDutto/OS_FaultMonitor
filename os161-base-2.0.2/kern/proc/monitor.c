#include <types.h>
#include <lib.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <thread.h>
#include <synch.h>
#include <monitor.h>

#define MAX_PROC 100

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

int monitor_start(void){
	spinlock_init(&monitor.lk);
	monitor.active=1;
	monitor.last_i=0;
	char args[20]="dummy";
	
	int result = thread_fork("monitor" /* thread name */,
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


