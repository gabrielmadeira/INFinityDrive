#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <filesystem>
#include <pthread.h>
#include <chrono>
#include <thread>
#include "client.hpp"
#include "connection.hpp"
#include "file.hpp"


using namespace std;

Client::Client(string username, string srvrAdd, int srvrPort)
{
    name = username; 
    this->srvrAdd = srvrAdd;
    this->srvrPort = srvrPort;
    
    getSyncDir();
}

Client::~Client() 
{
    delete(syncdir);
}

void Client::getSyncDir()
{
    string filepath = filesystem::current_path().string() +"/sync_dir";
    tuple<PROTOCOL_TYPE, string> sync_tuple;
    File* file;

    //cria diretório
    if (mkdir(filepath.c_str(), 0777) == -1)//falta somar filpath
        throw runtime_error("Failed to create sync_dir directory");
    else
        cout << "Directory sync_dir created";

    //Try connection
    socketfd = connectClient(name, srvrAdd, srvrPort); 
    if(socketfd == -1)
        throw runtime_error("Couldn't connect to server");
    string msg = name+'|';
    if (!sendProtocol(socketfd, msg, LOGN))
        throw runtime_error("Couldn't send machine info");

    //Connection ok

    syncdir = new SyncDir("./sync_dir");

    if (pthread_create(&syncDirID, NULL,
                        syncDirLoop, NULL) != 0)
        printf("Failed to create SyncDir thread\n");

    if (pthread_create(&clientID, NULL,
                        clientLoop, NULL) != 0)
        printf("Failed to create ClientLoop thread\n");
}
void * Client::syncDirLoop(void * param) {
    while(true)
    {
        vector<pair<string,int>> diff = syncdir->sync();

        for(pair<string,int> file : diff)
        {
            string name = "./sync_dir/" + file.first;
            switch(file.second) {
                case MODIFIED:
                    cout << "File " << file.first << " was created" << endl;
                    uploadFile(name);
                    break;
                case DELETED:
                    cout << "File " << file.first << " was deleted" << endl;
                    if(!sendProtocol(socketfd,file.first + '|',DELT))
                        throw runtime_error("Failed to delete file");
                    break;
                case CREATED:
                    cout << "File " << file.first << " was created" << endl;
                    uploadFile(name);
                    break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
}

void * Client::clientLoop(void * param) {
    // TODO: destruir UserConnection ao desconectar ou ocorrer erro

    protocol buffer;
    int n;
    while (1)
    {
        tuple<PROTOCOL_TYPE, string> message = receiveProtocol(socketfd);
        // if (n < 0)                                                                   adicionar no receive
        // {                                        
        //     printf("%d ERROR reading from socket\n", info.socket);
        //     break;
        // }
        // if (n == 0)
        // {
        //     printf("%d Disconnected\n", info.socket);
        //     break;
        // }

        // CMDs in the format "|parameter1|parameter2|etc|"
        switch (get<0>(message))
        {
        case DATA:
        case UPLD:
        case DWNL:  writeFile(get<1>(message));  break;
        case DELT:  deleteFile(get<1>(message)); break;
        case LSSV:  getServerList();             break;
        case LSCL:  
        case GSDR:
        case DVCE:
        default:    cout << "Spooky behavior!" << endl;
        }
    }
    return nullptr;
}
void Client::uploadFile(string filepath) 
{
    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;
    cout << filepath << "\n";
    File file(filepath);

    if (upload(socketfd, file))
        throw runtime_error("Failed to send file to server");
}

void Client::downloadFile(string filepath)
{
    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;

    File *newfile = (File *)download(socketfd, filepath);
    if (!newfile)
        cout << "No file found with name " + filepath << endl;
    newfile->write(filepath);
}

void Client::deleteFile(string filepath)
{
    string fullFilepath = filesystem::current_path().string() +"/sync_dir/" + filepath;
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;
    else if(remove(fullFilepath.c_str()) != 0)
    {
        cout << "Couldn't delete file" << endl;
    }
}

void Client::listClient()
{
    string fullFilepath = filesystem::current_path().string() +"/sync_dir";
    SyncDir syncdir = SyncDir(fullFilepath);
    vector<File *> files = syncdir.getFiles();

    cout << "Name\t" << "\t\t\tLast Modified\t" << "\t\tLast Acessed\t" << "\t\tLast changed" << endl;
    for(File * file : files) //Iterates throw each file
    {
        //Displays its metadata
        tm * mod = localtime(&file->mod_time);
        tm * acc = localtime(&file->acc_time);
        tm * chg = localtime(&file->chg_time);
        char buffer[3][32];
        // Format: Mo, 15.06.2009 20:20:00
        strftime(buffer[0], 32, "%a, %d.%m.%Y %H:%M:%S", mod);
        strftime(buffer[1], 32, "%a, %d.%m.%Y %H:%M:%S", acc);
        strftime(buffer[2], 32, "%a, %d.%m.%Y %H:%M:%S", chg);

        cout << file->name << "\t\t" << buffer[0] << "\t\t" << buffer[1] << "\t\t" << buffer[2] << endl;
    }
}

void Client::listServer()
{
  string msg = "LIST";
  tuple<PROTOCOL_TYPE, string> listtuple;
  if(!sendProtocol(socketfd, msg, LSSV))
    throw runtime_error("Failed to send file to server");

  listtuple = receiveProtocol(socketfd);
  //lista todos os nomes no terminal
  cout<< "Arquivos do seu diretório:" << get<1>(listtuple) << endl;
}

void Client::getServerList() {
    //TODO colocar while aqui
}

void Client::getSyncDir() 
{
     if(getSyncDir(socketfd)) 
        throw runtime_error("Failed to send file to server");
}

void Client::listClient()
{
    string fullFilepath = filesystem::current_path().string() +"/sync_dir";
    SyncDir syncdir = SyncDir(fullFilepath);
    vector<File *> files = syncdir.getFiles();

    cout << "Name\t" << "\t\t\tLast Modified\t" << "\t\tLast Acessed\t" << "\t\tLast changed" << endl;
    for(File * file : files) //Iterates throw each file
    {
        //Displays its metadata
        tm * mod = localtime(&file->mod_time);
        tm * acc = localtime(&file->acc_time);
        tm * chg = localtime(&file->chg_time);
        char buffer[3][32];
        // Format: Mo, 15.06.2009 20:20:00
        strftime(buffer[0], 32, "%a, %d.%m.%Y %H:%M:%S", mod);
        strftime(buffer[1], 32, "%a, %d.%m.%Y %H:%M:%S", acc);
        strftime(buffer[2], 32, "%a, %d.%m.%Y %H:%M:%S", chg);

        cout << file->name << "\t\t" << buffer[0] << "\t\t" << buffer[1] << "\t\t" << buffer[2] << endl;
    }
}
