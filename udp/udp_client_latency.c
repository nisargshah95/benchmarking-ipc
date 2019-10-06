#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#define REPEAT 100000
#define PORT 8768
#define SA struct sockaddr 


void func(int sockfd, struct sockaddr_in servaddr, int size) 
{ 
    int len = sizeof(servaddr);
    int buff[size / sizeof(int)]; 
    memset(buff,0, size);

    ssize_t read_bytes=0, write_bytes=0, remaining_bytes=size;

    uint64_t diff=0, min=0;
    unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
    
    int i=0;  
    for (;i<REPEAT; i++) {
        // printf("i = %d\n", i);
	    asm volatile ("CPUID\n\t"
	        "RDTSC\n\t"
	        "mov %%edx, %0\n\t"
	        "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
	        "%rax", "%rbx", "%rcx", "%rdx");

        int ack_received = 0;

        buff[0] = i;
        
        write_bytes = sendto(sockfd, (void *)buff, size, 0, (SA *)&servaddr, len);
        if (write_bytes == -1) {
            printf("Error sending data to server\n");
            exit(1);
        }
                
        buff[0] = -1;
        
        read_bytes = recvfrom(sockfd, (void *)buff, size, 0, (SA *)&servaddr, (socklen_t *)&len);
        // printf("buff[0]: %d read_bytes: %lu i: %d\n ", buff[0], read_bytes, i);
        if (buff[0] == i) {
            ack_received = 1;
        }
	    
        // printf("i = %d, buff[0] = %d\n", i, buff[0]);    

	    asm volatile (
	        "RDTSCP\n\t"
	        "mov %%edx, %0\n\t"
	        "mov %%eax, %1\n\t"
	        "CPUID\n\t":"=r" (cycles_high1), "=r" (cycles_low1)::
	        "%rax", "%rbx", "%rcx", "%rdx");
	    
	    diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );
	    
	    if(ack_received && (min>diff || min == 0))
	        min = diff;
    } 
    printf("Latency for message size %d bytes = %.3f us\n",size, min / (6.4 * 1000));
} 
  
int main(int argv, char *argc[])
{
    printf("\n\n============== UDP latency measurement ===========\n");
    if (argv != 2) {
        printf("Usage: %s <server IP>\n", argc[0]);
        exit(1);
    }
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    memset(&servaddr,0,sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(argc[1]);
    servaddr.sin_port = htons(PORT); 
    
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 100000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        printf("Error setting time out!!\n");
    }
      
    int size;
    for(size=4; size<=16*1024;size*=4){
        //sleep(10);
    	func(sockfd,servaddr,size);
    };
    
    func(sockfd,servaddr,65507);
    // close the socket 
    close(sockfd); 
} 
