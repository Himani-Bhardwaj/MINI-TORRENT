#include <unistd.h> 
#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>  
#include <arpa/inet.h>
#include<bits/stdc++.h>
#include <fstream>
#include <vector>
#include<pthread.h>
#define MAX_SOCKETS 3
#define STRING_SIZE 11
#define SEND_SIZE 512
#define CHUNK_SIZE 512
using namespace std;
struct sockaddr_in addressOfServer;
struct sockaddr_in addressOfClient;
struct userDetails{
	string username;
	string password;
	string client;
	bool flag;
};
struct groupDetails{
	string group_id;
	string owner;
	string client;
};
struct fileDetails{
	string file_path;
	string hashValue;
	string client;
	string group_id;
	bool sharable;
};
struct thread_data {
   	int  new_socket;
   	struct sockaddr_in addressOfClient;
};
vector<struct groupDetails> gDetails;
vector<struct userDetails> uDetails;
vector<struct fileDetails> fDetails;
vector< pair< string,struct groupDetails >> pendingRequests;
string prevClient;
string trackerFile;
//sending welcome message to client
void sendWelcomeMessage(int new_socket){
	string message = "Welcome to the system.";
	send(new_socket,message.c_str(),message.size(),0);	
}
//if user is not their in the system
void sendMessageForCreation(int new_socket){
	string message = "In order to fulfill requests kindly signup first";
	send(new_socket,message.c_str(),message.size(),0);	
}
//sending login message
void sendLoginMessage(int new_socket){
	string message = "In order to get your request fulfilled ,you need to login.";
	send(new_socket,message.c_str(),message.size(),0);	
}
//sending successfull createion message
void sendSuccessfulMessage(int new_socket){
	string message = "You are successfully registered. Kindly login to proceed.";
	send(new_socket,message.c_str(),message.size(),0);	
}
//get larger string 
string getLargeString(int new_socket,int size){
	int n=0;
	char hash1[SEND_SIZE]={0};
	string newString ="";
	while(size > 0 && (n = recv(new_socket, hash1, sizeof(SEND_SIZE), 0))>0 )
	        	{
		    		string s = hash1;
				newString += s;
		    		memset ( hash1 , '\0', SEND_SIZE);
		    		size = size - n;
	        	}
return newString;
}
//send large string in chunks
void sendLargeString(int new_socket,string finalHash){
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
			if(send(new_socket, buffer, fileSizeSending, 0) < 0)
	        	{
	        	    cout<<"File CAnnot Be send"<<endl;
			    break;
	        	}
	        	memset ( buffer , '\0', SEND_SIZE);
			size = size - fileSizeSending ;
	    	}
}
//writing data to a file
void writeToAFile(){
	ofstream out("../commonFiles/userdetails.txt");
    	for (int i=0;i< uDetails.size();i++) out << uDetails[i].username<<";"<<uDetails[i].password<<";"<<uDetails[i].client<<";"<<uDetails[i].flag << endl;
	out.close();
	out.open("../commonFiles/groupDetails.txt");
    	for (int i=0;i< gDetails.size();i++) out << gDetails[i].group_id<<";"<<gDetails[i].owner<<";"<<gDetails[i].client<<endl;
	out.close();
	out.open("../commonFiles/fileDetails.txt");
    	for (int i=0;i< fDetails.size();i++) out << 		fDetails[i].file_path<<";"<<fDetails[i].hashValue<<";"<<fDetails[i].client<<";"<<fDetails[i].group_id<<";"<<fDetails[i].sharable << endl;
	out.close();
	out.open("../commonFiles/pendingRequests.txt");
    	for (int i=0;i< pendingRequests.size();i++) {
		out << pendingRequests[i].first<<";"<<pendingRequests[i].second.group_id<<";"<<pendingRequests[i].second.owner<<";"<<pendingRequests[i].second.client<<endl;
		out<<endl;
	}
	out.close();
}
//read from file when tracker is started
void readFromAFile(){
	ifstream in("../commonFiles/userdetails.txt");
	userDetails ud;
	string str;
	if(in.is_open())
	{
		while (getline(in, str))
		{
			if(str.size() > 0){
				stringstream s(str); 
  				string word;
				userDetails ud;
				int count = 0; 
		 	 	while(getline(s, word, ';')) {
					count++;
					switch(count) {
						case 1:ud.username = word;break;
						case 2:ud.password = word;break;
						case 3:ud.client =word;break;
						case 4:if(word == "true")
								ud.flag = true;
							else ud.flag = false;
							break;
					}
		   		}
				uDetails.push_back(ud);
			}	
		}
	}
    	in.close();
	in.open("../commonFiles/groupDetails.txt");
	groupDetails gd;
	if(in.is_open())
	{
	    	while (getline(in, str))
		{
			if(str.size() > 0){
				stringstream s(str); 
	  			string word;
				userDetails ud;
				int count = 0; 
		 	 	while(getline(s, word, ';')) {
					count++;
					switch(count) {
						case 1:gd.group_id = word;break;
						case 2:gd.owner = word;break;
						case 3:gd.client =word;break;
					}
		   		}
				gDetails.push_back(gd);
			}	
		}
	}
	in.close();
	in.open("../commonFiles/fileDetails.txt");
	fileDetails fd;
	if(in.is_open())
	{
	    	while (getline(in, str))
		{
			if(str.size() > 0){
				stringstream s(str); 
	  			string word;
				userDetails ud;
				int count = 0; 
		 	 	while(getline(s, word, ';')) {
					count++;
					switch(count) {
						case 1:fd.file_path = word;break;
						case 2:fd.hashValue = word;break;
						case 3:fd.client =word;break;
						case 4:fd.group_id =word;break;
						case 5:if(word == "true")
								fd.sharable = true;
							else fd.sharable = false;
							break;
					}
		   		}
				fDetails.push_back(fd);
			}	
		}
	}
	in.close();
	in.open("../commonFiles/pendingRequests.txt");
	groupDetails gd1;
	string socket;
    	if(in.is_open())
	{
		while (getline(in, str))
		{
			if(str.size() > 0){
				stringstream s(str); 
  				string word;
				userDetails ud;
				int count = 0; 
		 	 	while(getline(s, word, ':')) {
					count++;
					switch(count) {
						case 1:socket = word;break;
						case 2:gd1.group_id = word;break;
						case 3:gd1.owner =word;break;
						case 4:gd1.client =word;break;
					}
		   		}
				pendingRequests.push_back(make_pair(socket,gd1));
			}	
		}
	}
	in.close();
}
//sending tracker user information files
void sendingUserDetailsFile(int socket_fd){
	int bytes_read,ack ;
	string file_path ="../commonFiles/userdetails.txt";
	int file_size = 0;
	FILE *fp = fopen(file_path.c_str(), "rb");
	fseek ( fp , 0 , SEEK_END);
  	file_size = ftell ( fp );
  	rewind ( fp );	
	send(socket_fd,&file_size,sizeof(file_size),0);
	char m[11] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	char buff[SEND_SIZE] ;
	int bytesToBeSend = file_size;
	while( bytesToBeSend > 0 && (bytes_read = fread(buff, sizeof(char), sizeof buff, fp)) > 0 )
	{
		send(socket_fd, buff, bytes_read, 0);
		bytesToBeSend -= bytes_read ;
	}
	fclose(fp) ;
}
//sending group details file
void sendingGroupDetailsFile(int socket_fd){
	int bytes_read,ack ;
	string file_path ="../commonFiles/groupDetails.txt";
	int file_size = 0;
	FILE *fp = fopen(file_path.c_str(), "rb");
	fseek ( fp , 0 , SEEK_END);
  	file_size = ftell ( fp );
  	rewind ( fp );	
	send(socket_fd,&file_size,sizeof(file_size),0);
	char m[11] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	char buff[SEND_SIZE] ;
	int bytesToBeSend = file_size;
	while( bytesToBeSend > 0 && (bytes_read = fread(buff, sizeof(char), sizeof buff, fp)) > 0 )
	{
		send(socket_fd, buff, bytes_read, 0);
		bytesToBeSend -= bytes_read ;
	}
	fclose(fp) ;
}
//sending file details
void sendingFileDetailsFile(int socket_fd){
	int bytes_read,ack ;
	string file_path ="../commonFiles/fileDetails.txt";
	int file_size = 0;
	FILE *fp = fopen(file_path.c_str(), "rb");
	fseek ( fp , 0 , SEEK_END);
  	file_size = ftell ( fp );
  	rewind ( fp );	
	send(socket_fd,&file_size,sizeof(file_size),0);
	char m[11] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	char buff[SEND_SIZE] ;
	int bytesToBeSend = file_size;
	while( bytesToBeSend > 0 && (bytes_read = fread(buff, sizeof(char), sizeof buff, fp)) > 0 )
	{
		send(socket_fd, buff, bytes_read, 0);
		bytesToBeSend -= bytes_read ;
	}
	fclose(fp) ;
}
//sending pending requests
void sendingPendingRequestsFile(int socket_fd){
	int bytes_read;
	string file_path ="../commonFiles/pendingRequests.txt";
	int file_size = 0;
	FILE *fp = fopen(file_path.c_str(), "rb");
	fseek ( fp , 0 , SEEK_END);
  	file_size = ftell ( fp );
  	rewind ( fp );	
	send(socket_fd,&file_size,sizeof(file_size),0);
	char m[11] = {0};
	recv(socket_fd, m, sizeof(m), 0);
	char buff[SEND_SIZE] ;
	int bytesToBeSend = file_size;
	while( bytesToBeSend > 0 && (bytes_read = fread(buff, sizeof(char), sizeof buff, fp)) > 0 )
	{
		send(socket_fd, buff, bytes_read, 0);
		bytesToBeSend -= bytes_read ;
	}
	fclose(fp) ;
}
//receive user details
void receiveUserDetailsFile(int socket_fd){
	FILE* fp ;
	string file_path ="../commonFiles/userdetails.txt";
	fp = fopen(file_path.c_str(), "r+");
	int sizeToBeRecieved;
	recv(socket_fd, &sizeToBeRecieved, sizeof(sizeToBeRecieved), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);
	int bytes_recv ;
		char buf[SEND_SIZE] ;
		while( sizeToBeRecieved > 0 && (bytes_recv = recv(socket_fd, buf, sizeof buf, 0)) > 0)
		{
			fwrite(buf, sizeof(char), bytes_recv, fp);
			sizeToBeRecieved -= bytes_recv ;
		}
	fclose(fp);
}
//receive group details
void receiveGroupDetailsFile(int socket_fd){
	FILE* fp ;
	string file_path ="../commonFiles/groupDetails.txt";
	fp = fopen(file_path.c_str(), "r+");
	int sizeToBeRecieved;
	recv(socket_fd, &sizeToBeRecieved, sizeof(sizeToBeRecieved), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);
	int bytes_recv ;
		char buf[SEND_SIZE] ;
		while( sizeToBeRecieved > 0 && (bytes_recv = recv(socket_fd, buf, sizeof buf, 0)) > 0)
		{
			fwrite(buf, sizeof(char), bytes_recv, fp);
			sizeToBeRecieved -= bytes_recv ;
		}
	fclose(fp);
}
//receive file details
void receiveFileDetailsFile(int socket_fd){
	FILE* fp ;
	string file_path ="../commonFiles/fileDetails.txt";
	fp = fopen(file_path.c_str(), "r+");
	int sizeToBeRecieved;
	recv(socket_fd, &sizeToBeRecieved, sizeof(sizeToBeRecieved), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);
	int bytes_recv ;
		char buf[SEND_SIZE] ;
		while( sizeToBeRecieved > 0 && (bytes_recv = recv(socket_fd, buf, sizeof buf, 0)) > 0)
		{
			fwrite(buf, sizeof(char), bytes_recv, fp);
			sizeToBeRecieved -= bytes_recv ;
		}
	fclose(fp);
}
//receive pending requests
void receivePendingRequestsFile(int socket_fd){
	FILE* fp ;
	string file_path ="../commonFiles/pendingRequests.txt";
	fp = fopen(file_path.c_str(), "r+");
	int sizeToBeRecieved;
	recv(socket_fd, &sizeToBeRecieved, sizeof(sizeToBeRecieved), 0);
	string str = "got string";
	send(socket_fd,str.c_str(),str.size(),0);
	int bytes_recv ;
		char buf[SEND_SIZE] ;
		while( sizeToBeRecieved > 0 && (bytes_recv = recv(socket_fd, buf, sizeof buf, 0)) > 0)
		{
			fwrite(buf, sizeof(char), bytes_recv, fp);
			sizeToBeRecieved -= bytes_recv ;
		}
	fclose(fp);
}
//getting info on starting tracker
void getAllFileDetails(int new_socket){
	string str = "got string";
	send(new_socket,str.c_str(),str.size(),0);
	receiveUserDetailsFile(new_socket);
	send(new_socket,str.c_str(),str.size(),0);
	receiveGroupDetailsFile(new_socket);
	send(new_socket,str.c_str(),str.size(),0);
	receiveFileDetailsFile(new_socket);
	send(new_socket,str.c_str(),str.size(),0);
	receivePendingRequestsFile(new_socket);
	readFromAFile();
}
//sending all file details
void sendAllFileDetails(int socket_fd){
	char message[SEND_SIZE]={0};
	recv(socket_fd, message, sizeof(message), 0);
	sendingUserDetailsFile(socket_fd);
	memset(message,'\0',SEND_SIZE);
	recv(socket_fd, message, sizeof(message), 0);
	sendingGroupDetailsFile(socket_fd);
	memset(message,'\0',SEND_SIZE);
	recv(socket_fd, message, sizeof(message), 0);
	sendingFileDetailsFile(socket_fd);
	memset(message,'\0',SEND_SIZE);
	recv(socket_fd, message, sizeof(message), 0);
	sendingPendingRequestsFile(socket_fd);
}
//sending group lists
void listGroups(int new_socket){
	string toBeSend = "";
	vector<string> groups;
	for(int i=0;i< gDetails.size();i++){
		bool flag = false;
		if(groups.size() == 0) groups.push_back(gDetails[i].group_id);
		else{
			for(int j=0;j<groups.size();j++){
				if(groups[j] == gDetails[i].group_id){
					flag = true;break;
				}
			}
			if(flag == false) groups.push_back(gDetails[i].group_id);
		}
			
	}
	for(int j=0;j< groups.size();j++) toBeSend += groups[j];
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
}
//sending file lists
void listFiles(int new_socket,string group_id){
	string toBeSend = "";
	for(int i=0;i< fDetails.size();i++){
		if( fDetails[i].group_id == group_id && fDetails[i].sharable == true)
			toBeSend += to_string(i)+":"+fDetails[i].file_path+":"+fDetails[i].hashValue+";";
	}
	int size = toBeSend.size();
	send(new_socket, &size, sizeof(size), 0);
	char m[11] = {0};
	recv(new_socket, m, sizeof(m), 0);
	sendLargeString(new_socket,toBeSend);
}
//accept a pending request
void acceptRequest(int new_socket,string group_id,string client){
	string toBeSend = "Sory no such group id exists";
	for(int i=0;i< pendingRequests.size();i++){
		if( pendingRequests[i].second.group_id == group_id){
			gDetails.push_back(pendingRequests[i].second);
			pendingRequests.erase(pendingRequests.begin()+i);
			toBeSend="Request is successfully accepted";
		}
	}
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
}
// send pending requestes to the owner
void listRequests(int new_socket,string group_id,string client){
	string toBeSend = "";
	for(int i=0;i< pendingRequests.size();i++){
		if( pendingRequests[i].second.group_id == group_id && pendingRequests[i].second.owner == client)
			toBeSend += pendingRequests[i].second.client+";";
	}
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
}
//checking if already  loggd in
bool checkLoginDetails(string client){
	if( uDetails.size() == 0)
		return false;
	else{
		for(int i=0;i< uDetails.size();i++){
			if(uDetails[i].client == client && uDetails[i].flag == true){
			 	return true;
				}
		}
		return false;
	}
}
//check if client exixts in the system
bool checkUserInSystem(string recievedSocket){
	if( uDetails.size() == 0)
		return false;
	else{
		bool flag1 = false;
		for(int i=0;i< uDetails.size();i++){
			if(uDetails[i].client == recievedSocket){
			 	return true;
				}
		}
		return false;
	}
}
//check if group is exixting in the system or not
bool checkGroup(string group_id){
	if(gDetails.size() == 0) return false;
	for(int i=0;i< gDetails.size();i++){
		if( gDetails[i].group_id == group_id)
			return true;
	}
	return false;
}
//check if specified client is already a member of the group or not
bool checkUserExistsInGroup(string group_id,string client){
	if(gDetails.size() == 0) return false;
	for(int i=0;i< gDetails.size();i++){
		if( gDetails[i].group_id == group_id && gDetails[i].client == client)
			return true;
	}
	return false;
}
//this is to get owner of the group
string getOwnerOfGroup(string group_id){
	if(gDetails.size() == 0) return "NULL";
	for(int i=0;i< gDetails.size();i++){
		if( gDetails[i].group_id == group_id)
			return gDetails[i].owner;
	}
	return "NULL";
}
//creating pending requests for users 
void addToPendingRequestes(int new_socket,string group_id,string client,string owner){
	groupDetails gd;
	gd.group_id = group_id;
	gd.client = client;
	gd.owner = owner;
	pendingRequests.push_back(make_pair(group_id,gd));
	string successmessage = "Your request is send to the corresponding owner and is pending for approval";
	send(new_socket,successmessage.c_str(),successmessage.size(),0);
}
//this is when no group with specified groupId exists
void createNewGroupEntry(int new_socket,string group_id,string client,string owner){
	groupDetails gd;
	gd.group_id = group_id;
	gd.client = client;
	gd.owner = owner;
	gDetails.push_back(gd);
	string successmessage = "Your Group is successfully created";
	send(new_socket,successmessage.c_str(),successmessage.size(),0);
}
//download file request to be entertained
void downloadFile(int new_socket,string hashValue,string group_id, string recievedSocket){
	if(!checkUserInSystem(recievedSocket)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(recievedSocket)){
		sendLoginMessage(new_socket);
	}
	else if(!checkUserExistsInGroup(group_id,recievedSocket)){
		string message = "You are not a member of the group. Kindly create or join an existing group in order to download file";
		send(new_socket,message.c_str(),message.size(),0);
	}
	else{
		string toBeSend = "";
		for(int i=0;i< fDetails.size();i++){
			if( fDetails[i].hashValue == hashValue){
				 toBeSend += fDetails[i].client+";";
			}
		}
		send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
	}
}
//logout from system
void logout(int new_socket,string recievedSocket){
	if(!checkUserInSystem(recievedSocket)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(recievedSocket)){
		sendLoginMessage(new_socket);
	}
	else{
		for(int i=0;i< fDetails.size();i++){
			if( fDetails[i].client == recievedSocket)
				fDetails[i].sharable = false;
		}
		for(int i=0;i< uDetails.size();i++){
			if( uDetails[i].client == recievedSocket)
				uDetails[i].flag = false;
		}
		string success = "You have been successfully logged out from the system";
		send(new_socket,success.c_str(),success.size(),0);
	}
}
//stop sharing of a file
void stopSharing(int new_socket,string hashValue,string group_id,string recievedSocket){
	if(!checkUserInSystem(recievedSocket)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(recievedSocket)){
		sendLoginMessage(new_socket);
	}
	else if(!checkUserExistsInGroup(group_id,recievedSocket)){
		string message = "You are not a member of the group. Kindly create or join an existing group in order to download file";
		send(new_socket,message.c_str(),message.size(),0);
	}
	else{
		for(int i=0;i< fDetails.size();i++){
			if( fDetails[i].group_id == group_id && fDetails[i].hashValue == hashValue && fDetails[i].client == recievedSocket)
				fDetails[i].sharable = false;
		}
		string success = "Your requested file is no more sharable";
		send(new_socket,success.c_str(),success.size(),0);
	}
}
//this is to upload a file by a user
void uploadFile(int new_socket,string file_path,string hashValue,string client,string group_id){
	fileDetails fd;
	fd.file_path = file_path;
	fd.hashValue=hashValue;
	fd.client = client;
	fd.group_id = group_id;
	fd.sharable=true;
	if(!checkUserInSystem(client)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(client)){
		sendLoginMessage(new_socket);
	}
	else if(checkUserExistsInGroup(group_id,client)){
		if(fDetails.size() == 0) {
			fDetails.push_back(fd);
			string message = "Your File is uploaded successfully";
			send(new_socket,message.c_str(),message.size(),0);
		}
		else{
			bool flag1 = false;
			for(int i=0;i< fDetails.size();i++){
				if(fDetails[i].client == client && fDetails[i].hashValue == hashValue){
					flag1 = true;break;
				}
			}
			if(flag1 == false){
				fDetails.push_back(fd);
				string message = "Your File is uploaded successfully";
				send(new_socket,message.c_str(),message.size(),0);
			}
			else{
				string message = "Your File already exist in the system";
				send(new_socket,message.c_str(),message.size(),0);
			}
		}
	}
	else{
		string message = "You are not a member of the group. Kindly create or join an existing group in order to upload file";
		send(new_socket,message.c_str(),message.size(),0);
	}
}
//this is to leave the group
void leaveGroup(int new_socket,string group_id,string recievedSocket){
	if(!checkUserInSystem(recievedSocket)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(recievedSocket)){
		sendLoginMessage(new_socket);
	}
	else if(checkGroup(group_id)){
		if(!checkUserExistsInGroup(group_id,recievedSocket)){
			string failiure = "You are already not registered as a memeber of the group";
			send(new_socket,failiure.c_str(),failiure.size(),0);
		}
		else{
			string owner = getOwnerOfGroup(group_id);
			if(owner == recievedSocket) {
				string failiure = "You cannot exit the group as you are its owner";
				send(new_socket,failiure.c_str(),failiure.size(),0);
			}
			else{
				 for(int i=0;i< gDetails.size();i++){
					if( gDetails[i].group_id == group_id && gDetails[i].client == recievedSocket)
						gDetails.erase(gDetails.begin()+i);
					}
				 for(int i=0;i< fDetails.size();i++){
					if( fDetails[i].group_id == group_id && fDetails[i].client == recievedSocket)
						fDetails[i].sharable = false;
				}
				string success = "You are successfully removed from the group";
				send(new_socket,success.c_str(),success.size(),0);
			}
		}
	}
	else{
		string message = "This group doesn't exist.";
		send(new_socket,message.c_str(),message.size(),0);
	}
}
//this is to join an existing group
void joinGroup(int new_socket,string group_id,string recievedSocket){
	if(!checkUserInSystem(recievedSocket)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(recievedSocket)){
		sendLoginMessage(new_socket);
	}
	else if(checkGroup(group_id)){
		if(checkUserExistsInGroup(group_id,recievedSocket)){
			string failiure = "You are already a memeber of the group";
			send(new_socket,failiure.c_str(),failiure.size(),0);
		}
		else{
			string owner = getOwnerOfGroup(group_id);
			if(owner == "NULL") createNewGroupEntry(new_socket,group_id,recievedSocket,recievedSocket);
			else addToPendingRequestes(new_socket,group_id,recievedSocket,owner);
		}
	}
	else{
		string message = "This group doesn't exist. Kindly create the group before joining.";
		send(new_socket,message.c_str(),message.size(),0);
	}
}
//this is to create a new group 
void createGroup(int new_socket,string group_id,string recievedSocket){
	if(!checkUserInSystem(recievedSocket)){
		sendMessageForCreation(new_socket);
	}
	else if(!checkLoginDetails(recievedSocket)){
		sendLoginMessage(new_socket);
	}
	else if(checkGroup(group_id)){
		if(checkUserExistsInGroup(group_id,recievedSocket)){
			string failiure = "You are already a memeber of the group";
			send(new_socket,failiure.c_str(),failiure.size(),0);
		}
		else{
			string message = "This group already exists. You can only join this group";
			send(new_socket,message.c_str(),message.size(),0);
		}
	}	
	else{
		createNewGroupEntry(new_socket,group_id,recievedSocket,recievedSocket);
	}
}
//login user in the system
void loginUser(int new_socket,string username,string password,string client){		
	string message;
	if(checkLoginDetails(client))
		message = "You are already logged in";
	else{
		bool flag = false;
		for(int i=0;i< uDetails.size();i++){
			if(uDetails[i].username == username && uDetails[i].password == password){
				uDetails[i].flag = true;
				flag = true;
				break;
				}
		}
		if(flag == true) {
			message = "You can now access system";
			for(int i=0;i< fDetails.size();i++){
				if( fDetails[i].client == client)
				fDetails[i].sharable = true;
			}
		}
		else message ="You are not the member of the system.Please Signup first";
	}
	send(new_socket,message.c_str(),message.size(),0);
}
//updating newclient in group
void updateClientInGroup(string client){
	if(gDetails.size() == 0) return;
	for(int i=0;i< gDetails.size();i++){
		if(gDetails[i].client == prevClient ) gDetails[i].client = client;
		if(gDetails[i].owner == prevClient ) gDetails[i].owner = client;
	}
}
//updating new client for file
void updateClientInFile(string client){
	if(fDetails.size() == 0) return;
	for(int i=0;i< fDetails.size();i++){
		if(fDetails[i].client == prevClient ) fDetails[i].client = client;
	}
}
//check if user exists or not;
bool checkUserDetails(string username,string password,string client){
	if( uDetails.size() == 0)
		return false;
	else{
		for(int i=0;i< uDetails.size();i++){
			if(uDetails[i].username == username && uDetails[i].password == password){	
			 	prevClient = uDetails[i].client;
				uDetails[i].client = client;
				return true;
				}
		}
		return false;
	}
}
//for creating user in the system
void createUser(int new_socket,string username,string password,string client){
	if(checkUserDetails(username,password,client)){
		updateClientInGroup(client);
		updateClientInFile(client);
		sendLoginMessage(new_socket);
	}
	else{
		userDetails ud;
		ud.username = username;
		ud.password = password;
		ud.client=client;
		ud.flag=false;
		uDetails.push_back(ud);
		sendSuccessfulMessage(new_socket);
	}
}
//created thread for each client
void *clientConnect(void *threadarg){
	struct thread_data *my_data;
   	my_data = (struct thread_data *) threadarg;
	int new_socket = my_data->new_socket;
	struct sockaddr_in addressOfClient = my_data->addressOfClient ;
	char *ip; 
        ip = new char[INET_ADDRSTRLEN]; 
        inet_ntop(AF_INET, &(addressOfClient.sin_addr), ip, INET_ADDRSTRLEN);
	string str = ip;
	string str1 = to_string(ntohs(addressOfClient.sin_port));	
	string recievedSocket = str+":"+str1; 
	sendWelcomeMessage(new_socket);
	while(1){
	char toBeRecieved[SEND_SIZE]={0};
	string command;	
	memset ( toBeRecieved , '\0', SEND_SIZE);
	recv(new_socket, toBeRecieved, sizeof(toBeRecieved), 0);
	string recieved = toBeRecieved;
	stringstream s(recieved); 
  	string word;
	userDetails ud;
	int count = 0; 
	  while(getline(s, word, ';')) {
		count++;
		if(count == 1) {
			command = word;
			break;
		}
	   }
		count = 0;
		if(command == "create_user") {
			string username,password,client;
			while(getline(s, word, ';')) {
				count++;
				switch(count){
				 case 1: username = word; break;
				 case 2: password = word; break;
				 case 3: client = word; break;
	   			}
			}
			createUser(new_socket,username,password,client);	
		}
		if(command == "login") {
			string username,password;
			while(getline(s, word, ';')) {
				count++;
				switch(count){
				 case 1: username = word; break;
				 case 2: password = word; break;
	   			}
			}
			loginUser(new_socket,username,password,recievedSocket);
		}
		if(command == "create_group") {
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				switch(count){
				 case 1: group_id = word; break;
	   			}
			}
			createGroup(new_socket,group_id,recievedSocket);	
		}
		if(command == "join_group"){
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word; 
	   		}
			joinGroup(new_socket,group_id,recievedSocket);	
		}
		if(command == "upload_file"){
			string file_path,hashValue,group_id;
			int size;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) file_path = word;
				else if(count == 2)  group_id = word;
				else if(count == 3) size = stoi(word);
				
	   		}
			string m = "got string";
			send(new_socket,m.c_str(),m.size(),0);
			hashValue = getLargeString(new_socket,size);
			uploadFile(new_socket,file_path,hashValue,recievedSocket,group_id);
		}
		if(command == "stop_sharing"){
			string file_path,group_id;
			int size;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1)  group_id = word;
				else if(count == 2) size = stoi(word);
			}
			string m = "got string";
			send(new_socket,m.c_str(),m.size(),0);
			string hashValue = getLargeString(new_socket,size);
			stopSharing(new_socket,hashValue,group_id,recievedSocket);
		}
		if(command == "list_files"){
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word;
	   		}
			listFiles(new_socket,group_id);	
		}
		if(command == "list_requests" ){
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word;
	   		}
			listRequests(new_socket,group_id,recievedSocket);
		}
		if(command == "list_groups" ){
			listGroups(new_socket);
		}
		if(command == "logout" ){
			logout(new_socket,recievedSocket);
		}
		if(command=="leave_group"){
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word;
	   		}
			leaveGroup(new_socket,group_id,recievedSocket);
		}
		if(command=="accept_request"){
			string group_id,client;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word;
				if(count == 2) client = word;
	   		}
			acceptRequest(new_socket,group_id,client);
		}
		if(command == "download_file"){
			string hashValue,group_id;
			int size;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) size = stoi(word);
				else if(count == 2) group_id = word;
				
	   		}
			string str = "got string";
			send(new_socket,str.c_str(),str.size(),0);
			hashValue = getLargeString(new_socket,size);
			downloadFile(new_socket,hashValue,group_id,recievedSocket);	
		}
		if(command == "quit"){
			getAllFileDetails(new_socket);
		}
		if(command == "up"){
			sendAllFileDetails(new_socket);
		}
		writeToAFile();
	}
	
	close(new_socket);
}
//starting tracker sending it in listen mode
int startTracker(){
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
    	addressOfServer.sin_port = htons( 12004 );
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
int main(int argc,char **argv){
	
	int socket_fd = startTracker(),new_socket;	
	struct sockaddr_in addressOfClient;
	int addrlen = sizeof(addressOfClient);
	int i = 0;	
	pthread_t connectToClient[MAX_SOCKETS];
	struct thread_data td[MAX_SOCKETS];
	while(1)
    	{
        	new_socket = accept(socket_fd, (struct sockaddr *)&addressOfClient, (socklen_t*)&addrlen);
		if (new_socket < 0 )
    		{ 
        		cout<<"Cannot accept socket"<<endl; 
        		exit(EXIT_FAILURE); 
    		} 
		else{
			td[i].new_socket=new_socket;
			td[i].addressOfClient=addressOfClient;
		}
		if( pthread_create(&connectToClient[i], NULL, clientConnect, (void *)&td[i]) != 0 )
           	printf("Failed to create thread\n");		
		if( i >= MAX_SOCKETS)
        	{
          		i = 0;
          		while(i < MAX_SOCKETS)
          		{
            			pthread_join(connectToClient[i++],NULL);
          		}
         		 i = 0;
        	}
    	}
	close(socket_fd);
    	return 0;  
}
