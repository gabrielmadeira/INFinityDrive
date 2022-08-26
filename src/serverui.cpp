#include "server.hpp"
#include <sys/stat.h>

int main(int argc, char *argv[])
{   
    if (argc != 2)
        throw runtime_error("Wrong use of server! Expected <port>");
    int port = stoi(argv[1]);

    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/clients";

    if(!filesystem::exists(fpath))
    {
        if(mkdir(fpath.c_str(), 0777) == -1)
            throw runtime_error("Failed to create clients directory");
        else
            cout << "Directory clients created" << endl;
    }

    Server server(port);
    server.serverLoop();
    return 0;
}