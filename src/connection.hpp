#pragma once
#include <string>
#include <tuple>
#include "file.hpp"

// Protocol related methods and definitions
#define PAYLOAD_SIZE 256

enum PROTOCOL_TYPE : short { DATA, UPLD, DWNL, DELT, LSSV, LSCL, GSDR, LOGN, ERRO };
static int BUFFER_SIZE = PAYLOAD_SIZE + sizeof(PROTOCOL_TYPE) + 2 * sizeof(uint16_t);
typedef struct protocol
{
    char payload[PAYLOAD_SIZE]; // 1 byte * 256
    PROTOCOL_TYPE type;         // 2 bytes
    uint16_t total_chunks;      // 2 bytes
    uint16_t chunk;             // 2 bytes
} Protocol;

using tProtocol = tuple<PROTOCOL_TYPE, string>;

bool sendProtocol(int socketfd, string message, PROTOCOL_TYPE type);
tProtocol receiveProtocol(int socketfd);

// TCP related methods and definitions
int connectClient(string name, string srvrAdd, int srvrPort);
bool upload(int socketfd, File * file);
void writeFile(string data, int socket, string path);

string serializeFile(File* file);
File* deserializeFile(string message);
string serializePack(vector<File *> pack);
std::vector<File *> deserializePack(string message);
