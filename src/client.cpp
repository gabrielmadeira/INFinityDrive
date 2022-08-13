#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include "client.hpp"
#include "connection.hpp"
#include "file.hpp"

using namespace std;

Client::Client(string username, string srvrAdd, int srvrPort) 
{
    name = username; 
    if(gethostname(device, HOST_NAME_MAX)) 
        throw runtime_error("Couldn't get machine info");
    socketfd = connectClient(name, srvrAdd, srvrPort); 
    if(socketfd == -1)
        throw runtime_error("Couldn't connect to server");
    string msg = name+'|'+device+'|';
    if(!sendProtocol(socketfd, msg, DVCE))
        throw runtime_error("Couldn't send machine info");
}

void Client::uploadFile(string filepath) 
{
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;
    File file(filepath);

    if(upload(socketfd, file)) 
        throw runtime_error("Failed to send file to server");
}

void Client::downloadFile(string filepath) 
{
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;

    File* newfile = (File*)download(socketfd, filepath);
    if(!newfile) cout << "No file found with name "+filepath << endl;
    newfile->write(filepath);
}

void deleteFile(string filepath)
{
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;

}

void Client::listServer() 
{

}






