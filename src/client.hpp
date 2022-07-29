#pragma once
#include <string>
using namespace std;

class Client {
    public:

        string name;
        char * cmdarg;
        long hostid;
        /* datas */


    bool isUserSubscribed(std::string username, long hostid);
    void createAccount(std::string username, long hostid);
    bool uplodFile(std::string & filepath);
    bool downloadFile(std::string & filepath);
    bool deleteFile(std::string & filepath);
    bool listServer();
    bool listClient();
    bool getSyncDir();

};