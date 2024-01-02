#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>
#include "stdbool.h"


typedef struct {
    sqlite3 *db;
}Database;

void initialize_database(Database *db);
int open_database(Database *db, const char *db_name);
void close_database(Database *db);

int check_user_exists(Database *db, char *username, char *password);
int add_new_user(Database *db, const char *username, const char *password, const char *role);
bool check_username_exists(Database *db, char *username);
void get_role_user(Database *db, int userId, char *role);
int add_new_product(Database *db, const char *name, const char *category, float price, int stock, char *unit_of_measure, int id_user);
void display_products_by_user_id(Database *db, int user_id, char *products);
int delete_product(Database *db, int id_product, int id_user);
#endif
