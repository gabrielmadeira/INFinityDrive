#pragma once
#include <string>
#include <unistd.h>
#include "syncdir.hpp"

using namespace std;

class Client : SyncDir {
    public:
        string name;
        int socketfd;

    Client(string username, string srvrAdd, int srvrPort);
    void uploadFile(string filepath);
    void downloadFile(string filepath);
    void deleteFile(string filepath);
    void listServer();
    void listClient();
    void getSyncDir();
    void disconnect() { close(socketfd); }
};