#ifndef USER_SET_H
#define USER_SET_H

#include "utils.h"
#include "user.h"

typedef struct user_set user_set;

user_set *create_user_set_empty(size_t max_users);

user *get_user(user_set *set, char *username);

int insert_new_user(user_set *set, user *u);

int send_msg_all_users(user_set *set,
        int (*send_msg)(int, user *, char *),
        int fd, char *username, char *msg);

void user_set_dispose(user_set **ptr);

#endif
