#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <arpa/inet.h>

#include "history.h"

int create_history() {
    int f = open(FILENAME_HISTORY, O_RDWR | O_CREAT,
            S_IRWXU | S_IRGRP | S_IROTH);
    if (f == -1) {
        perror("open");
        return -1;
    }
    if (close(f) == -1) {
        perror("close");
        return -1;
    }
    return 0;
}

int send_history(int fd, sem_t *sem, user *u) {
    if (sem_wait(sem) == -1) {
        perror("sem_wait");
        return -1;
    }
    FILE *f = NULL;
    if ((f = fopen(FILENAME_HISTORY, "r")) == NULL) {
        perror("fopen");
        return -1;
    }
    int nb_msg = 0;
    char *history = malloc(sizeof(*history) * SIZE_MAX_IN_HISTORY);
    if (history == NULL) {
        perror("malloc");
        return -1;
    }
    char *msg = malloc(sizeof(*msg) * (SIZE_MAX_IN_HISTORY
                + strlen(HEADER_HISTORY) + 1));
    if (msg == NULL) {
        perror("malloc");
        return -1;
    }
    while ((nb_msg < NB_MSG_HISTORY)
            && (fgets(history, (ssize_t)SIZE_MAX_IN_HISTORY, f) != NULL)) {
        sprintf(msg, "%s%s%s", HEADER_HISTORY, SEPARATOR, history); 
        send_msg(fd, u, msg);
        nb_msg++;
    }
    if (fclose(f) == -1) {
        perror("fclose");
        return -1;
    }
    if (sem_post(sem) == -1) {
        perror("sem_wait");
        return -1;
    }
    free(history);
    free(msg);
    return 0;
}

int add_in_history(sem_t *sem, char *date, user *u, char *msg) {
    if (sem_wait(sem) == -1) {
        perror("sem_wait");
        return -1;
    }
    char *addr = malloc(sizeof(*addr) * SIZE_MAX_ADDR);
    if (addr == NULL) {
        perror("malloc");
        return -1;
    }
    if (inet_ntop(AF_INET, &u->addr.sin_addr, addr,
            SIZE_MAX_ADDR) == NULL) {
        perror("inet_ntop");
        return -1;
    }
    FILE *f = NULL;
    if ((f = fopen(FILENAME_HISTORY, "a")) == NULL) {
        perror("fopen");
        return -1;
    }
    fprintf(f, "%s%s%s%s%s%s%s\n", addr, SEPARATOR, PORT, SEPARATOR,
            date, SEPARATOR, msg);
    if (fclose(f) == -1) {
        perror("fclose");
        return -1;
    }
    if (sem_post(sem) == -1) {
        perror("sem_wait");
        return -1;
    }
    return 0;
}
