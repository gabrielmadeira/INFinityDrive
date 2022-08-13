#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <tuple>
#include "connection.hpp"

#define PORT 4000

int main()
{
	int sockfd, newsockfd, n;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	listen(sockfd, 50);
	cout << "Say something" << endl;
	
	clilen = sizeof(struct sockaddr_in);
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
		printf("ERROR on accept");
	
    tuple<PROTOCOL_TYPE, string> device = receiveProtocol(newsockfd);
    std::cout << "Servidor :: " << get<1>(device) << endl;

    while(1)
    {
        tuple<PROTOCOL_TYPE, string> ret = receiveProtocol(newsockfd);
        std::cout << "Servidor :: " << get<1>(ret);
        if(get<0>(ret) == DELT) break;
    }

    sendProtocol(newsockfd, "Vrum dum dum dum duuuuuum siuuuu", DATA);

	close(newsockfd);
	close(sockfd);
	return 0; 
}