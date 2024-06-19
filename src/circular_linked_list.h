#include <stdlib.h>
#ifndef CIRCULAR_LINKED_LIST_DOT_H
#define CIRCULAR_LINKED_LIST_DOT_H

typedef struct Node {
  struct Node *next;
  struct Node *prev;
  void *value;
} node_t;
typedef node_t Node;

typedef struct {
  Node *head;
  Node *tail;
  size_t size;
} cll_t;
typedef cll_t CircularLinkedList;

/**
 * @brief
 * @return
 */
void cll_new(CircularLinkedList *cll);
void cll_destroy(CircularLinkedList *cll);
Node *cll_push_back(CircularLinkedList *cll, void *element);
void *cll_pop_node(CircularLinkedList *cll, Node *node);
Node *node_new(Node *node, void *element);
Node *cll_push_front(CircularLinkedList *cll, void *element);
void *cll_pop_front(CircularLinkedList *cll);
void *cll_pop_back(CircularLinkedList *cll);
int cll_remove_node(CircularLinkedList *cll, Node *node);
int cll_is_empty(const CircularLinkedList *cll);
Node *cll_find_node(const CircularLinkedList *cll, const Node *target);
size_t cll_get_size(const CircularLinkedList *cll);

#endif
