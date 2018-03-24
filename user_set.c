#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "user_set.h"

typedef struct cell_user_set cell_user_set;

struct cell_user_set {
    user *u;
    cell_user_set *next;
};

struct user_set {
    cell_user_set *head;
    cell_user_set *tail;
    size_t length;
    size_t max_users;
};

user_set *create_user_set_empty(size_t max_users) {
    user_set *set = malloc(sizeof *set);
    if (set == NULL) {
        perror("malloc");
        return NULL;
    }
    set -> head = NULL;
    set -> tail = NULL;
    set -> length = 0;
    set -> max_users = max_users;
    return set;
}

user *get_user(user_set *set, char *username) {
    cell_user_set *p = set -> head;
    while (p != NULL) {
        if (strcmp(p -> u -> username, username) == 0) {
            return p -> u;
        }
        p = p -> next;
    }
    return NULL;
}

void remove_head(user_set *set) {
    if (set != NULL && set -> head != NULL) {
        cell_user_set *old_head = set -> head;
        set -> head = set -> head -> next;
        free(old_head -> u);
        free(old_head);
        set -> length -= 1;
    }
}

int insert_new_user(user_set *set, user *u) {
    cell_user_set *p = malloc(sizeof(*p));
    if (p == NULL) {
        perror("malloc");
        return -1;
    }
    if (set -> length >= set -> max_users) {
        remove_head(set);
    }
    if (set -> length == 0) {
        set -> head = p;
    } else {
        (set -> tail)  ->  next = p;
    }
    p -> u = u;
    p -> next = NULL;
    set -> tail = p;
    set -> length += 1;
    return 0;
}

int send_msg_all_users(user_set *set,
        int (*send_msg)(int, user *, char *),
        int fd, char *username, char *msg) {
    cell_user_set *p = set -> head;
    while (p != NULL) {
        if (strcmp(p -> u -> username, username) != 0) {
            if (send_msg(fd, p -> u, msg) == -1) {
                return -1;
            }
        }
        p = p -> next;
    }
    return 0;
}

void user_set_dispose(user_set **ptr) {
    if (*ptr != NULL) {
        cell_user_set *p = (*ptr) -> head;
        while (p != NULL) {
            cell_user_set *t = p;
            p = p -> next;
            if (t != NULL) {
                free(t -> u);
            }
            free(t);
        }
        free(*ptr);
        *ptr = NULL;
    }
}

//~ void print_user(user *u) {
    //~ printf("User:(");
     //~ if (u == NULL) {
        //~ printf("NULL");
        //~ return;
    //~ }
    //~ printf("%s, ", u -> username);
    //~ printf("%s)", (u -> addr == NULL)?"NULL":"addr");
//~ }

//~ void print_cell_user_set(cell_user_set *cell) {
    //~ if (cell == NULL) {
        //~ printf("NULL");
        //~ return;
    //~ }
    //~ print_cell(cell -> u);
    //~ printf(" -> ");
//~ }

//~ void print_user_set(user_set *set) {
    //~ if (set == NULL) {
        //~ printf("NULL\n");
        //~ return;
    //~ }
    //~ printf("length = %zu\n", set -> length);
    //~ for(cell_user_set *p = set -> head; p != NULL; p = p -> next) {
        //~ print_cell_user_set(p);
    //~ }
    //~ print_cell_user_set(NULL);
    //~ printf("\n");
    //~ printf("tail :");
    //~ print_cell_user_set(set -> tail);
    //~ printf("\n");
//~ }

//~ int main(void) {
    //~ user_set *set = create_user_set_empty(2);
    //~ print_user_set(set);
    //~ insert_new_user(set, "salut", NULL, 0);
    //~ insert_new_user(set, "salut2", NULL, 0);
    //~ print_user_set(set);
    //~ insert_new_user(set, "salut3", NULL, 0);
    //~ print_user_set(set);
    //~ user_set_dispose(&set);
    //~ print_user_set(set);
    //~ return EXIT_SUCCESS;
//~ }
