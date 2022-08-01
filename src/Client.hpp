#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>

using namespace std;

class Client {
    public:
        string name;
        char hostname[HOST_NAME_MAX];
        int socketfd;

    Client(string username, string srvrAdd, int srvrPort);
    void uploadFile(string filepath);
    void downloadFile(string filepath);
    void deleteFile(string filepath);
    void listServer();
    void listClient();
    void getSyncDir();

};