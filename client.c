#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
int main(int argc, char **argv) {
  int client;
  char directory[20] = "clients/client_";
  char *name;

  int s_flag, j_flag, n_flag;
  s_flag = j_flag = n_flag = 0;

  int opt;
  while ((opt = getopt(argc, argv, "sjn:")) != -1) {
    switch (opt) {
    case 's':
      fprintf(stdout, "Start flag recieved.\n");
      s_flag = 1;
      break;
    case 'j':
      fprintf(stdout, "Join flag recieved.\n");
      j_flag = 1;
      break;
    case 'n':
      n_flag = 1;
      name = optarg;
      fprintf(stdout, "Name flag recieved %s.\n", name);
      break;
    case '?':
      fprintf(stderr, "Invalid character %s entered in command line.\n",
              (char *)optopt);
      return 1;
    }
  }

  struct sockaddr_un serv_addr;
  struct sockaddr_un clie_addr;

  if ((client = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "CLIENT ERROR Failed to create socket: %s\n",
            strerror(errno));
    return 1;
  }
  fprintf(stdout, "CLIENT SUCCESS Successfully created socket\n");

  if (mkdir("./clients", 0777) && errno != EEXIST) {
    fprintf(stderr, "Failed to create directory for client sockets: %s\n",
            strerror(errno));
    return 1;
  }
  fprintf(stdout,
          "CLIENT SUCCESS Successfully created directory for client sockets\n");

  clie_addr.sun_family = AF_UNIX;
  strcpy(clie_addr.sun_path, strcat(directory, name));
  fprintf(stdout, "CLIENT SUCCESS client address is %s\n", clie_addr.sun_path);

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, "./serv_socket");

  int size = sizeof(clie_addr);

  if (unlink(clie_addr.sun_path) == -1 && errno != ENOENT) {
    fprintf(stderr, "Failed to unlink socket: %s\n", strerror(errno));
    close(client);
    return 1;
  }

  if (bind(client, (struct sockaddr *)&clie_addr, size) == -1) {
    fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
    close(client);
    return 1;
  }
  fprintf(stdout, "CLIENT SUCCESS Successfully bound socket\n");

  if (connect(client, (struct sockaddr *)&serv_addr, size) == -1) {
    fprintf(stderr, "Error connecting to server socket: %s\n", strerror(errno));
    close(client);
    return 1;
  }

  if (close(client) == -1) {
    fprintf(stderr, "CLIENT ERROR Failed to close socket: %s\n",
            strerror(errno));
    return 1;
  }
  fprintf(stdout, "CLIENT SUCCESS Successfully closed socket\n");
}
