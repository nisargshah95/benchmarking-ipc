#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>

#include <inttypes.h>

#define REPEAT 100000
#define PORT 8769
#define SA struct sockaddr 


// Function designed for chat between client and server. 
void func(int sockfd, int size) 
{ 
    struct sockaddr_in cli;
    int len = sizeof(cli);

    int buff[size / sizeof(int)], packets_recvd=0;
    memset(buff, 0, size);
    
    ssize_t read_bytes=0, write_bytes=0;
    
    while(1) {
        // printf("Server waiting to receive packet from client\n");
        read_bytes = recvfrom(sockfd, (void *)buff, size, 0, (SA *)&cli, &len);
        if (read_bytes == -1) {
            printf("Error reading data from client\n");
            exit(1);
        }
	//printf("%ld\n", read_bytes);
        
        if (read_bytes == size) {
             packets_recvd++;
        }
        // printf("size = %d buf[0] = %d, read_bytes = %lu\n", size, buff[0], read_bytes);
        if (buff[0] == REPEAT-1 && read_bytes == size) {
            printf("server received last packet for size %d\n", size);
            break;
        }
    }
    // send lots of acks of size 1
    // printf("sending ack to client\n");
    int i = 0;
    buff[0] = size;
    buff[1] = packets_recvd;
    for (i = 0; i < 2; i++) {
        write_bytes = sendto(sockfd, (void *)buff, 8, 0, (SA *)&cli, len);
        if (write_bytes == -1) {
            printf("Error sending data to client\n");
            exit(1);
        }
    }
    // printf("sent ack to client\n");
} 
  
// Driver function
int main() 
{ 
    int sockfd; 
    struct sockaddr_in servaddr;
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    memset(&servaddr, 0,sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
 
    /*int i = 1024*1024;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*)&i, sizeof(i))){
        printf("Error sock option cannot be set in IP level...\n");
        exit(0);
    }


    size_t len;
    socklen_t j = sizeof(len);
    printf("%d\n",getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,&len,&j)); 
    
    printf("%ld\n", len);*/
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
 
    int size; 
    for(size=4; size<=16*1024; size*=4)
    {	//sleep(10);
    	func(sockfd, size); 
    }
    
    func(sockfd, 65507);
    // After chatting close the socket 
    close(sockfd); 
} 

