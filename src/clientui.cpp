#include <iostream>
#include <unistd.h>
#include <regex>
#include "client.hpp"

using namespace std;

regex upl("upload ([a-zA-Z0-9_/\\.]+)"), dow("download ([a-zA-Z0-9_/\\.]+)"), del("delete ([a-zA-Z0-9_/\\.]+)"),
    lsr("list_server"), lcl("list_client"), gsd("get_sync_dir"), ext("exit");

int main(int argc, char *argv[])
{
    if (argc != 4)
        throw runtime_error("Wrong use of client! Expected <username> <server_ip> <port>");
    string username = argv[1], srvrAdd = argv[2];
    int srvrPort = stoi(argv[3]);

    Client newClient(username, srvrAdd, srvrPort);

    string cmdline;
    smatch cmdarg;
    while (getline(cin, cmdline))
    {
        if (regex_match(cmdline, cmdarg, upl))
        {
            newClient.uploadFile(cmdarg[1].str(), 1);
        }
        else if (regex_match(cmdline, cmdarg, dow))
        {
            newClient.downloadFile(cmdarg[1].str());
        }
        else if (regex_match(cmdline, cmdarg, del))
        {
            newClient.deleteFile(cmdarg[1].str());
        }
        else if (regex_match(cmdline, cmdarg, lsr))
        {
            newClient.listServer();
        }
        else if (regex_match(cmdline, cmdarg, lcl))
        {
            newClient.listClient();
        }
        else if (regex_match(cmdline, cmdarg, gsd))
        {
            newClient.getSyncDir();
        }
        else if (regex_match(cmdline, cmdarg, ext))
            exit(0);
        else
            cout << "Unrecognized command, please try again." << endl;
    }
    newClient.disconnect();
    return 0;
}