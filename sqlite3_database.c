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

int check_user_exists(Database *db, char *username, char *password) {
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *res;
    char sql[200];

    sql[0] = '\0';

    snprintf(sql, sizeof(sql), "SELECT ID,ROLE FROM USERS WHERE USERNAME=? AND PASSWORD=?");

    /* Execute SQL statement */
    rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return -1;  // Return -1 to indicate an error
    }

    /* Bind parameters (username and password) to the prepared statement */
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, 2, password, -1, SQLITE_STATIC);

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW) {
        /* If a row is returned, the user exists */
        int user_id = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        return user_id;
    } else {
        /* Handle other errors */
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(res);
        return -1;  // Return -1 to indicate an error
    }
}


bool check_username_exists(Database *db, char *username) {
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *res;
    char sql[200];   

    sql[0] = '\0';

    snprintf(sql, sizeof(sql), "SELECT COUNT(ID) FROM USERS WHERE USERNAME=?");

    /* Execute SQL statement */
    rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    /* Bind parameters (username and password) to the prepared statement */
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);

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
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(res);
        return false;
    }
}


int add_new_user(Database *db, const char *username, const char *password, const char *role) {
    
    char insert_query[100];
    snprintf(insert_query, sizeof(insert_query), "INSERT INTO USERS (username, password, role) VALUES ('%s', '%s', '%s');", username, password, role);

    int rc = sqlite3_exec(db->db, insert_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute INSERT query: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    printf("User added successfully.\n");
    return 1;
}



void get_role_user(Database *db, int userId, char *role) {
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *res;
    char sql[200];

    sql[0] = '\0';

    snprintf(sql, sizeof(sql), "SELECT ROLE FROM USERS WHERE ID=?");

    /* Execute SQL statement */
    rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return;
    }

    /* Bind parameters (username and password) to the prepared statement */
    sqlite3_bind_int(res, 1, userId);

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW) {
        /* If a row is returned, the user exists */
        const char *role1 = sqlite3_column_text(res, 0);
        strcpy(role, role1);
        sqlite3_finalize(res);
        return;
    } else {
        /* Handle other errors */
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(res);
        return;
    }
}

int add_new_product(Database *db, const char *name, const char *category, float price, int stock, char *unit_of_measure, int id_user)
{
    char insert_query[300];
    snprintf(insert_query, sizeof(insert_query), "INSERT INTO PRODUCTS (name, category, price, stock, unit_of_measure, id_user, created_at) VALUES ('%s', '%s', '%f', '%d', '%s', '%d', datetime('now'));", name, category, price, stock, unit_of_measure, id_user);

    int rc = sqlite3_exec(db->db, insert_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute INSERT query: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    printf("Product added successfully.\n");
    return 1;
}

void display_products_by_user_id(Database *db, int user_id, char *products) 
{
        char *zErrMsg = 0;
        int rc;
        sqlite3_stmt *res;
        char sql[200];

        sql[0] = '\0';

        snprintf(sql, sizeof(sql), "SELECT * FROM PRODUCTS WHERE ID_USER=?");

        /* Execute SQL statement */
        rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
            return;
        }

        sqlite3_bind_int(res, 1, user_id);

        while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
            const char *id = sqlite3_column_text(res, 0);
            const char *name = sqlite3_column_text(res, 1);
            const char *category = sqlite3_column_text(res, 2);
            const char *price = sqlite3_column_text(res, 3);
            const char *stock = sqlite3_column_text(res, 4);
            const char *unit_of_measure = sqlite3_column_text(res, 5);
            const char *id_user = sqlite3_column_text(res, 6);
            const char *created_at = sqlite3_column_text(res, 7);
            char product[1000];
            snprintf(product, sizeof(product), "ID: %s      NAME: %s        CATEGORY: %s       PRICE: %s       STOCK: %s       UNIT OF MEASURE: %s        CREATED AT: %s\n", id, name, category, price, stock, unit_of_measure, created_at);
            strcat(products, product);
        }

        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        }

        sqlite3_finalize(res);
}

int delete_product(Database *db, int id_product, int id_user)
{
    char delete_query[100];
    snprintf(delete_query, sizeof(delete_query), "DELETE FROM PRODUCTS WHERE ID=%d AND ID_USER=%d;", id_product, id_user);

    int rc = sqlite3_exec(db->db, delete_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute DELETE query: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    printf("Product deleted successfully.\n");
    return 1;
}
