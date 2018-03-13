#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT "34252"
#define SIZE_MAX_USERNAME 64
#define SIZE_MAX_MESSAGE 512
#define SEPARATOR ":"
#define TIME 10000

#define MAX_LENGTH_FILE 1024
#define BUF_SIZE 128

int create_bind_socket(char *port) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *result;
    if (getaddrinfo(NULL, port, &hints, &result) != 0) {
        perror("getaddrinfo");
        return -1;
    }
    if (result == NULL) {
        perror("getaddrinfo : result == NULL");
        return -1;
    }
    if (bind(fd, (struct sockaddr *)result->ai_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return -1;
    }
    freeaddrinfo(result);
    return fd;
}

char *read_file(char *filename) {
    char *result = malloc(sizeof(*result));
    if (result == NULL) {
        perror("malloc");
        return NULL;
    }
    int fd;
    size_t nb_char = 1;
    char buf[BUF_SIZE];
    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("open");
        return NULL;
    }
    ssize_t rr;
    while ((rr = read(fd, buf, BUF_SIZE)) > 0) {
        nb_char += (size_t) rr;
        printf("%zu\t%zu\n", nb_char, rr);
        if ((result = realloc(result, sizeof(*result) * nb_char)) == NULL) {
            perror("realloc");
            return NULL;
        }
        printf("\n%s\n", result);
        strncat(result, buf, (size_t)rr);
        result += '\0';
    }
    if (rr == -1) {
        perror("read");
        return NULL;
    }
    if (close(fd) == -1) {
        perror("close");
        return NULL;
    }
    return result;
}

void run(int *fd) {
    char filename[MAX_LENGTH_FILE];
    struct sockaddr_in addr_client;
    socklen_t addr_client_len = sizeof(struct sockaddr_in);
    if (recvfrom(*fd, filename, MAX_LENGTH_FILE, 0,
        (struct sockaddr *)&addr_client, &addr_client_len) == -1) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    char *content_file = read_file(filename);
    if (content_file == NULL) {
        exit(EXIT_FAILURE);
    }
    if (sendto(*fd, content_file, strlen(content_file), 0, (struct sockaddr *)&addr_client, addr_client_len) == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

int create_thread(pthread_t *th, void *elem) {
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        perror("pthread_attr_init");
        return -1;
    }
    //if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        //perror("pthread_attr_setdetachstate");
        //return -1;
    //}
    if (pthread_create(th, &attr, (void *(*)(void *))run, elem) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

void init_fd_set(fd_set *set, int fd1) {
    FD_ZERO(set);
    FD_SET(fd1, set);
}

void init_tempo(struct timeval *tempo, int time) {
    tempo->tv_sec = time / 1000;
    tempo->tv_usec = time % 1000;
}

int send_message(struct sockaddr_in addr_sender, char *s, fd_set set) {

    return 0;
}

int create_server(char *port, int time) {
    int fd = create_bind_socket(port);
    if (fd == -1) {
        return -1;
    }
    fd_set set;
    init_fd_set(&set, fd);
    struct timeval tempo;
    init_tempo(&tempo, time);
    int r_select = 0;
    while (r_select != -1) {
        r_select = select(1, &set, NULL, NULL, &tempo);

        char *s = malloc(sizeof(*s) * SIZE_MAX_MESSAGE);
        struct sockaddr_in addr_sender;
        if ((recvfrom(fd, s, sizeof(s), 0,
                (struct sockaddr*)&addr_sender, NULL)) == -1) {
            perror("recvftom");
            return EXIT_FAILURE;
        }
        if (send_message(addr_sender, s, set) == -1) {
            return -1;
        }
    }
    return 0;
}

int main(void) {
    if (create_server(PORT, TIME) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
