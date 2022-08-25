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
            cout << "SEND FILE SIZE: " << file->size << endl;
            sendFile(path, it.first);
        }
    }
}

void User::download(string message, userConnectionData info)
{
    File file("./clients/" + data.name + "/" + message);
    string path = "./clients/" + data.name + "/" + message;

    sendProtocol(info.socket, serializeFile(&file) + '|', DWNL);
    cout << "SEND FILE SIZE: " << file.data.size() << endl;
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
        cout << it.first << " " << data.socket << endl;
        if ((it.first != info.socket) && (it.second.name == info.name) && (it.second.on == 1))
            sendProtocol(it.first, filename, DELT);
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
            // send(); // colocar uma thread no cliente para receber comandos do server
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
        if (mkdir(fpath.c_str(), 0777) == -1){
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

        printf("%d %s CMD received: %d %s\n", info.socket, info.name.c_str(), get<0>(message), get<1>(message).c_str());

        string delimiter = "|";
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
            break; // Not needed
        default:
            cout << "Spooky behavior!" << endl;
        }
    }
    return nullptr;
}

void Server::serverLoop()
{
    while (1)
    {
        newSocket = accept(serverSocket,
                           (struct sockaddr *)&serverStorage,
                           &addr_size);

        tProtocol user = receiveProtocol(newSocket);
        cout << "CON:" << get<0>(user) << "-" << get<1>(user) << "\n";
        if (!usersHash.count(get<1>(user)))
        {
            usersHash[get<1>(user)] = User(get<1>(user), newSocket);
        }
        cout << "Connection #" << newSocket << " from user " << get<1>(user) << "\n";
        usersHash[get<1>(user)].newUserConnection(newSocket);
    }
}