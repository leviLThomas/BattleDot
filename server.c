#include "battledot.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>

#define BACKLOG 10

int main(int argc, char **argv) {
  int sfd, cfd;
  char buffer[BUF_SIZE];
  struct sockaddr_un server_addr, client_addr;

  if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "SERVER ERROR Failed to create socket: %s\n",
            strerror(errno));
    return 1;
  }
  fprintf(stdout, "SERVER SUCCESS Successfully created socket\n");

  strcpy(server_addr.sun_path, SERVER_PATH);
  server_addr.sun_family = AF_UNIX;

  if (unlink(server_addr.sun_path) == -1 && errno != ENOENT) {
    fprintf(stderr, "SERVER Failed to unlink socket: %s\n", strerror(errno));
    close(sfd);
    return 1;
  }

  if (bind(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
    close(sfd);
    return 1;
  }

  if (listen(sfd, BACKLOG) == -1) {
    fprintf(stderr, "Server listen failed: %s\n", strerror(errno));
    close(sfd);
    return 1;
  }

  fprintf(stdout,
          "Server initialization finished. now accepting connections\n");

  socklen_t client_size = sizeof(client_addr);
  while ((cfd = accept(sfd, (struct sockaddr *)&client_addr, &client_size)) !=
         -1) {
    fprintf(stdout, "Server accepted connection from client: %s\n",
            client_addr.sun_path);

    if (recv(cfd, buffer, sizeof(buffer), 0) == -1) {
      fprintf(stderr, "Server failed to received from client: %s\n",
              strerror(errno));
    }

    if (strncmp(buffer, "S", 1) == 0) {
      if (close(cfd) == -1) {
        fprintf(stderr, "client close failed: %s\n", strerror(errno));
        return 1;
      }
      fprintf(stdout, "Server recieved start! Now closing\n");
      break;
    }

    fprintf(stdout, "Server recieved:\n\t%s\n", buffer);
  }

  if (close(sfd) == -1) {
    fprintf(stderr, "Server close failed: %s\n", strerror(errno));
    return 1;
  }

  if (unlink(SERVER_PATH) == -1) {
    fprintf(stderr, "Server unlink failed: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}
