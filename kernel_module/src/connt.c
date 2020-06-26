#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>

#include "../include/connt.h"
#include "../include/f_list.h"
#define PORT 2325

/*
This code is taken by Aby Sam Ross
github:  https://github.com/abysamross/simple-linux-kernel-tcp-client-server/
This is used to send features to be evaluated to a python socket server
NOTE: I could have sent information through device file, but it is easier to decouple features sending and evaluation in this way, having in mind in the future the actual evaluator could be placed on another host over the internet
*/
int tcp_client_send(struct socket *sock, const char *buf, const size_t length,\
                unsigned long flags);
int tcp_client_receive(struct socket *sock, char *str,\
                        unsigned long flags);

static struct socket *conn_socket = NULL;

static u32 create_address(u8 *ip)
{
        u32 addr = 0;
        int i;

        for(i=0; i<4; i++)
        {
                addr += ip[i];
                if(i==3)
                        break;
                addr <<= 8;
        }
        return addr;
}



void tcp_sendf_waitr(struct work_struct *work){
        unsigned int pid=0;
        unsigned int *feat;
        char response[8];
        char reply[8];
        DECLARE_WAIT_QUEUE_HEAD(recv_wait);
        
        struct features_info *ft = container_of(work, struct features_info, work);
        pid = ft->pid;
        feat = ft->features;
        
        pr_info("PID calling: %u", pid);
        pr_info("feat[0]: %u", feat[0]);
        memset(&reply, 0, 4);
        memcpy(reply, &feat[0], 4);
        sprintf(reply,"%d", pid);
        tcp_client_send(conn_socket, reply, 4, MSG_DONTWAIT);

	
        wait_event_timeout(recv_wait,\
                        !skb_queue_empty(&conn_socket->sk->sk_receive_queue),\
                                                                        5*HZ);
        
	if(!skb_queue_empty(&conn_socket->sk->sk_receive_queue))
	{
		memset(&response, 0, 4);
		tcp_client_receive(conn_socket, response, MSG_DONTWAIT);
		if(response[0]=='0')
			f_list_add(pid, feat);
		else{
			pr_info("Proc %u faulty, sending kill message", pid);
			kfree(feat);
			kill_pid(find_vpid(pid), SIGKILL, 1);
		}
	}

        kfree(ft);
        return;
}

int tcp_client_connect(void)
{
        struct sockaddr_in saddr;
        /*
        struct sockaddr_in daddr;
        struct socket *data_socket = NULL;
        */
        unsigned char destip[5] = {127,0,0,1,'\0'};
        /*
        char *response = kmalloc(4096, GFP_KERNEL);
        char *reply = kmalloc(4096, GFP_KERNEL);
        */
        
        int ret = -1;

        //DECLARE_WAITQUEUE(recv_wait, current);
        
        ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &conn_socket);
        if(ret < 0)
        {
                pr_info(" *** mtp | Error: %d while creating first socket. | "
                        "setup_connection *** \n", ret);
                goto err;
        }

        memset(&saddr, 0, sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(PORT);
        saddr.sin_addr.s_addr = htonl(create_address(destip));

        ret = conn_socket->ops->connect(conn_socket, (struct sockaddr *)&saddr\
                        , sizeof(saddr), O_RDWR);
        if(ret && (ret != -EINPROGRESS))
        {
                pr_info(" *** mtp | Error: %d while connecting using conn "
                        "socket. | setup_connection *** \n", ret);
                goto err;
        }
        
        return 0;

err:
        return -1;
}

void tcp_client_disconnect(void){
	if(conn_socket != NULL)
        {
                sock_release(conn_socket);
        }
}

int tcp_client_send(struct socket *sock, const char *buf, const size_t length,\
                unsigned long flags)
{
        struct msghdr msg;
        //struct iovec iov;
        struct kvec vec;
        int len, written = 0, left = length;
        mm_segment_t oldmm;

        msg.msg_name    = 0;
        msg.msg_namelen = 0;
        /*
        msg.msg_iov     = &iov;
        msg.msg_iovlen  = 1;
        */
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags   = flags;

        oldmm = get_fs(); set_fs(KERNEL_DS);
repeat_send:
        /*
        msg.msg_iov->iov_len  = left;
        msg.msg_iov->iov_base = (char *)buf + written; 
        */
        vec.iov_len = left;
        vec.iov_base = (char *)buf + written;

        //len = sock_sendmsg(sock, &msg, left);
        len = kernel_sendmsg(sock, &msg, &vec, left, left);
        if((len == -ERESTARTSYS) || (!(flags & MSG_DONTWAIT) &&\
                                (len == -EAGAIN)))
                goto repeat_send;
        if(len > 0)
        {
                written += len;
                left -= len;
                if(left)
                        goto repeat_send;
        }
        set_fs(oldmm);
        return written ? written:len;
}

int tcp_client_receive(struct socket *sock, char *str,\
                        unsigned long flags)
{
        //mm_segment_t oldmm;
        struct msghdr msg;
        //struct iovec iov;
        struct kvec vec;
        int len;
        int max_size = 50;

        msg.msg_name    = 0;
        msg.msg_namelen = 0;
        /*
        msg.msg_iov     = &iov;
        msg.msg_iovlen  = 1;
        */
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags   = flags;
        /*
        msg.msg_iov->iov_base   = str;
        msg.msg_ioc->iov_len    = max_size; 
        */
        vec.iov_len = max_size;
        vec.iov_base = str;

        //oldmm = get_fs(); set_fs(KERNEL_DS);
read_again:
        //len = sock_recvmsg(sock, &msg, max_size, 0); 
        len = kernel_recvmsg(sock, &msg, &vec, max_size, max_size, flags);

        if(len == -EAGAIN || len == -ERESTARTSYS)
        {
                pr_info(" *** mtp | error while reading: %d | "
                        "tcp_client_receive *** \n", len);

                goto read_again;
        }


        pr_info(" *** mtp | the server says: %s | tcp_client_receive *** \n", str);
        //set_fs(oldmm);
        return len;
}
