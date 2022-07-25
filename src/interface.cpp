#include <iostream>
#include <unistd.h>
#include <regex>
#include <thread>
#include "client.hpp"

using namespace std;

regex upl("(upload )([a-zA-Z_/]+)"), dow("(download )([a-zA-Z_/]+)"), del("(delete )([a-zA-Z_/]+)"), lsr("(list server)"), lcl("(list client)"), gsd("(get sync_dir)"), ext("(exit)");

int main(int argc, char* argv[])
{
    if (argc != 4) throw runtime_error("Wrong use of client! Expected <username> <server_ip> <port>");
    string username = argv[1], srvrAdd = argv[2];
    int srvrPort = stoi(argv[2]);

    if(client::isUserSubscribed(username, gethostid()) == false) client::createAccount(username, gethostid());

    string cmdline;
    smatch cmdarg;
    while(getline(cin, cmdline))
    {
        thread thHelper;
        if(regex_match(cmdline, cmdarg, upl)) { string filename = cmdarg[1]; thHelper = thread(&client::uplodFile, ref(filename));    thHelper.detach(); }
        if(regex_match(cmdline, cmdarg, dow)) { string filename = cmdarg[1]; thHelper = thread(&client::downloadFile, ref(filename)); thHelper.detach(); }
        if(regex_match(cmdline, cmdarg, del)) { string filename = cmdarg[1]; thHelper = thread(&client::deleteFile, ref(filename));   thHelper.detach(); }
        if(regex_match(cmdline, cmdarg, lsr)) {                              thHelper = thread(&client::listServer);                  thHelper.detach(); }
        if(regex_match(cmdline, cmdarg, lcl)) {                              thHelper = thread(&client::listClient);                  thHelper.detach(); }
        if(regex_match(cmdline, cmdarg, gsd)) {                              thHelper = thread(&client::getSyncDir);                  thHelper.detach(); }
        if(regex_match(cmdline, cmdarg, ext)) exit(0);
    }

    return 0;
}