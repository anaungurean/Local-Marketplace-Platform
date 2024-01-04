#include "sqlite3_database.h"
#include <stdio.h>
#include "stdbool.h"
#include "string.h"
#include <stdlib.h>

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

int check_product(Database *db, int id_product, int id_user) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT COUNT(ID) FROM PRODUCTS WHERE ID=%d AND ID_USER=%d;", id_product, id_user);

    sqlite3_stmt *stmt;
    int result = 0;

    if (sqlite3_prepare_v2(db->db, select_query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

int delete_product(Database *db, int id_product, int id_user)
{
    if (check_product(db, id_product,id_user) == 0)
        return 0;
    else
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
}

void display_product(Database *db, int user_product, char *product)
{
        char *zErrMsg = 0;
        int rc;
        sqlite3_stmt *res;
        char sql[200];

        sql[0] = '\0';

        snprintf(sql, sizeof(sql), "SELECT * FROM PRODUCTS WHERE ID=?");

        rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
            return;
        }

        sqlite3_bind_int(res, 1, user_product);

        if ((rc = sqlite3_step(res)) == SQLITE_ROW) {
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
        }

        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        }

        sqlite3_finalize(res);
}

int update_product(Database *db, int id_product, const char *name, const char *category, float price, int stock, char *unit_of_measure, int id_user)
{
    if (check_product(db, id_product,id_user) == 0)
        return 0;
    else
    {
    char update_query[300];
    snprintf(update_query, sizeof(update_query), "UPDATE PRODUCTS SET NAME='%s', CATEGORY='%s', PRICE='%f', STOCK='%d', UNIT_OF_MEASURE='%s' WHERE ID=%d AND ID_USER=%d;", name, category, price, stock, unit_of_measure, id_product, id_user);

    int rc = sqlite3_exec(db->db, update_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute UPDATE query: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    printf("Product updated successfully.\n");
    return 1;
    }
}

void display_all_products(Database *db, char *products) 
{
        char *zErrMsg = 0;
        int rc;
        sqlite3_stmt *res;
        char sql[200];

        sql[0] = '\0';

        snprintf(sql, sizeof(sql), "SELECT p.ID, p.NAME, p.CATEGORY, p.PRICE, p.STOCK, p.UNIT_OF_MEASURE, p.ID_USER, p.CREATED_AT, u.USERNAME FROM PRODUCTS p JOIN USERS u ON p.ID_USER = u.ID");

        rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
            return;
        }

        while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
            const char *id = sqlite3_column_text(res, 0);
            const char *name = sqlite3_column_text(res, 1);
            const char *category = sqlite3_column_text(res, 2);
            const char *price = sqlite3_column_text(res, 3);
            const char *stock = sqlite3_column_text(res, 4);
            const char *unit_of_measure = sqlite3_column_text(res, 5);
            const char *id_user = sqlite3_column_text(res, 6);
            const char *created_at = sqlite3_column_text(res, 7);
            const char *username = sqlite3_column_text(res, 8);
            char product[1000];
            snprintf(product, sizeof(product), "ID: %s      NAME: %s        CATEGORY: %s       PRICE: %s       STOCK: %s       UNIT OF MEASURE: %s        CREATED AT: %s        USERNAME SELLER %s \n", id, name, category, price, stock, unit_of_measure, created_at, username);
            strcat(products, product);
        }

        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        }

        sqlite3_finalize(res);
}

int check_quantity_product(Database *db, int id_product, int quantity) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT STOCK FROM PRODUCTS WHERE ID=%d;", id_product);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        int stock = sqlite3_column_int(res, 0);
        if (stock >= quantity) {
            sqlite3_finalize(res);
            return 1;
        }
    }

    sqlite3_finalize(res);
    return 0;
}
    
int update_quantity_product(Database *db, int id_product, int quantity) {
    char update_query[100];
    snprintf(update_query, sizeof(update_query), "UPDATE PRODUCTS SET STOCK=STOCK-%d WHERE ID=%d;", quantity, id_product);

    int rc = sqlite3_exec(db->db, update_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute UPDATE query: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    printf("Product updated successfully.\n");
    return 1;
}

int check_existence_product(Database *db, int id_product) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT * FROM PRODUCTS WHERE ID=%d;", id_product);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        sqlite3_finalize(res);
        return 1;
    }
    sqlite3_finalize(res);
    return 0;
}

int select_product_price(Database *db, int id_product) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT PRICE FROM PRODUCTS WHERE ID=%d;", id_product);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        int price = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        return price;
    }
    sqlite3_finalize(res);
    return 0;
}

int select_id_seller_product(Database *db, int id_product) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT ID_USER FROM PRODUCTS WHERE ID=%d;", id_product);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        int id_seller = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        return id_seller;
    }
    sqlite3_finalize(res);
    return 0;
}

void insert_new_transactions(Database *db, int id_product, int quantity, int id_user) {
    char insert_query[500];
    int total_cost = select_product_price(db, id_product) * quantity;
    int id_seller = select_id_seller_product(db, id_product);

    snprintf(insert_query, sizeof(insert_query), "INSERT INTO TRANSACTIONS (id_product, id_seller, id_buyer, total_cost, date, quantity) VALUES (%d, %d, %d, %d,  datetime('now'), %d);", id_product, id_seller, id_user, total_cost, quantity);

    int rc = sqlite3_exec(db->db, insert_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute INSERT query: %s\n", sqlite3_errmsg(db->db));
    }
}

void select_transactions_by_buyer_id(Database *db, int id_buyer, char *transactions)
{
    char select_query[1000];
    snprintf(select_query, sizeof(select_query), "SELECT p.name, t.total_cost, t.date, t.quantity, u.username, t.id \
                       FROM transactions t \
                       JOIN products p ON t.id_product = p.id \
                       JOIN users u ON t.id_seller = u.id \
                       WHERE t.id_buyer = %d;", id_buyer);
    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
    }
    int spent_sum = 0;
    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        const char *product_name = sqlite3_column_text(res, 0);
        const char *total_cost = sqlite3_column_text(res, 1);
        const char *transaction_date = sqlite3_column_text(res, 2);
        const char *quantity = sqlite3_column_text(res, 3);
        const char *seller_username = sqlite3_column_text(res, 4);
        const char *id_transaction = sqlite3_column_text(res, 5);
        char transaction[1000];
        snprintf(transaction, sizeof(transaction), "ID_TRANSACTION: %s PRODUCT NAME: %s        TOTAL COST: %sEURO      QUANTITY: %s       SELLER USERNAME: %s      TRANSACTION DATE: %s\n", id_transaction, product_name, total_cost, quantity, seller_username, transaction_date);
        strcat(transactions, transaction);
        spent_sum += atoi(total_cost);
    }
    char total_spent[100];
    snprintf(total_spent, sizeof(total_spent), "\nTOTAL SPENT: %dEURO\n",spent_sum);
    strcat(transactions, total_spent);

}

void select_sales_by_seller_id(Database *db, int id_seller, char *sales)
{
    char select_query[1000];
    snprintf(select_query, sizeof(select_query), "SELECT p.name AS product_name, t.total_cost, t.date AS transaction_date, t.quantity, u.username AS seller_username \
                       FROM transactions t \
                       JOIN products p ON t.id_product = p.id \
                       JOIN users u ON t.id_buyer = u.id \
                       WHERE t.id_seller = %d;", id_seller);
    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
    }
    int earned_sum = 0;
    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        const char *product_name = sqlite3_column_text(res, 0);
        const char *total_cost = sqlite3_column_text(res, 1);
        const char *transaction_date = sqlite3_column_text(res, 2);
        const char *quantity = sqlite3_column_text(res, 3);
        const char *buyer_username = sqlite3_column_text(res, 4);
        char sale[1000];
        snprintf(sale, sizeof(sale), "PRODUCT NAME: %s        TOTAL COST: %sEURO      QUANTITY: %s       BUYER USERNAME: %s      TRANSACTION DATE: %s\n", product_name, total_cost, quantity, buyer_username, transaction_date);
        strcat(sales, sale);
        earned_sum += atoi(total_cost);
    }
    char total_earned[100];
    snprintf(total_earned, sizeof(total_earned), "\nTOTAL EARNED: %dEURO\n", earned_sum);
    strcat(sales, total_earned);

   
}

void select_products_filtred_by_category(Database *db, char *category, char *products)
{
        char *zErrMsg = 0;
        int rc;
        sqlite3_stmt *res;
        char sql[200];

        sql[0] = '\0';

        snprintf(sql, sizeof(sql), "SELECT p.ID, p.NAME, p.CATEGORY, p.PRICE, p.STOCK, p.UNIT_OF_MEASURE, p.ID_USER, p.CREATED_AT, u.USERNAME FROM PRODUCTS p JOIN USERS u ON p.ID_USER = u.ID WHERE p.CATEGORY= '%s'", category);
        

        rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
            return;
        }

        while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
            const char *id = sqlite3_column_text(res, 0);
            const char *name = sqlite3_column_text(res, 1);
            const char *category = sqlite3_column_text(res, 2);
            const char *price = sqlite3_column_text(res, 3);
            const char *stock = sqlite3_column_text(res, 4);
            const char *unit_of_measure = sqlite3_column_text(res, 5);
            const char *id_user = sqlite3_column_text(res, 6);
            const char *created_at = sqlite3_column_text(res, 7);
            const char *username = sqlite3_column_text(res, 8);
            char product[1000];
            snprintf(product, sizeof(product), "ID: %s      NAME: %s        CATEGORY: %s       PRICE: %s       STOCK: %s       UNIT OF MEASURE: %s        CREATED AT: %s        USERNAME SELLER %s \n", id, name, category, price, stock, unit_of_measure, created_at, username);
            strcat(products, product);
        }

        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        }

        sqlite3_finalize(res);
}

void select_products_filtred_by_price(Database *db, int min_price, int max_price, char *products)
{
        char *zErrMsg = 0;
        int rc;
        sqlite3_stmt *res;
        char sql[200];

        sql[0] = '\0';

        snprintf(sql, sizeof(sql), "SELECT p.ID, p.NAME, p.CATEGORY, p.PRICE, p.STOCK, p.UNIT_OF_MEASURE, p.ID_USER, p.CREATED_AT, u.USERNAME FROM PRODUCTS p JOIN USERS u ON p.ID_USER = u.ID WHERE p.PRICE >= '%d' AND p.PRICE <= '%d'", min_price, max_price);
        rc = sqlite3_prepare_v2(db->db, sql, -1, &res, 0);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
            return;
        }

        while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
            const char *id = sqlite3_column_text(res, 0);
            const char *name = sqlite3_column_text(res, 1);
            const char *category = sqlite3_column_text(res, 2);
            const char *price = sqlite3_column_text(res, 3);
            const char *stock = sqlite3_column_text(res, 4);
            const char *unit_of_measure = sqlite3_column_text(res, 5);
            const char *id_user = sqlite3_column_text(res, 6);
            const char *created_at = sqlite3_column_text(res, 7);
            const char *username = sqlite3_column_text(res, 8);
            char product[1000];
            snprintf(product, sizeof(product), "ID: %s      NAME: %s        CATEGORY: %s       PRICE: %s       STOCK: %s       UNIT OF MEASURE: %s        CREATED AT: %s        USERNAME SELLER %s \n", id, name, category, price, stock, unit_of_measure, created_at, username);
            strcat(products, product);
        }

        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
        }

        sqlite3_finalize(res);
}

int check_existence_transaction_made_by_user(Database *db, int id_transaction, int id_buyer) {
    char select_query[100];
    snprintf(select_query, sizeof(select_query), "SELECT * FROM TRANSACTIONS WHERE ID=%d AND ID_BUYER=%d;", id_transaction, id_buyer);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        sqlite3_finalize(res);
        return 1;
    }

    sqlite3_finalize(res);
    return 0;
    
}

int check_valid_transaction(Database *db, int id_transaction){
    char select_query[300];
    snprintf(select_query, sizeof(select_query), "SELECT CAST((julianday('now') - julianday(SUBSTR(DATE, 1, 10))) AS INTEGER)  FROM TRANSACTIONS WHERE ID=%d;", id_transaction);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        int days = sqlite3_column_int(res, 0);
        if (days >= 14) {
            sqlite3_finalize(res);
            return 0;
        }
    }

    sqlite3_finalize(res);
    return 1;

}


int select_quantity_from_transaction(Database *db, int id_transaction)
{
    char select_query[300];
    snprintf(select_query, sizeof(select_query), "SELECT QUANTITY FROM TRANSACTIONS WHERE ID=%d;", id_transaction);

    int rc;
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db->db, select_query, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        return sqlite3_column_int(res, 0);

    }

    sqlite3_finalize(res);
}

void update_quantity_product_after_return(Database *db, int id_transaction)
{
    int quantity = select_quantity_from_transaction(db, id_transaction);
    char update_query[300];
    snprintf(update_query, sizeof(update_query), "UPDATE PRODUCTS SET STOCK=STOCK+%d WHERE ID=(SELECT ID_PRODUCT FROM TRANSACTIONS WHERE ID=%d);", quantity, id_transaction);

    int rc = sqlite3_exec(db->db, update_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute UPDATE query: %s\n", sqlite3_errmsg(db->db));
    }

    printf("Product quantity returned successfully.\n");
}

void delete_transaction(Database *db, int id_transaction)
{
    char delete_query[300];
    snprintf(delete_query, sizeof(delete_query), "DELETE FROM TRANSACTIONS WHERE ID=%d;", id_transaction);

    int rc = sqlite3_exec(db->db, delete_query, callback, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute DELETE query: %s\n", sqlite3_errmsg(db->db));
    }

    printf("Transaction deleted successfully.\n");
}