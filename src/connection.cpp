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
  // filebuffer << file->name << '|' << file->acc_time << '|' << file->chg_time << '|' << file->mod_time;
  filebuffer << file->name << '|' << file->acc_time << '|' << file->chg_time << '|' << file->mod_time << '|' << file->size;
  return filebuffer.str();
}

File *deserializeFile(string message)
{
  File *file = new File();
  /*
  vector<string> fullstream;
  cout << message << endl;

  stringstream mstream(message);
  while(mstream.good()) {
    string substr;
    getline(mstream, substr, '|');
    fullstream.push_back(substr);
  }
  file->name = fullstream.at(0);
  file->acc_time = fullstream.at(1);
  file->chg_time = fullstream.at(2);
  file->mod_time = fullstream.at(3);
  file->data = fullstream.at(4);
  */

  stringstream mstream(message);
  getline(mstream, file->name, '|');
  getline(mstream, file->acc_time, '|');
  getline(mstream, file->chg_time, '|');
  getline(mstream, file->mod_time, '|');
  string temp;
  getline(mstream, temp, '|');
  file->size = stoi(temp);
  /*
  mstream >> file->acc_time;
  mstream.seekg(ios::cur + 1);
  mstream >> file->chg_time;
  mstream.seekg(ios::cur + 1);
  mstream >> file->mod_time;
  mstream.seekg(ios::cur+1);
  mstream >> file->data;
  */
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

void sendFile(string path, int socket)
{
  cout << "READ " << path << endl;
  if (FILE *fp = fopen(path.c_str(), "rb"))
  {
    cout << "READING........." << endl;
    size_t readBytes;
    char buffer[4096];
    while ((readBytes = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
      // cout << readBytes << "\n";
      // cout << buffer << "\n";
      if (send(socket, buffer, readBytes, 0) != readBytes)
      {
        cout << "OUTTT READ\n";
        // handleErrors();
        // break;
        return;
      }
    }
    send(socket, buffer, 0, 0);
    // close(socketfd);
  }
}

void receiveFile(string path, int socket, int size)
{
  int BUFFER_SIZE = 4096;

  int chunk = 0;
  int msgsize = size + (BUFFER_SIZE - size % BUFFER_SIZE);
  int total_chunks = msgsize / BUFFER_SIZE;

  cout << "WRITE" << path << " SIZE: " << size << endl;
  if (FILE *fp = fopen(path.c_str(), "wb"))
  {
    cout << "WRITING.............\n";
    size_t readBytes;
    char buffer[BUFFER_SIZE];
    while ((readBytes = recv(socket, buffer, sizeof(buffer), 0)) > 0)
    {
      // cout << buffer << "\n";
      if (fwrite(buffer, 1, readBytes, fp) != readBytes)
      {
        cout << "OUTTT WRITE\n";
        // handleErrors();
        // break;
        return;
      }
      chunk++;
      if (chunk >= total_chunks)
      {
        break;
      }
    }
    cout << "WRITE CLOSE\n";
    fclose(fp);
  }
}

bool upload(int socketfd, File *file)
{
  // string msg = file->name + '|' + file->acc_time + '|' + file->chg_time + '|' + file->mod_time + '|';

  if (!sendProtocol(socketfd, serializeFile(file) + '|', UPLD))
    return false;

  // SEND ----------------------------- TEST

  string path = "./sync_dir/" + file->name;
  cout << "SEND FILE SIZE: " << file->size << endl;
  sendFile(path, socketfd);

  // ---------------------------------

  // if (!sendProtocol(socketfd, file->data, DATA))
  //   return false;

  return true;
}

void writeFile(string data, int socket, string path)
{
  File *file = deserializeFile(data);

  // tProtocol filedata = receiveProtocol(socket);

  path += file->name;
  receiveFile(path, socket, file->size);
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
  int nBytes;
  do
  {
    nBytes = recv(socketfd, &buffer, BUFFER_SIZE, 0);
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

    cout << "CHUNK: " << buffer.chunk << "\n";
    cout << "TOTAL CHUNKS: " << buffer.total_chunks << "\n";
    cout << "BUFFER PAYLOAD: " << buffer.payload << "\n";

    message += buffer.payload;
  } while (buffer.chunk < buffer.total_chunks);
  if (nBytes <= 0)
  {
    // Error or disconnected
    return make_tuple(ERRO, "");
  }

  cout << "MESSAGE RECEIVED: " << message << "\n";
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
bool listServer(int socketfd, string username)
{
  tProtocol listtuple;
  if (!sendProtocol(socketfd, "|", LSSV))
    return false;

  listtuple = receiveProtocol(socketfd);
  // lista todos os nomes no terminal
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