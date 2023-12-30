#include "sqlite3_database.h"
#include <stdio.h>
#include "stdbool.h"
#include "string.h"
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

int execute_query(Database *db, const char *query) {
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db->db, query, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

bool check_user_exists(Database *db, char *username, char *password) {
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *res;
    char sql[200];   

    sql[0] = '\0';

    snprintf(sql, sizeof(sql), "SELECT COUNT(ID) FROM USERS WHERE USERNAME=? AND PASSWORD=?");

    /* Execute SQL statement */
    rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    /* Bind parameters (username and password) to the prepared statement */
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, 2, password, -1, SQLITE_STATIC);

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW) {
        /* If a row is returned, the user exists */
        const char *count = sqlite3_column_text(res, 0);

        if (strcmp(count, "0") == 0) {
            sqlite3_finalize(res);
            return false;
        } else {
            sqlite3_finalize(res);
            return true;
        }
    } else {
        /* Handle other errors */
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(res);
        return false;
    }
}
