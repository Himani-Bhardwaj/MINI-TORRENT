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
using namespace std;
struct sockaddr_in addressOfServer;
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
};
struct thread_data {
   	int  new_socket;
   	struct sockaddr_in addressOfClient;
};
vector<struct groupDetails> gDetails;
vector<struct userDetails> uDetails;
vector<struct fileDetails> fDetails;
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
    	addressOfServer.sin_port = htons( 12000 );
	if( bind(socket_fd,(struct sockaddr*)&addressOfServer,sizeof(addressOfServer)) < 0){
		cout<<"Cannot Bind Socket"<<endl;
		exit(EXIT_FAILURE);
	}
	if (listen(socket_fd, 3) < 0) 
    	{ 
    	    cout<<"Cannot listen to socket"<<endl; 
    	    exit(EXIT_FAILURE); 
    	}
return socket_fd;
}
void sendMessageForCreation(int new_socket){
	string message = "SignUp First";
	send(new_socket,message.c_str(),message.size(),0);	
}
void sendLoginMessage(int new_socket){
	string message = "You are a member of this system. Kindly login to proceed.";
	send(new_socket,message.c_str(),message.size(),0);	
}
void sendWelcomeMessage(int new_socket){
	string message = "Welcome back to the system.";
	send(new_socket,message.c_str(),message.size(),0);	
}
bool checkLoginDetails(string username,string password){
	if( uDetails.size() == 0)
		return false;
	else{
		for(int i=0;i< uDetails.size();i++){
			if(uDetails[i].username == username && uDetails[i].password == password && uDetails[i].flag == true){
			 	return true;
				}
		}
		return false;
	}
}
bool checkUserDetails(string username,string password,string client){
	if( uDetails.size() == 0)
		return false;
	else{
		for(int i=0;i< uDetails.size();i++){
			if(uDetails[i].client == client && uDetails[i].username == username && uDetails[i].password == password){	
			 	return true;
				}
		}
		return false;
	}
}
void loginUser(int new_socket,string username,string password,string client){		
	string message;
	if(checkLoginDetails(username,password))
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
		if(flag == true) message = "You can now access system";
		else message ="Please Signup first";
	}
	send(new_socket,message.c_str(),message.size(),0);
}
void createUser(int new_socket,string username,string password,string client){
	if(checkUserDetails(username,password,client))
		sendLoginMessage(new_socket);
	else{
		userDetails ud;
		ud.username = username;
		ud.password = password;
		ud.client=client;
		ud.flag=false;
		uDetails.push_back(ud);
		sendLoginMessage(new_socket);
	}
}

bool checkUserInSystem(int new_socket,string recievedSocket){
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
bool checkGroup(string group_id){
	for(int i=0;i< gDetails.size();i++){
		if( gDetails[i].group_id == group_id)
			return true;
	}
	return false;
}
bool checkUserExistsInGroup(string group_id,string client){
	for(int i=0;i< gDetails.size();i++){
		if( gDetails[i].group_id == group_id && gDetails[i].client == client)
			return true;
	}
	return false;
}
string getOwnerOfGroup(string group_id){
	for(int i=0;i< gDetails.size();i++){
		if( gDetails[i].group_id == group_id)
			return gDetails[i].owner;
	}
	return NULL;
}
void createNewGroupEntry(int new_socket,string group_id,string client,string owner){
	groupDetails gd;
	gd.group_id = group_id;
	gd.client = client;
	gd.owner = owner;
	gDetails.push_back(gd);
	string successmessage = "You have been successfully added to the group";
	send(new_socket,successmessage.c_str(),successmessage.size(),0);
}
void createGroup(int new_socket,string group_id,string recievedSocket){
	if(checkGroup(group_id)){
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
void joinGroup(int new_socket,string group_id,string recievedSocket){
	if(checkGroup(group_id)){
		if(checkUserExistsInGroup(group_id,recievedSocket)){
			string failiure = "You are already a memeber of the grup";
			send(new_socket,failiure.c_str(),failiure.size(),0);
		}
		else{
			string owner = getOwnerOfGroup(group_id);
			createNewGroupEntry(new_socket,group_id,recievedSocket,owner);
		}
	}
	else{
		string message = "This group doesn't exist. Kindly create the group before joining.";
		send(new_socket,message.c_str(),message.size(),0);
	}
}
void uploadFile(int new_socket,string file_path,string hashValue,string client,string group_id){
	fileDetails fd;
	fd.file_path = file_path;
	fd.hashValue=hashValue;
	fd.client = client;
	fd.group_id = group_id;
	if(checkUserExistsInGroup(group_id,client)){
		if(fDetails.size() == 0) {
			fDetails.push_back(fd);
			cout<<fDetails[0].file_path<<endl;
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
void listFiles(int new_socket,string group_id){
	string toBeSend = "";
	for(int i=0;i< fDetails.size();i++){
		if( fDetails[i].group_id == group_id)
			cout<<fDetails[i].group_id<<endl;
			toBeSend = i+";"+fDetails[i].file_path+";"+fDetails[i].hashValue;
			send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
	}
	toBeSend = "";
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
}
void downloadFile(int new_socket,string file_path){
	string toBeSend = "",hashValue;
	for(int i=0;i< fDetails.size();i++){
		if( fDetails[i].file_path == file_path){
			 hashValue=fDetails[i].hashValue;
			 break;
		}
	}
	cout<<hashValue<<endl;
	for(int i=0;i< fDetails.size();i++){
		if( fDetails[i].hashValue == hashValue){
			 toBeSend += fDetails[i].client+";";
		}
	}
	cout<<toBeSend<<endl;
	send(new_socket,toBeSend.c_str(),toBeSend.size(),0);
}
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
	if(checkUserInSystem(new_socket,recievedSocket) == false ){
		sendMessageForCreation(new_socket);
	}
	else{
		sendWelcomeMessage(new_socket);
	}
	while(1){
	char toBeRecieved[1024]={0};
	string command;
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
		else if(command == "join_group"){
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word; 
	   		}
			joinGroup(new_socket,group_id,recievedSocket);	
		}
		else if(command == "upload_file"){
			string file_path,hashValue,group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) file_path = word;
				else if(count == 2)  hashValue = word;
				else if(count == 3)  group_id = word;
	   		}
			uploadFile(new_socket,file_path,hashValue,recievedSocket,group_id);	
		}
		else if(command == "list_files"){
			string group_id;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) group_id = word;
	   		}
			listFiles(new_socket,group_id);	
		}
		else if(command == "download_file"){
			string file_path;
			while(getline(s, word, ';')) {
				count++;
				if(count == 1) file_path = word;
	   		}
			downloadFile(new_socket,file_path);	
		}
	}
	close(new_socket);
}
int main(int argc,char **argv){
	
	int socket_fd = startTracker(),new_socket;	
	struct sockaddr_in addressOfClient;
	int addrlen = sizeof(addressOfClient);
	int i = 0;
	pthread_t connectToClient[3];
	struct thread_data td[3];
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
		if( i >= 50)
        	{
          		i = 0;
          		while(i < 50)
          		{
            			pthread_join(connectToClient[i++],NULL);
          		}
         		 i = 0;
        	}
    	}
	close(socket_fd);
    	return 0;  
}
