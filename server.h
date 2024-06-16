#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <sys/wait.h>

#ifndef SERVER_DOT_H
#define SERVER_DOT_H

#define MAX_THREADS 16
#define MAXDATASIZE 1024 // max number of bytes we can get and send at once
#define MAXBUFSIZE 512   // max number of bytes we can get and send at once
#define BACKLOG 10
#define PORT "8080" // the port client will be connecting to

#define JOIN 0x0001
#define START 0x0010
#define EXIT 0x0100
#define ATTACK 0x1000

struct DataPacket {
  uint16_t flags;
  char buffer[MAXBUFSIZE];
  uint32_t x;
  uint32_t y;
};

void sigchld_handler(int);
void *get_in_addr(struct sockaddr *sa);

#endif
