#include "battledot.h"
#include <stdio.h>

void pinfo_new(PlayerInfo *pinfo, uint32_t x, uint32_t y, char *name) {
  pinfo->x = x;
  pinfo->y = y;
  snprintf(pinfo->name, MAX_NAME_LENGTH, "%s", name);
}

void pstatus_new(PlayerStatus *pstatus, pthread_cond_t is_changed) {
  pstatus->is_changed = is_changed;
}
void pstatus_update(PlayerStatus *pstatus, uint32_t x, uint32_t y,
                    uint32_t flags) {
  pstatus->x = x;
  pstatus->y = y;
  pstatus->flags = flags;
  pthread_cond_signal(&pstatus->is_changed);
}
void pstatus_clear(PlayerStatus *pstatus) {
  pstatus->x = 0;
  pstatus->y = 0;
  pstatus->flags &= 0;
}

void pinstance_new(PlayerInstance *pinstance, PlayerInfo pinfo,
                   PlayerStatus pstatus, pthread_mutex_t statusMutex) {
  pinstance->pinfo = pinfo;
  pinstance->pstatus = pstatus;
  pinstance->statusMutex = statusMutex;
}

void bdot_config_new(BattleDotConfig *bdot_config, uint32_t flags,
                     size_t max_players, FILE *log) {
  bdot_config->flags = flags;
  bdot_config->max_players = max_players;
  bdot_config->log = log;
}
void bdot_instance_new(BattleDotInstance *bdot_instance,
                       BattleDotConfig bdot_config) {
  bdot_instance->config = bdot_config;
  CircularLinkedList cll;
  cll_new(&cll);
  bdot_instance->player_cll = cll;
}
void bdot_instance_destroy(BattleDotInstance *);
int bdot_instance_add_player(BattleDotInstance *bdot_instance,
                             PlayerInstance *pinstance) {
  if (cll_get_size(&bdot_instance->player_cll) >
      bdot_instance->config.max_players - 1) {
    return 0;
  }
  cll_push_back(&bdot_instance->player_cll, pinstance);
  return 1;
}
int bdot_instance_remove_player(BattleDotInstance *bdot_instance, Node pnode) {
  if (cll_get_size(&bdot_instance->player_cll) <= 0)
    return 0;
  int res = cll_remove_node(&bdot_instance->player_cll, pnode);
  return res;
}
void bdot_instance_run(BattleDotInstance *bdot_instance) {
  for (Node cur = bdot_instance->player_cll.head; !cll_is_empty(&bdot_instance->player_cll);) {
    PlayerInstance *cur_pinstance = (PlayerInstance *)cur->value;
    PlayerInstance *next_pinstance = (PlayerInstance *)cur->next->value;

    pthread_mutex_lock(&cur_pinstance->statusMutex);
    if (!(cur_pinstance->pstatus.flags & ATTACK))
      pthread_cond_wait(&cur_pinstance->pstatus.is_changed,
                        &cur_pinstance->statusMutex);

    uint32_t attack_x = cur_pinstance->pinfo.x;
    uint32_t attack_y = cur_pinstance->pinfo.y;
    pstatus_clear(&cur_pinstance->pstatus);
    pthread_mutex_unlock(&cur_pinstance->statusMutex);

    if ((next_pinstance->pinfo.x == attack_x) &&
        (next_pinstance->pinfo.y == attack_y)) {
      cll_remove_node(&bdot_instance->player_cll, cur->next);
    }

    cur = cur->next;
  }
}
