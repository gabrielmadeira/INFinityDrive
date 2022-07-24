#include "File.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

File::File() {}

File::File(string filePath) {read(filePath);}

File::~File() {delete this->data;}

void File::read(string filePath) {
    ifstream in(filePath, ios::ate | ios::binary);
    if(in)
    {
        this->length = in.tellg();
        this->data = new char[this->length];
        in.seekg(0, ios::beg);
        in.read(this->data, this->length);
        in.close();

        updateMetadata(filePath);
    }
    else
    {
        throw runtime_error(string("Error: ") + strerror(errno));
    }
}
void File::write(string filePath) {
    ofstream out(filePath, ios::out | ios::binary);      // open output file
    if(out)
    {
        out.write(this->data, this->length);       // write the whole buffer into a file
        out.close();                  // close file handle
    }
    else 
    {
        throw runtime_error(string("Error: ") + strerror(errno));
    }
}
void File::updateMetadata(string filePath) {
    struct stat result;
    if(stat(filePath.c_str(), &result)==0)
    {
        this->name = filePath.substr(filePath.find_last_of("/\\") + 1);
        this->mod_time = result.st_mtime;
        this->acc_time = result.st_atime;
        this->chg_time = result.st_ctime;
    }
    else
    {
        throw runtime_error(string("Error: No such file or directory"));
    }
}