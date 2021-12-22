#include "libraries.h"

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  string msg;		// mesajul trimis
  int funct;
  char msg_receive[100000];

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

  cout << "\nIntroduceti cifra corespunzatoare comnenzii pe care doriti sa o folositi: \n";
  cout << "[1] adaugare de aplicatie in baza de date.\n";
  cout << "[2] cautare de aplicatie in baza de date. \n";
  cout << "[3] inchidere sesiune client. \n\n";

  while(1)
  {
    /* citirea mesajului */
    cout << "[client]Introduceti numarul comenzii: ";
    cin >> funct;
    cin.ignore();       // flush the new line chr out of the buffer

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
    else if( funct == 1 )     // adaugare
    {
      send_integer( funct , sd );
      msg.clear();

      msg = receive_msg( sd );
      cout << "[client]Mesajul primit este: \n[server]" << msg << endl;
      msg.clear();

      cout << "[client]Introduceti specificatiile aici: ";


      // to do - sa introduc aici cin pentru a lua inf despre adaugare
      getline( cin , msg );


      send_msg( msg , sd );
      msg.clear();

      msg = receive_msg( sd );
      cout << "[client]Mesajul primit este: \n[server]" << msg << endl;
    }
    else if( funct == 2 )   // cautare
    {
      send_integer( funct , sd );
      msg.clear();

      msg = receive_msg( sd );
      cout << "[client]Mesajul primit este: \n[server]" << msg << endl;
      fflush(stdout);
      msg.clear();

      cout << "[client]Introduceti specificatiile aici: \n";

      string cautare = read_for_search();
      if( cautare.empty() == 1 )
        cautare = "-";
      cout << cautare << endl;

      send_msg( cautare , sd );
      msg.clear();

      msg = receive_msg( sd );
      cout << "[client]Am primit urmatoarele aplicatii: \n\n" << msg << endl;
    }
    else
    {
      printf("Comanda eronata!\n\n");
    }
  }

  close (sd);
}
