#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>

#include <inttypes.h>

#define REPEAT 100000
#define PORT 8768
#define SA struct sockaddr 
  
// Function designed for chat between client and server. 
void func(int sockfd, int size) 
{ 
    struct sockaddr_in cli;
    int len = sizeof(cli);

    int buff[size / sizeof(int)];
    memset(buff,0, size);
    
    ssize_t read_bytes=0, write_bytes=0;
    
    while(1) {     

            //printf("Server waiting to receive packet from client\n");
	        read_bytes = recvfrom(sockfd, (void *)buff, size, 0, (SA *)&cli, &len);
	        if (read_bytes == -1) {
	            printf("Error reading data from client\n");
	            exit(0);
	        }   
	        

	        write_bytes = sendto(sockfd, (void *)buff, size, 0, (SA *)&cli, len);
	        if (write_bytes == -1) {
	            printf("Error reading data from client\n");
	            exit(0);
	        }
	        
	        // printf("buf[0] = %d\n", buff[0]);
            if (buff[0] == REPEAT-1) {
                printf("server received last packet for size %d\n", size);
                break;
            }
    }  
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
    	func(sockfd,size); 
    }
    
    func(sockfd,65507); 
    // After chatting close the socket 
    close(sockfd); 
} 

