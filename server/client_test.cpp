#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 4000

int main(int argc, char *argv[])
{
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  if (argc < 3)
  {
    fprintf(stderr, "usage %s hostname\n", argv[0]);
    exit(0);
  }

  server = gethostbyname(argv[1]);
  if (server == NULL)
  {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    printf("ERROR opening socket\n");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serv_addr.sin_zero), 8);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    printf("ERROR connecting\n");

  /* write in the socket: username */
  n = write(sockfd, argv[2], strlen(argv[2]));
  if (n < 0)
    printf("ERROR writing to socket\n");

  while (1)
  {
    printf("Enter CMD: ");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);

    /* write in the socket: cmd */
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
      printf("ERROR writing to socket\n");
  }
  close(sockfd);
  return 0;
}