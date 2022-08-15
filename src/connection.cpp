#include <netdb.h>
#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <filesystem>
#include <unistd.h>
#include <strings.h>
#include "connection.hpp"

#define PORT 4000

using namespace std;

string serializeFile(File* file)
{
  stringstream filebuffer;
  filebuffer << file->name << '|' << file->acc_time << '|' << file->chg_time << '|' << file->mod_time << '|' << file->data << '|';
  return filebuffer.str();
}

File* deserializeFile(string message)
{
  File *file;
  stringstream mstream(message);
  getline(mstream, file->name, '|');
  mstream >> file->acc_time;
  mstream.seekg(ios::cur + 1);
  mstream >> file->chg_time;
  mstream.seekg(ios::cur + 1);
  mstream >> file->mod_time;
  mstream.seekg(ios::cur+1); 
  mstream >> file->data;
  return file;
}

int connectClient(string name, string srvrAdd, int srvrPort)
{
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int socketfd, res;

  server = gethostbyname(srvrAdd.c_str());
  if (!server)
    return -1;

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if ((socketfd) == -1)
    return -1;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(srvrPort);
  serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serv_addr.sin_zero), 8);

  res = connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (res == -1)
    return -1;

  return socketfd;
}

bool upload(int socketfd, File &file)
{
  string msg = file.name + '|';
  /*
  if (!sendProtocol(socketfd, msg, UPLD))
    return false;*/
  if (!sendProtocol(socketfd, serializeFile(&file), UPLD))
    return false;
  return true;
}

File *download(int socketfd, string filename)
{
  Protocol buffer;
  tProtocol filetuple;
  File *file;
  string msg = filename + '|';
  if (!sendProtocol(socketfd, msg, DWNL))
    return NULL;
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
  buffer.total_chunks = msgsize / PAYLOAD_SIZE;
  cout << "MESSAGE SENT: " << message << "\n";
  cout << "TOTAL CHUNKS: " << buffer.total_chunks << "\n";
  const char *bufmsg = message.c_str();
  for (int i = 0; i < buffer.total_chunks; i++)
  {
    buffer.type = type;
    buffer.chunk = i + 1;
    // cout << "MESSAGE SUBSTR: " << message.substr(i * PAYLOAD_SIZE, PAYLOAD_SIZE).c_str() << "\n";
    // const char *bufmsg = message.substr(i * PAYLOAD_SIZE, PAYLOAD_SIZE).c_str();
    // cout << "MESSAGE BUFMSG: " << bufmsg << "\n";
    strncpy(buffer.payload, &bufmsg[i * PAYLOAD_SIZE], PAYLOAD_SIZE);
    cout << "BUFFER_PAYLOAD: " << buffer.payload << "\n";
    if (send(socketfd, &buffer, BUFFER_SIZE, 0) == -1)
      return false;
  }
  return true;
}

tProtocol
receiveProtocol(int socketfd)
{
  Protocol buffer;
  string message;

  do
  {
    recv(socketfd, &buffer, BUFFER_SIZE, 0);
    cout << "CHUNK: " << buffer.chunk << "\n";
    cout << "TOTAL CHUNKS: " << buffer.total_chunks << "\n";
    cout << "BUFFER PAYLOAD: " << buffer.payload << "\n";

    message += buffer.payload;
  } while (buffer.chunk < buffer.total_chunks);
  cout << "MESSAGE RECEIVED: " << message << "\n";
  return make_tuple(buffer.type, message);
}

vector<File *> deserializePack(string message)
{ 
  size_t pos = 0;
  string arquivo, nome, dado, delimiter = "||";

  vector<File *> files;

  while ((pos = message.find(delimiter)) != std::string::npos) {
    File * file;
    stringstream stream(message.substr(0, pos));
    getline(stream, file->name, '|');
    getline(stream, file->data, '|');
    files.push_back(file);
    message.erase(0, pos + delimiter.length());
  }
  return files;
}

string serializePack(vector<File *> pack)
{ 
  string message;
  for(File* file: pack)
  {
    message += serializeFile(file);
    message += "||";
  }
  return message;
}
/*
bool getSyncDir(int socketfd)
{
  string filepath = filesystem::current_path().string() +"/sync_dir";
  tProtocol sync_tuple;
  File* file;

  if(!sendProtocol(socketfd, "|", GSDR)) return false;

  if (mkdir(filepath.c_str(), 0777) == -1) 
    cerr << "Error :  " << strerror(errno) << endl;
  else cout << "sync_dir folder created";

  sync_tuple = receiveProtocol(socketfd); 
  return deserializePack(get<1>(sync_tuple), filepath);
}
*/
bool listServer(int socketfd,string username)
{
  tProtocol listtuple;
  if(!sendProtocol(socketfd, "|", LSSV)) return false;

  listtuple = receiveProtocol(socketfd);
  //lista todos os nomes no terminal
  string file;
  stringstream list(get<1>(listtuple));
  while(getline(list, file, '|'))
  {
    int indent = 0;
    while(getline(list, file, '/'))
    {
      std::cout << std::string(indent, ' ') << file << std::endl;
      indent++;
    }
  }
  return true;
}