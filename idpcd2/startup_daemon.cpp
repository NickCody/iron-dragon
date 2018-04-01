// =---------------------------------------------------------------------------
// s t a r t u p _ d a e m o n . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Iron Dragon Server Daemon Startup Routines
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Aug 10   nic   Created.
//

#include "PreComp.h"
#include "idpcd_defaults.h"
#include "startup_daemon.h"
#include "connection_pool.h"
#include "group_pool.h"
#include "command_line.h"
#include "idpcd_group.h"
#include "version.h"
#include "database_subsystem.h"

#include "net_messages_admin.h"     // for DATETIME_BYTES

#ifdef WIN32
#include <process.h> // for _beginthread
#endif 

#if UNIX
extern bool Quiet_Mode;
extern File_Sink Logfile_Sink;
#endif

// =---------------------------------------------------------------------------
// Global Variables
//
// =---------------------------------------------------------------------------

db_metastructure  Client_MS;
connection_pool   Busy_Pool;
group_pool        Group_Pool;
sockaddr_in       Sock_Addr;

char              Start_DateTime[DATETIME_BYTES];

// =---------------------------------------------------------------------------
// Synchronization variables
//
// =---------------------------------------------------------------------------
listen_info     Listen_Info_IDPCD;
bool            Continue_Processing = false;

// =---------------------------------------------------------------------------
// (global) S t a r t u p _ D a e m o n
//
// PRE: Databases need to be opened
// =---------------------------------------------------------------------------
_result Startup_Daemon ( )
{
   _result res = RS_OK;

   // Here is where we set up our listening sockets
   //
   res = Setup_Listening_Socket ( TCP_Port_IDPCD, Listen_Info_IDPCD );
   RRETURNONFAILURE(res);

   if ( !Busy_Pool.Resize  ( Max_Clients ) )    return RS_NOMEMORY;
   if ( !Group_Pool.Resize ( Max_Clients ) )    return RS_NOMEMORY;

   #if UNIX
      // Set up log file
      //
      int Logfile_Count = 1;
      res = RS_ERR;

      while ( RFAILED(res) && Logfile_Count < 500 )
      {
         if ( !strlen(Log_Filename) )
            sprintf ( Log_Filename, "%s/idpcd.%s.%d.log", Data_Directory, IDPCD_FILE_VERSION, Logfile_Count );

         res = Logfile_Sink.CreateNew ( Log_Filename );
         //res = Logfile_Sink.Open ( Log_Filename );

         if ( RSUCCEEDED(res) )
         {
            #if UNIX
            if ( !Quiet_Mode )
            #endif
               printf ( "Opened Logfile %s\n", Log_Filename );
         }
         else
         {
            Logfile_Count++;
            Log_Filename[0] = 0;
         }
      }
   #endif

   // Initialize the databases
   //
   res = Open_Databases( Data_Directory );
   RRETURNONFAILURE(res);

   // Record the time that this daemon started up
   //
   char datestr[32];
   SS_Port_Get_DateTime_String ( datestr );
   SS_Port_Strcpy_Len ( Start_DateTime, datestr, DATETIME_BYTES );
   Sys.Message ( CHANNEL_NORMAL, "<action name=\"Daemon_Start\">" );
   Sys.Message ( CHANNEL_NORMAL, "  <datetime>%s</datetime>", datestr );
   Sys.Message ( CHANNEL_NORMAL, "</action>" );

   // Clear counters
   //
   Group_Pool.Clear_Counters();
   Busy_Pool.Clear_Counters();

   // Set processing flag so while() loop (below) continues forever...
   //
   Continue_Processing = true;

   // Main Loop - listen for new connections (non-block), and process existing
   //
   
   while ( Continue_Processing )
   {
      Process_Listening ( Listen_Info_IDPCD );
      Process_Busy ( );

      Process_Heartbeat();

      // Find out how busy the server is and sleep more or less
      //
      //uint_32 busy, free;
      //Busy_Pool.Num_Connections ( busy, free );
      uint_32 lobby, game_setup, ranked;
      Group_Pool.Get_Group_Connection_Counts ( lobby, game_setup, ranked );

      uint_32 busy_count = lobby + game_setup;

      if ( busy_count == 0 )
         SS_Port_Sleep ( IDLE_LISTEN_LOOP_TIMEOUT_MS );
      else if ( busy_count <= 5 )
         SS_Port_Sleep ( MAIN_LISTEN_LOOP_TIMEOUT_MS );
      else
         SS_Port_Sleep ( BUSY_LISTEN_LOOP_TIMEOUT_MS );
   }

   // Clear start time since we've stopped
   //
   SS_Port_Strcpy_Len ( Start_DateTime, "", DATETIME_BYTES );

   SS_Net_Close_Socket ( Listen_Info_IDPCD.Listen_Sock );
   Listen_Info_IDPCD.Listen_Sock = 0;

   return RS_OK;
}

// =---------------------------------------------------------------------------
// S h u t d o w n _ D a e m o n
//
//
_result Shutdown_Daemon ( void )
{
   Continue_Processing = false;

   // That *should* take care of all connections, but this will get zombie
   // connections (active connections not in a group) and reserved connections
   //
   Busy_Pool.Free_All_Connections();

   SS_Net_Close_Socket ( Listen_Info_IDPCD.Listen_Sock );

   Listen_Info_IDPCD.Clear();

   _result res = Close_Databases ( );

   char datestr[32];
   SS_Port_Get_DateTime_String ( datestr );
   Sys.Message ( CHANNEL_NORMAL, "<action name=\"Daemon_Stop\">" );
   Sys.Message ( CHANNEL_NORMAL, "  <datetime>%s</datetime>", datestr );
   Sys.Message ( CHANNEL_NORMAL, "</action>" );

   return res;
}

// =---------------------------------------------------------------------------
// (global) R e p o r t _ L i s t e n i n g _ H e a l t h
//
bool Report_Listening_Health  ( bool verbose, const listen_info& li )
{
   if ( li.Listen_Sock && 
        li.Listen_Sock != INVALID_SOCKET )
   {
      if ( verbose )
         Sys.Message ( CHANNEL_NORMAL, "   Thread_Status<IDPCD> : running on port %d", 
            (int)li.Listen_Port );

      return true;
   }
   else
   {
      if ( verbose )
         Sys.Message ( CHANNEL_NORMAL, "   Thread_Status<IDPCD> : stopped" );

      return false;
   }

   if ( Continue_Processing )
   {
      if ( verbose )
         Sys.Message ( CHANNEL_NORMAL, "   Thread_Status<Processing> : started" );
      return true;
   }
   else
   {
      if ( verbose )
         Sys.Message ( CHANNEL_NORMAL, "   Thread_Status<Processing> : stopped" );

      return false;
   }

}

// =---------------------------------------------------------------------------
// (global) R e p o r t _ S e r v e r _ S t a t s
//
bool Report_Server_Stats ( bool verbose )
{
   bool running = true;
   char ip[IP_STRING_BYTES];

   if ( verbose )
   {
      Sys.Message ( CHANNEL_NORMAL, "" );
      Sys.Message ( CHANNEL_NORMAL, "DAEMON SERVER STATISTICS" );
      Sys.Message ( CHANNEL_NORMAL, "------------------------" );

      if ( !Data_Directory || !strlen(Data_Directory)  )
         Sys.Message ( CHANNEL_NORMAL, "    Data_Directory<default> : (idpcd.exe directory)" );
      else
         Sys.Message ( CHANNEL_NORMAL, "    Data_Directory<default> : %s", 
             Data_Directory );

      if ( !Log_Directory || !strlen(Log_Directory)  )
         Sys.Message ( CHANNEL_NORMAL, "    Log_Directory<default> : (idpcd.exe directory)" );
      else
         Sys.Message ( CHANNEL_NORMAL, "    Log_Directory<default> : %s", 
             Log_Directory );

      Sys.Message ( CHANNEL_NORMAL, " " );

      Sys.Message ( CHANNEL_NORMAL, "   Server_Name<%s>", Listen_Info_IDPCD.Listen_Host_Name );

      for ( uint_08 i=0; i < Listen_Info_IDPCD.Listen_IP_Count; i++ )
      {
         SS_Net_Print_IP ( ip, Listen_Info_IDPCD.Listen_IPs[i] );
         Sys.Message ( CHANNEL_NORMAL, "   Server_IP<%s>", ip );
      }
   }

   running = running && Report_Listening_Health ( verbose, Listen_Info_IDPCD )
      && running;

   if ( verbose ) Sys.Message ( CHANNEL_NORMAL, " " );

   return running;
}

// =---------------------------------------------------------------------------
// (global) R e p o r t _ S e r v e r _ C o n n e c t i o n s
//
void Report_Server_Connections ( )
{
   Sys.Message ( CHANNEL_NORMAL, "" );
   Sys.Message ( CHANNEL_NORMAL, "DAEMON CONNECTION STATISTICS" );
   Sys.Message ( CHANNEL_NORMAL, " " );
   Sys.Message ( CHANNEL_NORMAL, "Connection Dump:" );
   Sys.Message ( CHANNEL_NORMAL, "----------------------------" );

   Busy_Pool.Report_Connections();

   Sys.Message ( CHANNEL_NORMAL, " " );
   Sys.Message ( CHANNEL_NORMAL, "Group Dump:" );
   Sys.Message ( CHANNEL_NORMAL, "----------------------------" );

   Group_Pool.Report_Groups();
}

// =---------------------------------------------------------------------------
// (global) D a e m o n _ S t a r t _ D a t e T i m e
//
// =---------------------------------------------------------------------------
const char* Daemon_Start_DateTime ( void )
{
   return Start_DateTime;
}

// =---------------------------------------------------------------------------
// (global) W r i t e _ S e r v e r _ M e s s a g e
//
void Write_Server_Message ( char* p_msg )
{
   char Platform[20];
   char Platform_Version[8];

   #if UNIX

     strcpy ( Platform, "BSD/OS" );
     strcpy ( Platform_Version, "4.0" );

   #elif WIN32

      OSVERSIONINFO osinfo;

      osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      GetVersionEx  ( &osinfo );
      
      // Fill out Platform
      //
      switch ( osinfo.dwPlatformId )
      {
         case VER_PLATFORM_WIN32_NT :

            strcpy ( Platform, "Windows NT" );
            sprintf ( Platform_Version, "%d.%d", 
               osinfo.dwMajorVersion, osinfo.dwMinorVersion );
            
            break;

         case VER_PLATFORM_WIN32_WINDOWS:
            
            if ( osinfo.dwMinorVersion == 0 )
               strcpy ( Platform, "Windows 95" );
            else
               strcpy ( Platform, "Windows 98" );

            Platform_Version[0] = 0;

            break;

         default:

            strcpy ( Platform, "Windows" );
            sprintf ( Platform_Version, "%d", osinfo.dwMajorVersion );

            break;
      }

   #endif

   sprintf ( p_msg, "Welcome to the Iron Dragon Server Version %s running on %s %s.",
         IDPCD_FILE_VERSION,
         Platform,
         Platform_Version );
}

// =---------------------------------------------------------------------------
// (global) O p e n _ D a t a b a s e s
//
// =---------------------------------------------------------------------------
_result Open_Databases ( const char* Data_Directory )
{
   char* Data_Dir = (char*)Data_Directory;
   _result res    = RS_OK;

   if ( !Data_Directory || !strlen(Data_Directory)  )
      Data_Dir = "./";

   // Open up each database
   //
   SS_DB_Initialize_MetaStructure ( &Client_MS, sizeof(client_record), 0, 0 );
   res = Open_Single_Database ( &Client_MS, Data_Dir, "clients.db" );
   RRETURNONFAILURE(res);

   return res;
}

// =---------------------------------------------------------------------------
// (global) C l o s e _ D a t a b a s e s
//
// =---------------------------------------------------------------------------
_result Close_Databases ( void )
{
   _result res = RS_OK;

   Close_Single_Database ( &Client_MS, "clients.db" );

   return res;
}

// =---------------------------------------------------------------------------
// (global) O p e n _ S i n g l e _ D a t a b a s e
//
// =---------------------------------------------------------------------------
_result Open_Single_Database ( db_metastructure* p_db, const char* path, const char* dbname )
{
   _result res = (_result)SS_DB_Open ( p_db, path, dbname );
   bool fCreate = false;

   if ( RFAILED(res) )
   {
      if ( Create_Databases )
      {
         fCreate = true;
         res = (_result)SS_DB_Create ( p_db, path, dbname );
      }
   }

   Sys.Message ( CHANNEL_NORMAL, "<action name=\"%s\">", fCreate ? "Create_Database" : "Open_Database" );
   Sys.Message ( CHANNEL_NORMAL, "  <dbname>%s</dbname>", dbname );
   Sys.Message ( CHANNEL_NORMAL, "  <dbpath>%s</dbpath>", path );

   if ( RFAILED(res) )
      Sys.Message ( CHANNEL_NORMAL, "<error>%s</error>", Result_String(res) );

   Sys.Message ( CHANNEL_NORMAL, "</action>" );

   return res;
}

// =---------------------------------------------------------------------------
// (global) O p e n _ S i n g l e _ D a t a b a s e
//
// =---------------------------------------------------------------------------
_result Close_Single_Database ( db_metastructure* p_db, const char* dbname )
{
   _result res = (_result)SS_DB_Close ( p_db );
 
   Sys.Message ( CHANNEL_NORMAL, "<action name=\"Close_Database\">" );
   Sys.Message ( CHANNEL_NORMAL, "  <dbname>%s</dbname>", dbname );

   if ( RFAILED(res) )
      Sys.Message ( CHANNEL_NORMAL, "<error>%s</error>", Result_String(res) );

   Sys.Message ( CHANNEL_NORMAL, "</action>" );

   return res;
}

// =---------------------------------------------------------------------------
// (global) B e g i n _ D a e m o n _ T h r e a d
//
// Starts a listening thread
//
// =---------------------------------------------------------------------------

_result Begin_Daemon_Thread ( void )
{

#if UNIX
   
   //pthread_t tid;
   //int ret = pthread_create ( &tid, 0, IDPCD_Thread_Proc, 0 );
   
   // Call directly, no thread
   //
   IDPCD_Thread_Proc ( 0 );

   return RS_OK;

#elif WIN32

   int ret = _beginthread ( IDPCD_Thread_Proc, 0, 0 );

   if ( NONZERO(ret) ) return RS_OK;
   else                return RS_ERR;

#endif
}


// =---------------------------------------------------------------------------
// Main Daemon Thread Entry Point
//
// Should be called as a parameter to CreateThread
//
// =---------------------------------------------------------------------------
#if UNIX
void* IDPCD_Thread_Proc ( void* linfo )
#elif WIN32
void IDPCD_Thread_Proc ( void* linfo )
#endif
{

   Startup_Daemon();

#if UNIX
   return 0;
#endif
}

// =---------------------------------------
// F i n d _ C l i e n t _ B y _ U s e r n a m e
//
_result Find_Client_By_Username ( const char* username, 
                                  uint_32& found_recnum )
{
   if ( !SS_DB_Is_Open ( &Client_MS ) ) return RS_NOTOPEN;

   // TODO:
   // we'll do a sequential search for now, despite the fact that
   // we all know sequential searches are ludicrous

   try
   {
      for ( uint_32 i=0; i < SS_DB_Get_Record_Count(&Client_MS); i++ )
      {
         client_record client;
         if ( 0 == SS_DB_Get_Record ( &Client_MS, i, (void**)&client ) )
         {
            if ( strcmp( username, client.username ) == 0 )
            {
               found_recnum = i;
               return RS_OK;
            }
         }
      }
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Find_Client_By_Username>" );
      return RS_EXCEPTION;
   }
   
   return RS_ERR;
}
