/**
 * HTTP Server for receiving POST from ComputerCraft to populate an SQLite
 * database with the contents of a chest.
 *
 * Main reference: https://beej.us/guide/bgnet/html/
 */

#include "arena.h"
#include "db.h"
#include "parser.h"
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
void handle_client(sqlite3 *db, int client_fd);

int main(void)
{
  sqlite3 *db;
  int sockfd;

  sockfd = bind_socket();

  printf("server: waiting for connections...\n");

  db = init_db();
  accept_loop(db, sockfd);

  close(sockfd);

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

    handle_client(db, client_fd);

    close(client_fd);
  }
}

void handle_client(sqlite3 *db, int client_fd)
{
  ssize_t bytes_received;
  struct chestql_csv *csv = NULL;

  struct arena *allocator = arena_make(DEFAULT_ARENA_SIZE);
  char *buf =
      (char *)malloc(DEFAULT_ARENA_SIZE); // scratch arena for big buffer

  // receive http -------------------------------------------------------
  if ((bytes_received = recv(client_fd, buf, DEFAULT_ARENA_SIZE, 0)) == -1) {
    perror("recv");
    arena_free(allocator);
    free(buf);
    exit(1);
  }

  // parse http ---------------------------------------------------------
  if (parse_http_request(allocator, buf, bytes_received, &csv) != 0) {
    arena_free(allocator);
    free(buf);
    exit(1);
  }

  // db handling ---------------------------------------------------------
  reset_db_table(db);        // clear the db after every successful parse
  load_csv_into_db(db, csv); // insert new rows into db
}
