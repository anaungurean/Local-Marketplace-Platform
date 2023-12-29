#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>


typedef struct {
    sqlite3 *db;
}Database;

void initialize_database(Database *db);
int open_database(Database *db, const char *db_name);
void close_database(Database *db);

int check_existence_user(Database *db, const char *username, const char *password,const char *role);
int add_new_user(Database *db, const char *username, const char *password, const char *role);

#endif
