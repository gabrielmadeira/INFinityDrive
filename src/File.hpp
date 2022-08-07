#pragma once
#include <string>
using namespace std;

class File {
    public:
        string name;
        string data;
        /* Metadata */

        // Modification time
        time_t mod_time;

        // Access time
        time_t acc_time;
        
        // Change or Creation time
        time_t chg_time;

        File();
        File(string filePath);
        void read(string filePath);
        void write(string filePath);
        void updateMetadata(string filePath);
        ~File();
};