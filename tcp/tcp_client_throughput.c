#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define REPEAT 100000
#define PORT 8766
#define SA struct sockaddr 

void func(int sockfd, int size) 
{ 
    char buff[size]; 
    memset(buff,43, size*sizeof(char));
    
    uint64_t diff=0;
    unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;

    ssize_t read_bytes=0, write_bytes=0, remaining_bytes=size;
    
    int i=0, j=0;

    asm volatile ("CPUID\n\t"
        "RDTSC\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%rax", "%rbx", "%rcx", "%rdx");

  
    for (;i<REPEAT; i++) { 

	//Write properly
        remaining_bytes = size;
	write_bytes=0;
  	do{        
	    write_bytes = write(sockfd, (void *)(buff+size-remaining_bytes), remaining_bytes);
	    if(write_bytes == -1)
	    {
		printf("Error writing!!!\n");
		exit(0);
	    }
	    remaining_bytes -= write_bytes;	
	}while(remaining_bytes>0);
    }
    
    //Read properly        
    remaining_bytes = 1;
    read_bytes=0;
    do{        
        read_bytes = read(sockfd, (void *)(buff+1-remaining_bytes), remaining_bytes);
	if(read_bytes == -1)
        {
	    printf("Error reading!!!\n");
	    exit(0);
        }
        remaining_bytes -= read_bytes;	
    }while(remaining_bytes>0);

    asm volatile (
        "RDTSCP\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "CPUID\n\t":"=r" (cycles_high1), "=r" (cycles_low1)::
        "%rax", "%rbx", "%rcx", "%rdx");

    diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );
	
    printf("Throughput for message size %d bytes = %f MB/s\n",size, (uint64_t)size*REPEAT/(diff/3200.0));
} 
  
int main(int argv, char *argc[]) 
{ 
    //Create socket and connect to server
    printf("\n\n===================== TCP throughput measurement =======================\n");

    if(argv!=2)
    {
	printf("Error no server spesified\n"); 
        exit(0);
    }

    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Error socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    memset(&servaddr,0,sizeof(servaddr)); 
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(argc[1]); 
    servaddr.sin_port = htons(PORT);

    int i = 1;
    if(setsockopt(sockfd, IPPROTO_IP, TCP_NODELAY, (void*)&i, sizeof(i))){
        printf("Error sock option cannot be set in IP level...\n");
        exit(0);
    }
    
    if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void*)&i, sizeof(i))){
        printf("Error sock option cannot be set in TCP level...\n");
        exit(0);
    }

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("Error connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("Connected to the server..\n"); 
  
    int size;
    for(size=4; size<=256*1024;size*=4)
    	func(sockfd,size);

    func(sockfd, size/2);

    close(sockfd); 
} 
