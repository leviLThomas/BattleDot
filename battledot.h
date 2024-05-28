#ifndef BATTLEDOT_DOT_H
#define BATTLEDOT_DOT_H

#define BUF_SIZE 24
#define SERVER_PATH "./server_socket"
#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define CLIENT_PATH "./clients"

struct PlayerList {
  struct PlayerNode *head;
  struct PlayerNode *last;
};

struct PlayerNode {
  // Three characters and null char
  char moniker[4];
  int x_ship;
  int y_ship;

  // next and previous player in the list
  struct PlayerNode *next;
  struct PlayerNode *last;
};

int checkEmpty();
void insertPlayer(char moniker[4], int x_ship, int y_ship);
void removePlayer(struct PlayerNode *loser);

#endif
