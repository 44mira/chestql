/**
 * HTTP Server for receiving POST from ComputerCraft to populate an SQLite
 * database with the contents of a chest.
 *
 * Main reference: https://beej.us/guide/bgnet/html/
 */

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "4499" // port to be used by server
#define BACKLOG 5   // constant for listen() queue

void sigchld_handler(int s)
{
  (void)s; // quiet unused variable warning

  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void)
{
  struct sigaction sa;
  struct sockaddr_storage client_addr;
  struct addrinfo hints, *serv_info, *p;
  int status, sockfd, client_fd, bytes_received; // gai status
  socklen_t sin_size;

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

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead subprocesses
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  // main accept loop, clients are handled in subprocesses.
  while (1) {
    sin_size = sizeof client_addr;
    client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
    if (client_fd == -1) {
      perror("accept");
      continue;
    }

    if (!fork()) {   // child process dedicated for client
      close(sockfd); // doesn't need the listener socket

      char buf[BUFSIZ];

      if ((bytes_received = recv(client_fd, buf, BUFSIZ, 0)) == -1) {
        perror("recv");
        exit(1);
      }

      printf("%s\n", buf);
      close(client_fd);
      exit(0);
    }
    close(client_fd);
  }

  close(sockfd);
  return 0;
}
