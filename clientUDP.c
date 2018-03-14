#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>

#define HEADER_MESSAGE_RECEVED "Message receved :\t"
#define HEADER_MESSAGE_SENT "Message sent"
#define PORT "34252"
#define SIZE_MAX_USERNAME 64
#define SIZE_MAX_MESSAGE 512
#define SEPARATOR ":"

struct param{
    int fd;
    struct sockaddr *addr;
};

//char *define_username() {
    //uid_t id = geteuid();
    //struct passwd *pass = getpwuid(id);
    //char *username = malloc(sizeof(*username) * SIZE_MAX_USERNAME);
    //strncpy(username, pass->pw_name, SIZE_MAX_USERNAME);
    //return username;
//}

char *define_username() {
    char *username = malloc(sizeof(*username) * SIZE_MAX_USERNAME);
    if (username == NULL) {
        perror("malloc");
        return NULL;
    }
    printf("Username ? : ");
    scanf("%s", username);
    return username;
}

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

void *wait_response(void *p) {
    struct param *parameters = (struct param *)p;
    while(1) {
        char s[SIZE_MAX_MESSAGE];
        socklen_t size;
        if (recvfrom(parameters->fd, s, sizeof(s), 0, parameters->addr, &size) == -1) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("%s%s\n", HEADER_MESSAGE_RECEVED, s);
    }
}

int create_thread_wait_response(int fd, struct sockaddr *addr) {
    struct param *p = malloc(sizeof(*p));
    if (p == NULL) {
        perror("malloc");
        return -1;
    }
    p->fd = fd;
    p->addr = addr;
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
    if (pthread_create(&th, &attr, wait_response, p) !=0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

int main(void) {
    char *username = define_username();
    if (username == NULL) {
        return EXIT_FAILURE;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
    struct sockaddr *addr = define_addr();
    if (addr == NULL) {
        return EXIT_FAILURE;
    }
    if (create_thread_wait_response(fd, addr) != 0) {
        return EXIT_FAILURE;
    }

    while(1) {
        char input[SIZE_MAX_MESSAGE];
        printf("Message ? : ");
        scanf("%s", input);
        char s[SIZE_MAX_MESSAGE];
        sprintf(s, "%s%s%s", username, SEPARATOR, input);
        if (sendto(fd, s, SIZE_MAX_MESSAGE, 0, addr, sizeof(struct sockaddr_in)) == -1) {
            perror("send_to");
            return EXIT_FAILURE;
        }
        printf("%s\n", HEADER_MESSAGE_SENT);
    }

    return EXIT_SUCCESS;
}
