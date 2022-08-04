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
        // TODO: semaforo
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

    void test()
    {
        // inicio SC
        // mexe nos arquivos
        // fim SC
        cout << "test working \n";
    }

    static void *userConnectionLoop(void *param)
    {
        // TODO: destruir UserConnection ao desconectar ou ocorrer erro

        userConnectionData data = *(userConnectionData *)param;

        char buffer[256];
        int n;
        while (1)
        {
            bzero(buffer, 256);

            n = read(data.socket, buffer, 256);
            if (n < 0)
            {
                printf("%d ERROR reading from socket\n", data.socket);
                break;
            }
            if (n == 0)
            {
                printf("%d Disconnected\n", data.socket);
                break;
            }

            // TODO: parser comandos
            printf("%d %s CMD received: %s", data.socket, data.username, buffer);

            // (*data.ref).test();

            char up[256] = "upload";
            if (strcmp(buffer, up) == 0)
            {
                printf("UPLOAD!\n");
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
        }
    }
};

int main()
{
    Server server;
    server.serverLoop();
    return 0;
}