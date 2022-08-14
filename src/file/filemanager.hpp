#pragma once
#include "file.hpp"
#include <map>
#include <vector>

using namespace std;

class FileManager
{
    private:
        string username;
        vector<File *> clientData;
    public:
        void loadClientFiles(string userName);
        void saveClientFiles();
        void addClientFiles(vector<File *> newFiles);

        vector<File *> getClientFiles() { return clientData; }
};