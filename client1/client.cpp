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
#include<pthread.h>
#include <fstream>
#include <string>
#include <deque> 
#define STRING_SIZE 11
#define SEND_SIZE 512
#define CHUNK_SIZE 512
using namespace std;
struct sockaddr_in addressOfServer;
struct sockaddr_in addressOfClient;
struct fileChunks{
	string file_path;
	int fileSize;
	int noOfChunks;
	string calculatedSHA;
	vector<int> chunks;
};
struct downloadFileDetials{
	string file_path;
	string hashValue;
	string client;
	int fileSize;
	int noOfChunks;
	vector<pair<string,vector<int>>> chunks;
};
vector<struct downloadFileDetials> dfDetails;
vector<struct fileChunks> fChunks;
struct thread_data {
   	string ipaddress;
   	string port;
	string file_path;
};
struct thread_data_for_file {
   	string ipaddress;
   	string port;
	string file_path;
	int chunk;
};
struct recieved_file_data {
	int file_size;
	int noOfChunks;
	string file_path ;
};
struct recieved_file_data rd;
void displayMessage(int socket_fd){
	char message[SEND_SIZE]={0};
	recv(socket_fd, message, sizeof(message), 0);
	cout<<message<<endl;
}
string getHashValue(int socket_fd,int size){
	int n=0;
	char hash1[SEND_SIZE]={0};
	string newString ="";
	while(size > 0 && (n = recv(socket_fd, hash1, sizeof(SEND_SIZE), 0))>0 )
	        	{
		    		string s = hash1;
				newString += s;
		    		memset ( hash1 , '\0', SEND_SIZE);
		    		size = size - n;
	        	}
return newString;
}
bool checkExistanceOfFile(downloadFileDetials df){
	if(dfDetails.size() == 0) return true;
	for(int i=0;i< dfDetails.size();i++){
		if(dfDetails[i].hashValue == df.hashValue) return false;
	}
	return true;
}
void displayListOfFiles(int socket_fd){
	char message[SEND_SIZE]={0};
	string recieved,word;
	int fileSizeReceiving=0;
	vector<string> availableFilesAtGroup;
	int size = 0;
	recv(socket_fd, &size, sizeof(size), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);
	recieved = getHashValue(socket_fd,size);
	stringstream s(recieved); 
	downloadFileDetials df;
	while(getline(s, word, ';')) availableFilesAtGroup.push_back(word);
	for(int i=0;i< 	availableFilesAtGroup.size();i++){
		recieved = availableFilesAtGroup[i];
		stringstream ss(recieved);
		int count = 0;
		while(getline(ss, word, ':')){
			count++;
			if(count ==1 ) ;
			else if(count == 2) {
				df.file_path = word;
				cout<<"FILE NAME : "<< word<<endl;
			}
			else {
				df.hashValue = word;
				cout<<"HASH VALUE : "<< word<<endl;
			}
		}
		if(checkExistanceOfFile(df)) dfDetails.push_back(df);
	}
}
string getIPAddress(){ 
	string str = inet_ntoa(addressOfClient.sin_addr);
	string str1 = to_string(ntohs(addressOfClient.sin_port));	
	string recievedSocket = str+":"+str1;
	return recievedSocket;
}
int connectToServer(string ipaddress,string port){
	int socket_fd,new_socket,valread;
	char buffer[SEND_SIZE] = {0}; 
	socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd == 0){
		cout<<"Error while creating socket"<<endl;
		exit(EXIT_FAILURE); 
	}
	addressOfServer.sin_family = AF_INET; 
    	addressOfServer.sin_addr.s_addr = inet_addr(ipaddress.c_str()); 
    	addressOfServer.sin_port = htons( stoi(port) );
	if (connect(socket_fd, (struct sockaddr *)&addressOfServer, sizeof(addressOfServer)) < 0) 
    	{ 
	    cout<<"Connection failed"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
	return socket_fd;
}
void sendHashValue(int socket_fd,string finalHash){
	int fileSizeSending=0;
	char buffer[SEND_SIZE];
	ofstream out("hash.txt");
	out << finalHash;
    	out.close();
	FILE *fs = fopen ( "hash.txt"  , "rb" );
	fseek ( fs , 0 , SEEK_END);
  	int size = ftell ( fs );
  	rewind ( fs );
	while(size > 0 && (fileSizeSending = fread(buffer, sizeof(char), SEND_SIZE, fs))>0)
	    	{
			if(send(socket_fd, buffer, fileSizeSending, 0) < 0)
	        	{
	        	    cout<<"File CAnnot Be send"<<endl;
			    break;
	        	}
	        	memset ( buffer , '\0', SEND_SIZE);
			size = size - fileSizeSending ;
	    	}
}
string getHash(string file_path){
	string hashValue="";
	for(int i=0;i < dfDetails.size();i++){
		if(dfDetails[i].file_path == file_path) hashValue = dfDetails[i].hashValue;
	}
	return hashValue;
}
void getFile(int socket_fd,int chunk,string file_path,int noOfChunks,int file_size){
	FILE* fp ;
	fp = fopen(file_path.c_str(), "r+");
	int bytes_recv ;
		int ack = chunk;
		int sizeToBeRecieved = CHUNK_SIZE;
		if(ack == noOfChunks) sizeToBeRecieved = file_size-(CHUNK_SIZE*(noOfChunks-1));
		else sizeToBeRecieved = CHUNK_SIZE;
		fseek(fp, CHUNK_SIZE*(ack-1), SEEK_SET) ;
		send(socket_fd, &ack, sizeof ack, 0) ;
		char buf[SEND_SIZE] ;
		while( sizeToBeRecieved > 0 && (bytes_recv = recv(socket_fd, buf, sizeof buf, 0)) > 0)
		{
			fwrite(buf, sizeof(char), bytes_recv, fp);
			sizeToBeRecieved -= bytes_recv ;
		}
	fclose(fp);
	close(socket_fd);
	pthread_exit(NULL);
}
void *getFileFunction(void *threadarg){
	struct thread_data_for_file *my_data;
   	my_data = (struct thread_data_for_file *) threadarg;

	int file_size =0;
	int socket_fd = connectToServer(my_data->ipaddress,my_data->port);
	string toBeSend = "Send File Size";
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	char m[STRING_SIZE] = {0};
	recv(socket_fd, m, sizeof(m), 0);


	int noOfChunks;	
	for(int i=0;i<dfDetails.size();i++){
		if( my_data->file_path == dfDetails[i].file_path){
			noOfChunks = dfDetails[i].noOfChunks;
			file_size = dfDetails[i].fileSize;
		}
	}
	getFile(socket_fd,my_data->chunk,my_data->file_path,noOfChunks,file_size);
}
void connectToGetChunks(string file_path){
	int noOfChunks,noOfSockets;
	deque <string> socketArray;
	vector<pair<string,vector<int>>> chunks;	
	for(int i=0;i<dfDetails.size();i++){
		if( file_path == dfDetails[i].file_path){
			noOfChunks = dfDetails[i].noOfChunks;
			noOfSockets = dfDetails[i].chunks.size();
			chunks = dfDetails[i].chunks;
			for(int j=0;j< noOfSockets;j++){
				socketArray.push_back(dfDetails[i].chunks[j].first);
			}
		}
	}
	int i=1;
	thread_data_for_file td;
	td.file_path = file_path;
	while(i <= noOfChunks){
		bool flag = false;
		string recieved;
		string socket = socketArray.front();
		socketArray.push_back(socket);
		socketArray.pop_front();
		for(int j=0;j<chunks.size();j++){
			if( socket == chunks[j].first){
				for(int k=0;k< chunks[j].second.size();k++){
					if(i == chunks[j].second[k]){
						flag = true;
						td.chunk = i;
						recieved = socket;
						stringstream ss(recieved);
						string word;
						int count =0; 
  						while(getline(ss, word, ':')){
							count++;
							if(count ==1 ) td.ipaddress = word;
							else td.port= word;
						}
						pthread_t getFileThread;
						i++;
						pthread_create(&getFileThread,NULL,getFileFunction,(void *)&td);
						pthread_join(getFileThread,NULL);
						break;
					}
				}
			}
		}
	}
}
void getChunksForSocket(int socket_fd,string fileChunks,string file_path,string socket,int noOfChunks,int file_size){
	stringstream ss(fileChunks);
	vector<int> chunksOfFileToBeDownloaded;
	int count = 0;
	string word;
	while(getline(ss, word, ';')){
		count++;
		chunksOfFileToBeDownloaded.push_back(stoi(word));
	}
	for(int i=0;i<dfDetails.size();i++){
		if( file_path == dfDetails[i].file_path){
			dfDetails[i].chunks.push_back(make_pair(socket,chunksOfFileToBeDownloaded));
			dfDetails[i].noOfChunks=noOfChunks;
			dfDetails[i].fileSize=file_size;
		}
	}
	close(socket_fd);
	pthread_exit(NULL);

}
void downloadFinalFile(int socket_fd,string file_path,string socket){
	
	string hashValue = getHash(file_path);
	int size = hashValue.size();
	send(socket_fd,&size,sizeof(size),0);
	char m[STRING_SIZE] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	sendHashValue(socket_fd,hashValue);
	size = 0;
	recv(socket_fd, &size, sizeof(size), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);

	int noOfChunks = 0;
	recv(socket_fd, &noOfChunks, sizeof(noOfChunks), 0);
	str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);

	int file_size;
	recv(socket_fd, &file_size, sizeof(file_size), 0);
	FILE* fp ;
	fp = fopen(file_path.c_str(), "wb");
	char ch ='0' ;
	for(int i=0;i<file_size;i++){
		fwrite(&ch, sizeof(ch), 1, fp);
	}
	fclose(fp);
	str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);

	string recieved = getHashValue(socket_fd,size);
	getChunksForSocket(socket_fd,recieved,file_path,socket,noOfChunks,file_size);
}
void *downloadFunction(void *threadarg){
	struct thread_data *my_data;
   	my_data = (struct thread_data *) threadarg;
	int socket_fd = connectToServer(my_data->ipaddress,my_data->port);
	string str = "Send File Details";
	send(socket_fd,str.c_str(),str.size(),0);
	char message[SEND_SIZE]={0};
	recv(socket_fd, message, sizeof(message), 0);
	string socket = my_data->ipaddress +":"+ my_data->port;
	downloadFinalFile(socket_fd,my_data->file_path,socket);
}
void displayServersForFiles(int socket_fd,string file_path){
	char message[SEND_SIZE]={0};	
	vector<string> availableSockets;
	recv(socket_fd, message, sizeof(message), 0);
	string recieved = message;
	stringstream s(recieved); 
  	string word;
	string ipaddress,port;
	int count = 0,countOnThreads=0;
	thread_data td;
	td.file_path = file_path;
	while(getline(s, word, ';')) availableSockets.push_back(word);
	for(int i=0;i< 	availableSockets.size();i++){
		recieved = availableSockets[i];
		stringstream ss(recieved); 
  		while(getline(ss, word, ':')){
			count++;
			if(count ==1 ) td.ipaddress = word;
			else td.port= word;
		}
		pthread_t downloadThread;
		countOnThreads++;
		pthread_create(&downloadThread,NULL,downloadFunction,(void *)&td);
		pthread_join(downloadThread,NULL);
		countOnThreads--;
	}
	if(countOnThreads == 0) connectToGetChunks(file_path);
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
void createUser(int socket_fd,string command,string username,string password){	
	string client = getIPAddress();
	string toBeSend = command+";"+username+";"+password+";"+client+";";
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
void login(int socket_fd,string command,string username,string password){
	string toBeSend = command+";"+username+";"+password+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
void createGroup(int socket_fd,string command,string group_id){
	string toBeSend = command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
void joinGroup(int socket_fd,string command,string group_id){
	string toBeSend = command+";"+group_id+";";	
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
    const int bufSize = SEND_SIZE;
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
	        	memset ( buffer , '\0', SEND_SIZE);
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
	if(file_size % CHUNK_SIZE == 0) noOfChunks = file_size/CHUNK_SIZE;
	else noOfChunks = (file_size/CHUNK_SIZE)+1;
	string finalHash = sha256_file(fs,file_size,noOfChunks);
	int j=0;
	int size = finalHash.size();
	string toBeSend	= command+";"+file_path+";"+group_id+";"+to_string(size)+";";
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	char m[STRING_SIZE]={0};
	recv(socket_fd, m, sizeof(m), 0);
	sendHashValue(socket_fd,finalHash);
	displayMessage(socket_fd);
	fileChunks fc;
	fc.file_path = file_path;
	fc.fileSize = file_size;
	fc.calculatedSHA = finalHash;
	fc.noOfChunks = noOfChunks;
	for(int i=0;i<noOfChunks;i++){
		fc.chunks.push_back(i+1);
	}
	fChunks.push_back(fc);
}
void listFiles(int socket_fd,string command,string group_id){
	string toBeSend	= command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayListOfFiles(socket_fd);
}

void downloadFile(int socket_fd,string command,string file_path){
	string hashValue = getHash(file_path);
	int size = hashValue.size();
	string toBeSend	= command+";"+to_string(size)+";";
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	char m[STRING_SIZE] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	sendHashValue(socket_fd,hashValue);
	displayServersForFiles(socket_fd,file_path);
}
int startServer(){
	int socket_fd,opt=1;
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
    	addressOfServer.sin_port = htons( 12001 );
	if( bind(socket_fd,(struct sockaddr*)&addressOfServer,sizeof(addressOfServer)) < 0){
		cout<<"Cannot Bind Socket"<<endl;
		exit(EXIT_FAILURE);
	}
	if (listen(socket_fd, 10000) < 0) 
    	{ 
    	    cout<<"Cannot listen to socket"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
return socket_fd;
}
void sendFile(int new_socket,char m[SEND_SIZE]){
	string file_path=rd.file_path;	
	int noOfChunks = rd.noOfChunks;
	int file_size = rd.file_size;
	if(strcmp(m,"Send File Size") == 0){
		string str = "got string";
		send(new_socket,str.c_str(),str.size(),0);	
	}
	int bytes_read,ack ;
	FILE* fp ;
	fp = fopen(file_path.c_str(), "rb");
		recv(new_socket, &ack, sizeof ack, 0) ;
		printf("Client asked for : %d\n", ack) ;
		char buff[CHUNK_SIZE] ;
		int bytesToBeSend = CHUNK_SIZE;
		if(ack == noOfChunks ) bytesToBeSend = file_size - (CHUNK_SIZE * (noOfChunks-1));
		else bytesToBeSend = CHUNK_SIZE;
		fseek(fp, CHUNK_SIZE*(ack-1), SEEK_SET);
		while( bytesToBeSend > 0 && (bytes_read = fread(buff, sizeof(char), sizeof buff, fp)) > 0 )
		{
			send(new_socket, buff, bytes_read, 0);
			bytesToBeSend -= bytes_read ;
		}
	fclose(fp) ;
}
void sendingFileDetails(int new_socket){
	string toBeSend="Please send your filename";
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
	int size = 0,file_size =0,noOfChunks=0;
	string file_path;
	recv(new_socket, &size, sizeof(size), 0);
	string str = "got string";
	send(new_socket,str.c_str(),str.size(),0);
	string recieved = getHashValue(new_socket,size);
	string newString ="";
	for(int i=0;i< fChunks.size();i++){
		if(fChunks[i].calculatedSHA == recieved){
			for(int j=0;j< fChunks[i].chunks.size();j++){
				newString += to_string(fChunks[i].chunks[j])+";";	
			}
			rd.file_size = fChunks[i].fileSize;
			rd.noOfChunks = fChunks[i].noOfChunks;
			rd.file_path = fChunks[i].file_path;
		}
	}
	size = newString.size();
	noOfChunks= rd.noOfChunks;
	send(new_socket, &size, sizeof(size), 0);
	char m[STRING_SIZE] = {0};
	recv(new_socket, m, sizeof(m), 0);
	send(new_socket, &noOfChunks, sizeof(noOfChunks), 0);
	char n[STRING_SIZE] = {0};
	recv(new_socket, n, sizeof(n), 0);

	file_size = rd.file_size;
	send(new_socket, &file_size, sizeof(file_size), 0);
	char p[STRING_SIZE] = {0};
	recv(new_socket, p, sizeof(p), 0);

	sendHashValue(new_socket,newString);
	close(new_socket);
}
void *connectForSendingFileFunc(void *threadarg){
	int new_socket = *((int *)threadarg);
	char m[SEND_SIZE] = {0};
	int val = recv(new_socket, m, sizeof(m), 0);
	if(strcmp(m,"Send File Details") == 0) sendingFileDetails(new_socket);
	else sendFile(new_socket,m);
}
void *serverFunction(void *threadarg){
	int socket_fd = startServer(),new_socket;	
	struct sockaddr_in addressOfClient;
	int addrlen = sizeof(addressOfClient);
	pthread_t connectForSendingFile;
	while(1)
    	{
        	new_socket = accept(socket_fd, (struct sockaddr *)&addressOfClient, (socklen_t*)&addrlen);
		if (new_socket < 0 )
    		{ 
        		cout<<"Cannot accept socket"<<endl; 
        		exit(EXIT_FAILURE); 
    		} 
		if( pthread_create(&connectForSendingFile, NULL, connectForSendingFileFunc, &new_socket ))
           	printf("Failed to create thread\n");		
		pthread_join(connectForSendingFile,NULL);
          }
}
void *clientFunction(void *threadarg){
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
			string file_path;
			cin>>file_path;
			downloadFile(socket_fd_for_tracker,command,file_path);
		}
	}
	close(socket_fd_for_tracker);
}
int main(int argc,char **argv){
	pthread_t clientThread,serverThread;
	pthread_create(&clientThread,NULL,clientFunction,NULL);		
	pthread_create(&serverThread,NULL,serverFunction,NULL);
	pthread_join(clientThread,NULL);
	pthread_join(serverThread,NULL);
	
    return 0;  
}
