#pragma once
#include <string>
#include <tuple>
#include "file.hpp"

enum PROTOCOL_TYPE : short { DATA, UPLD, UPLF, DWNL, DELT, LSSV, LSCL, GSDR, LOGN, CLT, ERRO };
static const char* PROTOCOL_NAME[] = { "DATA", "UPLD", "UPLF", "DWNL", "DELT", "LSSV", "LSCL", "GSDR", "LOGN", "CLT", "ERRO" };
inline static int PAYLOAD_SIZE = 256;

using tProtocol = tuple<PROTOCOL_TYPE, string>;

void sendFile(string path, int socket);
void receiveFile(string path, int socket, int size);
bool sendProtocol(int socketfd, string message, PROTOCOL_TYPE type);
tProtocol receiveProtocol(int socketfd);

// TCP related methods and definitions
int connectClient(string name, string srvrAdd, int srvrPort);
bool upload(int socketfd, File *file, string path, int forcePropagation = 0);
void writeFile(string data, int socket, string path);

string serializeFile(File *file);
File *deserializeFile(string message);
string serializePack(vector<File *> pack);
std::vector<File *> deserializePack(string message);
