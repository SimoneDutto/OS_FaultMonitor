obj-m += monitor.o 
monitor-objs := src/monitor.o src/connt.o src/m_pfile.o src/f_list.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
