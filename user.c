#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <arpa/inet.h>

#include "user.h"

user *create_new_user(char *username, struct sockaddr_in addr) {
    user *u = malloc(sizeof(*u));
    if (u == NULL) {
        return NULL;
    }
    strncpy(u -> username, username, SIZE_MAX_USERNAME);
    memcpy(&(u -> addr), &addr, sizeof(addr));
    return u;
}

int send_msg(int fd, user *u, char *msg) {
    if (sendto(fd, msg, SIZE_MAX_MSG, 0, (struct sockaddr *)&u->addr, sizeof(u->addr)) == -1) {
        perror("send_to");
        return -1;
    }
    printf("%s%s\tto : %s\n", HEADER_MSG_SENT, msg, u->username);
    return 0;
}
