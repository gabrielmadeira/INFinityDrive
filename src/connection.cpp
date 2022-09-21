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

  int chunk = 0;
  int msgsize = size + (BUFFER_SIZE - size % BUFFER_SIZE);
  int total_chunks = msgsize / BUFFER_SIZE;

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

bool sendProtocol(int socketfd, string message, PROTOCOL_TYPE type)
{
  Protocol buffer;
  int msgsize = message.size() + (PAYLOAD_SIZE - message.size() % PAYLOAD_SIZE);
  message.resize(msgsize, '\0');
  buffer.total_chunks = msgsize / PAYLOAD_SIZE;
  const char *bufmsg = message.c_str();
  cout << "message send: " << message << endl;
  for (int i = 0; i < buffer.total_chunks; i++)
  {
    buffer.type = type;
    buffer.chunk = i + 1;
    strncpy(buffer.payload, &bufmsg[i * PAYLOAD_SIZE], PAYLOAD_SIZE);
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
  int nBytes;
  do
  {
    nBytes = recv(socketfd, &buffer, BUFFER_SIZE, 0);
    cout << "bytes read: " << nBytes << endl;
    if (nBytes < 0)
    {
      printf("%d ERROR reading from socket\n", socketfd);
      break;
      ;
    }
    if (nBytes == 0)
    {
      printf("%d Disconnected\n", socketfd);
      break;
    }

    message += buffer.payload;
  } while (buffer.chunk < buffer.total_chunks);
  cout << "message: " << message << endl;
  if (nBytes <= 0)
  {
    // Error or disconnected
    return make_tuple(ERRO, "");
  }
  return make_tuple(buffer.type, message);
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