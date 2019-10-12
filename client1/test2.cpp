#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <regex>
using namespace std; 

void getProcessDetails(string path)
{
	int get_Name,get_processId,get_state;
	get_Name=0;
	get_processId = 0;
	get_state = 0;
	ifstream infile;
	string sourceName = path + "/" + "status";
	infile.open(sourceName.c_str(), ios::in);
	string str;
	while(infile){
	getline(infile, str);
	if(str.substr(0,5) == "Name:"){		
			cout<< "1. Process Name is : " << str.substr(5, str.size()-5)<<endl;
			get_Name = 1;
		}
	else if(str.substr(0,5) == "PPid:"){
			cout<<"2. Parent ID is : "<<str.substr(5, str.size()-5)<<endl;
			get_processId = 1;
	}
	else if(str.substr(0,6) == "State:"){
			cout<< "3. Process state is : "<<str.substr(6, str.size()-6)<<endl;
			get_state = 1;
	}
	}
	infile.close();
	if(get_Name && get_processId && get_state) return;
}

void getSessionDetails(string path)
{
	ifstream infile;
	string sourceName = path + "/" + "sessionid";
	infile.open(sourceName.c_str(), ios::in);
	string str;
	while(infile){
	getline(infile, str);
	cout<<"4. Session id is :"<<str<<endl; break;
	}
	infile.close();

}
void getRootFolder(string path){
	char buf[1024];  
	path = path+"/exe";
    ssize_t len = ::readlink(path.c_str(), buf, sizeof(buf)-1);
    if (len != -1) {
      buf[len] = '\0';
      cout<<"5. Root folder is: "<<string(buf)<<endl;
    }
}
void getEnvironmentVariables(string path){
	ifstream infile;
	string sourceName = path + "/" + "environ";
	infile.open(sourceName.c_str(), ios::in);
	string str;
	while(infile){
	getline(infile, str);
	cout<<"6. Environment variable is :"<<str<<endl;break;
	}
	infile.close();
}
void getFileDirectories(string path){
	string sourceName = path + "/" + "fd";
	string sourceName1 = sourceName;
	struct dirent *de;
	DIR *sourcedr = opendir(sourceName.c_str());
	if (sourcedr == NULL)
   		 { 
       			 cout<<"Could not open source directory" ; 
       			 return; 
   		 } 
	cout<<"7. File Directories Are: ";
	while((de = readdir(sourcedr)) != NULL){			
			char buf[1024];  
			string newPath = sourceName +"/" + string(de->d_name);
			ssize_t len = ::readlink(newPath.c_str(), buf, sizeof(buf)-1);
    			if (len != -1) {
    			  buf[len] = '\0';
    			 cout <<string(buf)<<endl;
				
		}
	}
	closedir(sourcedr);
}
void getOtherDetails(string path){
	ifstream infile;
	string sourceName = path + "/" + "stat";
	infile.open(sourceName.c_str(), ios::in);
	string str;
	while(infile){
	getline(infile, str);
	if(str.substr(0,4) == "ctxt"){		
			cout<<"8. Number of context switches  are  : " << str.substr(4, str.size()-4)<<endl;
		}
	else if(str.substr(0,13) == "procs_running"){
			cout<<"9. Currently running processes are : "<<str.substr(13, str.size()-13)<<endl;
	}
	else if(str.substr(0,13) == "procs_blocked"){
			cout<< "10. Currently blocked processes are : "<<str.substr(13, str.size()-13)<<endl;
	}
	}
	infile.close();

	
}
int main(int argc,char **argv)
{
	if(argc > 1)
	{	
			string processId = argv[1];
			string sourceName = "/proc/"+processId;	
			getProcessDetails(sourceName);
			getSessionDetails(sourceName);
			getRootFolder(sourceName);			
			getEnvironmentVariables(sourceName);
			getFileDirectories(sourceName);
			getOtherDetails("/proc");
	}
	else{
		string sourceName = "/proc";
		regex integer("(\\+|-)?[[:digit:]]+");
		vector<string> listOfProcFiles;	
		struct dirent *de;
		DIR *sourcedr = opendir(sourceName.c_str());
		while((de = readdir(sourcedr)) != NULL){			
			if(regex_match(de->d_name,integer))			
			listOfProcFiles.push_back(de->d_name);
		}
		closedir(sourcedr);
		for(int i=0;i<	listOfProcFiles.size();i++){			
			sourceName = "/proc/"+listOfProcFiles[i];
			getProcessDetails(sourceName);
			getSessionDetails(sourceName);
			getRootFolder(sourceName);
			getEnvironmentVariables(sourceName);
			getFileDirectories(sourceName);
		}
		getOtherDetails("/proc");
	}
	return 0;
}
