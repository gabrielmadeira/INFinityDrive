#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 4000

class Server : public CommunicationModule, public SyncronizationModule, public FileManagementModule
{
private:
    // Hash hashTable
    // Queue fileQueue
    // LinkedList metaData[][], data[][] <--- pode ser a mesma coisa
    //  vector<vector<>>

    int sockfd, newsockfd, n;
    struct sockaddr_in serv_addr, cli_addr;
    Server()
    {

        // Initialize DB Structure

        // hashTable = new Hash();
        // queue = new Queue();

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            printf("ERROR opening socket");

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(serv_addr.sin_zero), 8);

        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            printf("ERROR on binding");
    }

    static void serverConnection()
    {
        listen(sockfd, 5);

        clilen = sizeof(struct sockaddr_in);
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
            printf("ERROR on accept");

        // ------> FORK
        // Thread principal -> serverLoop()
        // Thread conexão -> novo PC/Usuário
    }

    static void pcLoop()
    {
        // SYNC
        // LOOP Waiting for CMD
    }

    // void accessStructure () PROTEGIDA

    // void CMD1(user, etc)

    // void CMD2(user, etc)

    // void CMD3(user, etc)

    // void sync()
}

int
main(int argc, char *argv[])
{

    return 0;
}