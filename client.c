/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
*/
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


  do{
  /* citirea mesajului */
  printf ("[client]Introduce the number of command: ");
  fflush (stdout);
  read (0, buf, sizeof(buf));
  input_command=atoi(buf);
  //scanf("%d",&input_command);
  // printf("[client] Am citit %d\n",input_command);
  /* trimiterea mesajului la server */
  if (write (sd,&input_command,sizeof(int)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }
 
  //  handle_command(input_command);

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read (sd, &received_message,sizeof(received_message)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  printf ("[client]Mesajul primit este: %s\n", received_message);
  } while(input_command != 3);
  /* inchidem conexaiunea, am terminat */
  close (sd);

}



















void handle_command(int input_command)
{
    switch(input_command)
    {
        case 1:
        printf("You have selected login option");
        break;
        case 2:
        printf("You have selected register option");
        break;
        case 3:
        printf("You have selected exit option");
        break;
        default:
        printf("You have selected an invalid option");
        break;
    }
}