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

    if (pthread_create(&(userConnectionsHash[socket].thread), NULL,
                       userConnectionLoop, &(userConnectionsHash[socket])) != 0)
        printf("Failed to create thread\n");
    pthread_detach(userConnectionsHash[socket].thread);
}

void User::upload(string message, int socket)
{
    // cout << message << " upload!\n"; //Seg Fault
    File *file = deserializeFile(message);
    cout << file->name << endl;

    file->write("./clients/" + data.name + "/" + file->name);

    for (auto &it : userConnectionsHash)
    {
        if ((it.second.socket != socket) && (it.second.name == data.name))
            sendProtocol(it.second.socket, message, UPLD);
    }
}

void User::download(string message)
{
    File file("./clients/" + data.name + "/" + data.name);

    sendProtocol(data.socket, serializeFile(&file), DWNL);
}

void User::del(string filename)
{
    // inicio seção crítica
    // deletar arquivo
    // cout << "file " << filename << " from user " << data.name << " deleted!\n";
    // syncAllUserConnections() -> propagar para todos as conexões do usuário conectadas
    // fim seção crítica

    string fullFilepath = "./clients/" + data.name + "/" + filename;
    if (fullFilepath.empty())
        cout << "Couldn't understand filename" << endl;
    else if (remove(fullFilepath.c_str()) != 0)
        cout << "Couldn't delete file" << endl;

    for (auto &it : userConnectionsHash)
    {
        if ((it.second.socket != data.socket) && (it.second.name == data.name))
            sendProtocol(it.second.socket, filename, DELT);
    }
}

void User::listServer()
{
    // 5 | name | data | name | data |

    FileManager manager;
    manager.loadClientFiles(data.name);
    string message = serializePack(manager.getClientFiles());
    sendProtocol(data.socket, message, LSSV);
}

void User::syncAllUserConnections()
{
    FileManager manager;
    cout << "Syncing " << data.name << " socket connections: ";
    manager.loadClientFiles(data.name);
    string message = serializePack(manager.getClientFiles());
    for (auto &it : userConnectionsHash)
    {
        cout << it.first << " "; // first é a chave(ou numero do socket) e second o struct com os dados da conexão
        sendProtocol(data.socket, message, LSSV);
        // send(); // colocar uma thread no cliente para receber comandos do server
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
            throw runtime_error("Failed to create " + info.name + " directory");
        else
            cout << "Directory " + info.name + " created" << endl;
    }

    while (1)
    {
        tProtocol message = receiveProtocol(info.socket);
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

        // printf("%d %s CMD received: %s\n", info.socket, info.name.c_str(), get<1>(message).c_str());

        string delimiter = "|";
        switch (get<0>(message))
        {
        case UPLD:
            (*info.ref).upload(get<1>(message), info.socket);
            break;
        case DWNL:
            (*info.ref).download(get<1>(message));
            break;
        case DELT:
            (*info.ref).del(get<1>(message));
            break;
        case LSSV:
            (*info.ref).listServer();
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
            usersHash[get<1>(user)] = User(get<1>(user));
        }
        cout << "Connection #" << newSocket << " from user " << get<1>(user) << "\n";
        usersHash[get<1>(user)].newUserConnection(newSocket);
    }
}