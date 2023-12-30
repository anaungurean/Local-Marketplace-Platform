#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
#define WelcomeMenu "\n Welcome to the LocalMarketPlacePlatform! Please select one option from below. The commands are: \n 1. login -> in case you have an account already \n 2. register -> in case you don't have an account yet \n 3. exit -> to close the app \n"
void handle_command(int input_command, int sd);
void login_command(int sd);
void register_command(int sd);

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int input_command=0;
  char buf[10];
  char received_message[1000];

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
 
  handle_command(input_command, sd);
  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read (sd, &received_message,sizeof(received_message)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  printf ("\n%s\n", received_message);
  } while(input_command != 3);
  /* inchidem conexiunea, am terminat */
  close (sd);
}

void handle_command(int input_command, int sd)
{
  switch(input_command)
  {
    case 1:
    {
      login_command(sd);
      break;
    }
    case 2:
    {  register_command(sd);
      break;
    }
    case 3:
    printf("\nYou have selected exit option.\n");
    break;
    default:
    printf("You have selected an invalid option.\n");
    break;
  }
}


void login_command(int sd)
{ printf("\nYou have selected login option.\n");
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
      
      // printf("Username: %s\n", username);
      // printf("Password %s\n", password);


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
      // printf("Username: %s\n", username);
      // printf("Password %s\n", password);

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