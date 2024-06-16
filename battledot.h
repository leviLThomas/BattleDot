#include "circular_linked_list.h"
#include "server.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#ifndef BATTLEDOT_DOT_H
#define BATTLEDOT_DOT_H

typedef struct {
  uint32_t flags;
  uint32_t x;
  uint32_t y;
  pthread_cond_t is_changed;
} PlayerStatus;

typedef struct {
  uint32_t x;
  uint32_t y;
  PlayerStatus status;
  pthread_mutex_t statusMutex;
} PlayerInfo;

void run_game(CircularLinkedList *cll, FILE *fd) {
  for (Node cur = cll->head; !cll_is_empty(cll);) {
    PlayerInfo *player = (PlayerInfo *)cur->value;
    PlayerInfo *next_player = (PlayerInfo *)cur->next->value;


    pthread_mutex_lock(&player->statusMutex);
    if (!(player->status.flags & ATTACK))
      pthread_cond_wait(&player->status.is_changed, &player->statusMutex);

    fprintf(fd, "flags: %u\nX: %u\nY: %u\n", player->status.flags, player->status.x,
            player->status.y);
    uint32_t attack_x = player->x;
    uint32_t attack_y = player->y;
    player->status.flags &= 0;
    pthread_mutex_unlock(&player->statusMutex);

    if(( next_player->x == attack_x ) && (next_player->y == attack_y)) {
      cll_remove_node(cll, cur->next);
    }

    cur = cur->next;
  }

  fprintf(fd, "Game Over!\n");
}

#endif
