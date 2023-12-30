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

int check_username_exists(Database *db, const char *username) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT * FROM users WHERE username = ?;");

    printf("Checking if username '%s' exists...\n", username);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, select_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare SELECT query: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute SELECT query: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);

    return (rc == SQLITE_ROW);  
}

int add_new_user(Database *db, const char *username, const char *password, const char *role) {
    
    if (check_username_exists(db, username))
        return -2;

    char insert_query[100];
    printf("%s", password);
    snprintf(insert_query, sizeof(insert_query), "INSERT INTO users (username, password, role) VALUES ('%s', '%s', '%s');", username, password, role);
    printf("%s", insert_query);
    int rc = sqlite3_exec(db->db, insert_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute INSERT query: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }

    printf("User added successfully.\n");
    return 0;
}

