#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "battledot.h"

pthread_mutex_t thread;
pthread_mutex_t players;
pthread_mutex_t log_file;
pthread_mutex_t server_client;

extern struct Player *front;
extern struct Player *current;

FILE *logfile;
time_t localltime;
struct tm finalTime;
char charTime[32];

typedef struct {
  int *serv;
  int *clie;
  char *moniker;
} socket_data;

/*
Description:
Gets the local time and formats it into a character string with asctime_r. This
is not thread-safe.

Arguments:
        none

Returns:
        Local time formatted as WWW MMM DD HH:MM:SS YYYY
*/
char *getTime() {
  localltime = time(NULL);
  localtime_r(&localltime, &finalTime);
  return asctime_r(&finalTime, charTime);
}

/*
Description:
Runs the game once all players have been entered. Chooses random target
coordinates between 1 and 10 then checks if those coordinates hit the next
player in line. If they hit that player then they are eliminated and the player
who attacked targets the player after the one who was eliminated. Also handles
cleanup when a winner has been declared.

Arguments:
        None

Returns:
        None
*/
void gameStart() {
  while (!(current->next == current)) {
    int x_target = rand() % (10 - 1 + 1) + 1,
        y_target = rand() % (10 - 1 + 1) + 1;

    int hit = 0;
    if (x_target == current->next->x_ship &&
        y_target == current->next->y_ship) {
      hit = 1;
    }

    pthread_mutex_lock(&log_file);
    fprintf(logfile,
            "%s\t: > It is %s's turn and they bombed %s with values x = %d and "
            "y = %d.\n",
            getTime(), current->moniker, current->next->moniker, x_target,
            y_target);
    pthread_mutex_unlock(&log_file);

    if (hit == 1) {
      pthread_mutex_lock(&log_file);
      fprintf(logfile, "%s\t: > %s hit %s. It is now %s's turn.\n", getTime(),
              current->moniker, current->next->moniker,
              current->next->next->moniker);
      pthread_mutex_unlock(&log_file);
      current = current->next->next;
      removePlayer(current->last);
    }

    else {
      pthread_mutex_lock(&log_file);
      fprintf(logfile, "%s\t: > %s missed, it is now %s's turn\n", getTime(),
              current->moniker, current->next->moniker);
      pthread_mutex_unlock(&log_file);
      current = current->next;
    }
  }

  pthread_mutex_lock(&log_file);
  fprintf(logfile, "%s\t: > %s has won!\n", getTime(), current->moniker);
  pthread_mutex_unlock(&log_file);
  fclose(logfile);
  pthread_mutex_unlock(&server_client);
  removePlayer(current);
  exit(0);
}

/*
Description:
Handles reading the monikers and coordinates of player ships from the buffer.
Also initializes the current player as the second player who joined. It will
also check if the start signal has been read from the buffer. If it has then it
will execute gameStart().

Arguments:
        void *socket_info : Contains the server socket, client socket, and
moniker for a player. This is used in recv to extract from the buffer.

Returns:
        NULL
*/
void *client_handler(void *socket_info) {
  char moniker[4];
  char xChar[9], yChar[9];
  int xInt, yInt;
  char buffer[24];

  socket_data *info = socket_info;
  int client = *info->clie;
  strncpy(moniker, info->moniker, 4);
  recv(client, buffer, sizeof(buffer), 0);
  pthread_mutex_unlock(&server_client);

  if (strncmp(buffer, "STRT", 4) == 0) {
    close(client);
    free(info);
    gameStart();
  }

  else {
    pthread_mutex_lock(&players);
    int i;
    int j = 0;
    for (i = 4; i < 12; i++) {
      xChar[j] = buffer[i];
      j++;
    }

    j = 0;
    for (i = 12; i < 20; i++) {
      yChar[j] = buffer[i];
      j++;
    }

    xInt = atoi(xChar);
    yInt = atoi(yChar);

    insertPlayer(moniker, xInt, yInt);

    pthread_mutex_lock(&log_file);
    fprintf(logfile,
            "%s\t > %s has joined the game. Their ship is at x = %d, y = %d.\n",
            getTime(), moniker, xInt, yInt);
    pthread_mutex_unlock(&log_file);

    // second player joins
    if (front->next != front && front->next->next == front) {
      current = front->last;
    }

    pthread_mutex_unlock(&players);
  }

  close(client);
  free(info);
  return NULL;
}

void logMessage(const char *level, const char *message, const char *err) {
  fprintf(logfile, "%s\t > %s %s: %s\n", getTime(), level, message, err);
  fprintf(stdout, "%s\t > %s %s: %s\n", getTime(), level, message, err);
}

/*
Description:
Initializes the server socket, threads, file, and begins the process of adding
clients to the list of players


Arguments:
        None

Returns:
        None
*/
int main(void) {
  socklen_t size;
  int serv, clie;

  struct sockaddr_un server_addr, client_addr;

  // int threadAmnt = 0;
  // pthread_t threads[1024];
  // pthread_mutex_init(&players, NULL);
  // pthread_mutex_init(&thread, NULL);
  // pthread_mutex_init(&server_client, NULL);

  srand(time(NULL));

  logfile = fopen("server.log", "a+");

  if ((serv = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    logMessage("ERROR", "Failed to create server socket", strerror(errno));
    exit(EXIT_FAILURE);
  }
  logMessage("INFO", "Server socket created successfully", strerror(errno));

  strcpy(server_addr.sun_path, "./serv_socket");
  server_addr.sun_family = AF_UNIX;
  size = sizeof(server_addr);

  if (unlink("./serv_socket") == -1 && errno != ENOENT) {
    fprintf(logfile, "%s\t > ERROR Failed to unlink from serv_socket: %s\n",
            getTime(), strerror(errno));
    exit(EXIT_FAILURE);
  } else {
  }

  if (bind(serv, (struct sockaddr *)&server_addr, size) == -1) {
    fprintf(logfile, "%s\t > ERROR Failed to bind serv_socket: %s\n", getTime(),
            strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    fprintf(logfile, "%s\t > Socket bound to %s successfully\n", getTime(),
            server_addr.sun_path);
  }

  if (listen(serv, 3) == -1) {
    fprintf(logfile, "%s\t > ERROR Server listen failed: %s\n", getTime(),
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  fprintf(logfile, "%s\t > Server is now listening on socket\n", getTime());

  fprintf(logfile,
          "%s\t > Server initialization complete, now accepting connections\n",
          getTime());
  while ((clie = accept(serv, (struct sockaddr *)&client_addr, &size)) != -1) {

    fprintf(logfile, "%s\t > Connection accepted from client at %s\n",
            getTime(), client_addr.sun_path);
    // pthread_mutex_init(&server_client, NULL);

    socket_data *info = malloc(sizeof *info);
    info->serv = &serv;
    info->clie = &clie;
    info->moniker = malloc(4);

    info->moniker = client_addr.sun_path + strlen(client_addr.sun_path) - 3;

    // pthread_mutex_lock(&thread);
    // pthread_create(&threads[threadAmnt], NULL, client_handler, info);
    // threadAmnt++;
    // pthread_mutex_unlock(&thread);
  }

  if (clie == -1) {
    fprintf(logfile, "%s\t > ERROR Failed to accept client: %s\n", getTime(),
            strerror(errno));
    fclose(logfile);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
