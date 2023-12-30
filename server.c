#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>
#include "sqlite3_database.h"

#define PORT 2002
extern int errno;

typedef struct thData {
    int idThread;
    int cl;
} thData;

static void *treat(void *);
void raspunde(void *);
struct thData tdL;
Database db;

static void *treat(void *);
void raspunde(void *arg);
void handler_command(int input_command);
void register_command();


int main() {
    struct sockaddr_in server;
    struct sockaddr_in from;
    int nr;
    int sd;
    int pid;
    pthread_t th[100];
    int i = 0;

    if (open_database(&db, "database_marketplace.db") != SQLITE_OK) {
        perror("[server] Error at opening the database.\n");
        return errno;
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server] Error at creating the socket!\n");
        return errno;
    }

    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[server] Error at bind(). \n");
        return errno;
    }

    if (listen(sd, 2) == -1) {
        perror("[server]Eroare at listen().\n");
        return errno;
    }

    while (1) {
        int client;
        thData *td;
        int length = sizeof(from);
        printf("[server] We are waiting at %d port..\n", PORT);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) {
            perror("[server] Error at accept(). \n");
            continue;
        }

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        pthread_create(&th[i], NULL, &treat, td);
    }
}

static void *treat(void *arg) {
    struct thData tdL;
    tdL = *((struct thData *)arg);
    pthread_detach(pthread_self());
    raspunde((struct thData *)arg);
    close((intptr_t)arg);
    return NULL;
}

void raspunde(void *arg) {
    int input_command;
    tdL = *((struct thData *)arg);
    if (read(tdL.cl, &input_command, sizeof(int)) <= 0) {
        printf("[Thread] %d\n", tdL.idThread);
        perror("Error reading command from client! \n");
    }
    handler_command(input_command);

    close(tdL.cl);
}

void handler_command(int input_command) {
    switch (input_command) {
    case 2:
        register_command();
        break;
    default:
        printf("[Thread %d] Unknown command. %d\n", tdL.idThread, input_command);
        break;
    }
}

void register_command() {
    printf("[Thread %d] Received command 2 (register).\n", tdL.idThread);
    char username[50];
    char password[50];
    char role[50];
    int roleValue;
    
    if (read(tdL.cl, &roleValue, sizeof(int)) <= 0) {
        perror("[Thread] Error reading role from client.\n");
    }

    if (read(tdL.cl, username, sizeof(username) - 1) <= 0 ||
        read(tdL.cl, password, sizeof(password) - 1) <= 0) {
        perror("[Thread] Error reading registration data from client.\n");
    }

    username[sizeof(username) - 1] = '\0';
    password[sizeof(password) - 1] = '\0';

    printf("[Thread %d] After reading registration data:\n", tdL.idThread);
    printf("Username: %s\n", username);
    printf("Password: %s\n", password);
    printf("Role: %d\n", roleValue);

    if (roleValue == 1)
        strcpy(role, "Seller");
    else
        strcpy(role, "Buyer");

    int ok = add_new_user(&db, username, password, role);
    char message_to_be_sent[200];
    if (ok == -2)
        strcpy(message_to_be_sent, "This username already exists. Please, select another one!");
    else if (ok == -1)
        strcpy(message_to_be_sent, "An error has happened to database. Please, try again.");
    else
        strcpy(message_to_be_sent, "Your account has been registred. Now, please log in.");

    if (write(tdL.cl, &message_to_be_sent, sizeof(message_to_be_sent)) <= 0) {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    } else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}
