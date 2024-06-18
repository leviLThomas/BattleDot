#include "circular_linked_list.h"
#include <stdlib.h>

void cll_new(CircularLinkedList *cll) {
  cll->head = NULL;
  cll->tail = NULL;
  cll->size = 0;
}

void cll_destroy(CircularLinkedList *cll) {
  for (Node *cur = cll->head; cur != NULL;) {
    Node *next = cur->next;
    free(cur);
    cur = next;
  }
}

int cll_is_empty(const CircularLinkedList *cll) {
  return cll_get_size(cll) == 0;
}

size_t cll_get_size(const CircularLinkedList *cll) { return cll->size; }

Node *cll_push_front(CircularLinkedList *cll, void *element) {
  Node node = node_new(element);

  if (!cll->head) {
    node->next = node;
    node->prev = node;
    cll->head = node;
    cll->tail = node;
  }

  if (cll->head) {
    node->next = cll->head;
    node->prev = cll->tail;
    cll->head->prev->next = node;
    cll->head->prev = node;
  }

  cll->head = node;
  cll->size++;

  return node;
}
Node cll_push_back(CircularLinkedList *cll, void *element) {
  Node node = node_new(element);

  if (!cll->head) {
    node->next = node;
    node->prev = node;
    cll->head = node;
    cll->tail = node;
  } else if (cll->tail) {
    node->next = cll->head;
    node->prev = cll->tail;
    cll->tail->next->prev = node;
    cll->tail->next = node;
  }

  cll->tail = node;
  cll->size++;

  return node;
}
void *cll_pop_node(CircularLinkedList *cll, Node node) {
  void *element;
  if (!node) {
    return (NULL);
  }

  element = node->value;
  cll_remove_node(cll, node);
  return (element);
}
void *cll_pop_front(CircularLinkedList *cll) {
  return (cll_pop_node(cll, cll->head));
}
void *cll_pop_back(CircularLinkedList *cll) {
  return (cll_pop_node(cll, cll->tail));
}
int cll_remove_node(CircularLinkedList *cll, Node node) {
  Node found = cll_find_node(cll, node);
  if (found == NULL)
    return 0;

  if (cll->head == found && cll->tail == found) {
    cll->head = NULL;
    cll->tail = NULL;
  } else {
    found->prev->next = found->next;
    found->next->prev = found->prev;
  }
  if (cll->head == found) {
    cll->head = found->next;
  }
  if (cll->tail == found) {
    cll->tail = found->prev;
  }
  cll->size--;

  return 1;
}
Node cll_find_node(CircularLinkedList *cll, Node target) {
  for (Node cur = cll->head; cur != cll->tail; cur = cur->next) {
    if (cur == target) {
      return cur;
    }
  }
  return NULL;
}
Node node_new(void *element) {
  Node node;

  node->next = NULL;
  node->prev = NULL;
  node->value = element;
  return node;
}

void node_destroy(Node node) {
  free(node);
}
