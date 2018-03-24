#ifndef USER_H
#define USER_H

#include <stdlib.h>
#include <netdb.h>

#include "utils.h"

typedef struct s_user {
    char username[SIZE_MAX_USERNAME];
    struct sockaddr_in addr;
} user;

user *create_new_user(char *username, struct sockaddr_in addr);

int send_msg(int fd, user *u, char *msg);

#endif
