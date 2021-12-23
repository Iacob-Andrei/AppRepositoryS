#include "libraries.h"
#define PORT 2024

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
	string msg, msg2, msg3, sql, copy_msg, sql2, id_app, id_kit, alegere, path;		
	int sd;						//descriptorul de socket 


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

	int enable = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		cout << "eroare\n";
		return 0;
	}

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
				int funct = 0;

				/* citirea mesajului */
				if (read (client, &funct, sizeof(int)) <= 0)
				{
					perror ("[server]Eroare la read() de la client.\n");
					close (client);	/* inchidem conexiunea cu clientul */
					continue;		/* continuam sa ascultam */
				}
			
				cout << "[server]Am primit comanda nr: " << funct << endl;
				fflush(stdout);

				if( funct == 3 )
				{
					cout << "[server]Conexiune terminata.\n";
					close (client);
					sqlite3_close (db);
					exit(1);
				}
				else if( funct == 1 ) // clientul doreste sa ADAUGE o aplicatie
				{
					msg.clear();
					msg = "Astept mai multe informatii pentru adaugarea aplicatiei. ";
				
					send_msg(msg, client);		
					msg.clear();
					msg2.clear();

					msg = receive_msg(client);
					msg2 = receive_msg(client);
					sql.clear();
					sql  = "SELECT * FROM AppRepository WHERE " + msg + ";";
					sql2 = "INSERT INTO AppRepository(name, manufacturer) VALUES(" + msg2 + ");";

					copy_msg = msg;
					msg = select_sql( sql , db );

					if( msg.empty() == 0 )		// exista deja o aplicatie cu acel nume si developer
					{
						msg.clear();
						msg = "exista";
						send_msg( msg , client );

						msg.clear();
						msg = receive_msg(client);

						if( msg == "da" )
						{
							sql2.clear();
							id_app.clear();
							sql2 = "SELECT id_app FROM AppRepository WHERE " + copy_msg + ";";
							id_app = return_id_app( sql2 , db );

							do{
								msg.clear();
								msg2.clear();
								
								msg = receive_msg( client );
								msg2 = receive_msg( client );

								if( msg == "-" )
								{
									break;
								}

								msg3.clear();
								msg3 = receive_msg( client );

								if( msg3 != "nu")
								{
									path.clear();
									path =  receive_file_from_client( client , db );

									if( path != "-" )
									{
										id_kit.clear();
										id_kit = return_max_id_kit(db);
										msg = "id_app," + msg + " , id_kit , kit_install ";
										msg2 = id_app + "," + msg2 + "," + id_kit + ", \"" + path +"\""; 
										sql.clear();
										sql = "INSERT INTO OS_Version(" + msg + ") VALUES(" + msg2 + ");";
										insert_sql( sql , db );

										alegere.clear();
										alegere = receive_msg( client );
									}
									else
									{
										alegere.clear();
										alegere = receive_msg( client );
									}
								}
								else
								{
									msg = "id_app," + msg;
									msg2 = id_app + "," + msg2;
									sql.clear();
									sql = "INSERT INTO OS_Version(" + msg + ") VALUES(" + msg2 + ");";
									insert_sql( sql , db );

									alegere.clear();
									alegere = receive_msg( client );
								}
							}while( alegere == "da" );
						}
					}
					else						// nu exista nicio aplicatie asa, cer restul inf
					{
						insert_sql( sql2 , db );

						sql2.clear();
						id_app.clear();
						sql2 = "SELECT id_app FROM AppRepository WHERE " + copy_msg + ";";
						id_app = return_id_app( sql2 , db );

						msg.clear();
						msg = "nu exista";
						send_msg( msg , client );

						msg.clear();
						msg2.clear();

						msg = receive_msg( client );
						msg2 = receive_msg( client );

						if( msg != "-" )		// am primit specificatii pentru req
						{
							msg = "id_app," + msg;
							msg2 = id_app + "," + msg2;
							sql.clear();
							sql = "INSERT INTO hardware_req(" + msg + ") VALUES(" + msg2 + ");";

							insert_sql( sql , db );
						}

						msg.clear();
						msg2.clear();
						alegere.clear();

						alegere = receive_msg ( client );

						if( alegere == "da" )
						{
							do{
								msg.clear();
								msg2.clear();
								
								msg = receive_msg( client );
								msg2 = receive_msg( client );

								if( msg == "-" )
								{
									break;
								}

								msg3.clear();
								msg3 = receive_msg( client );

								if( msg3 != "nu" )
								{
									path.clear();
									path =  receive_file_from_client( client , db );

									if( path != "-" )
									{
										id_kit.clear();
										id_kit = return_max_id_kit(db);
										msg = "id_app," + msg + " , id_kit , kit_install ";
										msg2 = id_app + "," + msg2 + "," + id_kit + ", \"" + path +"\""; 
										sql.clear();
										sql = "INSERT INTO OS_Version(" + msg + ") VALUES(" + msg2 + ");";
										insert_sql( sql , db );

										alegere.clear();
										alegere = receive_msg( client );
									}
									else
									{
										alegere.clear();
										alegere = receive_msg( client );
									}

								}
								else
								{
									msg = "id_app," + msg;
									msg2 = id_app + "," + msg2;
									sql.clear();
									sql = "INSERT INTO OS_Version(" + msg + ") VALUES(" + msg2 + ");";
									insert_sql( sql , db );

									alegere.clear();
									alegere = receive_msg( client );
								}
							}while( alegere == "da" );
						}
					}
				}
				else if( funct == 2 ) // clientul doreste sa CAUTE o aplicatie
				{
					msg.clear();
					msg = "Astept mai multe informatii pentru cautare. ";

					send_msg( msg , client );			

					msg.clear();
					msg = receive_msg(client);

					sql.clear();
					if(msg != "-" )
						sql =   "SELECT * FROM AppRepository LEFT JOIN OS_Version USING(id_app) LEFT JOIN hardware_req USING(id_app) WHERE " + msg + " ;";
					else
						sql =   "SELECT * FROM AppRepository Left JOIN OS_Version USING(id_app) left JOIN hardware_req USING(id_app);";

					msg.clear();
					msg = select_sql( sql , db );
					send_msg(msg , client );


					alegere.clear();
					alegere = receive_msg( client );

					if( alegere != "nu" )
					{
						if( send_file_to_client( client , alegere , db ) == 0 )
						{
							cout << "[server]eroare la trimitere fisier\n";
							fflush(stdout);
						}
						else{
							cout << "[server]Fisier trimis cu succes!\n";
							fflush(stdout);
						}
					}
				}
			}
		}	// convorbire copil - client

		close(client);
	}
}