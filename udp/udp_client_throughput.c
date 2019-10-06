#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define REPEAT 100000
#define PORT 8769
#define SA struct sockaddr 


void func(int sockfd, struct sockaddr_in servaddr, int size) 
{ 
    int len = sizeof(servaddr);
    int buff[size / sizeof(int)], numOfPackets = 0; 
    memset((void*)buff,0, size);

    ssize_t read_bytes=0, write_bytes=0, remaining_bytes=size;

    uint64_t diff=0, min=0;
    unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
    
    // ========= start time
    asm volatile ("CPUID\n\t"
            "RDTSC\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
            "%rax", "%rbx", "%rcx", "%rdx");

    int i=0;  
    for (; i < REPEAT; i++) {
        //printf("i = %d\n", i);

        buff[0] = i;
        
        write_bytes = sendto(sockfd, (void *)buff, size, 0, (SA *)&servaddr, len);
        if (write_bytes == -1) {
            printf("Error sending data to server\n");
            exit(1);
        }
	//printf("%ld\n",write_bytes);
    /*struct timespec tv,ts;
    tv.tv_sec = 0;
    tv.tv_nsec = 1;
	nanosleep(&tv,&ts);*/
        // printf("i = %d, buff[0] = %d\n", i, buff[0]);
    }
    // send the last packet lots of times
    // printf("sending last packet...\n");
    //buff[0] = 0;
   /* for (i = 0; i < 2; i++) {
        write_bytes = sendto(sockfd, (void *)buff, size, 0, (SA *)&servaddr, len);
        if (write_bytes == -1) {
            printf("Error sending data to server\n");
            exit(1);
        }
        // printf("sent buf[0] = %d, size = %d, write_bytes = %lu\n", buff[0], size, write_bytes);
    }*/
    // printf("sent last packet...\n");
    int ack_received = 0;
 
    while(!ack_received) {
	buff[0] = REPEAT-1;
	for (i = 0; i < 2; i++) {
            write_bytes = sendto(sockfd, (void *)buff, size, 0, (SA *)&servaddr, len);
            if (write_bytes == -1) {
                printf("Error sending data to server\n");
                exit(1);
            }
        // printf("sent buf[0] = %d, size = %d, write_bytes = %lu\n", buff[0], size, write_bytes);
        }

        buff[0] = -1;

        read_bytes = recvfrom(sockfd, (void *)buff, 8, 0, (SA *)&servaddr, (socklen_t *)&len);
        if (read_bytes == -1) {
            continue;
	    printf("Error reading data from server\n");
            exit(1);
        }
        // printf("buff[0]: %d read_bytes: %lu i: %d\n ", buff[0], read_bytes, i);
        if (buff[0] == size) {
            ack_received = 1;
	    numOfPackets = buff[1];
            // printf("packets received = %d\n", buff[1]);
        }
    }
    // printf("received ack from server\n");

    // =========== end time
    asm volatile (
            "RDTSCP\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            "CPUID\n\t":"=r" (cycles_high1), "=r" (cycles_low1)::
            "%rax", "%rbx", "%rcx", "%rdx");

    diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );
    printf("Throughput for message size %d bytes = %.3Lf MBps\n", size, (long double) size * (uint64_t)numOfPackets * 3.2 * 1000 / diff);
}
  
int main(int argv, char *argc[])
{
    printf("\n\n============== UDP throughput measurement ===========\n");
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
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        printf("Error setting time out!!\n");
    }


    /*int i = 1024*1024;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*)&i, sizeof(i))){
        printf("Error sock option cannot be set in IP level...\n");
        exit(0);
    }

    size_t len;
    socklen_t j = sizeof(len);
    printf("%d\n",getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,&len,&j)); 
    
    printf("%ld\n", len);
    */
    int size;
    for(size=4; size<=16*1024; size*=4){
    	func(sockfd, servaddr, size);
	//sleep(2);
    };
    
    func(sockfd,servaddr,65507);
    // close the socket 
    close(sockfd); 
} 
