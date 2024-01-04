#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
char role[10];
#define WelcomeMenu "\n\n Welcome to the LocalMarketPlacePlatform! Please select one option from below. The commands are: \n 1. Login \n 2. Register -> to create an account \n 3. Exit -> to close the app \n\n"
#define HomeMenuSeller "\n\n You are a Seller. You have the permissions to: \n 4. Add a new product. \n 5. Edit a product  -> in case you want to change the price or add stock. \n 6. Delete a product -> you need to know the id of the product \n 7. See your products \n 8. See all products \n 9. See your sales. \n 10. See the best sellers. \n 11. See the most sold products. 12. LogOut  \n\n"
#define HomeMenuBuyer "\n\n You are a Buyer. You have the opportunities to: \n 4. See all products .\n 5. Buy a product -> you need to know the id of the product. \n 6. View a history of purchases made. \n 7. Find a product by category \n 8. Find a product by price \n 9. Return a produs -> you need to know the id of the transaction \n 10. See the best seller.\n 11. See the most sold products. 12. LogOut  \n\n"

void handle_command(int input_command, int sd, char *received_message);
void login_command(int sd);
void register_command(int sd);
void print_home_menu(char *received_message);
int read_commands(int sd);
void set_role(char *received_message);
void add_product_command(int sd);
void view_my_products_command();
void delete_product_command(int sd);
void display_info_own_product_command(int sd);
void edit_product_command(int sd);
void view_all_products_command(int sd);
void buy_a_product_command(int sd);
void view_of_purchases_command(int sd);
void view_of_sales_command(int sd);
void find_product_by_category_command(int sd);
void find_product_by_price_command(int sd);
void return_a_product_command(int sd);
void view_the_best_seller_command(int sd);
void view_the_most_sold_products_command(int sd);
void logout_command(int sd);

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  printf("%s", WelcomeMenu);
  read_commands(sd);
  close (sd);
}

int read_commands(int sd)
{ 
  int input_command=0;
  char buf[10];
  char received_message[9000];

 do{

  printf ("Introduce the number of command: ");
  fflush (stdout);
  read (0, buf, sizeof(buf));
  input_command=atoi(buf);
  
  /* trimiterea comenzii la server */
  if (write (sd,&input_command,sizeof(int)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }
 
  handle_command(input_command, sd, received_message);
  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read (sd, &received_message,sizeof(received_message)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  printf ("\n%s\n", received_message);
  
  if(input_command == 1)
    set_role(received_message);
  
  if(input_command != 3)
    print_home_menu(received_message);
  
  } while(input_command != 3);

  return 0;
}

void handle_command(int input_command, int sd, char *received_message)
{
  switch(input_command)
  {
    case 1:
      login_command(sd);
      break;
    case 2:
      register_command(sd);
      break;
    case 3:
      printf("\nYou have selected exit option.\n");
      break;
    case 4:
      if (strcmp(role,"Seller") == 0)
          add_product_command(sd);
      else
          view_all_products_command(sd);
      break;
    case 5:
      if (strcmp(role, "Seller") == 0)  
          edit_product_command(sd);
      else if (strcmp(role,"Buyer") == 0)
          buy_a_product_command(sd);
      break;
    case 6:
      if (strcmp(role,"Seller") == 0)
          delete_product_command(sd);
      else if (strcmp(role,"Buyer") == 0)
          view_of_purchases_command(sd);
      break;
    case 7:
      if (strcmp(role,"Seller") == 0)
          view_my_products_command();
      else if (strcmp(role,"Buyer") == 0)
        find_product_by_category_command(sd);
      break;
    case 8:
      if (strcmp(role,"Seller") == 0)
          view_all_products_command(sd);
      else if (strcmp(role,"Buyer") == 0)
        find_product_by_price_command(sd);
      break;
    case 9:
      if (strcmp(role,"Seller") == 0)
          view_of_sales_command(sd);
      else if (strcmp(role,"Buyer") == 0)
         return_a_product_command(sd);
      break;
    case 10:
        if (strlen(role) > 0)
          view_the_best_seller_command(sd);
        break;
    case 11:
        if (strlen(role) > 0)
          view_the_most_sold_products_command(sd);
        break;
    case 12:
        if (strlen(role) > 0)
          logout_command(sd);
        break;
    default:
      printf("You have selected an invalid option.\n");
      break;
  }
}


void login_command(int sd)
{     printf("\nYou have selected login option.\n");
      char username[100];
      char password[100];
      
      printf("Enter username: ");
      fflush(stdout);
      fgets(username, sizeof(username), stdin);
      
      printf("Enter password: ");
      fflush(stdout);
      fgets(password, sizeof(password), stdin);
      
      username[strcspn(username, "\n")] = '\0';
      password[strcspn(password, "\n")] = '\0';
  
      if(write(sd, username, sizeof(username)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, password, sizeof(password)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
}

  
void register_command(int sd)
{
   printf("\nYou have selected register option.\n");
      char username[100];
      char password[100];
      char role[10];
      
      printf("Enter username: ");
      fflush(stdout);
      fgets(username, sizeof(username), stdin);
      
      printf("Enter password: ");
      fflush(stdout);
      fgets(password, sizeof(password), stdin);
      
      username[strcspn(username, "\n")] = '\0';
      password[strcspn(password, "\n")] = '\0';
      
      int roleValue;
       do {
        printf("Enter role (1 for seller, 2 for buyer): ");
        fflush(stdout);
        read(0, role, sizeof(role));
        roleValue = atoi(role);

        if (roleValue != 1 && roleValue != 2) {
            printf("[client] Invalid role. Please enter 1 for seller or 2 for buyer.\n");
        }
    } while (roleValue != 1 && roleValue != 2);

      if (roleValue == 1)
         strcpy(role,"Seller");
      else
          strcpy(role,"Buyer");

      if(write(sd, username, sizeof(username)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, password, sizeof(password)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, role, sizeof(role)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
  }


void set_role(char *received_message)
{
  if (strstr(received_message, "Seller!") != NULL)
          strcpy(role,"Seller");    
  else if (strstr(received_message, "Buyer!") != NULL)
          strcpy(role,"Buyer");
  else
          strcpy(role,"");
}

void print_home_menu(char *received_messagem)
{
   if(strcmp(role,"Seller") == 0)
      printf("%s", HomeMenuSeller);
    else if (strcmp(role,"Buyer") == 0)
      printf("%s", HomeMenuBuyer);
    else
      printf("You don't have a role yet. Please login or register.\n");
}

void add_product_command(int sd)
{
  printf("\nYou have selected add product option.\n");
      char name[100];
      char category[100];
      char price[100];
      char stock[100];
      char unit_of_measure[100];
      int ok = 0;

      
      printf("\nEnter the name of product: ");
      fflush(stdout);
      fgets(name, sizeof(name), stdin);
      
      do{
        ok = 0;
        printf("\nChoose a category: \n");
        printf("1. Fruits \n");
        printf("2. Vegetables \n");
        printf("3. Food \n");
        printf("4. Drinks \n");
        printf("5. Clothes \n");
        printf("6. Electronics \n");
        printf("7. Books \n");
        printf("8. Other \n");
        printf("Enter the category: ");
        fflush(stdout);
        fgets(category, sizeof(category), stdin);

        if (atoi(category) < 1 || atoi(category) > 8)
          { printf("Invalid category. Please choose a number between 1 and 8.\n");
            ok = 1;
          }
        else{
          switch(atoi(category))
          {
            case 1:
            strcpy(category,"Fruits");
            break;
            case 2:
            strcpy(category,"Vegetables");
            break;
            case 3:
            strcpy(category,"Food");
            break;
            case 4:
            strcpy(category,"Drinks");
            break;
            case 5:
            strcpy(category,"Clothes");
            break;
            case 6:
            strcpy(category,"Electronics");
            break;
            case 7:
            strcpy(category,"Books");
            break;
            case 8:
            strcpy(category,"Other");
            break;
          }
        }
      }while(ok);
      
      do{
        ok = 0;
        printf("\nEnter the price in euro: ");
        fflush(stdout);
        fgets(price, sizeof(price), stdin);
        if (atoi(price) <= 0)
          {printf("Invalid price.\n");
            ok = 1;
          }
      }
      while(ok);

      do{
        ok = 0;
        printf("\nEnter stock: ");
        fflush(stdout);
        fgets(stock, sizeof(stock), stdin);
        if (atoi(stock) <= 0)
          {printf("Invalid stock.\n");
            ok = 1;
          }
      }
      while(ok);

      do{
        ok = 0;
        printf("\nChoose the unity of measure:\n");
        printf("1. Meter \n");
        printf("2. Kilogram \n");
        printf("3. Liter \n");
        printf("4. Piece \n");
        printf("Enter the unity of measure: ");
        fflush(stdout);
        fgets(unit_of_measure, sizeof(unit_of_measure), stdin);
        if (atoi(unit_of_measure) < 1 || atoi(unit_of_measure) > 4)
          { printf("Invalid category. Please choose a number between 1 and 4.\n");
            ok = 1;
          }
        else{
          switch(atoi(unit_of_measure))
          {
            case 1:
            strcpy(unit_of_measure,"Meter");
            break;
            case 2:
            strcpy(unit_of_measure,"Kilogram");
            break;
            case 3:
            strcpy(unit_of_measure,"Liter");
            break;
            case 4:
            strcpy(unit_of_measure,"Piece");
            break;
          }
        }
      }while(ok);
      
      name[strcspn(name, "\n")] = '\0';
      category[strcspn(category, "\n")] = '\0';
      price[strcspn(price, "\n")] = '\0';
      stock[strcspn(stock, "\n")] = '\0';
      unit_of_measure[strcspn(unit_of_measure, "\n")] = '\0';

      printf("Name: %s\n", name);
      printf("Category: %s\n", category);
      printf("Price: %s\n", price);
      printf("Stock: %s\n", stock);
      printf("Unit of measure: %s\n", unit_of_measure);
      
      if(write(sd, name, sizeof(name)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, category, sizeof(category)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, price, sizeof(price)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, stock, sizeof(stock)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, unit_of_measure, sizeof(unit_of_measure)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
}

void view_my_products_command()
 {
   printf("\nYou have selected view my products option.\n");
 }

void delete_product_command(int sd)
{
  char id_product[100];
  printf("\nYou have selected delete product option.\n");
 
  do
  {
    printf("\nType the id of the product you want to delete: ");
    fflush(stdout);
    fgets(id_product, sizeof(id_product), stdin);
    if (atoi(id_product) <= 0)
        printf("Invalid id.\n");
          
  }
  while(atoi(id_product) <= 0);

  if(write(sd, id_product, sizeof(id_product)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
}


void display_info_own_product_command(int sd)
{
  char id_product[100];
  do{
    printf("\nEnter the id of the product you want to see: ");
    fflush(stdout);
    fgets(id_product, sizeof(id_product), stdin);
    if (atoi(id_product) <= 0)
        printf("Invalid id.\n");
  }
  while(atoi(id_product) <= 0);

  if(write(sd, id_product, sizeof(id_product)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
  
}

void edit_product_command(int sd)
{
      printf("\nYou have selected edit product option.\n");
      char name[100];
      char category[100];
      char price[100];
      char stock[100];
      char unit_of_measure[100];
      int ok = 0;
      char id_product[100];

      do{
        printf("\nEnter the id of the product you want to edit: ");
        fflush(stdout);
        fgets(id_product, sizeof(id_product), stdin);
        if (atoi(id_product) <= 0)
            printf("Invalid id.\n");
      }
      while(atoi(id_product) <= 0);

      printf("\nEnter the new name of product: ");
      fflush(stdout);
      fgets(name, sizeof(name), stdin);
      
      do{
        ok = 0;
        printf("\nChoose the new category: \n");
        printf("1. Fruits \n");
        printf("2. Vegetables \n");
        printf("3. Food \n");
        printf("4. Drinks \n");
        printf("5. Clothes \n");
        printf("6. Electronics \n");
        printf("7. Books \n");
        printf("8. Other \n");
        printf("Enter the category: ");
        fflush(stdout);
        fgets(category, sizeof(category), stdin);

        if (atoi(category) < 1 || atoi(category) > 8)
          { printf("Invalid category. Please choose a number between 1 and 8.\n");
            ok = 1;
          }
        else{
          switch(atoi(category))
          {
            case 1:
            strcpy(category,"Fruits");
            break;
            case 2:
            strcpy(category,"Vegetables");
            break;
            case 3:
            strcpy(category,"Food");
            break;
            case 4:
            strcpy(category,"Drinks");
            break;
            case 5:
            strcpy(category,"Clothes");
            break;
            case 6:
            strcpy(category,"Electronics");
            break;
            case 7:
            strcpy(category,"Books");
            break;
            case 8:
            strcpy(category,"Other");
            break;
          }
        }
      }while(ok);
      
      do{
        ok = 0;
        printf("\nEnter the new price in euro: ");
        fflush(stdout);
        fgets(price, sizeof(price), stdin);
        if (atoi(price) <= 0)
          {printf("Invalid price.\n");
            ok = 1;
          }
      }
      while(ok);

      do{
        ok = 0;
        printf("\nEnter the new stock: ");
        fflush(stdout);
        fgets(stock, sizeof(stock), stdin);
        if (atoi(stock) <= 0)
          {printf("Invalid stock.\n");
            ok = 1;
          }
      }
      while(ok);

      do{
        ok = 0;
        printf("\nChoose the new unity of measure:\n");
        printf("1. Meter \n");
        printf("2. Kilogram \n");
        printf("3. Liter \n");
        printf("4. Piece \n");
        printf("Enter the unity of measure: ");
        fflush(stdout);
        fgets(unit_of_measure, sizeof(unit_of_measure), stdin);
        if (atoi(unit_of_measure) < 1 || atoi(unit_of_measure) > 4)
          { printf("Invalid category. Please choose a number between 1 and 4.\n");
            ok = 1;
          }
        else{
          switch(atoi(unit_of_measure))
          {
            case 1:
            strcpy(unit_of_measure,"Meter");
            break;
            case 2:
            strcpy(unit_of_measure,"Kilogram");
            break;
            case 3:
            strcpy(unit_of_measure,"Liter");
            break;
            case 4:
            strcpy(unit_of_measure,"Piece");
            break;
          }
        }
      }while(ok);
      
      id_product[strcspn(id_product, "\n")] = '\0';
      name[strcspn(name, "\n")] = '\0';
      category[strcspn(category, "\n")] = '\0';
      price[strcspn(price, "\n")] = '\0';
      stock[strcspn(stock, "\n")] = '\0';
      unit_of_measure[strcspn(unit_of_measure, "\n")] = '\0';

      if (write(sd, id_product,sizeof(id_product)) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
      }
      
      if(write(sd, name, sizeof(name)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, category, sizeof(category)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, price, sizeof(price)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, stock, sizeof(stock)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }

      if(write(sd, unit_of_measure, sizeof(unit_of_measure)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
}

void view_all_products_command(int sd)
{
  printf("\nYou have selected to view all products option.\n");
  printf("\nThese are all the products: \n");
}

void buy_a_product_command(int sd)
{
  char id_product[100];
  char quantity[100];
  printf("\nYou have selected buy a product option.\n");
  do{
    printf("\nEnter the id of the product you want to buy: ");
    fflush(stdout);
    fgets(id_product, sizeof(id_product), stdin);
    if (atoi(id_product) <= 0)
        printf("Invalid id.\n");
    printf("\nEnter the quantity you want to buy: ");
    fflush(stdout);
    fgets(quantity, sizeof(quantity), stdin);
    if (atoi(quantity) <= 0)
        printf("Invalid quantity.\n");
  }
  while(atoi(id_product) <= 0 || atoi(quantity) <= 0);

  if(write(sd, id_product, sizeof(id_product)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
  if(write(sd, quantity, sizeof(quantity)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      } 
 
}

void view_of_purchases_command(int sd)
{
  printf("\nThese are the purchases that you made: \n");
}

void view_of_sales_command(int sd)
{
  printf("\nThese are the sales that you made: \n");
}

void find_product_by_category_command(int sd)
{
  char category[100];
  int ok = 0;
  do{
        printf("\nChoose the category:\n");
        printf("1. Fruits \n");
        printf("2. Vegetables \n");
        printf("3. Food \n");
        printf("4. Drinks \n");
        printf("5. Clothes \n");
        printf("6. Electronics \n");
        printf("7. Books \n");
        printf("8. Other \n");
        printf("Enter the category: ");
        fflush(stdout);
        fgets(category, sizeof(category), stdin);

        if (atoi(category) < 1 || atoi(category) > 8)
          { printf("Invalid category. Please choose a number between 1 and 8.\n");
            ok = 1;
          }
        else{
          switch(atoi(category))
          {
            case 1:
              strcpy(category,"Fruits");
              break;
            case 2:
              strcpy(category,"Vegetables");
              break;
            case 3:
              strcpy(category,"Food");
              break;
            case 4:
            strcpy(category,"Drinks");
            break;
            case 5:
            strcpy(category,"Clothes");
            break;
            case 6:
            strcpy(category,"Electronics");
            break;
            case 7:
            strcpy(category,"Books");
            break;
            case 8:
            strcpy(category,"Other");
            break;
          }
        }
      }while(ok);

      if(write(sd, category, sizeof(category)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
}

void find_product_by_price_command(int sd)
{
  char min_price[100];
  char max_price[100];
  int ok = 0;
  do{
        ok = 0;
        printf("\nEnter the minimum price: ");
        fflush(stdout);
        fgets(min_price, sizeof(min_price), stdin);
        if (atoi(min_price) <= 0)
          {printf("Invalid price.\n");
            ok = 1;
          }
      }
  while(ok);

  do{
        ok = 0;
        printf("\nEnter the maximum price: ");
        fflush(stdout);
        fgets(max_price, sizeof(max_price), stdin);
        if (atoi(max_price) <= 0)
          {printf("Invalid price.\n");
            ok = 1;
          }
      }
  while(ok);

  if(write(sd, min_price, sizeof(min_price)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }   
  
  if(write(sd, max_price, sizeof(max_price)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
  
}

void return_a_product_command(int sd)
{
  char id_transaction[100];
  printf("\nYou have selected return a product option.\n");
  printf("\nAttention you can only return a product while 14 days after transaction. \n");
  int ok = 0;
  do{
    printf("\nEnter the id of the transaction you want to return: ");
    fflush(stdout);
    fgets(id_transaction, sizeof(id_transaction), stdin);
    if (atoi(id_transaction) <= 0)
        {printf("Invalid id.\n");
          ok = 1;
        }
  }
  while(ok);

  if(write(sd, id_transaction, sizeof(id_transaction)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
}

void view_the_best_seller_command(int sd)
{
  printf("\nThese are the best sellers: \n");
}

void view_the_most_sold_products_command(int sd)
{
  printf("\nThese are the most sold products: \n");
}

void logout_command(int sd)
{
  printf("\nYou have selected logout option.\n");
  strcpy(role,"");

}