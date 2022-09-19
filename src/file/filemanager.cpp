#include <dirent.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "filemanager.hpp"
#include "file.hpp"
using namespace std;

void FileManager::loadClientFiles(string username) {
    
    
    DIR *dir;
    struct dirent *ent;
    string path = "./clients/" + username; 
    this->username = username;

    if ((dir = opendir (path.c_str())) != NULL) {
    
        while ((ent = readdir (dir)) != NULL) {
                clientData.push_back(new File(string(ent->d_name))); 
    }
    closedir (dir);
    } else {
        cout << "Error: cannot access client "+username+" data folder" << endl;
    return;
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
