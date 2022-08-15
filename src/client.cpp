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

Client::~Client() { delete(syncdir); }

void Client::getSyncDir()
{
    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/sync_dir";
    tProtocol sync_tuple;
    File* file;

    if(!filesystem::exists(fpath))
    {
        if(mkdir(fpath.c_str(), 0777) == -1)
            throw runtime_error("Failed to create sync_dir directory");
        else
            cout << "Directory sync_dir created" << endl;
    }

    //Try connection
    socketfd = connectClient(name, srvrAdd, srvrPort); 
    if(socketfd == -1)
        throw runtime_error("Couldn't connect to server");
    if (!sendProtocol(socketfd, name, LOGN))
        throw runtime_error("Couldn't send user info");

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
            cout <<  file.first << " || " << file.second << endl;
            string name = "./sync_dir/" + file.first;
            switch(file.second) {
                case MODIFIED:
                    cout << "File " << file.first << " modified" << endl;
                    uploadFile(name);
                    break;
                case DELETED:
                    cout << "File " << file.first << " deleted" << endl;
                    if(!sendProtocol(socketfd,file.first,DELT))
                        throw runtime_error("Failed to delete file");
                    break;
                case CREATED:
                    cout << "File " << file.first << " created" << endl;
                    uploadFile(name);
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

void * Client::clientLoop(void * param) {
    protocol buffer;
    int n;
    while (1)
    {
        tProtocol message = receiveProtocol(socketfd);

        switch (get<0>(message))
        {
            case UPLD:
            case DWNL: writeFile(get<1>(message));   break;
            case DELT: deleteLocal(get<1>(message)); break;
            case LSSV: getServerList(get<1>(message));       break;
            default:   cout << "Spooky behavior!" << endl;
        }
    }
    return nullptr;
}
void Client::uploadFile(string filepath) 
{
    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;
    File file(filepath);

    if (!upload(socketfd, &file))
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
    deleteLocal(filepath);
    
    if(!sendProtocol(socketfd,filepath,DELT))
        throw runtime_error("Failed to delete file");
}

void Client::deleteLocal(string filepath)
{
    string fullFilepath = filesystem::current_path().string() +"/sync_dir/" + filepath;
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;
    else if(remove(fullFilepath.c_str()) != 0)
        cout << "Couldn't delete file" << endl;
}

void Client::listServer()
{
  string msg = "LIST";
  tProtocol listtuple;
  if(!sendProtocol(socketfd, msg, LSSV))
    throw runtime_error("Failed to communicate with server");
}

void Client::getServerList(string message) {
    // tProtocol ;

    std::vector<File *> files = deserializePack(message);

    for(File * file : files) //Iterates throw each file
    {
        cout << file->name << "\t\t" << file->mod_time << "\t\t" << file->acc_time << "\t\t" << file->chg_time << endl;
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
        cout << file->name << "\t\t" << file->mod_time << "\t\t" << file->acc_time << "\t\t" << file->chg_time << endl;
    }
}
