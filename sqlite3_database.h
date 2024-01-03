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
int check_product(Database *db, int id_product, int id_user);
void display_product(Database *db, int user_product, char *product);
int update_product(Database *db, int id_product, const char *name, const char *category, float price, int stock, char *unit_of_measure, int id_user);
void display_all_products(Database *db, char *products);
int check_existence_product(Database *db, int id_product);
int update_quantity_product(Database *db, int id_product, int quantity);
int check_quantity_product(Database *db, int id_product, int quantity);
int select_product_price(Database *db, int id_product);
void insert_new_transactions(Database *db, int id_product, int quantity, int id_user);
void display_my_transactions(int cl, int user_id, char *transactions);
void select_transactions_by_buyer_id(Database *db, int id_buyer, char *transactions);
void select_sales_by_seller_id(Database *db, int id_seller, char *sales);

#endif
