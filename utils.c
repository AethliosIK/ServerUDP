#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>

#include "utils.h"

struct sockaddr *define_addr() {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo *result;
    if (getaddrinfo(NULL, PORT, &hints, &result) != 0) {
        perror("getaddrinfo");
        return NULL;
    }
    if (result == NULL) {
        perror("getaddrinfo : result == NULL");
        return NULL;
    }
    struct sockaddr *addr = result->ai_addr;
    freeaddrinfo(result);
    return addr;
}

int create_bind_socket() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    struct sockaddr *addr = define_addr();
    if (addr == NULL) {
        return -1;
    }
    if (bind(fd, addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return -1;
    }
    return fd;
}

char *input_username() {
    char *username = malloc(sizeof(*username) * SIZE_MAX_USERNAME);
    if (username == NULL) {
        perror("malloc");
        return NULL;
    }
    printf("Username ? : ");
    scanf("%s", username);
    return username;
}

char *extract_msg(char *recv) {
    char *token;
    char *saveptr1;
    token = strtok_r(recv, SEPARATOR, &saveptr1);
    recv = NULL;
    if (token == NULL) {
        return NULL;
    }
    token = strtok_r(recv, SEPARATOR, &saveptr1);
    return token;
}

char *extract_username(char *recv) {
    char *token;
    char *saveptr1;
    token = strtok_r(recv, SEPARATOR, &saveptr1);
    if (token == NULL) {
        return NULL;
    }
    return token;
}

sem_t *init_sem(char *sem_name, unsigned int value) {
    sem_t *sem = sem_open(sem_name, O_RDWR | O_CREAT | O_EXCL,
        S_IRUSR | S_IWUSR, value);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return NULL;
    }
    return sem;
}

char *define_date() {
    time_t current_time = time(NULL);
    if (current_time == (time_t) -1)    {
        fprintf(stderr, "Failure to obtain the current time.\n");
        return NULL;
    }
    char *date = malloc(sizeof(*date) * SIZE_MAX_DATE);
    if (date == NULL) {
        perror("malloc");
        return NULL;
    }
    sprintf(date, "%s", ctime(&current_time));
    date[strlen(date) - 1] = '\0';
    return date;
}

int file_exits(const char *filename) {
    struct stat s;
    return (stat(filename, &s) != -1);
}

ssize_t define_nb_line(char *filename) {
    ssize_t nb_line = 0;
    FILE *f = NULL;
    if ((f = fopen(filename, "r")) == NULL) {
        perror("fopen");
        return -1;
    }
    int size = SIZE_MAX_USERNAME + strlen(SEPARATOR) + SIZE_MAX_MSG + 1;
    char buf[size];
    while (fgets(buf, size, f) != NULL) {
        nb_line++;
    }
    return nb_line;
}
