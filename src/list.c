#include "list.h"

#include <stdlib.h>  // for NULL, free, malloc

void list_init(list_t* list) {
    list->head.next = &list->tail;
    list->head.prev = NULL;
    list->tail.next = NULL;
    list->tail.prev = &list->head;
}

void list_push(list_t* list, void* data) {
    node_t* node = malloc(sizeof(node_t));
    if (node == NULL) {
        return;
    }
    node->data = data;

    node_t* prev = list->tail.prev;

    node->next = &list->tail;
    node->prev = prev;
    prev->next = node;

    list->tail.prev = node;
}

void* list_pop(list_t* list) {
    return list_pop_tail(list);
}

void* list_pop_head(list_t* list) {
    if (list->head.next == &list->tail) {
        return NULL;
    }

    node_t* node = list->head.next;
    void* data = node->data;
    node->next->prev = &list->head;
    list->head.next = node->next;
    free(node);

    return data;
}

void* list_pop_tail(list_t* list) {
    if (list->head.next == &list->tail) {
        return NULL;
    }

    node_t* node = list->tail.prev;
    void* data = node->data;
    node->prev->next = &list->tail;
    list->tail.prev = node->prev;
    free(node);

    return data;
}

void list_fini(list_t* list) {
    if (list == NULL) {
        return;
    }

    node_t* node = list->head.next;
    while (node != &list->tail) {
        node_t* next = node->next;
        free(node);
        node = next;
    }
}
