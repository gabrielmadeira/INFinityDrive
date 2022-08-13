#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "client.hpp"
#include "connection.hpp"

#define PORT 4000

int main(int argc, char *argv[])
{
    if (argc != 4) throw runtime_error("Wrong use of client! Expected <username> <server_ip> <port>");
    string username = argv[1], srvrAdd = argv[2];
    int srvrPort = stoi(argv[3]);

    Client newClient(username, srvrAdd, srvrPort);

    sendProtocol(newClient.socketfd, "tipo se liga", DATA);
    sendProtocol(newClient.socketfd, "o crime do parceiro", UPLD);
    sendProtocol(newClient.socketfd, "que droga", DELT);
    

    tuple<PROTOCOL_TYPE, string> ret = receiveProtocol(newClient.socketfd);
    std::cout << "Cliente :: " << get<1>(ret);

    newClient.disconnect();
    return 0;
}