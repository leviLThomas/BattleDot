#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "battledot.h"

int main(int argc, char **argv) {
  char buffer[BUF_SIZE], client_path[256];
  char *name, *x_char, *y_char;
  int cfd, s_flag, j_flag, n_flag, x_flag, y_flag;

  struct sockaddr_un server_addr, client_addr;
  s_flag = j_flag = n_flag = x_flag = y_flag = 0;

  int opt;
  while ((opt = getopt(argc, argv, "sjn:x:y:")) != -1) {
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
      snprintf(client_path, sizeof(client_path), "%s%s", CLIENT_PATH, name);
      fprintf(stdout, "Name flag recieved %s.\n", name);
      break;
    case 'x':
      x_flag = 1;
      x_char = optarg;
      break;
    case 'y':
      y_flag = 1;
      y_char = optarg;
      break;
    case '?':
      fprintf(stderr, "Invalid character %s entered in command line.\n",
              (char *)optopt);
      return 1;
    }
  }

  if ((cfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
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

  memset(&client_addr, 0, sizeof(client_addr));
  client_addr.sun_family = AF_UNIX;
  strncpy(client_addr.sun_path, client_path, sizeof(client_addr.sun_path) - 1);

  fprintf(stdout, "CLIENT SUCCESS client address is %s\n",
          client_addr.sun_path);

  if (s_flag) {
    strcat(buffer, "S");
  } else {
    strcat(buffer, "J");
    int i = 0;
    for (i = 0; i < 8 - strlen(x_char); i++) {
      strcat(buffer, "0");
    }
    strcat(buffer, x_char);

    for (i = 0; i < 8 - strlen(y_char); i++) {
      strcat(buffer, "0");
    }
    strcat(buffer, y_char);
    fprintf(stdout, "Buffer currently looks like:\n\t%s\n", buffer);
  }

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, "./server_socket");

  if (unlink(client_addr.sun_path) == -1 && errno != ENOENT) {
    fprintf(stderr, "Failed to unlink socket: %s\n", strerror(errno));
    close(cfd);
    return 1;
  }

  if (bind(cfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
    fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
    close(cfd);
    return 1;
  }
  fprintf(stdout, "CLIENT SUCCESS Successfully bound socket\n");

  if (connect(cfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    fprintf(stderr, "Error connecting to server socket: %s\n", strerror(errno));
    close(cfd);
    return 1;
  }

  if (send(cfd, buffer, strlen(buffer), 0) == -1) {
    fprintf(stderr, "Error sending request to server: %s\n", strerror(errno));
    close(cfd);
    return 1;
  }

  if (close(cfd) == -1) {
    fprintf(stderr, "CLIENT ERROR Failed to close socket: %s\n",
            strerror(errno));
    return 1;
  }
  fprintf(stdout, "CLIENT SUCCESS Successfully closed socket\n");

  return 0;
}
