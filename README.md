# MINI TORRENT
File sharing system with fallback multi-tracker system with synchronization and parallel downloading. Used openssl library for computing hash values of files. Implemented own algorithm for data piece selection in order to download efficiently a file from multiple servers(peers) piece by piece.

Prerequisites:
We need following to generate SHA1 hash of the files.

1. sudo apt-get install openssl
2. sudo apt-get install libssl-dev

Compiling:
Code is divided in folders: client and tracker
  g++ tracker.cpp -lpthread -o tracker
  g++ client.cpp -lssl -lcrypto -lpthread -o client 
Running :

  Run tracker
    ./tracker ../commonFiles/infoOfTrackers.txt
  Run client
    ./client IPADDRESS:PORT ../commonFiles/tracker_info.txt

Features Implemented:

1. Multi-Tracker(Server) to serve request for all clients.
2. User Authentication
3. File sharing visibility to specific Group and Users
4. Piece Selection Algorithm to balance Load Sharing
5. Compatabile to run on Local Host and different IP and Ports provided by User
6. Functions: 1.share 2.download 3.remove seeder 4.show downloads 5.to close application . press "quit".so as to notify the tracker to remove the current client from seeder list
