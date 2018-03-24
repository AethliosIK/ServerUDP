#ifndef UTIL_H
#define UTIL_H

#include <semaphore.h>

#define HEADER_MSG_RECEVED "Message receved :\t"
#define HEADER_MSG_SENT "Message sent :\t"
#define NAME_SEM_READ "/sem_98561451632"
#define FILENAME_HISTORY "history.log"
#define PORT "34252"
#define HEADER_HISTORY "h"
#define SIZE_MAX_ADDR 20
#define SIZE_MAX_USERNAME 64
#define SIZE_MAX_MSG 512
#define SIZE_MAX_DATE 32
#define SIZE_MAX_IN_HISTORY (SIZE_MAX_ADDR + strlen(PORT) + SIZE_MAX_DATE \
+ SIZE_MAX_USERNAME + SIZE_MAX_MSG)
#define SEPARATOR ";"
#define MAX_USER_IN_SET 10
#define NB_MSG_HISTORY 30
#define FILENAME_HISTORY "history.log"

struct sockaddr *define_addr();

int create_bind_socket();

char *input_username();

char *extract_msg(char *recv);

char *extract_username(char *recv);

sem_t *init_sem(char *sem_name, unsigned int value);

char *define_date();

#endif
