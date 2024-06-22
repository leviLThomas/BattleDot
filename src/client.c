#include "server.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// TODO refactor
int bdot_connect(void) {
  int sockfd;
  int rv;
  char s[INET6_ADDRSTRLEN];
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and connect to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
            sizeof s);
  printf("client: connecting to %s\n", s);
  freeaddrinfo(servinfo); // all done with this structure
  return sockfd;
}

// TODO refactor
void bdot_parse(struct DataPacket *packet, int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "h:sjn:x:y:")) != -1) {
    switch (opt) {
    case 's':
      packet->flags |= START;
      break;
    case 'j':
      packet->flags |= JOIN;
      break;
    case 'n':
      strncpy(packet->buffer, optarg, sizeof(packet->buffer));
      break;
    case 'x':
      packet->x = htonl(atoi(optarg));
      break;
    case 'y':
      packet->y = htonl(atoi(optarg));
      break;
    default:
      fprintf(stderr,
              "usage: %s [-s] or [-j] [-n] name [-x] x_coord [-y] y_coord\n",
              argv[0]);
    }
  }

  if (strlen(packet->buffer) < 4 || strlen(packet->buffer) > 16 ||
      ntohl(packet->x) < 1 || ntohl(packet->x) > 10 || ntohl(packet->y) < 1 ||
      ntohl(packet->y) > 10) {
    fprintf(stderr,
            "usage: %s [-s] or [-j] [-n] name [-x] x_coord [-y] y_coord\n",
            argv[0]);
  }
}

// TODO refactor
void bdot_client_loop(int serverfd, struct DataPacket packet) {
  int numbytes;
  char s[INET6_ADDRSTRLEN];
  numbytes = send(serverfd, &packet, sizeof(packet), 0);
  if (numbytes == -1) {
    perror("send");
    exit(EXIT_FAILURE);
  }
  printf("client: sent \n\tname: %s\n\tx: %u\n\ty: %u\nto %s\n", packet.buffer,
         ntohl(packet.x), ntohl(packet.y), s);

  for (;;) {
    scanf("Enter X: %u", &packet.x);
    scanf("Enter Y: %u", &packet.y);
    packet.flags |= ATTACK;
    numbytes = send(serverfd, &packet, sizeof(packet), 0);
    if (numbytes == -1) {
      perror("send");
      exit(EXIT_FAILURE);
    }

    numbytes = recv(serverfd, &packet, sizeof(packet), 0);
    if (numbytes == -1) {
      perror("recv");
      exit(EXIT_FAILURE);
    }
    packet.buffer[MAXBUFSIZE - 1] = '\0';

    printf("client: received '%s'\n", packet.buffer);

    if (packet.flags & EXIT) // If the server tells us to exit we break;
      break;
  }
}

// TODO refactor
int main(int argc, char *argv[]) {

  struct DataPacket packet;
  memset(&packet, 0, sizeof(packet));

  char buf[MAXDATASIZE];
  memset(buf, 0, MAXDATASIZE - 1);

  bdot_parse(&packet, argc, argv);

  int sockfd = bdot_connect();

  bdot_client_loop(sockfd, packet);

  close(sockfd);

  exit(EXIT_SUCCESS);
}
