#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <pthread.h>
#include <chrono>
#include <thread>
#include "client.hpp"
#include "connection.hpp"
#include "file.hpp"

#include <dirent.h>
#define Define_CurrentDir getcwd


using namespace std;

Client::Client(string username, string srvrAdd, int srvrPort)
{
    name = username;
    this->srvrAdd = srvrAdd;
    this->srvrPort = srvrPort;

    getSyncDir();
}

Client::~Client() { delete (syncdir); }

void Client::getSyncDir()
{
    //filesystem::path filepath = filesystem::current_path();
    string fpath;
    tProtocol sync_tuple;
    File *file;
    char LOCAL_DIR[1000];

    if (!Define_CurrentDir(LOCAL_DIR, sizeof(LOCAL_DIR)))
             {
             	return;
             }
         
    else  
       { 
            fpath = string(LOCAL_DIR) + "/sync_dir";
              cout << "Directory" << fpath <<endl;
            if(mkdir(fpath.c_str(), 0777) == -1)
                 //throw runtime_error("Failed to create clients directory or directory already exists");
                cout << "Failed to create clients directory or directory already exists" << endl;
            else
                cout << "Directory clients created" << endl;
        }

    // Try connection
    socketfd = connectClient(name, srvrAdd, srvrPort);
    if (socketfd == -1){
        cout << "Couldn't connect to server" << endl;
        return;
    }
    if (!sendProtocol(socketfd, name, LOGN))
        cout << "Couldn't send user info" << endl;

    cout << "Client Connected" << endl;
    // Connection ok

    syncdir = new SyncDir("./sync_dir");

    if (pthread_create(&syncDirID, NULL,
                       syncDirLoop, NULL) != 0)
        printf("Failed to create SyncDir thread\n");

    if (pthread_create(&clientID, NULL,
                       clientLoop, NULL) != 0)
        printf("Failed to create ClientLoop thread\n");
}

void *Client::syncDirLoop(void *param)
{
    while (true)
    {
        vector<pair<string, int>> diff = syncdir->sync();
        if (serverUpdate)
        {
            serverUpdate = false;
            continue;
        }

        for (pair<string, int> file : diff)
        {
            string name = "./sync_dir/" + file.first;
            switch (file.second)
            {
            case MODIFIED:
                cout << "File " << file.first << " modified" << endl;
                uploadFile(name);
                break;
            case DELETED:
                cout << "File " << file.first << " deleted" << endl;
                if (!sendProtocol(socketfd, file.first, DELT))
                    cout << "Failed to delete file";
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

void *Client::clientLoop(void *param)
{
    protocol buffer;
    int n;
    while (1)
    {
        tProtocol message = receiveProtocol(socketfd);

        if (get<0>(message) == ERRO)
            break;

        switch (get<0>(message))
        {
        case UPLD:
            writeFile(get<1>(message), socketfd, "./sync_dir/");
            serverUpdate = true;
            break;
        case DWNL:
            writeFile(get<1>(message), socketfd, "./");
            serverUpdate = true;
            break;
        case DELT:
            serverUpdate = true;
            deleteLocal(get<1>(message));
            break;
        case LSSV:
            getServerList(get<1>(message));
            break;
        default:
            cout << "Spooky behavior!" << endl;
        }
    }
    return nullptr;
}
void Client::uploadFile(string filepath, int forcePropagation)
{
    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;
    File file(filepath);

    if (!upload(socketfd, &file, filepath, forcePropagation))
        cout << "Failed to send file to server";
}

void Client::downloadFile(string filename)
{
    if (filename.empty())
        cout << "Couldn't understand filename" << endl;

    if (!sendProtocol(socketfd, filename, DWNL))
        cout << "Failed to download file";
}

void Client::deleteFile(string filepath)
{
    deleteLocal(filepath);

    if (!sendProtocol(socketfd, filepath, DELT))
        cout << "Failed to delete file";
}

void Client::deleteLocal(string filepath)
{
    string fullFilepath;
    //filesystem::current_path().string() + "/sync_dir/" + filepath;
    char LOCAL_DIR[1000];

    if (!Define_CurrentDir(LOCAL_DIR, sizeof(LOCAL_DIR)))
         {
            	return ;
         }
    fullFilepath= string(LOCAL_DIR) + "/sync_dir/" + filepath;

    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;
    else if (remove(fullFilepath.c_str()) != 0)
        cout << "Couldn't delete file" << endl;
}

void Client::listServer()
{
    string msg = "LIST";
    tProtocol listtuple;
    if (!sendProtocol(socketfd, msg, LSSV))
        cout << "Failed to communicate with server" << endl;
}

void Client::getServerList(string message)
{
    std::vector<File *> files = deserializePack(message);

    for (File *file : files) // Iterates through each file
    {
        cout << file->name << "\t\t" << file->mod_time << "\t\t" << file->acc_time << "\t\t" << file->chg_time << endl;
    }
}

void Client::listClient()
{
    string fullFilepath;  //= filesystem::current_path().string() + "/sync_dir";

    vector<File *> files = syncdir->getFiles();

    char LOCAL_DIR[1000];

    if (!Define_CurrentDir(LOCAL_DIR, sizeof(LOCAL_DIR)))
         {
            	return;
         }
    fullFilepath= string(LOCAL_DIR) + "/sync_dir/";

    SyncDir syncdir = SyncDir(fullFilepath);
    
    cout << "Name\t"
         << "\t\t\tLast Modified\t"
         << "\t\tLast Acessed\t"
         << "\t\tLast changed" << endl;
    for (File *file : files) // Iterates through each file
    {
        cout << file->name << "\t\t" << file->mod_time << "\t\t" << file->acc_time << "\t\t" << file->chg_time << endl;
    }
}
