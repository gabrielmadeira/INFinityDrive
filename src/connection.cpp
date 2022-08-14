#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include  <bits/stdc++.h>
#include "connection.hpp"

#define PORT 4000

string serializeFile(File & file)
{
  stringstream filebuffer;
  filebuffer << file.data << '|' << file.acc_time 
             << '|' << file.chg_time << '|' 
             << file.mod_time << '|';
  return filebuffer.str();
}

File* deserializeFile(string message)
{
  File* file;
  stringstream mstream(message);
  getline(mstream, file->name, '|');
  mstream >> file->acc_time;
  mstream.seekg(ios::cur+1); 
  mstream >> file->chg_time;
  mstream.seekg(ios::cur+1); 
  mstream >> file->mod_time;
  mstream.seekg(ios::cur+1); 
  mstream >> file->data;
  return file;
}

bool deserializePack(string message, string filepath)
{ 
   //parser string nome| dado 
  stringstream pack(message), ss;
  string arquivo;

  while(std::getline(ss, arquivo, '|')) {
    //cout << arquivo<< '\n';
      File file;

      std::getline(ss, file.name, '|');
      getline(ss, file.data, '|');
      file.write(filepath + file.name);
  }
}

int connectClient(string name, string srvrAdd, int srvrPort)
{
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int socketfd, res;

  server = gethostbyname(srvrAdd.c_str());
  if (!server) return -1;
  
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if((socketfd) == -1) return -1;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(srvrPort);
  serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serv_addr.sin_zero), 8);

  res = connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (res == -1) return -1;

  return socketfd;
}

bool upload(int socketfd, File & file)
{
  string msg = file.name + '|';
  if(!sendProtocol(socketfd, msg, UPLD))                 return false;
  if(!sendProtocol(socketfd, serializeFile(file), DATA)) return false;
  return true;
}

File* download(int socketfd, string filename)
{
  Protocol buffer;
  tuple<PROTOCOL_TYPE, string> filetuple;
  File* file;
  string msg = filename + '|';
  if(!sendProtocol(socketfd, msg, DWNL)) return NULL;
  filetuple = receiveProtocol(socketfd);
  file = deserializeFile(get<1>(filetuple));
  file->name = filename;
  return file;
}

void writeFile(string data) {
  File * file = deserializeFile(data);
  file->write("./sync_dir/" + file->name);
}

bool sendProtocol(int socketfd, string message, PROTOCOL_TYPE type)
{
  Protocol buffer;
  int msgsize = message.size() + (PAYLOAD_SIZE - message.size() % PAYLOAD_SIZE);
  message.resize(msgsize, '\0');
  buffer.total_chunks = msgsize/PAYLOAD_SIZE;

  for(int i = 0; i < buffer.total_chunks; i++)
  {
      buffer.type = type;
      buffer.chunk = i+1;
      const char* bufmsg = message.substr(i*PAYLOAD_SIZE, PAYLOAD_SIZE).c_str();
      strcpy(buffer.payload, bufmsg);
      if(send(socketfd, &buffer, BUFFER_SIZE, 0) == -1)
          return false;
  }

  return true;
}

tuple<PROTOCOL_TYPE, string> 
receiveProtocol(int socketfd)
{
  Protocol buffer;
  string message;

  do {
      recv(socketfd, &buffer, BUFFER_SIZE, 0);
      message += buffer.payload;
  } while(buffer.chunk < buffer.total_chunks);
  return make_tuple(buffer.type, message);
}