#include "sqlite3_database.h"
#include <stdio.h>

void initialize_database(Database *db) {
    db->db = NULL;
}

int open_database(Database *db, const char *db_name) {
    return sqlite3_open(db_name, &db->db);
}

void close_database(Database *db) {
    if (db->db != NULL) {
        sqlite3_close(db->db);
        db->db = NULL;
    }
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
   int i;
   for (i = 0; i < argc; i++)
   {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
  printf("\n");
   return 0;
}


int add_new_user(Database *db, const char *username, const char *password, const char *role){
    char sql_query[100];
    snprintf(sql_query,sizeof(sql_query), "SELECT * FROM users WHERE username= '%s';", username);

    if(sqlite3_exec(db->db, sql_query,callback, 0,0) != SQLITE_OK){
        fprintf(stderr, "Failed to execute SELECT query: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }

    char insert_query[200];
    snprintf(insert_query, sizeof(insert_query), "INSERT INTO users (username, password, role) VALUES ('%s', '%s', '%s');", username, password, role);

    if (sqlite3_exec(db->db, insert_query, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute INSERT query: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }

    printf("User added successfully.\n");
    return 0;
}