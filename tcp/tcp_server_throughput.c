#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <inttypes.h>

#define REPEAT 100000
#define PORT 8766
#define SA struct sockaddr 

void func(int sockfd, int size) 
{ 
    char buff[size]; 
    memset(buff,43, size*sizeof(char)); 

    ssize_t read_bytes=0, write_bytes=0, remaining_bytes=size;
    
    int i=0;  
    for (;i<REPEAT; i++) {
	
	//Read properly        
	remaining_bytes = size;
	read_bytes=0;
	do{        
	    read_bytes = read(sockfd, (void *)(buff+size-remaining_bytes), remaining_bytes);
	    if(read_bytes == -1)
	    {
		printf("Error reading!!!\n");
		exit(0);
	    }
	    remaining_bytes -= read_bytes;	
	}while(remaining_bytes>0);
    }
    
    //Write properly
    remaining_bytes = 1;
    write_bytes=0;
    do{        
        write_bytes = write(sockfd, (void *)(buff+1-remaining_bytes), remaining_bytes);
	if(write_bytes == -1)
	{
	    printf("Error writing!!!\n");
	    exit(0);
	}
	remaining_bytes -= write_bytes;	
    }while(remaining_bytes>0); 
} 
  
int main() 
{ 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Error socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    memset(&servaddr, 0,sizeof(servaddr)); 
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
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

    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("Error socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    if ((listen(sockfd, 5)) != 0) { 
        printf("Error listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
  
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("Error server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server acccept the client...\n"); 
    int size; 
    for(size=4; size<=256*1024; size*=4)
    	func(connfd,size); 
  
    func(connfd, size/2);

    close(sockfd); 
} 

