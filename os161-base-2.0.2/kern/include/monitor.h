#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <proc.h>
#include "opt-monitor.h"


int  monitor_start(void);
int monitor_addproc(struct proc* p);
int monitor_removeproc(struct proc* p);

#endif /* _MONITOR_H_ */
