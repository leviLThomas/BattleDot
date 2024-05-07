#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
int main(int argc, char **argv) {
  int server, client;
  struct sockaddr_un server_addr, client_addr;

  if ((server = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "CLIENT ERROR Failed to create socket: %s\n",
            strerror(errno));
    return 1;
  }
  fprintf(stdout, "CLIENT SUCCESS Successfully created socket\n");

  strcpy(server_addr.sun_path, "./serv_socket");
  server_addr.sun_family = AF_UNIX;

  if (unlink(server_addr.sun_path) == -1 && errno != ENOENT) {
    fprintf(stderr, "SERVER Failed to unlink socket: %s\n", strerror(errno));
    close(server);
    return 1;
  }

  if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
    close(server);
    return 1;
  }

  if (listen(server, 1) == -1) {
    fprintf(stderr, "Server listen failed: %s\n", strerror(errno));
    close(server);
    return 1;
  }

  fprintf(stdout,
          "Server initialization finished. now accepting connections\n");

  socklen_t client_size = sizeof(client_addr);
  if ((client = accept(server, (struct sockaddr *)&client_addr,
                       &client_size)) == -1) {
    fprintf(stderr, "Server accept failed: %s\n", strerror(errno));
    return 1;
  }

  fprintf(stdout, "Server accepted connection from client: %s\n",
          client_addr.sun_path);

  if (close(server) == -1) {
    fprintf(stderr, "Server close failed: %s\n", strerror(errno));
    return 1;
  }

  if (unlink("./serv_socket") == -1) {
    fprintf(stderr, "Server unlink failed: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}
