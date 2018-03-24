#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include "clientUDP.h"

struct param{
    int fd;
    struct sockaddr_in *addr;
};

//char *define_username() {
    //uid_t id = geteuid();
    //struct passwd *pass = getpwuid(id);
    //char *username = malloc(sizeof(*username) * SIZE_MAX_USERNAME);
    //strncpy(username, pass->pw_name, SIZE_MAX_USERNAME);
    //return username;
//}

void *wait_response(void *p) {
    struct param *parameters = (struct param *)p;
    while(1) {
        char s[SIZE_MAX_MSG];
        socklen_t size = sizeof(parameters->addr);
        if (recvfrom(parameters->fd, s, sizeof(s), 0,
                (struct sockaddr *)parameters->addr, &size) == -1) {
            perror("recvfrom");
            close_client();
            exit(EXIT_FAILURE);
        }
        char tmp[SIZE_MAX_MSG];
        strncpy(tmp, s, SIZE_MAX_MSG);
        char *msg = extract_msg(tmp);
        if (msg == NULL) {
            close_client();
            exit(EXIT_FAILURE);
        }
        if (strcmp(tmp, HEADER_HISTORY) == 0) {
            printf("History : %s", s);
        } else  {
            printf("%s : %s\n", tmp, msg);
        }
    }
}

int create_thread_wait_response(int fd, struct sockaddr_in *addr) {
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

int create_client() {
    char *username = input_username();
    if (username == NULL) {
        return -1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in *addr = (struct sockaddr_in *)define_addr();
    if (create_thread_wait_response(fd, addr) != 0) {
        return -1;
    }
    while(1) {
        char input[SIZE_MAX_MSG];
        scanf("%s", input);
        char msg[SIZE_MAX_MSG];
        sprintf(msg, "%s%s%s", username, SEPARATOR, input);
        if (sendto(fd, msg, SIZE_MAX_MSG, 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
            perror("send_to");
            return -1;
        }
        printf("%s : %s\n", username, input);
    }
    return 0;
}

void close_client() {
    return;
}

static void handle_sigclient(int signum) {
    if (signum == SIGINT) {
        printf("\nSee u later !\n");
        close_client();
        exit(EXIT_SUCCESS);
    }
}

void manage_client_signals(void) {
    struct sigaction sigintact;
    sigintact.sa_handler = handle_sigclient;
    if (sigaction(SIGINT, &sigintact, NULL) < 0) {
        printf("Cannot manage SIGUSR1\n");
        close_client();
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    manage_client_signals();
    if (create_client() == -1) {
        close_client();
        return EXIT_FAILURE;
    }
    close_client();
    return EXIT_SUCCESS;
}
