#include <unistd.h> 
#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>  
#include <arpa/inet.h>
#include <fstream>
using namespace std;
void clearBuf(char* b) 
{ 
    int i; 
    for (i = 0; i < 1024; i++) 
        b[i] = '\0'; 
}
void writefile(int new_socket, FILE *fs)
{
	ssize_t total;
    	char buffer[1024] = {0};
	long int fileSizeReceiving = 0;
	int file_size;
	recv(new_socket, &file_size, sizeof(file_size), 0);
		while((fileSizeReceiving = recv(new_socket, buffer, sizeof(1024), 0))>0 && file_size > 0)
	        {
		    if(fileSizeReceiving < 0)
	            {
	                cout<<"File is not received successfully"<<endl;
	            }
	            long int write_sz = fwrite(buffer, sizeof(char), fileSizeReceiving, fs);
	            if(write_sz < fileSizeReceiving)
	            {
	                cout<<"File write failed"<<endl;
	            }
		    memset ( buffer , '\0', 1024);
		    file_size = file_size - fileSizeReceiving;
	        }
}
int main(){
	int socket_fd,new_socket,opt=1,valread;
	char buffer[1024] = {0};
	struct sockaddr_in addressOfServer;
	socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd == 0){
		cout<<"Error while creating socket"<<endl;
		exit(EXIT_FAILURE); 
	}
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))) 
    	{ 
    	    perror("setsockopt"); 
    	    exit(EXIT_FAILURE); 
    	} 
	addressOfServer.sin_family = AF_INET; 
    	addressOfServer.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    	addressOfServer.sin_port = htons( 8080 );
	int addrlen = sizeof(addressOfServer);
	struct sockaddr_in addressOfClient;
	if( bind(socket_fd,(struct sockaddr*)&addressOfServer,sizeof(addressOfServer)) < 0){
		cout<<"Cannot Bind Socket"<<endl;
		exit(EXIT_FAILURE);
	}
	if (listen(socket_fd, 3) < 0) 
    	{ 
    	    cout<<"Cannot listen to socket"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
	
	
	new_socket = accept(socket_fd, (struct sockaddr *)&addressOfClient, (socklen_t*)&addrlen);
	if (new_socket < 0 )
    	{ 
        	cout<<"Cannot accept socket"<<endl; 
        	exit(EXIT_FAILURE); 
    	}
	string fileName = "test1.txt";
	valread = read(new_socket, buffer, 1024);
	FILE *fs = fopen(buffer, "wb");
	writefile(new_socket, fs);
	    cout<<"File successfully received"<<endl;
    	return 0;  
}
