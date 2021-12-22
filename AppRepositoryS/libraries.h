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
#include <csignal>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <tuple>
#include <sqlite3.h>

extern int errno;
using namespace std;


void send_integer( int value , int fd )
{
    if ( write (fd, &value, sizeof(int)) <= 0)
    {
        perror ("[client]Eroare la write() spre server.\n");
        //return errno;
    }
}

void send_msg ( string mesaj , int fd )
{
    int lungime = mesaj.length();
    send_integer( lungime, fd );

    if ( write ( fd , mesaj.c_str() , lungime ) <= 0 )
    {
        perror ("[client]Eroare la write() spre server.\n");
        //return errno;
    }
}

string receive_msg( int fd )
{
    int length;
    char mesaj_chr[100000];
    bzero(mesaj_chr,100000);

    read ( fd, &length, sizeof(int));
    read ( fd, mesaj_chr, length );
    string mesaj = mesaj_chr;

    return mesaj;
}

static int callback(void* str, int argc, char** argv, char** azColName)
{
    int i;
    char* data = (char*) str;
    for (i = 0; i < argc; i++) 
    {
        strcat(data, azColName[i]);
        strcat(data, "=");

        if(argv[i])
        {
            strcat(data, argv[i]);
        }
        else
        {
            strcat(data,"-");
        }
        strcat(data,"\n");
    }

    strcat(data, "\n");
    return 0;
}

void insert_sql( string conditie_inserare , sqlite3* db)
{
    char* zErrMsg; // mesaju de eroare
    char str[1024]; // primul argument din callback
    string result;

    str[0]=0;
    result.clear();

    int rc = sqlite3_exec( db , conditie_inserare.c_str() , callback , str , &zErrMsg );
    if (rc != SQLITE_OK)
    {
        result = zErrMsg;
        cout << result;
        sqlite3_free (zErrMsg);
    }
    else
    {
        str[strlen(str)-1]='\0';
        result = str;
        cout << result;
    }
}

string select_sql( string conditie_select   , sqlite3* db)
{
    char* zErrMsg; // mesaju de eroare
    char str[100000]; // primul argument din callback
    string result;

    str[0]=0;
    result.clear();

    int rc = sqlite3_exec( db , conditie_select.c_str() , callback , str , &zErrMsg );
    if (rc != SQLITE_OK)
    {
        result = zErrMsg;
        sqlite3_free (zErrMsg);
        return result;
    }
    else
    {
        str[strlen(str)-1]='\0';
        result = str;
        return result;
    }
}

string return_id_app( string conditie_select , sqlite3* db) 
{
    char* zErrMsg; // mesaju de eroare
    char str[1024]; // primul argument din callback
    string result;

    str[0]=0;
    result.clear();

    int rc = sqlite3_exec( db , conditie_select.c_str() , callback , str , &zErrMsg );
    if (rc != SQLITE_OK)
    {
        result = zErrMsg;
        cout << result;
        sqlite3_free (zErrMsg);
        return 0;
    }
    else
    {
        str[strlen(str)-1]='\0';
        result = str;
    }

    int poz = result.find('=');
    string id_app = result.substr(poz+1,result.length()-poz-2);
    return id_app;
} 

//tuple<string,string> read_for_apprepo()
string read_for_apprepo()
{
    string cauta, insert, buff;

    do{
        cout << "[obligatoriu]Name: "; 
        getline(cin,buff);
        if(buff.empty() == 0)
        {
            //insert = insert + "\"" + buff + "\"";
            cauta = cauta + "name = \"" + buff + "\"";
        }
    }while(buff.empty() != 0 );

    buff.clear();
    do{
        cout << "[obligatoriu]Developer: ";
        getline(cin,buff);
        if( buff.empty() == 0 )
        {
            //insert = insert + ",\""+ buff + "\"";
            cauta = cauta + " AND manufacturer = \"" + buff + "\"";
        }
    }while( buff.empty() != 0 );

    //return make_tuple(cauta,insert);
    return cauta;
}

tuple<string,string> read_for_os()
{
    string values_list, inserare, buffmsg;
    int count_arg = 0;

    buffmsg.clear();
    cout << "Operating System: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        values_list = values_list + "OS";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    buffmsg.clear();
    cout << "Version: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        if( count_arg != 0 )
        {
            values_list = values_list + ',';
            inserare = inserare + ",";
        }

        values_list = values_list + "version";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    return make_tuple(values_list,inserare);
}

tuple<string,string> read_for_req()
{

    int count_arg = 0;
    string buffmsg, values_list, inserare;

    buffmsg.clear();
    cout << "Minimum requiement RAM[in GB]: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        values_list = values_list + "min_RAM";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    buffmsg.clear();
    cout << "Minimum requirement storage [in GB]: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        if( count_arg != 0 )
        {
            values_list = values_list + ',';
            inserare = inserare + ",";
        }

        values_list = values_list + "min_storage";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    buffmsg.clear();
    cout << "Minimum GPU requirement: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        if( count_arg != 0 )
        {
            values_list = values_list + ',';
            inserare = inserare + ",";
        }

        values_list = values_list + "min_GPU";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    buffmsg.clear();
    cout << "Minimum CPU requirement: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        if( count_arg != 0 )
        {
            values_list = values_list + ',';
            inserare = inserare + ",";
        }

        values_list = values_list + "min_CPU";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    buffmsg.clear();
    cout << "Internet requirement[yes/no]: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        if( count_arg != 0 )
        {
            values_list = values_list + ',';
            inserare = inserare + ",";
        }

        values_list = values_list + "internet_conn";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    cout << "Licensing type[free/paid - amount$]: ";
    getline( cin , buffmsg );
    if( buffmsg.empty() == 0 )
    {
        if( count_arg != 0 )
        {
            values_list = values_list + ',';
            inserare = inserare + ",";
        }

        values_list = values_list + "licensing";
        inserare = inserare + "\"" + buffmsg + "\"";
        count_arg ++;
    }

    return make_tuple(values_list,inserare);
}

string read_for_search()
{
    string buffmsg, cautare;
    int count_arg = 0;

    cout << "Introduceti date pentru cautare:\n";
    cout << "Name: "; getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        cautare = cautare + "name = \"" + buffmsg + "\"";
        count_arg++;
    }

    buffmsg.clear();
    cout << "Developer: ";
    getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        if(count_arg != 0)
        {
            cautare.append(" AND ");
        }
        cautare = cautare + "manufacturer = \"" + buffmsg + "\"";
        count_arg++;
    }

    buffmsg.clear();
    cout << "Operating System: ";
    getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        if(count_arg != 0)
        {
            cautare.append(" AND ");
        }
        cautare = cautare + "OS = \"" + buffmsg + "\"";
        count_arg++;
    }

    buffmsg.clear();
    cout << "Maximum RAM requirements[in GB]: ";
    getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        if(count_arg != 0)
        {
            cautare.append(" AND ");
        }
        cautare = cautare + "(min_RAM <= " + buffmsg + "OR min_RAM IS NULL)";
        count_arg++;
    }

    buffmsg.clear();
    cout << "Maximum storage requirements[in GB]: ";
    getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        if(count_arg != 0)
        {
            cautare.append(" AND ");
        }
        cautare = cautare + "(min_storage <= " + buffmsg + "OR min_storage IS NULL)";
        count_arg++;
    }

    buffmsg.clear();
    cout << "Internet connection?[yes/no]: ";
    getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        if(count_arg != 0)
        {
            cautare.append(" AND ");
        }
        cautare = cautare + "internet_conn = " + buffmsg ;
        count_arg++;
    }

    buffmsg.clear();
    cout << "Licensing type[free/paid]: ";
    getline(cin,buffmsg);
    if(buffmsg.empty() == 0)
    {
        if(count_arg != 0)
        {
            cautare.append(" AND ");
        }
        cautare = cautare + "( licensing like \" " + buffmsg + "%\" OR licensing IS NULL )";
        count_arg++;
    }


    return cautare;
}