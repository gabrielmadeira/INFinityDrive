#pragma once
#include "File.hpp"
#include <map>
#include <vector>

using namespace std;

class FileManagementModule : protected File
{
    public:
        // Associates many files to each username
        map<string,vector<File *>> clientData;

        // Loads clients files to memory
        void loadClientFiles(string userName);
        // Saves client files to disk
        void saveClientFiles(string userName);

        // Adds new files to the current client files in memory
        void addClientFiles(string userName, vector<File *> newFiles);
        // Returns all files of a client that are in memory
        vector<File *> getClientFiles(string userName);
};