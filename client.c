#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[100];		// mesajul trimis
  int funct;

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


  printf("\nIntroduceti cifra corespunzatoare comnenzii pe care doriti sa o folositi: \n");
  printf("1 - adaugare de aplicatie in baza de date.\n");
  printf("2 - cautare de aplicatie in baza de date. \n");
  printf("3 - inchidere sesiune client. \n\n");


  // BUCLA WHILE PANA LA FINAL
  while(1)
  {
    /* citirea mesajului */
    printf ("[client]Introduceti numarul comenzii: ");
    scanf("%d",&funct);

    if( funct == 3 )
    {
      if (write (sd, &funct, sizeof(int)) <= 0)
      {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
      }

      close (sd);
      return 0;
    }


    /* trimiterea mesajului la server */
    if (write (sd, &funct, sizeof(int)) <= 0)
    {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }

    /* citirea raspunsului dat de server 
      (apel blocant pina cand serverul raspunde) */
    if (read (sd, msg, 100) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    /* afisam mesajul primit */
    printf ("[client]Mesajul primit este: \n[server]%s\n", msg);

    bzero (msg, 100);
    printf ("[client]Introduceti specificatiile aici: ");
    fflush (stdout);
    read (0, msg, 100);

    if (write (sd, msg, 100) <= 0)
    {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }
    if (read (sd, msg, 100) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    printf ("[client]Mesajul primit este: \n[server]%s\n\n", msg);
  }

  /* inchidem conexiunea, am terminat */
  close (sd);
}
