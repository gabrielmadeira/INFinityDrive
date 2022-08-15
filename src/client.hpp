#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>
#include "syncdir.hpp"

using namespace std;

static int socketfd;
static SyncDir * syncdir;

class Client {
private:
    string srvrAdd;
    int srvrPort;
    pthread_t syncDirID, clientID;
public:
    string name;

    Client(string username, string srvrAdd, int srvrPort);
    ~Client();
    void downloadFile(string filepath);
    static void uploadFile(string filepath);
    static void deleteFile(string filepath);
    static void deleteLocal(string filepath);
    static void listServer();
    static void listClient();
    static void getServerList(string message);
    void getSyncDir();
    static void * syncDirLoop(void * param);
    static void * clientLoop(void * param);
    void disconnect() { close(socketfd); }
};