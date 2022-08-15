#include "server.hpp"
#include <sys/stat.h>

int main()
{   
    filesystem::path filepath = filesystem::current_path();
    string fpath = filepath.string() + "/clients";

    if(!filesystem::exists(fpath))
    {
        if(mkdir(fpath.c_str(), 0777) == -1)
            throw runtime_error("Failed to create clients directory");
        else
            cout << "Directory clients created" << endl;
    }

    Server server;
    server.serverLoop();
    return 0;
}