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

// inet_addr
#include <arpa/inet.h>

using namespace std;

Client::Client(string username, int clientPort, string srvrAdd, int srvrPort)
{
	const int opt = 1;
    name = username;
    this->srvrAdd = srvrAdd;
    this->srvrPort = srvrPort;
    this->port = clientPort;

    this->tempClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    this->tempClientAddr.sin_addr.s_addr = INADDR_ANY;
    this->tempClientAddr.sin_family = AF_INET;
    this->tempClientAddr.sin_port = htons(this->port);
	setsockopt(this->tempClientSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bind(this->tempClientSocket,
         (struct sockaddr *)&(this->tempClientAddr),
         sizeof(this->tempClientAddr));

    this->addr_size = sizeof(this->tempClientStorage);

    if (listen(this->tempClientSocket, 50) == 0)
        printf("Listening\n");
    else
        printf("Error\n");

    getSyncDir();
}

Client::~Client() { delete (syncdir); }

void Client::getSyncDir()
{
    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/sync_dir";
    tProtocol sync_tuple;
    File *file;

    if (!filesystem::exists(fpath))
    {
        if (mkdir(fpath.c_str(), 0777) == -1)
        {
            cout << "Failed to create sync_dir directory";
            return;
        }
        else
            cout << "Directory sync_dir created" << endl;
    }

    // Try connection
    socketfd = connectClient(name, srvrAdd, srvrPort);
    if (socketfd == -1)
    {
        cout << "Couldn't connect to server" << endl;
        return;
    }
    if (!sendProtocol(socketfd, name, LOGN))
        cout << "Couldn't send user info" << endl;

    sendProtocol(socketfd, to_string(port), DATA);

    cout << "Client Connected" << endl;
    // Connection ok
    connected = 1;

    syncdir = new SyncDir("./sync_dir");

    if (!syncDirLoopActive)
    {
        if (pthread_create(&syncDirID, NULL,
                           syncDirLoop, this) != 0)
            printf("Failed to create SyncDir thread\n");
        syncDirLoopActive = 1;
    }
    if (pthread_create(&clientID, NULL,
                       clientLoop, this) != 0)
        printf("Failed to create ClientLoop thread\n");
}

void *Client::syncDirLoop(void *param)
{
    Client *ref = (Client *)param;
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
        while (!ref->connected)
        { // PRIMARY BROKE
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
    }
}

void *Client::clientLoop(void *param)
{
    Client *ref = (Client *)param;
    while (1)
    {
        tProtocol message = receiveProtocol(socketfd);

        if (get<0>(message) == ERRO)
        {
            // PRIMARY BROKE
            // wait for new leader and then call getSyncDir
            ref->connected = 0;

            cout << "Waiting for new leader connection\n";
            int newLeaderSocket = accept(ref->tempClientSocket,
                                         (struct sockaddr *)&(ref->tempClientStorage),
                                         &(ref->addr_size));

            struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&(ref->tempClientStorage);
            struct in_addr ipAddr = pV4Addr->sin_addr;

            char ipNewLeader[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipAddr, ipNewLeader, INET_ADDRSTRLEN);

            ref->srvrAdd = ipNewLeader;
            cout << "Connection received, waiting for new leader port...\n";
            tProtocol newLeaderMessage = receiveProtocol(newLeaderSocket);
            ref->srvrPort = stoi(get<1>(newLeaderMessage));
            cout << "Received new leader: " << ref->srvrPort << "\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(5000));

            ref->getSyncDir();

            break;
            // clientLoop thread ends here and start again by getSyncDir
        }

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
    string fullFilepath = filesystem::current_path().string() + "/sync_dir/" + filepath;
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
    string fullFilepath = filesystem::current_path().string() + "/sync_dir";
    SyncDir syncdir = SyncDir(fullFilepath);
    vector<File *> files = syncdir.getFiles();

    cout << "Name\t"
         << "\t\t\tLast Modified\t"
         << "\t\tLast Acessed\t"
         << "\t\tLast changed" << endl;
    for (File *file : files) // Iterates through each file
    {
        cout << file->name << "\t\t" << file->mod_time << "\t\t" << file->acc_time << "\t\t" << file->chg_time << endl;
    }
}
