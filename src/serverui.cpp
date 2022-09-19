#include "server.hpp"
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
        throw runtime_error("Wrong use of server! Expected <port>");
    int port = stoi(argv[1]);

    // <port> <--- servidor primario sem backups
    // <port> <backupId> <--- servidor backup com id, caso 0 é primário
    // <port> <nBackupServers> <backup1Id> <backup1IP> <backup1Port> ... <--- servidor primario com n backups
    int backup = 0;
    if (argc == 3)
    {
        backup = stoi(argv[2]);
    }

    int nBackupServers = 0;
    vector<int> backupId;
    vector<string> backupIP;
    vector<int> backupPort;
    if (argc > 3)
    {
        nBackupServers = stoi(argv[3]);
        for (int i = 0; i < nBackupServers; i++)
        {
            int index = 3 + (i * 3);
            backupId.push_back(stoi(argv[index]));
            backupIP.push_back(argv[index + 1]);
            backupPort.push_back(stoi(argv[index + 2]));
        }
    }

    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/clients";

    if (!filesystem::exists(fpath))
    {
        if (mkdir(fpath.c_str(), 0777) == -1)
            throw runtime_error("Failed to create clients directory");
        else
            cout << "Directory clients created" << endl;
    }

    Server server(port, backup);
    server.backupId = backupId;
    server.backupIP = backupIP;
    server.backupPort = backupPort;

    while (!server.isLeader)
    {
        server.backupRole();
    }
    cout << "ServerLoop serveui\n";
    server.serverLoop();
    return 0;
}