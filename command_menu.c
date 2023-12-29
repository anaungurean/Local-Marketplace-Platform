#include <stdio.h>
#include <string.h>
#include "command_menu.h"
#include "sqlite3_database.h"


void register_command(struct thData *td, Database *db) {
    char username[50];
    char password[50];
    char role[20];

    // Citește numele de utilizator
    if (read(td->cl, username, sizeof(username)) <= 0) {
        perror("[Thread] Error reading username from client!\n");
        return;
    }

    // Citește parola
    if (read(td->cl, password, sizeof(password)) <= 0) {
        perror("[Thread] Error reading password from client!\n");
        return;
    }

    // Citește rolul
    if (read(td->cl, role, sizeof(role)) <= 0) {
        perror("[Thread] Error reading role from client!\n");
        return;
    }

    // Apelul funcției care adaugă un nou utilizator în baza de date
    int result = add_new_user(db, username, password, role);

    // Trimite rezultatul înapoi la client (poate fi o confirmare de succes sau eroare)
    if (write(td->cl, &result, sizeof(int)) <= 0) {
        perror("[Thread] Error writing result to client!\n");
    }
}

