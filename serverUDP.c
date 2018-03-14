#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include "cl_set.h"

#define PORT "34252"
#define SIZE_MAX_USERNAME 64
#define SIZE_MAX_MESSAGE 512
#define SEPARATOR ":"

//#define MAX_LENGTH_FILE 1024
//#define BUF_SIZE 128

//char *read_file(char *filename) {
    //char *result = malloc(sizeof(*result));
    //if (result == NULL) {
        //perror("malloc");
        //return NULL;
    //}
    //int fd;
    //size_t nb_char = 1;
    //char buf[BUF_SIZE];
    //if ((fd = open(filename, O_RDONLY)) == -1) {
        //perror("open");
        //return NULL;
    //}
    //ssize_t rr;
    //while ((rr = read(fd, buf, BUF_SIZE)) > 0) {
        //nb_char += (size_t) rr;
        //printf("%zu\t%zu\n", nb_char, rr);
        //if ((result = realloc(result, sizeof(*result) * nb_char)) == NULL) {
            //perror("realloc");
            //return NULL;
        //}
        //printf("\n%s\n", result);
        //strncat(result, buf, (size_t)rr);
        //result += '\0';
    //}
    //if (rr == -1) {
        //perror("read");
        //return NULL;
    //}
    //if (close(fd) == -1) {
        //perror("close");
        //return NULL;
    //}
    //return result;
//}

//void run(int *fd) {
    //char filename[MAX_LENGTH_FILE];
    //struct sockaddr_in addr_client;
    //socklen_t addr_client_len = sizeof(struct sockaddr_in);
    //if (recvfrom(*fd, filename, MAX_LENGTH_FILE, 0,
        //(struct sockaddr *)&addr_client, &addr_client_len) == -1) {
        //perror("recvfrom");
        //exit(EXIT_FAILURE);
    //}
    //char *content_file = read_file(filename);
    //if (content_file == NULL) {
        //exit(EXIT_FAILURE);
    //}
    //if (sendto(*fd, content_file, strlen(content_file), 0,
            // (struct sockaddr *)&addr_client, addr_client_len) == -1) {
        //perror("sendto");
        //exit(EXIT_FAILURE);
    //}
//}

//int create_thread(pthread_t *th, void *elem) {
    //pthread_attr_t attr;
    //if (pthread_attr_init(&attr) != 0) {
        //perror("pthread_attr_init");
        //return -1;
    //}
    //if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        //perror("pthread_attr_setdetachstate");
        //return -1;
    //}
    //if (pthread_create(th, &attr, (void *(*)(void *))run, elem) != 0) {
        //perror("pthread_create");
        //return -1;
    //}
    //return 0;
//}

int create_bind_socket(char *port) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
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
    if (bind(fd, (struct sockaddr *)result->ai_addr,
            sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return -1;
    }
    freeaddrinfo(result);
    return fd;
}

struct param{
    int fd;
    cl_set *set;
    char recv[SIZE_MAX_MESSAGE];
    struct sockaddr_in addr_sender;
};

char *extract_username(char *recv) {
    char *token;
    char *saveptr1;
    token = strtok_r(recv, SEPARATOR, &saveptr1);
    recv = NULL;
    if (token == NULL) {
        return NULL;
    }
    return token;
}

int send_message(int fd, client *cl, char *message) {
    if (sendto(fd, message, SIZE_MAX_MESSAGE, 0, cl->addr, sizeof(*(cl->addr))) == -1) {
        perror("send_to");
        return -1;
    }
    return 0;
}

void *send_message_everybody(void *parameters) {
    struct param *p = (struct param *)parameters;
    char tmp [SIZE_MAX_MESSAGE];
    strncpy(tmp, p->recv, SIZE_MAX_MESSAGE);
    char *username = extract_username(tmp);
    if (!client_is_in(p->set, username)) {
        if (insert_new_client(p->set, username,
                (struct sockaddr *)&(p->addr_sender),
                sizeof(p->addr_sender)) == -1) {
            exit(EXIT_FAILURE);
        }
    }
    if (send_message_all_client(p->set, &send_message, p->fd, username, p->recv) == -1) {
        exit(EXIT_FAILURE);
    }
    return NULL;
}

int create_thread_send_message(int fd, cl_set *set, char *recv,
        struct sockaddr_in addr_sender) {
    struct param *p = malloc(sizeof(*p));
    if (p == NULL) {
        perror("malloc");
        return -1;
    }
    p -> fd = fd;
    p -> set = set;
    strncpy(p->recv, recv, SIZE_MAX_MESSAGE);
    memcpy(&(p->addr_sender), &addr_sender, sizeof(addr_sender));

    pthread_t th;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) !=0) {
        perror("pthread_attr_init");
        return -1;
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        return -1;
    }
    if (pthread_create(&th, &attr, send_message_everybody, p) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

int create_server(char *port) {
    int fd = create_bind_socket(port);
    if (fd == -1) {
        return -1;
    }
    cl_set *set = create_cl_set_empty();
    while (1) {
        char *recv = malloc(sizeof(*recv) * SIZE_MAX_MESSAGE);
        struct sockaddr_in addr_sender;
        if ((recvfrom(fd, recv, sizeof(recv), 0,
                (struct sockaddr*)&addr_sender, NULL)) == -1) {
            perror("recvftom");
            return -1;
        }
        if (create_thread_send_message(fd, set, recv,
                addr_sender) == -1) {
            return -1;
        }
    }
    return 0;
}

int main(void) {
    if (create_server(PORT) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
