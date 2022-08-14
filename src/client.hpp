#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>
#include "./file/syncdir.hpp"

using namespace std;

class Client {
    private:
        string srvrAdd;
        int srvrPort;
        pthread_t syncDirID, clientID;
    public:
        string name;
        static int socketfd;
        static SyncDir * syncdir;

    Client(string username, string srvrAdd, int srvrPort);
    ~Client();
    static void uploadFile(string filepath);
    void downloadFile(string filepath);
    static void deleteFile(string filepath);
    static void deleteLocal(string filepath);
    static void listServer();
    static void listClient();
    static void getServerList();
    void getSyncDir();
    static void * syncDirLoop(void * param);
    static void * clientLoop(void * param);
    void disconnect() { close(socketfd); }
};