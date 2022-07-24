#include "FileManagementModule.hpp"
using namespace std;
#include <iostream>
//#include <cstring>
//#include <sstream>
//#include <ctime>
//#include <string>

int main(int argc, char *argv[])
{   
    string clientName = "user1";
    FileManagementModule fmm;
    fmm.loadClientFiles(clientName); //Loads user1 data to memory

    cout << "Name\t" << "\t\t\tLast Modified\t" << "\t\tLast Acessed\t" << "\t\tLast changed" << endl;
    for(File * file : fmm.getClientFiles(clientName)) //Iterates throw each user1's file in memory
    {
        //Displays its metadata
        tm * mod = localtime(&file->mod_time);
        tm * acc = localtime(&file->acc_time);
        tm * chg = localtime(&file->chg_time);
        char buffer[3][32];
        // Format: Mo, 15.06.2009 20:20:00
        strftime(buffer[0], 32, "%a, %d.%m.%Y %H:%M:%S", mod);
        strftime(buffer[1], 32, "%a, %d.%m.%Y %H:%M:%S", acc);
        strftime(buffer[2], 32, "%a, %d.%m.%Y %H:%M:%S", chg);

        cout << file->name << "\t\t" << buffer[0] << "\t\t" << buffer[1] << "\t\t" << buffer[2] << endl;
    }
    //Adds 2 new files from user2 to memory
    File * file = new File(".\\clientData\\user2\\Addendum.txt");
    File * file2 = new File(".\\clientData\\user2\\Addendum 2.txt");

    vector<File *> newFiles;
    newFiles.push_back(file);
    newFiles.push_back(file2);

    fmm.addClientFiles(clientName,newFiles); // Adds the 2 new files to user1 current files
    fmm.saveClientFiles(clientName); // Saves all user1's files to the disk

    return 0;
}