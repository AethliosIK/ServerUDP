#ifndef LIST_CLIENTS_H
#define LIST_CLIENTS_H

#define SIZE_MAX_USERNAME 64
#define MAX_CLIENTS

#include <stdlib.h>
#include <sys/socket.h>

typedef struct s_client {
    char username[SIZE_MAX_USERNAME];
    struct sockaddr *addr;
} client;

typedef struct cl_set cl_set;

cl_set *create_cl_set_empty(size_t max_client);

int client_is_in(cl_set *set, char *username);

int insert_new_client(cl_set *set, char *username, struct sockaddr *addr, socklen_t size);

int send_message_all_client(cl_set *set,
        int (*send_message)(int, client *, char *),
        int fd, char *username, char *message);

void cl_set_dispose(cl_set **ptr);

#endif
