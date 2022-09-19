#include "syncdir.hpp"
#include <iostream>
#include <sys/stat.h>
#include <string>
#include <ctime>
#include <dirent.h>


SyncDir::SyncDir(string path) {
    this->path = path;
    files = getFiles();
}
//Retorna uma vetor de pares de cada arquivo alterado, contendo o nome do arquivo e um inteiro referente ao seu estado
vector<pair<string,int>> SyncDir::sync() {
    vector<File *> oldFiles = files;
    vector<File *> newFiles = getFiles();
    vector<pair<string,int>> result;

    for(File * newFile : newFiles)
    {
        bool hasSameName = false, hasSameMetadata = false;
        for(File * oldFile : oldFiles)
        {
            if(newFile->name.compare(oldFile->name) == 0)
            {
                hasSameName = true;

                if((newFile->chg_time == oldFile->chg_time) &&
                (newFile->mod_time == oldFile->mod_time))
                {
                    hasSameMetadata = true;
                }
            }
        }

        if(!hasSameName)
        {
            result.push_back(make_pair(newFile->name,CREATED));
        }
        else if(!hasSameMetadata)
        {
            result.push_back(make_pair(newFile->name,MODIFIED));
        }
    }
    for(File * oldFile : oldFiles) {
        bool hasSameName = false;
        for(File * newFile : newFiles)
        {
            if(oldFile->name.compare(newFile->name) == 0)
                hasSameName = true;
        }
        if(!hasSameName)
        {
            result.push_back(make_pair(oldFile->name,DELETED));
        }
    }
    files = newFiles;

    return result;
}
//Retorna uma lista de arquivos da pasta syncDir
vector<File *> SyncDir::getFiles() {
    struct stat info;
    vector<File *> newFiles;
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (this->path.c_str())) != NULL) {
    
        while ((ent = readdir (dir)) != NULL) {
               File * file = new File(); 
               file->updateMetadata(this->path); //é ESSE O PATH CERTO???
               newFiles.push_back(file);
    }
    closedir (dir);
    } else {
            cout << "Error: cannot access sync_dir" << endl;
    }
    return newFiles;
}