// std
#include <bits/stdc++.h>

// inet_addr
#include <arpa/inet.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

class User
{
public:
    struct userConnectionData
    {
        int socket;
        char username[256];
        pthread_t thread;
        User *ref;
    };
    unordered_map<int, userConnectionData> userConnectionsHash;
    char username[256];
    User()
    {
        // TODO: seção crítica
        // TODO: files vector
    }

    void newUserConnection(int socket)
    {
        this->userConnectionsHash[socket] = {};
        this->userConnectionsHash[socket].socket = socket;
        strcpy(this->userConnectionsHash[socket].username, this->username);
        this->userConnectionsHash[socket].ref = this;

        if (pthread_create(&(this->userConnectionsHash[socket].thread), NULL,
                           this->userConnectionLoop, &(this->userConnectionsHash[socket])) != 0)
            printf("Failed to create thread\n");
    }

    void upload()
    {
        cout << this->username << " upload!\n";
    }

    void download()
    {
    }

    void del(string filename)
    {
        // inicio seção crítica
        // deletar arquivo
        cout << "file " << filename << " from user " << this->username << " deleted!\n";
        // syncAllUserConnections() -> propagar para todos as conexões do usuário conectadas
        // fim seção crítica
    }

    void listServer()
    {
    }

    void syncAllUserConnections()
    {
        // após o diretório ser atualizado, será necessário disparar um comando de sincronização para todos os clientes conectados
        // como isso será feito?
        // exemplo para iterar o hash de conexões
        cout << "Syncing " << this->username << " socket connections: ";
        for (auto &it : this->userConnectionsHash)
        {
            cout << it.first << " "; // first é a chave(ou numero do socket) e second o struct com os dados da conexão
        }
        cout << "\n";
    }

    static void *userConnectionLoop(void *param)
    {
        // TODO: destruir UserConnection ao desconectar ou ocorrer erro

        userConnectionData info = *(userConnectionData *)param;

        char buffer[256];
        int n;
        while (1)
        {
            bzero(buffer, 256);

            n = read(info.socket, buffer, 256);
            if (n < 0)
            {
                printf("%d ERROR reading from socket\n", info.socket);
                break;
            }
            if (n == 0)
            {
                printf("%d Disconnected\n", info.socket);
                break;
            }

            printf("%d %s CMD received: %s", info.socket, info.username, buffer);

            // CMDs in the format "cmd|parameter1|parameter2|etc|"
            string delimiter = "|";
            string data(buffer);
            string cmd = data.substr(0, data.find(delimiter));
            data.erase(0, data.find(delimiter) + delimiter.length());

            if (cmd == "upload")
            {
                (*info.ref).upload();
            }
            else if (cmd == "download")
            {
                (*info.ref).download();
            }
            else if (cmd == "delete")
            {
                string filename = data.substr(0, data.find("|"));
                data.erase(0, data.find(delimiter) + delimiter.length());

                (*info.ref).del(filename);
            }
            else if (cmd == "list_server")
            {
                (*info.ref).listServer();
            }
            else if (cmd == "sync")
            {
                (*info.ref).syncAllUserConnections();
            }
            else
            {
                cout << "unknown cmd!\n";
            }
        }
        return nullptr;
    }
};

class Server
{
public:
    unordered_map<string, User> usersHash;
    // TODO: recuperar usuários existentes do disco

    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;

    socklen_t addr_size;
    Server()
    {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(4000);

        bind(serverSocket,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr));

        addr_size = sizeof(serverStorage);

        if (listen(serverSocket, 50) == 0)
            printf("Listening\n");
        else
            printf("Error\n");
    }

    void serverLoop()
    {
        while (1)
        {
            newSocket = accept(serverSocket,
                               (struct sockaddr *)&serverStorage,
                               &addr_size);

            char user[256];
            read(newSocket, user, 256);

            if (!usersHash.count(user))
            {
                usersHash[user] = User();
                strcpy(usersHash[user].username, user);
            }
            cout << "Connection #" << newSocket << " from user " << user << "\n";
            usersHash[user].newUserConnection(newSocket);
            bzero(user, 256);
        }
    }
};

int main()
{
    Server server;
    server.serverLoop();
    return 0;
}