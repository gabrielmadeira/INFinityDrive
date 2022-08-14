// std
#include <bits/stdc++.h>
#include <strings.h>

// inet_addr
#include <arpa/inet.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
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

bool User::upload()
{

    cout << data.name << " upload!\n";
    return true;
}

void User::download()
{
}

void User::del(string filename)
{
    // inicio seção crítica
    // deletar arquivo
    cout << "file " << filename << " from user " << data.name << " deleted!\n";
    // syncAllUserConnections() -> propagar para todos as conexões do usuário conectadas
    // fim seção crítica
}

void User::listServer()
{
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

        printf("%d %s CMD received: %s", info.socket, info.name, get<1>(message));

        string delimiter = "|";
        switch (get<0>(message))
        {
        case UPLD: (*info.ref).upload(get<1>(message));   break;
        case DWNL: (*info.ref).download(get<1>(message)); break;
        case DELT: (*info.ref).del(get<1>(message));      break;
        case LSSV: (*info.ref).listServer();              break;
        case GSDR: (*info.ref).syncAllUserConnections();  break;
        default:   cout << "Spooky behavior!" << endl;
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