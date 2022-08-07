#include "SyncDir.hpp"
#include <filesystem>
#include <iostream>
#include <sys/stat.h>
#include <string>
#include <ctime>


SyncDir::SyncDir(string path) {
    this->path = path;
    files = getFiles();
}
//Retorna uma vetor de pares de cada arquivo alterado, contendo o nome do arquivo e um inteiro referente ao seu estado
vector<pair<string,int>> SyncDir::sync() {
    vector<File *> oldFiles = files;
    vector<File *> newFiles = getFiles();
    vector<File *> diff;
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

    if( stat( this->path.c_str(), &info ) != 0 || !(info.st_mode & S_IFDIR))
        throw runtime_error(string("Error: cannot access sync_dir"));
    else 
    {
        for (const auto & entry : filesystem::directory_iterator(this->path.c_str()))
        {
            File * file = new File();
            file->updateMetadata(entry.path().string());
            newFiles.push_back(file);
        }
    }
    return newFiles;
}