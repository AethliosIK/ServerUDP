#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cl_set.h"

#define SIZE_MAX_USERNAME 64

typedef struct cell_cl_set cell_cl_set;

struct cell_cl_set {
    client *cl;
    cell_cl_set *next;
};

struct cl_set {
    cell_cl_set *head;
    cell_cl_set *tail;
    size_t length;
};

cl_set *create_cl_set_empty(void) {
    cl_set *set = malloc(sizeof *set);
    if (set == NULL) {
        return NULL;
    }
    set->head = NULL;
    set->tail = NULL;
    set->length = 0;
    return set;
}

int client_is_in(cl_set *set, char *username) {
    cell_cl_set *p = set->head;
    while (p != NULL) {
        if (strcmp(p->cl->username, username) == 0) {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

client *create_new_client(char *username, struct sockaddr *addr, socklen_t size) {
    client *cl = malloc(sizeof(*cl));
    strncpy(cl->username, username, SIZE_MAX_USERNAME);
    memcpy(cl->addr, addr, size);
    return cl;
}

int insert_new_client(cl_set *set, char *username, struct sockaddr *addr, socklen_t size) {
    cell_cl_set *p = malloc(sizeof(*p));
    if (p == NULL) {
        return -1;
    }
    if (set->length == 0) {
        set->tail = p;
    }
    p->cl = create_new_client(username, addr, size);
    p->next = set->head;
    set->head = p;
    set->length += 1;
    return 0;
}

int send_message_all_client(cl_set *set,
        int (*send_message)(int, client *, char *),
        int fd, char *username, char *message) {
    cell_cl_set *p = set->head;
    while (p != NULL) {
        if (strcmp(p->cl->username, username) != 0) {
            if (send_message(fd, p->cl, message) == -1) {
                return -1;
            }
        }
        p = p->next;
    }
    return 0;
}

void cl_set_dispose(cl_set **ptr) {
    if (*ptr != NULL) {
        cell_cl_set *p = (*ptr)->head;
        while (p != NULL) {
            cell_cl_set *t = p;
            p = p->next;
            free(t->cl);
            free(t);
        }
        free(*ptr);
        *ptr = NULL;
    }
}
