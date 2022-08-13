#include "file.hpp"
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
File::File(string filePath) { read(filePath); }
File::~File()               { data.clear();   }

void File::read(string filePath) {
    ifstream in(filePath, ios::ate | ios::binary);                                  // open output file
    if(in)
    {
        data.resize(in.tellg());
        in.seekg(0, ios::beg);                          
        data.assign(istreambuf_iterator<char>(in), istreambuf_iterator<char>());    // write filestream into buffer 
        in.close();                                                                 // close file handle

        updateMetadata(filePath);
    }
    else throw runtime_error(string("Error: ") + strerror(errno));
}

void File::write(string filePath) {
    ofstream out(filePath, ios::out | ios::binary);                                 // open output file
    if(out)
    {
        out << data;                                                                // write the buffer into file 
        out.close();                                                                // close file handle
    }
    else throw runtime_error(string("Error: ") + strerror(errno));
}

void File::updateMetadata(string filePath) {
    struct stat result;
    if(stat(filePath.c_str(), &result)==0)
    {
        name = filePath.substr(filePath.find_last_of("/\\") + 1);
        mod_time = result.st_mtime;
        acc_time = result.st_atime;
        chg_time = result.st_ctime;
    }
    else throw runtime_error(string("Error: No such file or directory"));
}