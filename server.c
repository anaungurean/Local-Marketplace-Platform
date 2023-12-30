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
}thData;

Database db;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void handle_command(int input_command, char message_to_send[], int cl);
int login_command(int cl);
void register_command(int cl);


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
  char message_to_send[100];
  handle_command(input_command, message_to_send, tdL.cl);
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

void handle_command(int input_command, char message_to_send[], int cl)
{  

    switch(input_command)
    {
        case 1:
            if(login_command(cl)== 1)
              strcpy(message_to_send, "You are logged in!");
            else
              strcpy(message_to_send, "The username or the password is incorrect.Please try again to log in.");
            break;
        case 2:
            register_command(cl);
            strcpy(message_to_send, "Command 2");
            break;
        case 3:
            strcpy(message_to_send, "Command 3");
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

  // printf("Username: %s\n", username);
  // printf("Password: %s\n", password);

  if(check_user_exists(&db,username,password))
    return 1;
  else
    return 0;  
}


void register_command(int cl)
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

  printf("Username: %s\n", username);
  printf("Password: %s\n", password);

}