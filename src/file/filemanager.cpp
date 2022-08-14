#include <filesystem>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "filemanager.hpp"
#include "file.hpp"
using namespace std;

void FileManager::loadClientFiles(string username) {
    struct stat info;
    string path = "sync_dir_" + username; 
    this->username = username;

    if( stat( path.c_str(), &info ) != 0 || !(info.st_mode & S_IFDIR))
        throw runtime_error(string("Error: cannot access client "+username+" data folder"));
    else 
    {
        for (const auto & entry : filesystem::directory_iterator(path.c_str()))
            clientData.push_back(new File(entry.path().string()));
    }
}

void FileManager::saveClientFiles() {
    string path = "sync_dir_" + username + "\\";

    for(File * file : clientData) {
        string newPath = path + file->name;
        file->write(newPath);
    }
}

void FileManager::addClientFiles(vector<File *> newFiles) {
    vector<File *> * currentFiles = &(clientData);
    currentFiles->insert(currentFiles->end(), newFiles.begin(), newFiles.end());
}
