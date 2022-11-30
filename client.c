#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#define ECHO_ADDR "127.0.0.1"

void errorExit(char err[]);
short csum(char *buff);

struct linkHeader{
    unsigned char dest_mac[6];
    unsigned char source_mac[6];
    short int type;
};

struct udpHeader{
    short int source;
    short int dest;
    short int len; 
    short int check;
};


void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}

short csum(char *buff){
    int sum = 0;
    short *ptr;
    short tmp;

    ptr = (short*)buff;
    for(int i = 0; i < 10; i++){
        sum += *ptr; //* или без
        ptr++;
    }

    
        tmp = sum >> 16;
        sum = (sum & 0xFFFF) + tmp;
    

    return sum;
}

int main(){
    
    struct sockaddr_ll serv;
    char msg[] = "Hello, server";
    char buff[100];
    struct linkHeader link;
    struct iphdr ip;
    struct udpHeader header;
    int one = 1;
   
    //----------------------------------------------------
    
    memset((void*)&serv, 0, sizeof(serv));
    serv.sll_family   = AF_PACKET;
    serv.sll_ifindex  = if_nametoindex("wlp0s20f3");
    if(serv.sll_ifindex == 0)
        errorExit("index");
    serv.sll_halen    = 6;
    serv.sll_addr[0]  = 0xb8;
    serv.sll_addr[1]  = 0x97;
    serv.sll_addr[2]  = 0x5a;
    serv.sll_addr[3]  = 0xf3;
    serv.sll_addr[4]  = 0x5a;
    serv.sll_addr[5]  = 0xce;

    //------------------------------------------

    link.source_mac[0] = 0xd8;  
    link.source_mac[1] = 0xf2;  
    link.source_mac[2] = 0xca;
    link.source_mac[3] = 0xdc;
    link.source_mac[4] = 0x08;
    link.source_mac[5] = 0x02;
    link.dest_mac[0] = 0xb8;
    link.dest_mac[1] = 0x97;
    link.dest_mac[2] = 0x5a;
    link.dest_mac[3] = 0xf3;
    link.dest_mac[4] = 0x5a;
    link.dest_mac[5] = 0xce;
    link.type = htons(0x0800);
    //------------------------------------------
    ip.ihl      = 5;
    ip.version  = 4;
    ip.tos      = 16;
    ip.tot_len  = htons(sizeof(msg) + sizeof(ip) + sizeof(header));
    ip.id       = 20;
    ip.ttl      = 64;
    ip.protocol = 17;
    ip.saddr    = inet_addr("192.168.0.14");
    ip.daddr    = inet_addr("192.168.0.10");
    ip.check    =  csum((char *)&ip);

  
    //----------------------------------------------------
    header.source = htons(7777);
    header.dest = htons(9007);
    header.len = htons(sizeof(msg) + sizeof(struct udpHeader));
    header.check = 0;
    //----------------------------------------------------
    int fd = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(fd == -1)
        errorExit("soket");
    memcpy((void *)buff, (void* )&link, sizeof(link));
    memcpy((void *)(buff + sizeof(struct linkHeader)), (void* )&ip, sizeof(ip));
    memcpy((void *)(buff + sizeof(struct linkHeader) + sizeof(struct iphdr)), (void* )&header, sizeof(header));
    memcpy((void*)(buff + sizeof(struct linkHeader) + sizeof(ip) + sizeof(struct udpHeader)), (void*)msg, sizeof(msg));
   
    socklen_t size = sizeof(serv);
    if(sendto(fd, buff, (sizeof(struct linkHeader) + sizeof(struct iphdr)+sizeof(struct udpHeader) + sizeof(msg)), 0, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        errorExit("sendto");

    char out[100];
    while(1){
        if(recvfrom(fd, out, sizeof(out), 0, (struct sockaddr *)&serv, &size) == -1)
            errorExit("recvfrom");
        
        short *des = (short*)(out + 36);
        if(htons(*des) == 7777)
            printf("%s\n", out + 42);
    }

    exit(EXIT_SUCCESS);

}