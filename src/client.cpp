#include <iostream>
#include <filesystem>
#include "client.hpp"
#include "connection.hpp"
#include "file.hpp"

using namespace std;

Client::Client(string username, string srvrAdd, int srvrPort)
{
    name = username;
    socketfd = connectClient(name, srvrAdd, srvrPort);
    if (socketfd == -1)
        throw runtime_error("Couldn't connect to server");
    string msg = name + '|';
    if (!sendProtocol(socketfd, msg, LOGN))
        throw runtime_error("Couldn't send machine info");
}

void Client::uploadFile(string filepath)
{
    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;
    cout << filepath << "\n";
    File file(filepath);

    if (upload(socketfd, file))
        throw runtime_error("Failed to send file to server");
}

void Client::downloadFile(string filepath)
{
    if (filepath.empty())
        cout << "Couldn't understand filename" << endl;

    File *newfile = (File *)download(socketfd, filepath);
    if (!newfile)
        cout << "No file found with name " + filepath << endl;
    newfile->write(filepath);
}

void deleteFile(string filepath)
{
    string fullFilepath = filesystem::current_path().string() +"/sync_dir/" + filepath;
    if(filepath.empty()) cout << "Couldn't understand filename" << endl;
    else if(remove(fullFilepath.c_str()) != 0)
    {
        cout << "Couldn't delete file" << endl;
    }
}

void Client::listServer()
{
    if(listServer(socketfd, name)) 
        throw runtime_error("Failed to send file to server");
}

void Client::getSyncDir() 
{
     if(getSyncDir(socketfd)) 
        throw runtime_error("Failed to send file to server");
}

void Client::listClient()
{
    string fullFilepath = filesystem::current_path().string() +"/sync_dir";
    SyncDir syncdir = SyncDir(fullFilepath);
    vector<File *> files = syncdir.getFiles();

    cout << "Name\t" << "\t\t\tLast Modified\t" << "\t\tLast Acessed\t" << "\t\tLast changed" << endl;
    for(File * file : files) //Iterates throw each file
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
}
