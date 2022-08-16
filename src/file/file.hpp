#pragma once
#include <string>
using namespace std;

class File
{
public:
    string name;
    string data;
    /* Metadata */

    // Modification time
    string mod_time;

    // Access time
    string acc_time;

    // Change or Creation time
    string chg_time;

    int size;

    File();
    File(string filePath);
    void read(string filePath);
    void write(string filePath);
    void updateMetadata(string filePath);
    ~File();
};