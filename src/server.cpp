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

    cout << "upload message: " << message << "\n";
    receiveFile(path, info.socket, file->size);
    cout << "file received!\n";

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

void createClientDirectory(string name)
{
    // Creates directory for client if doesn't exist
    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/clients/" + name;

    if (!filesystem::exists(fpath))
    {
        if (mkdir(fpath.c_str(), 0777) == -1)
        {
            cout << "Failed to create " + name + " directory";
        }
        else
            cout << "Directory " + name + " created" << endl;
    }
}

void *User::userConnectionLoop(void *param)
{
    // TODO: destruir UserConnection ao desconectar ou ocorrer erro

    userConnectionData info = *(userConnectionData *)param;

    protocol buffer;
    int n;
    createClientDirectory(info.name);

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

    // if this server was a backup and now is a leader, remove it from backup list
    for (int i = 0; i < backupId.size(); i++)
    {
        if (backupId[i] == backup)
        {
            backupId.erase(backupId.begin() + i);
            backupIP.erase(backupIP.begin() + i);
            backupPort.erase(backupPort.begin() + i);
        }
    }

    // connect with backup servers
    for (int i = 0; i < backupId.size(); i++)
    {
        cout << "Connecting with backup server: " << backupPort[i] << "\n";
        int socket = connectClient("backup", backupIP[i], backupPort[i]);
        if (socket == -1)
        {
            cout << "Error connecting to backup server!\n";
            continue;
        }
        backupSocket.push_back(socket);

        // send backups info
        sendProtocol(socket, to_string(backupId.size()), DATA);
        for (int j = 0; j < backupId.size(); j++)
        {
            sendProtocol(socket, to_string(backupId[j]), DATA);
            sendProtocol(socket, backupIP[j], DATA);
            sendProtocol(socket, to_string(backupPort[j]), DATA);
        }
    }

    // if this server was a backup, send it IP and port to waiting clients
    for (int i = 0; i < clientIP.size(); i++)
    {
        cout << "Connecting with waiting client: " << clientIP[i] << " : " << clientPort[i] << "\n";
        int ntry = 4;
        int socketClient = -1;
        int sent = 0;
        while (!sent && ntry > 0 && socketClient <= 0)
        {
            socketClient = connectClient("", clientIP[i], clientPort[i]);
            cout << "Trying to connect with waiting client: " << clientIP[i] << " : " << clientPort[i] << "\n";
            if (sendProtocol(socketClient, to_string(serverPort), DATA))
            {
                sent = 1;
            }
            ntry--;
        }
    }

    cout << "Entering ServerLoop\n";

    while (1)
    {
        newSocket = accept(serverSocket,
                           (struct sockaddr *)&serverStorage,
                           &addr_size);

        tProtocol user = receiveProtocol(newSocket);
        tProtocol userConnectionPort = receiveProtocol(newSocket);
        if (!usersHash.count(get<1>(user)))
        {
            usersHash[get<1>(user)] = User(get<1>(user), newSocket, &backupSocket);
        }
        cout << "Connection #" << newSocket << " from user " << get<1>(user) << "\n";
        usersHash[get<1>(user)].newUserConnection(newSocket);

        // send new user connection ip and port to backups
        struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&serverStorage;
        struct in_addr ipAddr = pV4Addr->sin_addr;

        char ipUser[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ipAddr, ipUser, INET_ADDRSTRLEN);
        for (auto socket : backupSocket)
        {
            sendProtocol(socket, get<1>(user), DATA);
            sendProtocol(socket, ipUser, CLT);
            sendProtocol(socket, get<1>(userConnectionPort), CLT);
        }
    }
}

void *Server::backupRingReceive(void *param)
{
    Server *ref = (Server *)param;

    cout << "Backup Receive waiting for ring back position\n";
    int backRingSocket = accept(ref->serverSocket,
                                (struct sockaddr *)&(ref->serverStorage),
                                &(ref->addr_size));

    while (1)
    {
        cout << "Ring back position connected, waiting message...\n";
        tProtocol ringMessage = receiveProtocol(backRingSocket);
        cout << "Received: " << get<1>(ringMessage) << "\n";

        int alreadyParticipant = 1;
        if (!ref->electionStarted)
        {
            alreadyParticipant = 0;
            ref->electionStarted = 1;

            int index = 0;
            while (ref->backupId[index] != ref->backup)
                index++;
            index++;
            if (index == ref->backupId.size())
                index = 0;

            ref->nextRingId = ref->backupId[index];
            ref->nextRingSocket = connectClient("", ref->backupIP[index], ref->backupPort[index]);
        }
        if (get<1>(ringMessage) == "election")
        {
            ringMessage = receiveProtocol(backRingSocket);
            if (stoi(get<1>(ringMessage)) > ref->backup)
            {
                sendProtocol(ref->nextRingSocket, "election", DATA);
                sendProtocol(ref->nextRingSocket, get<1>(ringMessage), DATA);
            }
            else if (stoi(get<1>(ringMessage)) == ref->backup)
            {
                cout << ref->backup << " elected!\n";
                sendProtocol(ref->nextRingSocket, "elected", DATA);
                sendProtocol(ref->nextRingSocket, to_string(ref->backup), DATA);
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                ref->isLeader = true;
                ref->electionStarted = 0;
                cout << "Start serverLoop!\n";
                break;
            }
            else
            {
                if (!alreadyParticipant)
                {
                    sendProtocol(ref->nextRingSocket, "election", DATA);
                    sendProtocol(ref->nextRingSocket, to_string(ref->backup), DATA);
                }
            }
        }
        else
        {
            ringMessage = receiveProtocol(backRingSocket);
            if (stoi(get<1>(ringMessage)) != ref->nextRingId)
            {
                sendProtocol(ref->nextRingSocket, "elected", DATA);
                sendProtocol(ref->nextRingSocket, get<1>(ringMessage), DATA);
            }
            ref->backupId.clear();
            ref->backupPort.clear();
            ref->backupIP.clear();
            ref->clientPort.clear();
            ref->clientIP.clear();
            ref->electionStarted = 0;
            cout << "Restart backupRole!\n";
            break;
        }
    }
}

void Server::backupRole()
{

    // start thread backup ring receive

    int primarySocket = accept(serverSocket,
                               (struct sockaddr *)&serverStorage,
                               &addr_size);

    cout << "Succesfully connected to main server" << endl;

    // start thread backup ring receive
    if (pthread_create(&backupRingReceiveID, NULL,
                       backupRingReceive, this) != 0)
        printf("Failed to create thread\n");

    // receive backup list from primary
    tProtocol backupsInfo = receiveProtocol(primarySocket);
    int nBackups = stoi(get<1>(backupsInfo));
    for (int i = 0; i < nBackups; i++)
    {
        backupsInfo = receiveProtocol(primarySocket);
        backupId.push_back(stoi(get<1>(backupsInfo)));

        backupsInfo = receiveProtocol(primarySocket);
        backupIP.push_back(get<1>(backupsInfo));

        backupsInfo = receiveProtocol(primarySocket);
        backupPort.push_back(stoi(get<1>(backupsInfo)));
    }

    while (1)
    {
        cout << "Backup Role Loop\n";
        tProtocol user = receiveProtocol(primarySocket);

        if (get<0>(user) == ERRO)
        {
            // PRIMARY BROKE
            cout << "Primary Broke\n";
            if (backupId.size() == 1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                isLeader = true;
            }
            else
            {
                if (!electionStarted)
                {
                    // start election
                    // backup ring send
                    electionStarted = 1;

                    int index = 0;
                    while (backupId[index] != backup)
                        index++;
                    index++;
                    if (index == backupId.size())
                        index = 0;

                    nextRingId = backupId[index];
                    cout << "Starting Election\n";
                    nextRingSocket = connectClient("", backupIP[index], backupPort[index]);
                    sendProtocol(nextRingSocket, "election", DATA);
                    sendProtocol(nextRingSocket, to_string(backup), DATA);
                }
            }
            break;
        }
        if (!usersHash.count(get<1>(user)))
        {
            usersHash[get<1>(user)] = User(get<1>(user), primarySocket, new vector<int>());
            createClientDirectory(get<1>(user));
        }

        cout << "Backup: command received from user " << get<1>(user) << ": ";
        tProtocol message = receiveProtocol(primarySocket);

        switch (get<0>(message))
        {
        case UPLD:
            cout << "Upload\n";
            usersHash[get<1>(user)].upload(get<1>(message), usersHash[get<1>(user)].data);
            break;
        case DELT:
            cout << "Delete\n";
            usersHash[get<1>(user)].del(get<1>(message), usersHash[get<1>(user)].data);
            break;
        case CLT: // new client
            clientIP.push_back(get<1>(message));
            message = receiveProtocol(primarySocket);
            clientPort.push_back(stoi(get<1>(message)));
            break;
        // TODO: delete backup
        default:
            cout << "Spooky behavior!" << endl;
        }
    }
}