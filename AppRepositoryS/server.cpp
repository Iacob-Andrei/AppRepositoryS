#include "libraries.h"

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */


void sighandler(int sig)
{
	while(waitpid(-1, NULL, WNOHANG) > 0){};
}

int main ()
{
	sqlite3* db;
	int rc;
    rc = sqlite3_open("repository.db", &db);

    if (rc)
    {
        cout << "eroare la deschidere!\n";
        return 0;
    }


	struct sockaddr_in server;	// structura folosita de server
	struct sockaddr_in from;	
	string msg;		//mesajul primit de la client 
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

	cout << "[server]Asteptam la portul " << PORT << "...\n\n"; 

	while (1)
	{
		int client;
		socklen_t length = sizeof (from);

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
				msg.clear();
				//fflush (stdout);
				int funct = 0;

				/* citirea mesajului */
				if (read (client, &funct, sizeof(int)) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	/* inchidem conexiunea cu clientul */
					continue;		/* continuam sa ascultam */
				}
			
				cout << "[server]Am primit comanda nr: " << funct << endl;
				if( funct == 3 )
				{
					cout << "[server]Conexiune terminata.\n";
					close (client);
					exit(1);
				}
				else if( funct == 1 ) // clientul doreste sa ADAUGE o aplicatie
				{
					msg.clear();
					msg = "Astept mai multe informatii pentru adaugarea aplicatiei. ";
				
					send_msg(msg, client);		

					msg.clear();

					msg = receive_msg(client);

					cout << "[server]Am primit specificatiile: " << msg << endl;

					msg.clear();
					msg = "Aplicatie adaugata cu succes.";

					send_msg( msg , client );
				}
				else if( funct == 2 ) // clientul doreste sa CAUTE o aplicatie
				{
					msg.clear();
					msg = "Astept mai multe informatii pentru cautare. ";

					send_msg( msg , client );			

					msg.clear();

					msg = receive_msg(client);

					cout << "[server]Am primit specificatiile: " << msg;

					msg.clear();
					msg = "Rezultate pentru cautare....";

					send_msg(msg , client );
				}
			}
		}	// convorbire copil - client

		close(client);
	}
}