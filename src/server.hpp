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
public:
    struct userConnectionData
    {
        int socket;
        int on;
        string name;
        pthread_t thread;
        User *ref;
    };

public:
    userConnectionData data;
    unordered_map<int, userConnectionData> userConnectionsHash = unordered_map<int, userConnectionData>();
    vector<int> *backupSocket;

    User() {}
    User(string username, int newSocket, vector<int> *backup)
    {
        data.name = username;
        data.socket = newSocket;
        backupSocket = backup;
    };

    void newUserConnection(int socket);
    void upload(string message, userConnectionData info, int forcePropagation = 0);
    void download(string data, userConnectionData info);
    void del(string filename, userConnectionData info);
    void listServer(userConnectionData info);
    void syncAllUserConnections();
    static void *userConnectionLoop(void *param);
};

class Server
{
public:
    bool isLeader = false;

    unordered_map<string, User> usersHash;

    int serverSocket, newSocket, backup, serverPort;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    vector<int> backupId;
    vector<string> backupIP;
    vector<int> backupPort;
    vector<int> backupSocket;
    vector<string> clientIP;
    vector<int> clientPort;
    int nextRingSocket;
    int nextRingId;
    int electionStarted = 0;
    pthread_t backupRingReceiveID;

    socklen_t addr_size;
    Server(int port = 4000, int backupParam = 0)
    {
	    const int opt = 1;
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverPort = port;
        backup = backupParam;
        if (backupParam == 0)
            isLeader = true;

        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

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
    void backupRole();
    static void *backupRingReceive(void *param);
};