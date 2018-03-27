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

#define min(a,b) (a<=b?a:b)
#define max(a,b) (a>=b?a:b)

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
        free(history);
        return -1;
    }
    ssize_t nb_line = define_nb_line(FILENAME_HISTORY);
    for (int i = 0; i < max(0, nb_line - NB_MSG_HISTORY); i++) {
        if (fgets(history, (ssize_t)SIZE_MAX_IN_HISTORY, f) == NULL) {
            break;
        }
    }
    while (nb_msg < min(nb_line, NB_MSG_HISTORY)) {
        if (fgets(history, (ssize_t)SIZE_MAX_IN_HISTORY, f) == NULL) {
            break;
        }
        sprintf(msg, "%s%s%s", HEADER_HISTORY, SEPARATOR, history);
        send_msg(fd, u, msg);
        nb_msg++;
    }
    free(history);
    free(msg);
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
        free(addr);
        return -1;
    }
    FILE *f = NULL;
    if ((f = fopen(FILENAME_HISTORY, "a")) == NULL) {
        perror("fopen");
        free(addr);
        return -1;
    }
    fprintf(f, "%s%s%s%s%s%s%s\n", addr, SEPARATOR, PORT, SEPARATOR,
            date, SEPARATOR, msg);
    free(addr);
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

void remove_history() {
    if (file_exits(FILENAME_HISTORY)) {
        if (unlink(FILENAME_HISTORY) == -1) {
            perror("unlink");
            exit(EXIT_FAILURE);
        }
    }
}
