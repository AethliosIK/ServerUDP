#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <arpa/inet.h>

#include "serverUDP.h"

struct param{
    int fd;
    user_set *set;
    sem_t *sem;
    char date[SIZE_MAX_DATE];
    char recv[SIZE_MAX_MSG];
    struct sockaddr_in addr_sender;
};

struct param *parameters_thread = NULL;

void *send_msg_everybody(void *parameters) {
    struct param *p = (struct param *)parameters;
    char tmp [SIZE_MAX_MSG];
    strncpy(tmp, p->recv, SIZE_MAX_MSG);
    char *username = extract_username(tmp);
    if (username == NULL) {
        close_server();
        exit(EXIT_FAILURE);
    }
    user *u = get_user(p->set, username);
    if (u == NULL) {
        u = create_new_user(username, (p->addr_sender));
        if (u == NULL) {
            fprintf(stderr, "Error in create_new_client");
            close_server();
            exit(EXIT_FAILURE);
        }
        printf("New user : %s\n", username);
        if (insert_new_user(p->set, u) == -1) {
            close_server();
            exit(EXIT_FAILURE);
        }
        if (send_history(p->fd, p->sem, u) == -1) {
            close_server();
            exit(EXIT_FAILURE);
        }
    }
    if (add_in_history(p->sem, p->date, u, p->recv) == -1) {
        close_server();
        exit(EXIT_FAILURE);
    }
    if (send_msg_all_users(p->set, &send_msg,
            p->fd, username, p->recv) == -1) {
        close_server();
        exit(EXIT_FAILURE);
    }
    return NULL;
}

int create_thread_send_msg(int fd, user_set *set, sem_t *sem,
        char *date, char *recv, struct sockaddr_in addr_sender) {
    struct param *p = malloc(sizeof(*p));
    if (p == NULL) {
        perror("malloc");
        return -1;
    }
    parameters_thread = p;
    p -> fd = fd;
    p -> set = set;
    p -> sem = sem;
    strncpy(p->date, date, SIZE_MAX_DATE);
    free(date);
    strncpy(p->recv, recv, SIZE_MAX_MSG);
    free(recv);
    memcpy(&(p->addr_sender), &addr_sender, sizeof(addr_sender));

    pthread_t th;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) !=0) {
        perror("pthread_attr_init");
        return -1;
    }
    if (pthread_attr_setdetachstate(&attr,
            PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        return -1;
    }
    if (pthread_create(&th, &attr, send_msg_everybody, p) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

int create_server() {
    int fd = create_bind_socket();
    if (fd == -1) {
        return -1;
    }
    user_set *set = create_user_set_empty(MAX_USER_IN_SET);
    if (set == NULL) {
        return -1;
    }
    if (create_history() == -1) {
        return -1;
    }
    sem_t *sem = init_sem(NAME_SEM_READ, 1);
    if (sem == NULL) {
        return -1;
    }
    while (1) {
        char *recv = malloc(sizeof(*recv) * SIZE_MAX_MSG);
        if (recv == NULL) {
            perror("malloc");
            return -1;
        }
        struct sockaddr_in addr_sender;
        socklen_t size = sizeof(addr_sender);
        if ((recvfrom(fd, recv, SIZE_MAX_MSG, 0,
                (struct sockaddr *)&addr_sender, &size)) == -1) {
            perror("recvfrom");
            return -1;
        }
        char *date = define_date();
        printf("%s%s\n", HEADER_MSG_RECEVED, recv);
        if (create_thread_send_msg(fd, set, sem, date, recv,
                addr_sender) == -1) {
            return -1;
        }
    }
    return 0;
}

void close_server() {
    if (sem_unlink(NAME_SEM_READ) == -1) {
        perror("sem_unlink");
        close_server();
        exit(EXIT_FAILURE);
    }
    if (unlink(FILENAME_HISTORY) == -1) {
        perror("unlink");
        close_server();
        exit(EXIT_FAILURE);
    }
    if (parameters_thread != NULL) {
		free(parameters_thread);
	}
}

static void handle_sigserver(int signum) {
    if (signum == SIGINT) {
        printf("\nSee u later !\n");
        close_server();
        exit(EXIT_SUCCESS);
    }
}

void manage_server_signals(void) {
    struct sigaction sigintact;
    sigintact.sa_handler = handle_sigserver;
    if (sigaction(SIGINT, &sigintact, NULL) < 0) {
        printf("Cannot manage SIGUSR1\n");
        close_server();
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    manage_server_signals();
    if (create_server() == -1) {
        close_server();
        return EXIT_FAILURE;
    }
    close_server();
    return EXIT_SUCCESS;
}
