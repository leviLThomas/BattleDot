#include <stdint.h>
#ifndef BATTLEDOT_DOT_H
#define BATTLEDOT_DOT_H

typedef struct {
  uint32_t x;
  uint32_t y;
  uint16_t status;
} PlayerInfo;

typedef struct {
  struct Node *next;
  struct Node *prev;
  PlayerInfo value;
} Node;

typedef struct {
  Node *head;
  Node *last;
} CircularLinkedList;

int checkEmpty(void);
void insertPlayer(char moniker[4], int x_ship, int y_ship);
void removePlayer(struct PlayerNode *loser);

#endif
