#ifndef FORT_LIST_H
#define FORT_LIST_H

typedef struct node {
    void* data;
    struct node* next;
    struct node* prev;
} node_t;

typedef struct list {
    node_t head;
    node_t tail;
} list_t;

void list_init(list_t* list);

void list_push(list_t* list, void* data);
void* list_pop(list_t* list);

void list_fini(list_t* list);

#endif // FORT_LIST_H
