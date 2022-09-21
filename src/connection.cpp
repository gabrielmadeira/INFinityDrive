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

string serializeFile(File *file)
{
  stringstream filebuffer;
  filebuffer << file->name << '|' << file->acc_time << '|' << file->chg_time << '|' << file->mod_time << '|' << file->size;
  return filebuffer.str();
}

File *deserializeFile(string message)
{
  File *file = new File();

  stringstream mstream(message);
  getline(mstream, file->name, '|');
  getline(mstream, file->acc_time, '|');
  getline(mstream, file->chg_time, '|');
  getline(mstream, file->mod_time, '|');
  string temp;
  getline(mstream, temp, '|');
  file->size = stoi(temp);
  return file;
}

int connectClient(string name, string srvrAdd, int srvrPort)
{
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int socketfd, res;
	const int opt = 1;

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

	setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

  res = connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (res == -1)
    return -1;

  return socketfd;
}

void sendFile(string path, int socket)
{
  cout << "file: " << path << endl;
  if (FILE *fp = fopen(path.c_str(), "rb"))
  {
    size_t readBytes;
    char buffer[256];
    while ((readBytes = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
      if (send(socket, buffer, readBytes, 0) != readBytes)
      {
        return;
      }
    }
    send(socket, buffer, 0, 0);
    fclose(fp);
  }
}

void receiveFile(string path, int socket, int size)
{
  int BUFFER_SIZE = 256;

  int totalReadyBytes = 0;
  if (FILE *fp = fopen(path.c_str(), "wb"))
  {
    size_t readBytes;
    char buffer[BUFFER_SIZE];
    while ((readBytes = recv(socket, buffer, sizeof(buffer), 0)) > 0)
    {
      if (fwrite(buffer, 1, readBytes, fp) != readBytes)
      {
        return;
      }
      totalReadyBytes += readBytes;
      if (totalReadyBytes >= size)
      {
        break;
      }
    }
    fclose(fp);
  }
}

bool sendProtocol(int socketfd, string message, PROTOCOL_TYPE type)
{
  string bufmsg;
  const char *buffer;
  message = message + "|" + to_string(type); cout << "+|" << message << endl;
  bufmsg = message.substr(0, PAYLOAD_SIZE); cout << "bufmsg " << bufmsg << endl;
  message = message.erase(0, PAYLOAD_SIZE); cout << "message " << bufmsg << endl;
  while (bufmsg.size())
  {
    buffer = bufmsg.c_str(); cout << "buffer: "<<  buffer << endl;
    if (send(socketfd, buffer, bufmsg.size(), 0) != bufmsg.size())
      return false;
    bufmsg = message.substr(0, PAYLOAD_SIZE);
    message = message.erase(0, PAYLOAD_SIZE);
  }
  send(socketfd, buffer, 0, 0);
  return true;
}

tProtocol
receiveProtocol(int socketfd)
{
  string message;
  int totalReadyBytes = 0, size = 0;
  size_t readBytes;
  char buffer[PAYLOAD_SIZE];
  while ((readBytes = recv(socketfd, buffer, sizeof(buffer), 0)) > 0)
  {
    totalReadyBytes += readBytes;
    message += buffer;
    cout << "totalread: "<< totalReadyBytes << endl;
    cout << "message: "<< message << endl;
    if(!size)
    {
      cout << "message: "<< message << endl;
      size = stoi(message.substr(0, message.find("|")));
      message.erase(0, message.find("|") + 1);
      totalReadyBytes = message.size();
    }
    if (totalReadyBytes >= size)
      break;
  }
  if ((readBytes < 0) || ((totalReadyBytes < size) && !readBytes))
    return make_tuple(ERRO, "");
  PROTOCOL_TYPE type = static_cast<PROTOCOL_TYPE>(message[message.size()-1] - '0');
  return make_tuple(type, message.substr(0, message.size()-2));
}

bool upload(int socketfd, File *file, string path, int forcePropagation)
{
  PROTOCOL_TYPE protocol = forcePropagation ? UPLF : UPLD;

  if (!sendProtocol(socketfd, serializeFile(file) + '|', protocol))
    return false;

  sendFile(path, socketfd);
  return true;
}

void writeFile(string data, int socket, string path)
{
  File *file = deserializeFile(data);

  path += file->name;
  receiveFile(path, socket, file->size);
}

vector<File *> deserializePack(string message)
{
  size_t pos = 0;
  string arquivo, nome, dado, delimiter = "||";

  vector<File *> files;

  while ((pos = message.find(delimiter)) != std::string::npos)
  {
    File *file = new File();
    stringstream stream(message.substr(0, pos));
    getline(stream, file->name, '|');
    getline(stream, file->acc_time, '|');
    getline(stream, file->chg_time, '|');
    getline(stream, file->mod_time, '|');
    files.push_back(file);
    message.erase(0, pos + delimiter.length());
  }
  return files;
}

string serializePack(vector<File *> pack)
{
  string message;
  for (File *file : pack)
  {
    message += serializeFile(file);
    message += "||";
  }
  return message;
}

bool listServer(int socketfd, string username)
{
  tProtocol listtuple;
  if (!sendProtocol(socketfd, "|", LSSV))
    return false;

  listtuple = receiveProtocol(socketfd);

  string file;
  stringstream list(get<1>(listtuple));
  while (getline(list, file, '|'))
  {
    int indent = 0;
    while (getline(list, file, '/'))
    {
      std::cout << std::string(indent, ' ') << file << std::endl;
      indent++;
    }
  }
  return true;
}