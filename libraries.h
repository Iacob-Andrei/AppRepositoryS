#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
        cout << "Eroare la write()!.\n";
        close(fd);
        exit(0);
    }
}

void send_msg ( string mesaj , int fd )
{
    int lungime = mesaj.length();
    send_integer( lungime, fd );

    if ( write ( fd , mesaj.c_str() , lungime ) <= 0 )
    {
        cout << "Eroare la write()!.\n";
        close(fd);
        exit(0);
    }
}

string receive_msg( int fd )
{
    int length;

    if( read ( fd, &length, sizeof(int)) <= 0 )
    {
        return "error";
    }

    char mesaj_chr[length];
    bzero(mesaj_chr,length);

    if( read ( fd, mesaj_chr, length ) <= 0  )
    {
        return "error";
    }

    string mesaj;
    mesaj = string( mesaj_chr , length );

    return mesaj;
}

static int callback(void* str, int argc, char** argv, char** azColName)
{
    int i;
    char* data = (char*) str;
    for (i = 0; i < argc; i++) 
    {
        if(argv[i])
        {
            strcat(data, azColName[i]);
            strcat(data, "=");
            strcat(data, argv[i]);
            strcat(data,"\n");
        }
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

string return_max_id_kit(sqlite3* db)
{
    char* zErrMsg; // mesaju de eroare
    char str[1024]; // primul argument din callback
    string result, sql;

    str[0]=0;
    result.clear();
    sql = "SELECT MAX(id_kit)+1 FROM OS_Version;";
    int rc = sqlite3_exec( db , sql.c_str() , callback , str , &zErrMsg );
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
    string id_kit = result.substr(poz+1,result.length()-poz-2);

    return id_kit;
}

tuple<string,string> read_for_apprepo()
{
    string cauta, insert, buff;

    do{
        cout << "[obligatoriu]Name: "; 
        getline(cin,buff);
        if(buff.empty() == 0)
        {
            insert = insert + "\"" + buff + "\"";
            cauta = cauta + "name = \"" + buff + "\"";
        }
    }while(buff.empty() != 0 );

    buff.clear();
    do{
        cout << "[obligatoriu]Developer: ";
        getline(cin,buff);
        if( buff.empty() == 0 )
        {
            insert = insert + ",\""+ buff + "\"";
            cauta = cauta + " AND manufacturer = \"" + buff + "\"";
        }
    }while( buff.empty() != 0 );

    return make_tuple(cauta,insert);
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
        cautare = cautare + "(min_storage <= " + buffmsg + " OR min_storage IS NULL)";
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

int send_file_to_client( int fd , string id_app , sqlite3* db )
{
    string sql = "SELECT kit_install FROM OS_Version WHERE id_kit = " + id_app + ";";
    int pos, nread;
    string filename, exists;
    filename.clear();
    filename = select_sql( sql , db );

    if( filename.empty() == 1 )
    {
        exists = "nu";
        send_msg( exists , fd );
        return 2;
    }

    exists = "da";
    send_msg( exists , fd );

    filename.pop_back();

    pos = filename.find("=");
    filename = filename.substr(pos + 1);

    pos = filename.find(".");
    string extensie = filename.substr(pos + 1);
    string name_for_client = id_app + "." + extensie;

    send_msg( name_for_client , fd );

    FILE *fp;
    fp = fopen(filename.c_str(), "rb");

    if(fp == NULL)
    {
        exit(1);
    }

    while(1)
    {
        char buff[512];
        bzero( buff , 512);
        nread = fread(buff,1,512,fp);

        if( nread > 0 )
        {
            write( fd , buff , nread );
        }

        if( nread < 512 )
        {
            if( feof(fp) )
            {
                break;
            }
            if(ferror(fp))
            {
                return 0;
            }        
        }
    }

    fclose(fp);
    return 1;
}

int receive_file_from_server( int fd )
{
    string name_app, exists;

    exists = receive_msg( fd );

    if( exists == "nu" )
    {
        return 2;
    }

    name_app.clear(); 
    name_app = receive_msg( fd );

    FILE *fp;
    fp = fopen( name_app.c_str() , "wb" );

    if(NULL == fp)
    {
        return 0;
    }

    int bytesReceived = 0;
    char recvBuff[512];
    bzero(recvBuff , 512 );

    while ( (bytesReceived = read(fd, recvBuff, 512)) > 0 )
    {
        fwrite(recvBuff, 1, bytesReceived, fp);
        cout << "Am primit " << bytesReceived << endl;

        if(bytesReceived < 512)
            break;
    }

    if(bytesReceived < 0)
    {
        return 0;
    }

    fclose(fp);

    return 1;
}

int send_file_to_server( int fd , string filename )
{
    string exists;

    int pos = filename.find(".");
    string extensie = filename.substr(pos + 1);
    extensie[extensie.length()] = '\0';

    send_msg( extensie , fd );

    FILE *fp;
    fp = fopen(filename.c_str(), "rb");

    if(fp == NULL)
    {
        perror("[-]Error in reading file.");
        exists = "-";
        send_msg( exists , fd ); 
        return 0;
    }

    exists = "OK ";
    send_msg( exists , fd );

    char buff[512];
    while(1)
    {
        bzero(buff, 512);
        int nread = fread(buff,1,512,fp);

        if( nread > 0 )
        {
            write( fd , buff , nread );
        }

        if( nread < 512 )
        {
            if( feof(fp) )
            {
                break;
            }
            if(ferror(fp))
            {
                return 0;
            }        
        }
    }

    fclose(fp);

    return 1;
}

string receive_file_from_client( int fd , sqlite3* db )
{
    string name_app = "apps/" + return_max_id_kit(db);
    string extensie, exists;

    extensie = receive_msg( fd );
    name_app = name_app + "." + extensie;

    exists = receive_msg( fd );

    if( exists == "-" )
    {
        cout << "Nu se va primi fisierul!\n";
        return "no file";
    }

    FILE *fp;
    fp = fopen( name_app.c_str() , "wb" );

    if(NULL == fp)
    {
        cout << "Eroare!\n";
        return "-";
    }

    int bytesReceived = 0;
    char recvBuff[512];
    bzero(recvBuff , 512 );

    while ( (bytesReceived = read(fd, recvBuff, 512)) > 0 )
    {
        fwrite(recvBuff, 1, bytesReceived, fp);
        if(bytesReceived < 512 )
            break;
    }

    if(bytesReceived < 0)
    {
        cout << "[server]Eroare la primire fisier!\n";
        return "-";
    }

    fclose(fp);
    cout << "Fisier primit cu succes!\n";
    return name_app;
} 
