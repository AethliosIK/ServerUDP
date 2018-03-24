#ifndef HISTORY_H
#define HISTORY_H

#include <semaphore.h>

#include "utils.h"
#include "user.h"

int create_history();

int send_history(int fd, sem_t *sem, user *u);

int add_in_history(sem_t *sem, char *date, user *u, char *msg);


#endif
