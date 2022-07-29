#include <iostream>
#include <unistd.h>
#include <regex>
#include "client.hpp"

using namespace std;

regex upl("upload ([a-zA-Z_/\\.]+)"), dow("download ([a-zA-Z_/\\.]+)"), del("delete ([a-zA-Z_/\\.]+)"), lsr("list serverlist"), lcl("list client"), gsd("get sync_dir"), ext("exit");

int main(int argc, char* argv[])
{
    if (argc != 4) throw runtime_error("Wrong use of client! Expected <username> <server_ip> <port>");
    string username = argv[1], srvrAdd = argv[2];
    int srvrPort = stoi(argv[2]);

    //if(client::isUserSubscribed(username, gethostid()) == false) client::createAccount(username, gethostid());

    string cmdline;
    smatch cmdarg;
    //
    //create object client 
    Client newClient;
    //
    while(getline(cin, cmdline))
    {
        if(regex_match(cmdline, cmdarg, upl)) { cout <<"UPLOAD FOR "     << cmdarg[1];  newClient.upload(cmdarg[1])}
        if(regex_match(cmdline, cmdarg, dow)) { cout <<"DOWNLOAD FOR "   << cmdarg[1]; }
        if(regex_match(cmdline, cmdarg, del)) { cout <<"DEL FOR "        << cmdarg[1]; }
        if(regex_match(cmdline, cmdarg, lsr)) { cout <<"LIST_SERVER FOR "<< cmdarg[1]; }
        if(regex_match(cmdline, cmdarg, lcl)) { cout <<"LIST_CLIENT FOR "<< cmdarg[1]; }  
        if(regex_match(cmdline, cmdarg, gsd)) { cout <<"SYNC FOR "       << cmdarg[1]; }
        if(regex_match(cmdline, cmdarg, ext)) exit(0);
    }

    return 0;
}