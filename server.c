#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "stdbool.h"
#include "sqlite3_database.h"

#define PORT 2908

extern int errno;

typedef struct thData{
	int idThread; 
	int cl; 
  int userId;
  char role[10];
}thData;

Database db;

static void *treat_client(void *); 
void send_response_to_client(void *);
void handle_command(int input_command, char message_to_send[], struct thData *);
int login_command(int cl);
int register_command(int cl);
void add_product_command(int cl, int user_id);
void view_my_products_command(int cl, int user_id, char *products);
int delete_product_command(int cl, int user_id);
void check_if_user_can_edit_product(int cl, int user_id, char *info, char *id_product);
int edit_product_command(int cl, int user_id);
void view_all_products_command(int cl, char *products);
int buy_a_product_command(int cl, int user_id);
void display_my_transactions_command(int cl, int user_id, char *transactions);
void display_my_sales_command(int cl, int user_id, char *sales);
void view_products_filtred_by_category_command(Database *db, int cl, char *products_by_category);
void view_products_filtred_by_price_command(Database *db, int cl, char *products_filtred_by_price);
void return_a_product_command(int cl, int user_id, char *message_to_send);
void display_the_best_seller(char *message_to_send);
void display_the_most_sold_product_command(char *message_to_send);
void logout_command(char *message_to_send, int user_id, char *role);
int complete_profile_command(int cl, int user_id);
void display_profile_information_command(int cl, char *profile_info);

int main ()
{
  struct sockaddr_in server;
  struct sockaddr_in from;	
  int sd;		 
  int pid;
  pthread_t th[100];     
	int i=0;
  
  if (open_database(&db, "database_marketplace.db") != SQLITE_OK) {
        perror("[server] Error at opening the database.\n");
        return errno;
    }
 else
    {
      printf("Database is ready for queries!\n");
    }


  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  

  server.sin_family = AF_INET;	
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (PORT);
  
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

  while (1)
    {
      int client;
      thData * td; 
      int length = sizeof (from);

      printf ("[server]Waiting at the port  %d...\n",PORT);
      fflush (stdout);

      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
      

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat_client, td);	      
				
	}   
};			


static void *treat_client(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Waiting a command...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		send_response_to_client((struct thData*)arg);
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void send_response_to_client(void *arg)
{
  int input_command, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
  do{
	if (read (tdL.cl, &input_command,sizeof(int)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");

			}
      
  char message_to_send[9000];
  handle_command(input_command, message_to_send, &tdL);
  printf ("[Thread %d]Mesajul de trimis este: %s\n",tdL.idThread, message_to_send);
  if (write (tdL.cl, message_to_send, sizeof(message_to_send)) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
		
  }while(input_command != 3);

}

void handle_command(int input_command, char message_to_send[], struct thData * tdL)
{  

    switch(input_command)
    { int ok;
        case 1:
            if (strlen(tdL->role) > 0)
              strcpy(message_to_send, "You are already logged in.");
            else
            {
            int userId = login_command(tdL->cl);
            if(userId != -1)
              {
              tdL->userId = userId;
              get_role_user(&db, tdL->userId, tdL->role);
              if (strcmp(tdL->role, "Seller") == 0)
                strcpy(message_to_send, "You are logged in as a Seller!");
              else
                strcpy(message_to_send, "You are logged in as a Buyer!");
              }
            else
              strcpy(message_to_send, "The username or the password is incorrect.Please try again to log in.");
            }
            break;
        case 2:
            ok = register_command(tdL->cl);
            if(ok == -3)
              strcpy(message_to_send, "The info can not be empty.");
            else if (ok == -2)
              strcpy(message_to_send, "This username already exists.");
            else if (ok == 1)
              strcpy(message_to_send, "Your account has been registred.");
            else 
              strcpy(message_to_send, "Your account has not been registred due to an error. Try again.");
            break;
        case 3:
            strcpy(message_to_send, "Thank you for your visit!");
            break;
        case 4 :
            if (strcmp(tdL->role, "Seller") == 0)
              {
                add_product_command(tdL->cl, tdL->userId);
                strcpy(message_to_send, "You have added a new product.");
              }
            else if (strcmp(tdL->role, "Buyer") == 0)
              {
                char products[9000];
                products[0] = '\0';
                view_all_products_command(tdL->cl,products);
                strcpy(message_to_send, products);
              }
            else 
              strcpy(message_to_send, "You need to log in to have access to this command.");
            break;
        case 5 :
             if (strcmp(tdL->role, "Seller") == 0)
                {
                  ok = edit_product_command(tdL->cl, tdL->userId);
                  if (ok == 1)
                    strcpy(message_to_send, "You have edited the product.");
                  else
                    strcpy(message_to_send, "You don't have the permissions to edit this product.");
                }
             else if (strcmp(tdL->role, "Buyer") == 0)
                 {
                  ok = buy_a_product_command(tdL->cl, tdL->userId);
                  if (ok == -3)
                    strcpy(message_to_send, "The product doesn't exist.");
                  else if (ok == -2)
                     strcpy(message_to_send, "There's not enough quantity of this product.");
                  else 
                    sprintf(message_to_send, "The total cost is: %d euro. Thank you for your purchase.", ok); 
                 }
              else 
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 6 :
            if (strcmp(tdL->role, "Seller") == 0)
              {
                ok = delete_product_command(tdL->cl, tdL->userId);
                if (ok == 1)
                    strcpy(message_to_send, "You have deleted the product.");
                else
                  strcpy(message_to_send, "You don't have the permissions to delete this product.");
              }
            else if (strcmp(tdL->role, "Buyer") == 0)
            {   char transactions[9000];
                transactions[0] = '\0';
                display_my_transactions_command(tdL->cl, tdL->userId, transactions);
                strcpy(message_to_send, transactions);
            }
            else
              strcpy(message_to_send, "You need to log in to have access to this command.");
            break;
        case 7:
            if (strcmp(tdL->role, "Seller") == 0)
              {
                char products[9000];
                products[0] = '\0';
                view_my_products_command(tdL->cl, tdL->userId, products);
                strcpy(message_to_send, products);
              }
            else if (strcmp(tdL->role, "Buyer") == 0)
              {
                char products_by_category[9000];
                products_by_category[0] = '\0';
                view_products_filtred_by_category_command(&db, tdL->cl, products_by_category);
                strcpy(message_to_send, products_by_category);
              }
            else
              strcpy(message_to_send, "You need to log in to have access to this command.");
            break;
        case 8:
              if (strcmp(tdL->role, "Seller") == 0)
              {
                char products[9000];
                products[0] = '\0';
                view_all_products_command(tdL->cl,products);
                strcpy(message_to_send, products);
              }
              else if (strcmp(tdL->role, "Buyer") == 0)
              {
                char products_filtred_by_price[9000];
                products_filtred_by_price[0] = '\0';
                view_products_filtred_by_price_command(&db, tdL->cl, products_filtred_by_price);
                strcpy(message_to_send, products_filtred_by_price);
              }
              else
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 9:
              if (strcmp(tdL->role, "Seller") == 0)
                {
                  char sales[9000];
                  sales[0] = '\0';
                  display_my_sales_command(tdL->cl, tdL->userId, sales);
                  strcpy(message_to_send, sales);
                }
              else if (strcmp(tdL->role, "Buyer") == 0)
                  return_a_product_command(tdL->cl, tdL->userId, message_to_send);
              else
                  strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 10:
              if (strlen(tdL->role) > 0)
                display_the_best_seller(message_to_send);
              else
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 11:
              if (strlen(tdL->role) > 0)
                display_the_most_sold_product_command(message_to_send);
              else 
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 12:
              if (strlen(tdL->role) > 0)
              {
                if (complete_profile_command(tdL->cl, tdL->userId) == 1)
                  strcpy(message_to_send, "Your profile was updated. Thank you!");
                else
                  strcpy(message_to_send, "Sorry, but your profile was not updated.Please, try again!");
              }
              else
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 13:
              if (strlen(tdL->role) > 0)
                  {
                    char profile_info[9000];
                    profile_info[0] = '\0';
                    display_profile_information_command(tdL->cl, profile_info);
                    strcpy(message_to_send, profile_info);
                  }
              else
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        case 14:
              if (strlen(tdL->role) > 0)
                logout_command(message_to_send, tdL->userId, tdL->role);
              else
                strcpy(message_to_send, "You need to log in to have access to this command.");
              break;
        default:
              strcpy(message_to_send,"The command you typed is invalid. Try again.");
              break;
    }
}

      

int login_command(int cl)
{
  char username[100];
  char password[100];

  if (read(cl, username, sizeof(username)) <= 0)
  {
    perror("Error reading username from client.\n");
  }

  if (read(cl, password, sizeof(password)) <= 0)
  {
    perror("Error reading password from client.\n");
  }

  return check_user_exists(&db, username, password);
 
}

int register_command(int cl)
{  
  char username[100];
  char password[100];
  char role[10];

  if (read(cl, username, sizeof(username)) <= 0)
  {
    perror("Error reading username from client.\n");
  }

  if (read(cl, password, sizeof(password)) <= 0)
  {
    perror("Error reading password from client.\n");
  }

  if (read(cl, role, sizeof(role)) <= 0)
  {
    perror("Error reading role from client.\n");
  }

  if(strlen(username) == 0 || strlen(password) == 0)
    return -3;
  else if (check_username_exists(&db,username))
    return -2;
  
  if (add_new_user(&db, username, password, role) == 1)
    return 1;
  else
    return 0;
}

void add_product_command(int cl, int user_id)
{
    char name[100];
    char category[100];
    char price[100];
    char stock[100];
    char unit_of_measure[100];

    if (read(cl, name, sizeof(name)) <= 0)
    {
      perror("Error reading name from client.\n");
    }

    if (read(cl, category, sizeof(category)) <= 0)
    {
      perror("Error reading category from client.\n");
    }

    if (read(cl, price, sizeof(price)) <= 0)
    {
      perror("Error reading price from client.\n");
    }

    if (read(cl, stock, sizeof(stock)) <= 0)
    {
      perror("Error reading stock from client.\n");
    }

    if (read(cl, unit_of_measure, sizeof(unit_of_measure)) <= 0)
    {
      perror("Error reading unit_of_measure from client.\n");
    }

    if (add_new_product(&db, name, category, atof(price), atoi(stock), unit_of_measure, user_id) == 1)
      printf("Product added successfully.\n");
    else
      printf("Product not added.\n");
}

void view_my_products_command(int cl, int user_id, char *products)
{
  display_products_by_user_id(&db, user_id,products);
}

int delete_product_command(int cl, int user_id)
{
  char id[100];

  if (read(cl, id, sizeof(id)) <= 0)
  {
    perror("Error reading id from client.\n");
  }

  if (delete_product(&db, atoi(id), user_id) == 1)
    return 1;
  else 
    return 0;
}

void check_if_user_can_edit_product(int cl, int user_id, char *info, char *id_product)
{

  if (read(cl, id_product, sizeof(id_product)) <= 0)
  {
    perror("Error reading id_product from client.\n");
  }

  if (check_product(&db, atoi(id_product), user_id) == 1)
     {
       display_products_by_user_id(&db, user_id, info);
     }
  else
    strcpy(info, "You can't edit this product.");

}

int edit_product_command(int cl, int user_id)
{
  char id_product[100];
  char new_name[100];
  char new_category[100];
  char new_price[100];
  char new_stock[100];
  char new_unit_of_measure[100];

  if (read(cl, id_product, sizeof(id_product)) <= 0)
  {
    perror("Error reading id from client.\n");
  }
  
  if (read(cl, new_name, sizeof(new_name)) <= 0)
  {
    perror("Error reading name from client.\n");
  }

  if (read(cl, new_category, sizeof(new_category)) <= 0)
  {
    perror("Error reading category from client.\n");
  }

  if (read(cl, new_price, sizeof(new_price)) <= 0)
  {
    perror("Error reading price from client.\n");
  }

  if (read(cl, new_stock, sizeof(new_stock)) <= 0)
  {
    perror("Error reading stock from client.\n");
  }

  if (read(cl, new_unit_of_measure, sizeof(new_unit_of_measure)) <= 0)
  {
    perror("Error reading unit_of_measure from client.\n");
  }
   
  if (check_product(&db, atoi(id_product), user_id) == 1)
    {
      if (update_product(&db, atoi(id_product), new_name, new_category, atof(new_price), atoi(new_stock), new_unit_of_measure,user_id) == 1)
        return 1;
      else
        return 0;
    }
  else
    return 0;
  
}

void view_all_products_command(int cl, char *products)
{
  display_all_products(&db,products);
}

int buy_a_product_command(int cl, int user_id)
{
  char id_product[100];
  char quantity[100];

  if (read(cl, id_product, sizeof(id_product)) <= 0)
  {
    perror("Error reading id_product from client.\n");
  }

  if (read(cl, quantity, sizeof(quantity)) <= 0)
  {
    perror("Error reading quantity from client.\n");
  }

   if (check_existence_product(&db, atoi(id_product)) == 0)
      return -3;
   else if (check_quantity_product(&db, atoi(id_product), atoi(quantity)) == 0)
      return -2;
   else 
     {
        int total_cost = select_product_price(&db, atoi(id_product)) * atoi(quantity);
        update_quantity_product(&db, atoi(id_product), atoi(quantity));
        insert_new_transactions(&db, atoi(id_product), atoi(quantity), user_id);
        return total_cost;
     }

}

void display_my_transactions_command(int cl, int user_id, char *transactions)
{
  select_transactions_by_buyer_id(&db, user_id,transactions);
}

void display_my_sales_command(int cl, int user_id, char *sales)
{
  select_sales_by_seller_id(&db, user_id,sales);
}

void view_products_filtred_by_category_command(Database *db, int cl, char *products_by_category)
{
  char category[100];

  if (read(cl, category, sizeof(category)) <= 0)
  {
    perror("Error reading category from client.\n");
  }

  select_products_filtred_by_category(db, category, products_by_category);
}

void view_products_filtred_by_price_command(Database *db, int cl, char *products_filtred_by_price)
{
  char min_price[100];
  char max_price[100];

  if (read(cl, min_price, sizeof(min_price)) <= 0)
  {
    perror("Error reading price from client.\n");
  }

  if (read(cl, max_price, sizeof(max_price)) <= 0)
  {
    perror("Error reading price from client.\n");
  }

  select_products_filtred_by_price(db, atoi(min_price), atoi(max_price), products_filtred_by_price);
}

void return_a_product_command(int cl, int user_id, char *message_to_send)
{
  char id_transaction[100];

  if (read(cl, id_transaction, sizeof(id_transaction)) <= 0)
  {
    perror("Error reading id_transaction from client.\n");
  }

  if(check_existence_transaction_made_by_user(&db, atoi(id_transaction), user_id) == 0)
    {
        strcpy(message_to_send, "Invalid transaction id. You don't made this transaction. Please, check the id and try again.");
    }
  else if (check_valid_transaction(&db, atoi(id_transaction)) == 0)
    {
        strcpy(message_to_send, "Sorry, but the deadline for return has passed.");
    }
  else 
  {

        update_quantity_product_after_return(&db, atoi(id_transaction));
        delete_transaction(&db, atoi(id_transaction));
        strcpy(message_to_send, "The transaction was valid. The product has been  succesfully returned.");
  }

}


void display_the_best_seller(char *message_to_send)
{
  char best_sellers[9000];
  best_sellers[0] = '\0';
  select_the_best_seller(&db, best_sellers);
  strcpy(message_to_send, best_sellers);
}

void display_the_most_sold_product_command(char *message_to_send)
{
  char most_sold_product[9000];
  most_sold_product[0] = '\0';
  select_the_most_sold_product(&db, most_sold_product);
  strcpy(message_to_send, most_sold_product);
}

void logout_command(char *message_to_send, int user_id, char *role)
{
  strcpy(message_to_send, "You have been logged out. If you want to have acces again to functions, please log in again.");
  user_id = -1;
  strcpy(role, "");
  
}

int complete_profile_command(int cl, int user_id)
{
  char first_name[100];
  char last_name[100];
  char phone_number[100];
  char description[1000];

  if (read(cl, first_name, sizeof(first_name)) <= 0)
  {
    perror("Error reading first_name from client.\n");
  }

  if (read(cl, last_name, sizeof(last_name)) <= 0)
  {
    perror("Error reading last_name from client.\n");
  }

  if (read(cl, phone_number, sizeof(phone_number)) <= 0)
  {
    perror("Error reading phone_number from client.\n");
  }

  if (read(cl, description, sizeof(description)) <= 0)
  {
    perror("Error reading description from client.\n");
  }

  if (add_profile_information(&db, user_id, first_name, last_name, description, phone_number) == 1)
    return 1;
  else 
    return 0;

  
}

void display_profile_information_command(int cl, char *profile_info)
{
  char username[100];

  if (read(cl, username, sizeof(username)) <= 0)
  {
    perror("Error reading username from client.\n");
  }

  select_profile_information_based_on_username(&db, username, profile_info);

}