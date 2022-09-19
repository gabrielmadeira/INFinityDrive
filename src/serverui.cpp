#include "server.hpp"
#include <sys/stat.h>
//não tinha  FILESYSTEM AQUI COMO ISSO FUNCIONNAVA?
#define Define_CurrentDir getcwd
#include <dirent.h>

int main(int argc, char *argv[])
{   
    if (argc != 2)
        throw runtime_error("Wrong use of server! Expected <port>");
    int port = stoi(argv[1]);

    //filesystem::path filepath = filesystem::current_path();
     //string fpath = LOCAL_DIR + "/clients";
     char LOCAL_DIR[1000];

         if (!Define_CurrentDir(LOCAL_DIR, sizeof(LOCAL_DIR)))
             {
             	return 0;
             }
         
         else  
          { 
            string fpath = string(LOCAL_DIR) + "/clients";
            if(mkdir(fpath.c_str(), 0777) == -1)
                //throw runtime_error("Failed to create clients directory or directory already exists");
                cout << "Failed to create clients directory or directory already exists" << endl;
            else
                cout << "Directory clients created" << endl;
          }

    Server server(port);
    server.serverLoop();
    return 0;
}