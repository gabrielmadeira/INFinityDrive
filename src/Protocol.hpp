#include <cmath>
#include <string>
#include <string.h>
#include <iostream>
#include<algorithm>
#include <sys/socket.h>

#define PAYLOAD_SIZE 256
#define BUFFER_SIZE 320

using namespace std;

enum PROTOCOL_TYPE { DATA, UPLD, DWNL, DELT, LSSV, LSCL, GSDR };

typedef struct protocol
{
    uint16_t type;
    uint16_t total_chunks;
    uint16_t chunk;
    char payload[PAYLOAD_SIZE];
} Protocol;

bool sendProtocol(int socketfd, string message)
{
    Protocol buffer;
    message.resize(ceil(message.size()/PAYLOAD_SIZE), '|');
    buffer.total_chunks = message.size();
    const char* msg_pointer = message.c_str();

    for(int i = 0; i < buffer.total_chunks; i++)
    {
        buffer.type = DATA;
        buffer.chunk = i;
        copy(&msg_pointer[i*PAYLOAD_SIZE], 
             &msg_pointer[(i+1)*PAYLOAD_SIZE-1], 
             &buffer.payload);
             
        if(send(socketfd, &buffer, BUFFER_SIZE, 0) == -1)
            throw runtime_error("Failed to send message");
    }

    return true;
}

string receiveProtocol(int socketfd)
{
    Protocol buffer;
    recv(socketfd, &buffer, BUFFER_SIZE, 0);
    char msg_pointer[buffer.total_chunks*PAYLOAD_SIZE];
    strcpy(msg_pointer, buffer.payload);

    while(buffer.chunk != buffer.total_chunks)
    {
        recv(socketfd, &buffer, BUFFER_SIZE, 0);
        strcat(msg_pointer, buffer.payload);
    } 

    string message(msg_pointer);
    message.erase(remove(message.begin(), message.end(), '|'), message.end());
    return message;
}