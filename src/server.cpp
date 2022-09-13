// std
#include <bits/stdc++.h>
#include <strings.h>

// inet_addr
#include <arpa/inet.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filemanager.hpp>
#include "server.hpp"
#include "connection.hpp"

using namespace std;
void User::newUserConnection(int socket)
{
    userConnectionsHash[socket] = {};
    userConnectionsHash[socket].socket = socket;
    userConnectionsHash[socket].name = data.name;
    userConnectionsHash[socket].ref = this;
    userConnectionsHash[socket].on = 1;

    if (pthread_create(&(userConnectionsHash[socket].thread), NULL,
                       userConnectionLoop, &(userConnectionsHash[socket])) != 0)
        printf("Failed to create thread\n");
    pthread_detach(userConnectionsHash[socket].thread);
}

void User::upload(string message, userConnectionData info, int forcePropagation)
{
    File *file = deserializeFile(message);
    string path = "./clients/" + info.name + "/" + file->name;

    receiveFile(path, info.socket, file->size);

    for (auto &it : userConnectionsHash)
    {
        if ((forcePropagation || it.first != info.socket) && (it.second.name == info.name) && (it.second.on == 1))
        {
            sendProtocol(it.first, message, UPLD);
            sendFile(path, it.first);
        }
    }

    for (auto socket : (*backupSocket))
    {
        sendProtocol(socket, info.name, DATA);
        sendProtocol(socket, message, UPLD);
        sendFile(path, socket);
    }
}

void User::download(string message, userConnectionData info)
{
    File file("./clients/" + data.name + "/" + message);
    string path = "./clients/" + data.name + "/" + message;

    sendProtocol(info.socket, serializeFile(&file) + '|', DWNL);
    sendFile(path, info.socket);
}

void User::del(string filename, userConnectionData info)
{
    string fullFilepath = "./clients/" + info.name + "/" + filename;
    if (fullFilepath.empty())
        cout << "Couldn't understand filename" << endl;
    else if (remove(fullFilepath.c_str()) != 0)
        cout << "Couldn't delete file" << endl;

    for (auto &it : userConnectionsHash)
    {
        if ((it.first != info.socket) && (it.second.name == info.name) && (it.second.on == 1))
            sendProtocol(it.first, filename, DELT);
    }
    for (auto socket : (*backupSocket))
    {
        sendProtocol(socket, info.name, DATA);
        sendProtocol(socket, filename, DELT);
    }
}

void User::listServer(userConnectionData info)
{
    // 5 | name | data | name | data |

    FileManager manager;
    manager.loadClientFiles(info.name);
    string message = serializePack(manager.getClientFiles());
    sendProtocol(info.socket, message, LSSV);
}

void User::syncAllUserConnections()
{
    FileManager manager;
    cout << "Syncing " << data.name << " socket connections: ";
    manager.loadClientFiles(data.name);
    string message = serializePack(manager.getClientFiles());
    for (auto &it : userConnectionsHash)
    {
        if (it.second.on == 1)
        {
            cout << it.first << " "; // first é a chave(ou numero do socket) e second o struct com os dados da conexão
            sendProtocol(data.socket, message, LSSV);
        }
    }
    cout << "\n";
}

void *User::userConnectionLoop(void *param)
{
    // TODO: destruir UserConnection ao desconectar ou ocorrer erro

    userConnectionData info = *(userConnectionData *)param;

    protocol buffer;
    int n;
    // Creates directory for client if doesn't exist
    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/clients/" + info.name;

    if (!filesystem::exists(fpath))
    {
        if (mkdir(fpath.c_str(), 0777) == -1)
        {
            cout << "Failed to create " + info.name + " directory";
            return nullptr;
        }
        else
            cout << "Directory " + info.name + " created" << endl;
    }

    while (1)
    {
        tProtocol message = receiveProtocol(info.socket);

        if (get<0>(message) == ERRO)
        {
            (*info.ref).userConnectionsHash[info.socket].on = 0;
            break;
        }

        switch (get<0>(message))
        {
        case UPLD:
            (*info.ref).upload(get<1>(message), info);
            break;
        case UPLF:
            (*info.ref).upload(get<1>(message), info, 1);
            break;
        case DWNL:
            (*info.ref).download(get<1>(message), info);
            break;
        case DELT:
            (*info.ref).del(get<1>(message), info);
            break;
        case LSSV:
            (*info.ref).listServer(info);
            break;
        case GSDR:
            (*info.ref).syncAllUserConnections();
            break;
        default:
            cout << "Spooky behavior!" << endl;
        }
    }
    return nullptr;
}

void Server::serverLoop()
{
    // initialize backup servers

    for (int i = 0; i < backupId.size(); i++)
    {
        if (backupId[i] == backup)
        {
            backupId.erase(backupId.begin()+i);
        }
    }
    for (int i = 0; i < backupId.size(); i++)
    {
        int socket = connectClient("backup", backupIP[i], backupPort[i]);
        if (socket == -1)
        {
            cout << "Error connecting to backup server!\n";
            continue;
        }
        backupSocket.push_back(socket);
    }

    while (1)
    {
        newSocket = accept(serverSocket,
                           (struct sockaddr *)&serverStorage,
                           &addr_size);

        tProtocol user = receiveProtocol(newSocket);
        if (!usersHash.count(get<1>(user)))
        {
            usersHash[get<1>(user)] = User(get<1>(user), newSocket, &backupSocket);
        }
        cout << "Connection #" << newSocket << " from user " << get<1>(user) << "\n";
        usersHash[get<1>(user)].newUserConnection(newSocket);
    }
}

void Server::backupRole()
{

    // start thread backup ring receive

    int primarySocket;
    primarySocket = accept(serverSocket,
                           (struct sockaddr *)&serverStorage,
                           &addr_size);

    cout << "Succesfully connected to main server" << endl;


    // tProtocol clients = receiveProtocol(primarySocket);
    // for (clients)
    // {
    //     primaryClients.push_back(get<1>(message));
    // }
    // tProtocol backups = receiveProtocol(primarySocket);

    while (1)
    {

        tProtocol user = receiveProtocol(primarySocket);

        if (get<0>(user) == ERRO)
        {
            // PRIMARY BROKE
            break;
        }
        if (!usersHash.count(get<1>(user)))
        {
            usersHash[get<1>(user)] = User(get<1>(user), primarySocket,new vector<int>());
        }

        tProtocol message = receiveProtocol(primarySocket);


        switch (get<0>(message))
        {
        case UPLD:
            usersHash[get<1>(user)].upload(get<1>(message), usersHash[get<1>(user)].data);
            break;
        case DELT:
            usersHash[get<1>(user)].del(get<1>(message), usersHash[get<1>(user)].data);
            break;
        default:
            cout << "Spooky behavior!" << endl;
        }
    }

    // backup ring send
}