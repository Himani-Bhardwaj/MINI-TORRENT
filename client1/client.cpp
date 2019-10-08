#include <unistd.h> 
#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>  
#include <arpa/inet.h>
#include<bits/stdc++.h>
#include <openssl/sha.h>
using namespace std;
struct sockaddr_in addressOfServer;
struct sockaddr_in addressOfClient;
struct fileChunks{
	string file_path;
	string calculatedSHA;
	vector<int> chunks;
};
vector<struct fileChunks> fChunks;
void sendfile(FILE *fs, int socket_fd) 
{
	char buffer[1024] = {0};
	long int fileSizeSending = 0;
	fseek ( fs , 0 , SEEK_END);
  	int size = ftell ( fs );
  	rewind ( fs );
	send ( socket_fd , &size, sizeof(size), 0);
	while((fileSizeSending = fread(buffer, sizeof(char), 1024, fs))>0 && size > 0)
	    	{
			if(send(socket_fd, buffer, fileSizeSending, 0) < 0)
	        	{
	        	    cout<<"File CAnnot Be send"<<endl;
			    break;
	        	}
	        	memset ( buffer , '\0', 1024);
			size = size - fileSizeSending ;
	    	}
}
void connectToServer(){
	int socket_fd,new_socket,valread;
	char buffer[1024] = {0}; 
	socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd == 0){
		cout<<"Error while creating socket"<<endl;
		exit(EXIT_FAILURE); 
	}
	addressOfServer.sin_family = AF_INET; 
    	addressOfServer.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    	addressOfServer.sin_port = htons( 8080 );
	if (connect(socket_fd, (struct sockaddr *)&addressOfServer, sizeof(addressOfServer)) < 0) 
    	{ 
    	    cout<<"Connection failed"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
	string fileName = "img.jpg";
		send(socket_fd,fileName.c_str(),fileName.size(),0);
		FILE *fs = fopen(fileName.c_str(), "rb");
		if(fs == NULL)
		{
		    cout<<"FILE NOT FOUND"<<endl;
		    exit(EXIT_FAILURE); 
		}
		sendfile(fs, socket_fd);
		cout<<"File successfully sent"<<endl;
	        fclose ( fs );
		close( socket_fd);
}
int connectToTracker(){
	int socket_fd,con,opt=1;
	socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd == 0){
		cout<<"Error while creating socket"<<endl;
		exit(EXIT_FAILURE); 
	}
	addressOfServer.sin_family = AF_INET; 
    	addressOfServer.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    	addressOfServer.sin_port = htons( 12000 );
	addressOfClient.sin_family = AF_INET;  
        addressOfClient.sin_addr.s_addr = inet_addr("127.0.0.1"); 
        addressOfClient.sin_port = htons(12001 );
	if(setsockopt(socket_fd, SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&opt,sizeof(opt))){
		cout<<"setsockopt"<<endl;
		exit(EXIT_FAILURE);
	} 
   	 if( bind(socket_fd,(struct sockaddr*)&addressOfClient,sizeof(addressOfClient)) < 0){
		cout<<"Cannot Bind Socket"<<endl;
		exit(EXIT_FAILURE);
	}
	if ((con=connect(socket_fd, (struct sockaddr *)&addressOfServer, sizeof(addressOfServer))) < 0) 
    	{ 
    	    cout<<"Connection failed"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
	else cout<<"Connectng to server........."<<endl;
	return socket_fd;
}
void displayMessage(int socket_fd){
	char message[1024]={0};
	recv(socket_fd, message, sizeof(message), 0);
	cout<<message<<endl;
}
void displayServersForFiles(int socket_fd){
	char message[1024]={0};
	recv(socket_fd, message, sizeof(message), 0);
	cout<<message<<endl;
}
void displayListOfFiles(int socket_fd){
	char message[1024]={0};
	int fileSizeReceiving=0;
	while((fileSizeReceiving = recv(socket_fd, message, sizeof(message), 0))>0 )
	        {
			if(fileSizeReceiving != 0)
			{
				//cout<<message<<endl;
				string recieved = message;
				stringstream s(recieved); 
  				string word;
				int count = 0;
	 			 while(getline(s, word, ';')) {
					switch(count){
						case 0:cout<<"SERIAL NUMBER : "<< word<<endl;break;
						case 1:cout<<"FILE NAME : "<< word<<endl;break;
						case 2:cout<<"SHA : "<< word<<endl;break;
						}
					count++;
				}
				 cout<<endl;
				 memset ( message , '\0', 1024);
		         }
			else break;
	        }
}
string getIPAddress(){ 
	string str = inet_ntoa(addressOfClient.sin_addr);
	string str1 = to_string(ntohs(addressOfClient.sin_port));	
	string recievedSocket = str+":"+str1;
	return recievedSocket;
}
void createUser(int socket_fd,string command,string username,string password){	
	string client = getIPAddress();
	string toBeSend = command+";"+username+";"+password+";"+client;
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	cout<<"signup details sent"<<endl;
	displayMessage(socket_fd);
}
void login(int socket_fd,string command,string username,string password){
	string toBeSend = command+";"+username+";"+password;	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	cout<<"login details sent"<<endl;
	displayMessage(socket_fd);
}
void createGroup(int socket_fd,string command,string group_id){
	string toBeSend = command+";"+group_id;	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
void joinGroup(int socket_fd,string command,string group_id){
	string toBeSend = command+";"+group_id;	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
string sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH])
{
   stringstream ss;
   for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}
string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha256_file(FILE *file,int file_size,int noOfChunks)
{
    if(!file) return NULL;
    string finalHash="";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = 512;
    unsigned char *buffer =(unsigned char*) malloc(bufSize+1);
    int bytesRead = 0;
    if(!buffer) return NULL;
	int i=0;
		while((bytesRead = fread(buffer, sizeof(char), bufSize, file))){
			SHA256_Update(&sha256, buffer, bytesRead);
			SHA256_Final(hash, &sha256);
	        	string outputBuffer = sha256_hash_string(hash);
			string finalAnswer = outputBuffer.substr(0, 20);
			finalHash += finalAnswer;
	        	memset ( buffer , '\0', 512);
		}
	
   	fclose(file);
    free(buffer);
    return finalHash;
}
void uploadFile(int socket_fd,string command,string file_path,string group_id){
	FILE *fs = fopen ( file_path.c_str()  , "rb" );
	long int fileSizeSending = 0,noOfChunks=0;
	fseek ( fs , 0 , SEEK_END);
  	int file_size = ftell ( fs );
  	rewind ( fs );
	if(file_size % 512 == 0) noOfChunks = file_size/512;
	else noOfChunks = (file_size/512)+1;
	string finalHash = sha256_file(fs,file_size,noOfChunks);
	string toBeSend	= command+";"+file_path+";"+finalHash+";"+group_id;
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
	fileChunks fc;
	fc.file_path = file_path;
	fc.calculatedSHA = finalHash;
	for(int i=0;i<noOfChunks;i++){
		fc.chunks.push_back(i+1);
	}
}
void listFiles(int socket_fd,string command,string group_id){
	string toBeSend	= command+";"+group_id;	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayListOfFiles(socket_fd);
}
void downloadFile(int socket_fd,string command,string file_path){
	cout<<"hi"<<endl;
	string toBeSend	= command+";"+file_path;	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayServersForFiles(socket_fd);
}
int main(int argc,char **argv){
	int socket_fd_for_tracker = connectToTracker();
	int valread;
	string group_id,command,username,password;
	displayMessage(socket_fd_for_tracker);
	while(1){
		cin>>command;
		if(command == "create_user"){
			cin>>username;
			cin>>password;		
			createUser(socket_fd_for_tracker,command,username,password);
		}
		if(command == "login"){
			cin>>username;
			cin>>password;	
			login(socket_fd_for_tracker,command,username,password);
		}
		if( command == "create_group" ){
			string group_id;
			cin>>group_id;
			createGroup(socket_fd_for_tracker,command,group_id);
		}
		else if(command == "join_group" ){
			string group_id;
			cin>>group_id;
			joinGroup(socket_fd_for_tracker,command,group_id);
		}
		else if(command == "upload_file" ){
			string file_path,group_id;
			cin>>file_path>>group_id;
			uploadFile(socket_fd_for_tracker,command,file_path,group_id);
		}
		else if(command == "list_files" ){
			string group_id;
			cin>>group_id;
			listFiles(socket_fd_for_tracker,command,group_id);
		}
		else if(command == "download_file" ){
			cout<<"download_file"<<endl;
			string file_path;
			cin>>file_path;
			downloadFile(socket_fd_for_tracker,command,file_path);
		}
	}
	close(socket_fd_for_tracker);
    return 0;  
}
