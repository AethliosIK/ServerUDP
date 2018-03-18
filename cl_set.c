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
    size_t max_client;
};

cl_set *create_cl_set_empty(size_t max_client) {
    cl_set *set = malloc(sizeof *set);
    if (set == NULL) {
        return NULL;
    }
    set -> head = NULL;
    set -> tail = NULL;
    set -> length = 0;
    set -> max_client = max_client;
    return set;
}

int client_is_in(cl_set *set, char *username) {
    cell_cl_set *p = set -> head;
    while (p != NULL) {
        if (strcmp(p -> cl -> username, username) == 0) {
            return 1;
        }
        p = p -> next;
    }
    return 0;
}

client *create_new_client(char *username, struct sockaddr *addr, socklen_t size) {
    client *cl = malloc(sizeof(*cl));
    strncpy(cl -> username, username, SIZE_MAX_USERNAME);
    memcpy(cl -> addr, addr, size);
    return cl;
}

void remove_head(cl_set *set) {
    if (set != NULL && set -> head != NULL) {
        cell_cl_set *old_head = set -> head;
        set -> head = set -> head -> next;
        free(old_head -> cl);
        free(old_head);
        set -> length -= 1;
    }
}

int insert_new_client(cl_set *set, char *username, struct sockaddr *addr, socklen_t size) {
    cell_cl_set *p = malloc(sizeof(*p));
    if (p == NULL) {
        return -1;
    }
    if (set -> length >= set -> max_client) {
        remove_head(set);
    }
    if (set -> length == 0) {
        set -> head = p;
    } else {
        (set -> tail)  ->  next = p;
    }
    p -> cl = create_new_client(username, addr, size);
    p -> next = NULL;
    set -> tail = p;
    set -> length += 1;
    return 0;
}

int send_message_all_client(cl_set *set,
        int (*send_message)(int, client *, char *),
        int fd, char *username, char *message) {
    cell_cl_set *p = set -> head;
    while (p != NULL) {
        if (strcmp(p -> cl -> username, username) != 0) {
            if (send_message(fd, p -> cl, message) == -1) {
                return -1;
            }
        }
        p = p -> next;
    }
    return 0;
}

void cl_set_dispose(cl_set **ptr) {
    if (*ptr != NULL) {
        cell_cl_set *p = (*ptr) -> head;
        while (p != NULL) {
            cell_cl_set *t = p;
            p = p -> next;
            free(t -> cl);
            free(t);
        }
        free(*ptr);
        *ptr = NULL;
    }
}

//~ void print_cl(client *cl) {
    //~ printf("Client:(");
     //~ if (cl == NULL) {
        //~ printf("NULL");
        //~ return;
    //~ }
    //~ printf("%s, ", cl -> username);
    //~ printf("%s)", (cl -> addr == NULL)?"NULL":"addr");
//~ }

//~ void print_cell_cl_set(cell_cl_set *cell) {
    //~ if (cell == NULL) {
        //~ printf("NULL");
        //~ return;
    //~ }
    //~ print_cl(cell -> cl);
    //~ printf(" -> ");
//~ }

//~ void print_cl_set(cl_set *set) {
    //~ if (set == NULL) {
        //~ printf("NULL\n");
        //~ return;
    //~ }
    //~ printf("length = %zu\n", set -> length);
    //~ for(cell_cl_set *p = set -> head; p != NULL; p = p -> next) {
        //~ print_cell_cl_set(p);
    //~ }
    //~ print_cell_cl_set(NULL);
    //~ printf("\n");
    //~ printf("tail :");
    //~ print_cell_cl_set(set -> tail);
    //~ printf("\n");
//~ }

//~ int main(void) {
    //~ cl_set *set = create_cl_set_empty(2);
    //~ print_cl_set(set);
    //~ insert_new_client(set, "salut", NULL, 0);
    //~ insert_new_client(set, "salut2", NULL, 0);
    //~ print_cl_set(set);
    //~ insert_new_client(set, "salut3", NULL, 0);
    //~ print_cl_set(set);
    //~ cl_set_dispose(&set);
    //~ print_cl_set(set);
    //~ return EXIT_SUCCESS;
//~ }
