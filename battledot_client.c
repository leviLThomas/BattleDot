#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

/*
Description:
Processes entries from the terminal, creates the client socket, send the xCoord
and yCoord down the buffer along with either JOIN or STRT. It then binds to the
socket, connects with the server socket, and sends the buffer to the server.

Arguments:
        int argc 	 : the amount of command line arguments entered when
executing the client char ** argv : array of character arrays containing the
arguments entered on the command line

Returns:
        None
*/
int main(int argc, char **argv) {
  int client;
  int size;

  // 24 characters plus three character moniker
  char directory[28] = "./clients/client_socket_";
  char *moniker = NULL;
  char *xChar = NULL;
  char *yChar = NULL;

  int startFlag = 0, monikerFlag = 0, joinFlag = 0, xFlag = 0, yFlag = 0;

  // Four characters either JOIN or STRT followed by 8 digits, x and y coord
  // cushioned by 0's
  char buffer[24] = {0};

  int opt;

  while ((opt = getopt(argc, argv, "sm:jx:y:")) != -1) {
    switch (opt) {
    case 's':
      startFlag = 1;
      break;

    case 'm':
      monikerFlag = 1;
      moniker = optarg;
      break;

    case 'j':
      joinFlag = 1;
      break;

    case 'x':
      xFlag = 1;
      xChar = optarg;
      break;

    case 'y':
      yFlag = 1;
      yChar = optarg;
      break;

    case '?':
      fprintf(stderr, "Invalid character %s entered in command line.\n",
              (char)optopt);
      return 1;
    }
  }

  struct sockaddr_un serv_addr;
  struct sockaddr_un clie_addr;

  client = socket(AF_UNIX, SOCK_STREAM, 0);
  clie_addr.sun_family = AF_UNIX;

  // if not start game then check other flags and add JOIN+x+y to buffer
  if (!startFlag) {
    if (!monikerFlag) {
      fprintf(stderr, "Error: must specify moniker with -m\n");
    }

    if (!xFlag || !yFlag) {
      fprintf(
          stderr,
          "Error: must include both x and y coordinates with -x and -y flag\n");
    }

    if (!joinFlag) {
      fprintf(stderr, "Error: must include -j flag to join\n");
    }

    strcpy(clie_addr.sun_path, strcat(directory, moniker));
    strcat(buffer, "JOIN");
    int i = 0;
    for (i = 0; i < 8 - strlen(xChar); i++) {
      strcat(buffer, "0");
    }
    strcat(buffer, xChar);

    for (i = 0; i < 8 - strlen(yChar); i++) {
      strcat(buffer, "0");
    }
    strcat(buffer, yChar);
  }

  // if start game then add STRT to buffer
  else {
    strcat(buffer, "STRT");
  }

  size = sizeof(clie_addr);

  unlink(clie_addr.sun_path);
  int test = bind(client, (struct sockaddr *)&clie_addr, size);
  if (test == -1) {
    char *s = strerror(errno);
    fprintf(stderr, "Failed to bind socket: %s\n", s);
  }

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, "./serv_socket");

  test = connect(client, (struct sockaddr *)&serv_addr, size);
  if (test == -1) {
    char *s = strerror(errno);
    fprintf(stderr, "Error connecting to server socket: %s\n", s);
    return 1;
  }

  test = send(client, buffer, strlen(buffer), 0);
  if (test == -1) {
    char *s = strerror(errno);
    fprintf(stderr, "Error sending request to server socket: %s\n", s);
    return 1;
  }

  close(client);
  return 0;
}
