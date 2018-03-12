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

#define MAX_LENGTH_FILE 1024
#define BUF_SIZE 128

int create_bind_socket(uint32_t addr, uint16_t port) {
    int result = socket(AF_INET, SOCK_DGRAM, 0);
    if (result == -1) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port   = port;
    addr_in.sin_addr.s_addr = addr;
    if (bind(result, (struct sockaddr*) &addr_in, sizeof(addr_in)) == -1) {
        perror("bind");
        return -1;
    }
    return result;
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

void init_fd_set(fd_set *set, int fd1, int fd2) {
    FD_ZERO(set);
    FD_SET(fd1, set);
    FD_SET(fd2, set);
}

int get_fd_select(fd_set *set, int fd1, int fd2) {
    return (FD_ISSET(fd1, set) ? fd1 : fd2);
}

void init_tempo(struct timeval *tempo, int time) {
    tempo->tv_sec = time / 1000;
    tempo->tv_usec = time % 1000;
}

int max_fd(int fd1, int fd2) {
    return ((fd1 >= fd2) ? fd1 : fd2);
}

int create_server(uint32_t  addr, uint16_t p1, uint16_t p2, int time) {
    int fd1 = create_bind_socket(addr, p1);
    int fd2 = create_bind_socket(addr, p2);
    if (fd1 == -1 || fd2 == -1) {
        return -1;
    }
    fd_set set;
    struct timeval tempo;
    pthread_t th;
    while (1) {
        init_fd_set(&set, fd1, fd2);
        init_tempo(&tempo, time);
        int max = max_fd(fd1, fd2);
        int r = select(max + 1, &set, NULL, NULL, &tempo);
        if (r == -1) {
            perror("select");
            return -1;
        }
        if (r == 0) {
            printf("Server stopping...\n");
            break;
        }
        int fd_select = get_fd_select(&set, fd1, fd2);
        if (create_thread(&th, &fd_select) == -1) {
            return -1;
        }
        if (pthread_join(th, NULL) != 0) {
            perror("pthread_join");
            return -1;
        }
    }
    return 0;
}

int main(void) {
    printf("%s", read_file("serverUDP.c"));
    uint32_t addr = htonl(INADDR_LOOPBACK);
    uint16_t p1 = 5000;
    uint16_t p2 = 3000;
    int time = 10000;
    if (create_server(addr, p1, p2, time) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
