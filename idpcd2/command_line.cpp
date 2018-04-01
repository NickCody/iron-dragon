// =---------------------------------------------------------------------------
// c o m m a n d _ l i n e . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Command-line handling
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jun 23   nic   Created.
//

#include "PreComp.h"

#include "command_line.h"
#include "idpcd_defaults.h"
#include "../nic_lib/standard_config_file.h"
#include "version.h"

#ifdef UNIX

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

#ifdef __cplusplus
extern "C"
{
#endif
int getopt(int argc, char * const *argv, const char *optstring);
#ifdef __cplusplus
}
#endif

#endif

// Variable Declarations
// =---------------------------------------------------------------------------
//
char    Data_Directory[MAX_PATH] = "";
char    Log_Directory[MAX_PATH] = "";
char    Log_Filename[MAX_PATH] = "";
uint_16 TCP_Port_IDPCD           = DEF_IDPCD_PORT;
uint_16 Max_Clients              = DEF_MAX_CONNECTIONS;

#if UNIX

char    Cmd_String[]             = "Drovh?p:t:f:l:q"; //f:pth?c:d:rl:";
bool    Run_As_Daemon            = false;
char    Config_Filename[256]     = "";
bool    Quiet_Mode               = false;

#elif WIN32

char    Cmd_String[]             = "rovh?p:t:"; //pth?c:d:r";

#endif

bool    Create_Databases         = false;

// =---------------------------------------------------------------------------
// (global) P a r s e _ C o m m a n d _ L i n e 
//
// return true if we should continue execution or
//        false if we are to halt execution
//
// =---------------------------------------------------------------------------
bool Parse_Command_Line ( int argc, char** argv ) 
{
   int ch;

   while ((ch = getopt(argc, argv, Cmd_String)) != -1)
   {
      switch(ch) 
      {

#if UNIX
         case 'd':
         {
            strcpy ( Data_Directory, optarg );
            break;
         }

         case 'l':
         {
            strcpy ( Log_Filename, optarg );
            break;
         }

         case 'D':  
         {
            Run_As_Daemon = true;
            break;
         }

         case 'f':
         {
            strcpy ( Config_Filename, optarg );
            break;
         }

         case 'q':
         {
            Quiet_Mode = true;
            break;
         }
#endif
         
         case 'r':
         {
            Create_Databases = true;
            break;
         }

         case 'c':
         {
            Max_Clients = atoi( optarg );

            if ( Max_Clients > DEF_MAX_CLIENT_POOLSIZE )
            {
               //
               // display error (we can use printf because we haven't
               // become a daemon yet
               //
               printf ( "Too many connections specified (%d). "
                         "Current maximum is %d.", 
                         (int)Max_Clients, (int)DEF_MAX_CLIENT_POOLSIZE );
               return false;
            }
            break;
         }

         case 'p':
         {
            TCP_Port_IDPCD = atoi ( optarg );
            break;
         }

         case 'v':
         {
#if UNIX
            if ( !Quiet_Mode )
#endif
               printf ( "\nIron Dragon : Rail Empires Server Daemon Version %s\n\n", IDPCD_FILE_VERSION );
            return false;
         }

         case '?':
         case 'h':
         default:
         {
            Display_Usage();
            return false;
         }
      }
   }
   
   argc -= optind;
   
   return true;
}

// =---------------------------------------------------------------------------
// (global) P a r s e _ C o n f i g u r a t i o n
//
// =---------------------------------------------------------------------------
bool Parse_Configuration ( const char* file )
{
   if ( !strlen(file) )
      return true;

   standard_config_file sc ( file );

   if ( !sc.Get_Num_Sections() )
      return false;

   // Dump out the entries to the terminal...
   //

#if UNIX
   if ( !Quiet_Mode )
#endif
   {
      for ( int i=0; i < sc.Get_Num_Sections(); i++ )
      {
         printf ( "<%s>\n", sc.Get_Section_Name(i) );

         for ( int j=0; j < sc.Get_Num_Section_Entries(i); j++ )
         {
            char const * Entry_Label = sc.Get_Entry_Label_Name(i, j);
            char const * Entry_Value = sc.Get_Entry_Value(i,j);

            printf ( "\t%s = %s\n", Entry_Label, Entry_Value );
         }
      }
   }

   // Set the known options
   //
   uint_32 Active_Channel_Mask = CHANNEL_NORMAL;

   int Log_Section = sc.Get_Section_Number ( "LOG_OPTIONS" );
   int Log_Entry;

   if ( Log_Section != -1 )
   {
      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_DEBUG" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_DEBUG;

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_CONNECTIONLOG" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_CONNECTIONLOG;

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_ACTIVITYLOG" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_ACTIVITYLOG;

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_ERRORLOG" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_ERRORLOG;

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_MESSAGE_DETAIL" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_MESSAGE_DETAIL;

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_DATETIME" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_DATETIME;

     Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "CHANNEL_EXCEPTIONLOG" );
      if ( Log_Entry != -1 && 
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
         Active_Channel_Mask |= CHANNEL_EXCEPTIONLOG;
   }         

   Sys.Set_Active_MsgChannels ( Active_Channel_Mask );
   
   Log_Section = sc.Get_Section_Number ( "STARTUP_OPTIONS" );

   if ( Log_Section != -1 )
   {

#if UNIX
      char Port[12];

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "Run_As_Daemon" );
      
      if ( Log_Entry != -1 &&
           strcmp(sc.Get_Entry_Value(Log_Section,Log_Entry),"1")==0 )
           Run_As_Daemon = true;

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "Log_Filename" );
   
      if ( Log_Entry != -1 )
         strcpy ( Log_Filename, sc.Get_Entry_Value(Log_Section,Log_Entry) );

      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "Port" );
   
      if ( Log_Entry != -1 )
      {
         strcpy ( Port, sc.Get_Entry_Value(Log_Section,Log_Entry) );
         TCP_Port_IDPCD = (uint_16)atoi(Port);
      }
#endif
      
      Log_Entry = sc.Get_Entry_Label_Number ( Log_Section, "Data_Directory" );
   
      if ( Log_Entry != -1 )
         strcpy ( Data_Directory, sc.Get_Entry_Value(Log_Section,Log_Entry) );

   }

   return true;
}

// =---------------------------------------------------------------------------
// (global) D i s p l a y _ U s a g e
//
// =---------------------------------------------------------------------------
void Display_Usage( void )
{
#ifdef UNIX
   printf ( "Usage: idpcd [-Drovh?] [-p port] [-t port] [-f configfile] [-l logfile]\n" );
#elif WIN32
   printf ( "Usage: idpcd [-rovh?] [-p port] [-t port]\n" );
#endif

#ifdef UNIX
   printf ( "   -D                     run as a Daemon\n" );
   printf ( "   -f configfile          use the specified config filename\n" );
   printf ( "   -l logfile             set all output to logfilename\n" );
   printf ( "   -q                     quiet mode. No option summary and version info\n" );
   printf ( "                            on startup.\n" );
#endif

   printf ( "   -p port                idpcd main daemon port (44099)\n" );
   printf ( "   -d path                specify the path of the data directory\n" );
   printf ( "   -r                     create databases if not found\n" );
   printf ( "   -c max                 set the maximum number of active\n" );
   printf ( "                            client connections\n" );
   
   printf ( "   -v                     display daemon version/build number\n" );
   printf ( "   -h or -?               display usage and version information\n" );
   
   printf ( "\n" );
}


