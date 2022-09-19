#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>
#include "syncdir.hpp"

using namespace std;

static int socketfd;
static SyncDir *syncdir;
static bool serverUpdate = false;

class Client
{
private:
    string srvrAdd;
    int srvrPort;
    int port;
    pthread_t syncDirID, clientID;
    int syncDirLoopActive = 0;
    int connected = 0;

public:
    string name;

    Client(string username, int clientPort, string srvrAdd, int srvrPort);
    ~Client();
    void downloadFile(string filename);
    static void uploadFile(string filepath, int forcePropagation = 0);
    static void deleteFile(string filepath);
    static void deleteLocal(string filepath);
    static void listServer();
    static void listClient();
    static void getServerList(string message);
    void getSyncDir();
    static void *syncDirLoop(void *param);
    static void *clientLoop(void *param);
    void disconnect() { close(socketfd); }
};