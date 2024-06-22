#include "circular_linked_list.h"
#include "server.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#ifndef BATTLEDOT_DOT_H
#define BATTLEDOT_DOT_H

#define MAX_NAME_LENGTH 17

typedef struct playerInfo PlayerInfo;
struct playerInfo {
  uint32_t x;
  uint32_t y;
  char *name;
};

typedef struct playerStatus PlayerStatus;
struct playerStatus {
  uint32_t flags;
  uint32_t x;
  uint32_t y;
  pthread_cond_t is_changed;
  pthread_mutex_t status_mutex;
};

typedef struct playerInstance PlayerInstance;
struct playerInstance {
  PlayerInfo pinfo;
  PlayerStatus *pstatus;
};

typedef struct battleDotConfig BattleDotConfig;
struct battleDotConfig {
  uint32_t flags;
  size_t max_players;
  FILE *log;
};
typedef struct battleDotInstance BattleDotInstance;
struct battleDotInstance {
  BattleDotConfig config;
  CircularLinkedList player_cll;
};

void pinfo_new(PlayerInfo *, uint32_t, uint32_t, char[MAX_NAME_LENGTH]);
void pstatus_new(PlayerStatus *);
void pstatus_update(PlayerStatus *, uint32_t, uint32_t, uint32_t);
void pstatus_clear(PlayerStatus *);
void pinstance_new(PlayerInstance *, PlayerInfo, PlayerStatus *);

void bdot_config_new(BattleDotConfig *, uint32_t, size_t, FILE *);
void bdot_instance_new(BattleDotInstance *, BattleDotConfig);
void bdot_instance_destroy(BattleDotInstance *);
int bdot_instance_add_player(BattleDotInstance *, PlayerInstance *);
int bdot_instance_remove_player(BattleDotInstance *, Node *);
void bdot_instance_run(BattleDotInstance *);

#endif
