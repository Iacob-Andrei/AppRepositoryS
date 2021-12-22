#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

void sighandler()
{
	while(waitpid(-1, NULL, WNOHANG) > 0){};
}

int main ()
{
	struct sockaddr_in server;	// structura folosita de server
	struct sockaddr_in from;	
	char msg[100];		//mesajul primit de la client 
	char msgrasp[100]=" ";        //mesaj de raspuns pentru client
	int sd;			//descriptorul de socket 


	/* crearea unui socket */
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror ("[server]Eroare la socket().\n");
		return errno;
	}

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
	if (listen (sd, 5) == -1)
	{
		perror ("[server]Eroare la listen().\n");
		return errno;
	}

	if (signal(SIGCHLD, sighandler) == SIG_ERR)
    {
    	perror("signal()");
    	return 1;
    }


	printf ("[server]Asteptam la portul %d...\n\n",PORT);

	/* servim in mod iterativ clientii... */
	while (1)
	{
		int client;
		int length = sizeof (from);

		fflush (stdout);

		/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
		client = accept (sd, (struct sockaddr *) &from, &length);

		/* eroare la acceptarea conexiunii de la un client */
		if (client < 0)
		{
			perror ("[server]Eroare la accept().\n");
			continue;
		}

		if( fork() == 0 )
		{
			close(sd);

			while(1)
			{
				/* s-a realizat conexiunea, se astepta mesajul */
				bzero (msg, 100);
				fflush (stdout);
				int funct = 0;

				/* citirea mesajului */
				if (read (client, &funct, sizeof(int)) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	/* inchidem conexiunea cu clientul */
					continue;		/* continuam sa ascultam */
				}
			
				printf ("[server]Am primit comanda nr: %d\n", funct);
				
				if( funct == 3 )
				{
					printf("[server]Conexiune terminata.\n");
					close (client);
					exit(1);
				}
				else if( funct == 1 ) // clientul doreste sa ADAUGE o aplicatie
				{
					/*pregatim mesajul de raspuns */
					bzero(msgrasp,100);
					strcat(msgrasp,"Astept mai multe informatii pentru adaugarea aplicatiei. ");
				
					
					/* returnam mesajul clientului */
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						continue;		/* continuam sa ascultam */
					}			

					bzero (msg, 100);
					fflush (stdout);

					if (read (client, msg, 100) <= 0)
					{
						perror ("[server]Eroare la read() de la client.\n");
						close (client);	/* inchidem conexiunea cu clientul */
						continue;		/* continuam sa ascultam */
					}

					printf ("[server]Am primit specificatiile: %s", msg);

					bzero(msgrasp,100);
					strcat(msgrasp,"Aplicatie adaugata cu succes.");

					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						continue;		/* continuam sa ascultam */
					}

				}
				else if( funct == 2 ) // clientul doreste sa CAUTE o aplicatie
				{
					/*pregatim mesajul de raspuns */
					bzero(msgrasp,100);
					strcat(msgrasp,"Astept mai multe informatii pentru cautare. ");
				
					
					/* returnam mesajul clientului */
					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						continue;		/* continuam sa ascultam */
					}			

					bzero (msg, 100);
					fflush (stdout);

					if (read (client, msg, 100) <= 0)
					{
						perror ("[server]Eroare la read() de la client.\n");
						close (client);	/* inchidem conexiunea cu clientul */
						continue;		/* continuam sa ascultam */
					}

					printf ("[server]Am primit specificatiile: %s", msg);

					bzero(msgrasp,100);
					strcat(msgrasp,"Rezultate pentru cautare....");

					if (write (client, msgrasp, 100) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						continue;		/* continuam sa ascultam */
					}
				}
			}
		}	// convorbire copil - client

		close(client);
	}
}
