#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

extern int errno;
#define WelcomeMenu "\n Welcome to the LocalMarketPlacePlatform! Please select one option from below. The commands are: \n 1. login -> in case you have an account already \n 2. register -> in case you don't have an account yet \n 3. exit -> to close the app \n"
int port;
void handler_command(int input_command);
void register_command();
int sd;
struct sockaddr_in server;


int main(int argc, char *argv[]) {

    int input_command;
    char buf[10];

    if (argc != 3) {
        printf("The syntax is: %s <server_address> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error at socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Error at connect().\n");
        return errno;
    }

    printf("[client] %s\n", WelcomeMenu);
    printf("[client] Please, enter the command:");
    fflush(stdout);
    read(0, buf, sizeof(buf));
    input_command = atoi(buf);

    printf("[client] Am citit %d\n", input_command);

    if (write(sd, &input_command, sizeof(int)) <= 0) {
        perror("[client]Error writing command to server.\n");
        return errno;
    }
    handler_command(input_command);
}

void register_command() {
    char username[50];
    char password[50];
    char role[10];

    printf("[client] Enter username: ");
    fflush(stdout);
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("[client] Enter password: ");
    fflush(stdout);
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    int roleValue;
    do {
        printf("[client] Enter role (1 for seller, 2 for buyer): ");
        fflush(stdout);
        read(0, role, sizeof(role));
        roleValue = atoi(role);

        if (roleValue != 1 && roleValue != 2) {
            printf("[client] Invalid role. Please enter 1 for seller or 2 for buyer.\n");
        }
    } while (roleValue != 1 && roleValue != 2);

    role[sizeof(role) - 1] = '\0';

    if (write(sd, &roleValue, sizeof(int)) <= 0) {
        perror("[client] Error writing role to server.\n");
    }

    if (write(sd, username, sizeof(username)) <= 0 ||
        write(sd, password, sizeof(password)) <= 0) {
        perror("[client] Error writing registration data to server.\n");
    }
}

void handler_command(int input_command) {
    switch (input_command) {
    case 2:
        register_command();
        break;

    default:
        printf("Unknown command");
        break;
    }

    char message_from_server[200];
    if (read(sd, &message_from_server, sizeof(message_from_server)) <= 0) {
        perror("[client] Error reading message from server.\n");
    } else {
        printf("[client] Message from server: %s\n", message_from_server);
    }
}
