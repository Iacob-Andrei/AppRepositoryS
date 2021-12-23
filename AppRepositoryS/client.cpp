#include "libraries.h"

int main (int argc, char *argv[])
{
  int sd;			                // descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  string msg, msg2, alegere, nume_fisier;	// mesajul trimis
  int funct , port;
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

  while(1)
  {
    cout << "[1] adaugare de aplicatie in baza de date.\n";
    cout << "[2] cautare de aplicatie in baza de date. \n";
    cout << "[3] inchidere sesiune client. \n\n";
    cout << "\nIntroduceti cifra corespunzatoare comnenzii pe care doriti sa o folositi: \n";
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
      //cout << "[client]Mesajul primit este: \n[server]" << msg << endl;
      msg.clear();
      msg2.clear();
      cout << "[client]Introduceti specificatiile aici:\n";

      tie(msg, msg2) = read_for_apprepo();

      send_msg( msg , sd );
      send_msg( msg2 , sd );
      msg.clear();
      msg2.clear();

      msg = receive_msg( sd );

      if( msg == "exista" )   // exista deja aplicatia, intreb daca vrea sa introduca alta versiune
      {
        msg.clear();
        cout << "Exista deja aceasta aplicatie in repository. Doriti sa adaugati o versiune noua?[da/nu]";
        getline( cin , msg );
        if( msg.empty() == 1 )
          msg = "nu";

        send_msg( msg , sd );
        if( msg == "da" )
        {
          do{
            msg.clear();
            msg2.clear();
            tie( msg, msg2 ) = read_for_os();
            if( msg.empty() == 1 )
            {
              msg == "-";
              msg2 == "-";
              alegere = "nu";

              send_msg ( msg , sd );
              send_msg ( msg2, sd );
              break;
            }
            send_msg ( msg , sd );
            send_msg ( msg2, sd );

            cout << endl << "Doriti sa adaugati si un kit de instalare?\n";
            cout << "Daca da, introduceti numele fisierului, \"nu\", altfel: ";
            fflush(stdout);
            nume_fisier.clear();
            getline( cin, nume_fisier );

            if( nume_fisier.empty() == 1 )
              nume_fisier = "nu";

            send_msg( nume_fisier , sd );

            if( nume_fisier != "nu" )
            {
              send_file_to_server( sd , nume_fisier );
            }

            cout << endl << "Doriti sa mai adaugati si alte versiuni?[da/nu]";
            alegere.clear();
            getline( cin, alegere );

            if( alegere.empty() == 1 )
              alegere = "nu";

            send_msg ( alegere , sd );
          }while( alegere == "da" );
        }
      }
      else                    // nu exista aplicatia, cer celelalte date pentru 2 tabele
      {
        msg.clear();
        msg2.clear();

        cout << endl;
        fflush(stdout);

        tie( msg, msg2 ) = read_for_req();

        if( msg.empty() == 1 )
        {
          msg == "-";
          msg2 == "-";
        }

        send_msg ( msg , sd );
        send_msg ( msg2, sd );


        alegere.clear();
        cout << endl << "Doriti sa adaugati si versiuni?[da/nu]";
        getline( cin , alegere );
        if( alegere.empty() == 1 )
          alegere = "nu";

        send_msg ( alegere , sd );

        if( alegere == "da" )
        {
          do{
            msg.clear();
            msg2.clear();
            tie( msg, msg2 ) = read_for_os();
            if( msg.empty() == 1 )
            {
              msg == "-";
              msg2 == "-";
              alegere = "nu";

              send_msg ( msg , sd );
              send_msg ( msg2, sd );
              break;
            }
            send_msg ( msg , sd );
            send_msg ( msg2, sd );
            // to do - add kit install

            cout << endl << "Doriti sa mai adaugati si alte versiuni?[da/nu]";
            alegere.clear();
            getline( cin, alegere );
            if( alegere.empty() == 1 )
              alegere = "nu";

            send_msg ( alegere , sd );
          }while( alegere == "da" );
        }

      }
    }
    else if( funct == 2 )   // cautare
    {
      send_integer( funct , sd );
      msg.clear();

      msg = receive_msg( sd );
      //cout << "[client]Mesajul primit este: \n[server]" << msg << endl;
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

      cout << endl << "[client]Doriti sa descarcati si un kit de instalare?\n";
      cout << "Daca da, introduceti id_kit dorit, altfel [nu]:";
      fflush(stdout);

      alegere.clear();
      getline( cin , alegere );
      if( alegere.empty() == 1 )
        alegere = "nu";

      send_msg( alegere , sd );

      if( alegere != "nu" )
      {
        if( receive_file_from_server( sd ) == 1 )
        {
          cout << "[client]Fisier primit cu succes!\n";
          fflush(stdout);
        }
        else
        {
          cout << "[client]Eroare la fisier!\n";
          fflush(stdout);
        }
      }
    }
    else
    {
      cout << "Comanda eronata!\n\n";
    }
  }

  close (sd);
}
