#pragma once
#include "file.hpp"
#include <string>
#include <vector>
using namespace std;

#define MODIFIED 0
#define DELETED 1
#define CREATED 2

class SyncDir
{
    public:
        vector<File *> files;
        string path;

        SyncDir(string path);
        vector<File *> getFiles();
        vector<pair<string,int>> sync();
};