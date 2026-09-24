/**
 * HTTP Server for receiving POST from ComputerCraft to populate an SQLite
 * database with the contents of a chest.
 *
 * Main reference: https://beej.us/guide/bgnet/html/
 */

#include "db.h"
#include <netdb.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "4499" // port to be used by server
#define BACKLOG 5   // constant for listen() queue

int bind_socket(void);
void accept_loop(sqlite3 *db, int sockfd);

int main(void)
{
  int sockfd;
  sqlite3 *db;

  sockfd = bind_socket();

  db = init_db();
  printf("server: waiting for connections...\n");

  accept_loop(db, sockfd);

  close(sockfd);
  sqlite3_close(db);

  return 0;
}

int bind_socket()
{
  struct addrinfo hints, *serv_info, *p;
  int status, sockfd;

  memset(&hints, 0, sizeof hints); // clear hints struct
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // get socket addr
  if ((status = getaddrinfo(NULL, PORT, &hints, &serv_info)) != 0) {
    fprintf(stderr, "gai: %s\n", gai_strerror(status));
  }

  // bind until first match in linked list
  for (p = serv_info; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("bind");
      continue;
    }

    break;
  }

  freeaddrinfo(serv_info); // linked list now useless

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  return sockfd;
}

void accept_loop(sqlite3 *db, int sockfd)
{
  struct sockaddr_storage client_addr;
  int client_fd;
  ssize_t bytes_received;
  socklen_t sin_size;

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  // TODO: parse http requests
  // TODO: send POSTs into db

  // main accept loop
  while (1) {
    sin_size = sizeof client_addr;
    client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
    if (client_fd == -1) {
      perror("accept");
      continue;
    }

    char buf[BUFSIZ];

    if ((bytes_received = recv(client_fd, buf, BUFSIZ, 0)) == -1) {
      perror("recv");
      exit(1);
    }

    printf("%s\n", buf);
    close(client_fd);
  }
}
