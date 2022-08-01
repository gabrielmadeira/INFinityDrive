#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include "Client.hpp"
#include "Connection.hpp"

using namespace std;

Client::Client(string username, string srvrAdd, int srvrPort) 
{
    name = username; 
    if(gethostname(hostname, HOST_NAME_MAX)) 
        throw runtime_error("Couldn't get machine info");
    socketfd = connectClient(name, hostname, srvrAdd, srvrPort);
    if(!socketfd)
        throw runtime_error("Couldn't connect to server");
}

void Client::uploadFile(string filepath) 
{
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;

}

void Client::downloadFile(string filepath) 
{
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;

}

void deleteFile(string filepath)
{
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;

}

void Client::listServer() 
{

}






