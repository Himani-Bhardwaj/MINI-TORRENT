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
#define MAX_SOCKETS 3
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
	string destinationFilePath;
};
struct thread_data_for_file {
   	string ipaddress;
   	string port;
	string file_path;
	int chunk;
	string destinationFilePath;
};
struct recieved_file_data {
	int file_size;
	int noOfChunks;
	string file_path ;
};
struct recieved_file_data rd;
vector<string> availableRequests;
//display successful message
void displayMessage(int socket_fd){
	char message[SEND_SIZE]={0};
	recv(socket_fd, message, sizeof(message), 0);
	cout<<message<<endl;
}
//get larger string in chunks
string getLargeString(int socket_fd,int size){
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
//sending larger string is chunks
void sendLargerString(int socket_fd,string finalHash){
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
//getting ip address for a socket
string getIPAddress(){ 
	string str = inet_ntoa(addressOfClient.sin_addr);
	string str1 = to_string(ntohs(addressOfClient.sin_port));	
	string recievedSocket = str+":"+str1;
	return recievedSocket;
}
//check if hash value is already present in order to avoid duplicates
bool checkExistanceOfFile(downloadFileDetials df){
	if(dfDetails.size() == 0) return true;
	for(int i=0;i< dfDetails.size();i++){
		if(dfDetails[i].hashValue == df.hashValue) return false;
	}
	return true;
}
//mapping hash with filename
string getHash(string file_path){
	string hashValue="";
	for(int i=0;i < dfDetails.size();i++){
		if(dfDetails[i].file_path == file_path) hashValue = dfDetails[i].hashValue;
	}
	return hashValue;
}
//sending actual file
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
		char buff[SEND_SIZE] ;
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
//sending file details to client
void sendingFileDetails(int new_socket){
	string toBeSend="Please send your filename";
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
	int size = 0,file_size =0,noOfChunks=0;
	string file_path;
	recv(new_socket, &size, sizeof(size), 0);
	string str = "got string";
	send(new_socket,str.c_str(),str.size(),0);
	string recieved = getLargeString(new_socket,size);
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
	sendLargerString(new_socket,newString);
	close(new_socket);
}
//2nd level thread for sending file and file details
void *connectForSendingFileFunc(void *threadarg){
	int new_socket = *((int *)threadarg);
	char m[SEND_SIZE] = {0};
	int val = recv(new_socket, m, sizeof(m), 0);
	if(strcmp(m,"Send File Details") == 0) sendingFileDetails(new_socket);
	else sendFile(new_socket,m);
}
//starting server
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
	if (listen(socket_fd, MAX_SOCKETS) < 0) 
    	{ 
    	    cout<<"Cannot listen to socket"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
return socket_fd;
}
//inside server thread
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
//getting connected to individual server
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
//get actual file
void getFile(int socket_fd,int chunk,string file_path,int noOfChunks,int file_size,string destinationFilePath){
	FILE* fp ;
	fp = fopen(destinationFilePath.c_str(), "r+");
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
//2nd level thread for final file downloading
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
	getFile(socket_fd,my_data->chunk,my_data->file_path,noOfChunks,file_size,my_data->destinationFilePath);
}
//searching algorithm when all chunks from sockets are recieved
void searchingAlgorithm(string file_path,string destinationFilePath){
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
	td.destinationFilePath= destinationFilePath;
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
//getting all chunks from server and closing download thread
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
//getting file parameters like noOfChunks,file size
void getFileData(int socket_fd,string file_path,string socket,string destinationFilePath){
	
	string hashValue = getHash(file_path);
	int size = hashValue.size();
	send(socket_fd,&size,sizeof(size),0);
	char m[STRING_SIZE] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	sendLargerString(socket_fd,hashValue);
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
	fp = fopen(destinationFilePath.c_str(), "wb");
	char ch ='0' ;
	for(int i=0;i<file_size;i++){
		fwrite(&ch, sizeof(ch), 1, fp);
	}
	fclose(fp);
	str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);

	string recieved = getLargeString(socket_fd,size);
	getChunksForSocket(socket_fd,recieved,file_path,socket,noOfChunks,file_size);
}
//2nd level thread to downloadFile
void *downloadFunction(void *threadarg){
	struct thread_data *my_data;
   	my_data = (struct thread_data *) threadarg;
	int socket_fd = connectToServer(my_data->ipaddress,my_data->port);
	string str = "Send File Details";
	send(socket_fd,str.c_str(),str.size(),0);
	char message[SEND_SIZE]={0};
	recv(socket_fd, message, sizeof(message), 0);
	string socket = my_data->ipaddress +":"+ my_data->port;
	getFileData(socket_fd,my_data->file_path,socket,my_data->destinationFilePath);
}

// display serverList
void displayServersForFiles(int socket_fd,string file_path,string destinationFilePath){
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
	td.destinationFilePath =destinationFilePath;
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
	if(countOnThreads == 0) searchingAlgorithm(file_path,destinationFilePath);
}
//request to download a file
void downloadFile(int socket_fd,string command,string file_path,string group_id,string destinationFilePath){
	string hashValue = getHash(file_path);
	int size = hashValue.size();
	string toBeSend	= command+";"+to_string(size)+";"+group_id;
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	char m[STRING_SIZE] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	sendLargerString(socket_fd,hashValue);
	displayServersForFiles(socket_fd,file_path,destinationFilePath);
}
// display file contents
void displayListOfFiles(int socket_fd){
	char message[SEND_SIZE]={0};
	string recieved,word;
	int fileSizeReceiving=0;
	vector<string> availableFilesAtGroup;
	int size = 0;
	recv(socket_fd, &size, sizeof(size), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);
	recieved = getLargeString(socket_fd,size);
	if( recieved == "") 
		cout<<"No such file is present in the system"<<endl;
	else{
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
}
// listing all files at tracker for a group
void listFiles(int socket_fd,string command,string group_id){
	dfDetails.clear();
	string toBeSend	= command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayListOfFiles(socket_fd);
}
//calculate SHA for a file
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
//uploading file at tracker
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
	sendLargerString(socket_fd,finalHash);
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
//stop file sharing
void stopSharing(int socket_fd,string command,string file_path,string group_id){
	string hashValue = getHash(file_path);
	int size = hashValue.size();
	string toBeSend	= command+";"+group_id+";"+to_string(size);
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	char m[STRING_SIZE]={0};
	recv(socket_fd, m, sizeof(m), 0);
	sendLargerString(socket_fd,hashValue);
	displayMessage(socket_fd);
}
//displaying list at tracker
void displayGroupsPresentAtTracker(int socket_fd){
	char message[SEND_SIZE]={0};
	recv(socket_fd, message, sizeof(message), 0);
	string groups = message;
	stringstream s(groups);
	string word;
	while(getline(s, word, ';')) cout<<word<<endl;
}
//list groups
void listGroups(int socket_fd,string command){
	string toBeSend	= command+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayGroupsPresentAtTracker(socket_fd);
}
//logout from system
void logout(int socket_fd,string command){
	string toBeSend	= command+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
// leave group request
void leaveGroup(int socket_fd,string command,string group_id){
	string toBeSend	= command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
//request accpeted by owner
void acceptRequest(int socket_fd_for_tracker,string command,string group_id,string client){
	string toBeSend	= command+";"+group_id+";"+client+";";
	for(int i=0;i< 	availableRequests.size();i++){
		if(availableRequests[i] == client) {
			availableRequests.erase(availableRequests.begin()+i);
			}
		}
	send(socket_fd_for_tracker,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd_for_tracker);
}
// displaying and storing list of all pending requests
void displayListOfPendingRequests(int socket_fd){
	char message[SEND_SIZE]={0};
	string recieved,word;
	int fileSizeReceiving=0;
	recv(socket_fd, message, sizeof(message), 0);
	recieved=message;
	if(recieved == "")
		cout<<"There are no pending requests"<<endl;
	else{
		stringstream s(recieved); 
		downloadFileDetials df;
		while(getline(s, word, ';')) {
			if(availableRequests.size() == 0) availableRequests.push_back(word);
			else{
				bool flag = false;
				for(int i=0;i< 	availableRequests.size();i++){
					if(availableRequests[i] == word) {
						flag=true;break;
					}
				}
				if(flag == false) availableRequests.push_back(word);
			}
		}
		for(int i=0;i< 	availableRequests.size();i++)
			cout<< availableRequests[i]<<endl;
	}
}
//list pending requests for a client
void listRequest(int socket_fd,string command,string group_id){
	string toBeSend	= command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayListOfPendingRequests(socket_fd);
}
//sending request to join a group
void joinGroup(int socket_fd,string command,string group_id){
	string toBeSend = command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
//creating group for user
void createGroup(int socket_fd,string command,string group_id){
	string toBeSend = command+";"+group_id+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
// for login user at client
void login(int socket_fd,string command,string username,string password){
	string toBeSend = command+";"+username+";"+password+";";	
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
//this is for creating user at tracker
void createUser(int socket_fd,string command,string username,string password){	
	string client = getIPAddress();
	string toBeSend = command+";"+username+";"+password+";"+client+";";
	send(socket_fd,toBeSend.c_str(),toBeSend.size(),0);
	displayMessage(socket_fd);
}
//connecting to tracker
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
//this is for starting client
void *clientFunction(void *threadarg){
	int socket_fd_for_tracker = connectToTracker();
	int valread;
	string group_id,command,username,password;
	displayMessage(socket_fd_for_tracker);
	while(1){
		cin>>command;
		if(command=="accept_request"){
			string group_id,client;
			cin>>group_id>>client;
			acceptRequest(socket_fd_for_tracker,command,group_id,client);
		}
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
		if(command == "join_group" ){
			string group_id;
			cin>>group_id;
			joinGroup(socket_fd_for_tracker,command,group_id);
		}
		if(command == "upload_file" ){
			string file_path,group_id;
			cin>>file_path>>group_id;
			uploadFile(socket_fd_for_tracker,command,file_path,group_id);
		}
		if(command == "list_files" ){
			string group_id;
			cin>>group_id;
			listFiles(socket_fd_for_tracker,command,group_id);
		}
		if(command == "download_file" ){
			string file_path,group_id,destinationFilePath;
			cin>>file_path>>group_id>>destinationFilePath;
			downloadFile(socket_fd_for_tracker,command,file_path,group_id,destinationFilePath);
		}
		if(command == "list_requests" ){
			string group_id;
			cin>>group_id;
			listRequest(socket_fd_for_tracker,command,group_id);
		}
		if(command=="leave_group"){
			string group_id;
			cin>>group_id;
			leaveGroup(socket_fd_for_tracker,command,group_id);
		}
		if(command=="list_groups"){
			listGroups(socket_fd_for_tracker,command);
		}
		if(command=="logout"){
			logout(socket_fd_for_tracker,command);
		}
		if(command=="stop_sharing"){
			string file_path,group_id;
			cin>>file_path>>group_id;
			stopSharing(socket_fd_for_tracker,command,file_path,group_id);
		}
	}
	close(socket_fd_for_tracker);
}
//creating threads for client and server
int main(int argc,char **argv){
	pthread_t clientThread,serverThread;
	pthread_create(&clientThread,NULL,clientFunction,NULL);		
	pthread_create(&serverThread,NULL,serverFunction,NULL);
	pthread_join(clientThread,NULL);
	pthread_join(serverThread,NULL);
	
    return 0;  
}
