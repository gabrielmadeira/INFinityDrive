#include "FileManagementModule.hpp"
#include <filesystem>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

void FileManagementModule::loadClientFiles(string userName) {
    struct stat info;
    string path = ".\\clientData\\" + userName;

    if( stat( path.c_str(), &info ) != 0 || !(info.st_mode & S_IFDIR))
        throw runtime_error(string("Error: cannot access client "+userName+" data folder"));
    else 
    {
        for (const auto & entry : filesystem::directory_iterator(path.c_str()))
        {
            clientData[userName].push_back(new File(entry.path().string()));
        }
    }
}
void FileManagementModule::saveClientFiles(string userName) {
    string path = ".\\clientData\\" + userName + "\\";

    for(File * file : clientData[userName]) {
        string newPath = path + file->name;
        file->write(newPath);
    }
}
void FileManagementModule::addClientFiles(string userName, vector<File *> newFiles) {
    vector<File *> * currentFiles = &(clientData[userName]);
    currentFiles->insert(currentFiles->end(), newFiles.begin(), newFiles.end());
}
vector<File *> FileManagementModule::getClientFiles(string userName) {
    return clientData[userName];
}