# OS-ASSIGNMENT2
A mini p2p file sharing system.

Prerequisites
We need following to generate SHA1 hash of the files.

sudo apt-get install openssl
sudo apt-get install libssl-dev
Compiling
Code is divided in folders: client and tracker
g++ tracker.cpp -lpthread -o tracker
g++ client.cpp -lssl -lcrypto -lpthread -o client 
Running
Run tracker
./tracker ../commonFiles/infoOfTrackers.txt
Run client
./client IPADDRESS:PORT ../commonFiles/tracker_info.txt
