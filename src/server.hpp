#pragma once
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include "connection.hpp"
#include "file.hpp"

using namespace std;
// mutex mtx;


class User
{
    struct userConnectionData
    {
        int socket;
        string name;
        pthread_t thread;
        User *ref;
    };
public:
    userConnectionData data;
    unordered_map<int, userConnectionData> userConnectionsHash;

    User() {}
    User(string username, int newSocket){ data.name = username; data.socket = newSocket;};

    void newUserConnection(int socket);
    void upload(string message);
    void download(string data);
    void del(string filename);
    void listServer();
    void syncAllUserConnections();
    static void *userConnectionLoop(void *param);
};

class Server
{
public:
    unordered_map<string, User> usersHash;
    // TODO: recuperar usuários existentes do disco

    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;

    socklen_t addr_size;
    Server()
    {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(4000);

        bind(serverSocket,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr));

        addr_size = sizeof(serverStorage);

        if (listen(serverSocket, 50) == 0)
            printf("Listening\n");
        else
            printf("Error\n");
    }

    void serverLoop();
};