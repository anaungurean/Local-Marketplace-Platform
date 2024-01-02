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


/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
  int userId;
  char role[10];
}thData;

Database db;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void handle_command(int input_command, char message_to_send[], struct thData *);
int login_command(int cl);
int register_command(int cl);
void add_product_command(int cl, int user_id);
void view_my_products_command(int cl, int user_id, char *products);
void delete_product_command(int cl, int user_id, char *message_to_send);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  
  if (open_database(&db, "database_marketplace.db") != SQLITE_OK) {
        perror("[server] Error at opening the database.\n");
        return errno;
    }
 else
    {
      printf("Database is ready for queries!\n");
    }


  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Waiting at the port  %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};			


static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Waiting a command...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
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
            int userId = login_command(tdL->cl);
            if(userId != -1)
              {
              tdL->userId = userId;
              get_role_user(&db, tdL->userId, tdL->role);
              printf("User id: %d\n", tdL->userId);
              printf("User role: %s\n", tdL->role);
              if (strcmp(tdL->role, "Seller") == 0)
                strcpy(message_to_send, "You are logged in as a Seller!");
              else
                strcpy(message_to_send, "You are logged in as a Buyer!");
              }
            else
              strcpy(message_to_send, "The username or the password is incorrect.Please try again to log in.");
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
            else
              strcpy(message_to_send, "You don't have the permissions to add a product.");
            break;
        case 6 :
            if (strcmp(tdL->role, "Seller") == 0)
              {
                view_my_products_command(tdL->cl, tdL->userId, message_to_send);
                delete_product_command(tdL->cl, tdL->userId, message_to_send);
              }
            else
              strcpy(message_to_send, "You don't have the permissions to delete the products.");
        case 7:
            if (strcmp(tdL->role, "Seller") == 0)
              {
                char products[9000];
                products[0] = '\0';
                view_my_products_command(tdL->cl, tdL->userId, products);
                strcpy(message_to_send, products);
              }
            else
              strcpy(message_to_send, "You don't have the permissions to delete a product.");
            break;
        default:
            strcpy(message_to_send, "Invalid command");
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

    printf("Name: %s\n", name);
    printf("Category: %s\n", category);
    printf("Price: %s\n", price);
    printf("Stock: %s\n", stock);
    printf("Unit of measure: %s\n", unit_of_measure);

    if (add_new_product(&db, name, category, atof(price), atoi(stock), unit_of_measure, user_id) == 1)
      printf("Product added successfully.\n");
    else
      printf("Product not added.\n");
}

void view_my_products_command(int cl, int user_id, char *products)
{
  display_products_by_user_id(&db, user_id,products);
  printf("%s\n", products);
}

void delete_product_command(int cl, int user_id, char *message_to_send)
{
  char id[100];

  if (read(cl, id, sizeof(id)) <= 0)
  {
    perror("Error reading id from client.\n");
  }

  if (delete_product(&db, atoi(id), user_id) == 1)
    strcpy(message_to_send, "Product deleted successfully.");
  else
    strcpy(message_to_send, "You don't have the right to delete this product.");
}