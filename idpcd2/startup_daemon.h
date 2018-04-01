// =---------------------------------------------------------------------------
// s t a r t u p _ d a e m o n . h
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


#ifndef __STARTUP_DAEMON_H__
#define __STARTUP_DAEMON_H__

#include "net_subsystem.h"
#include "database_subsystem.h"
#include "net_subsystem.h"

// =---------------------------------------------------------------------------
// Type definitions
//
// =---------------------------------------------------------------------------

struct listen_info
{
   listen_info( )
   {  
      Clear(true);
   }

   void Clear ( bool init = false )
   {
      Listen_Sock          = INVALID_SOCKET; 
      Listen_IP_Count      = 0;
      Listen_Port          = 0;
      Listen_Host_Name[0]  = 0;

      if ( !init && Listen_IPs ) delete [] Listen_IPs;

      Listen_IPs           = 0;

   }

   int              Listen_Sock;
//   lockable_object  Listen_Locker;
   uint_32*         Listen_IPs;
   uint_16          Listen_IP_Count;
   uint_16          Listen_Port;
   char             Listen_Host_Name[HOSTNAME_BUFFER_BYTES];
};

// =---------------------------------------------------------------------------
// External Variable Declarations
//
// =---------------------------------------------------------------------------

extern char                   Version[];

//extern class client_database  Client_DB;
extern struct db_metastructure Client_MS;
extern class connection_pool  Busy_Pool;
extern class group_pool       Group_Pool;

extern struct sockaddr_in     Sock_Addr;
extern struct listen_info     Listen_Info_IDPCD;
extern bool                   Continue_Processing;
extern bool                   Is_CGI_ID;

// =---------------------------------------------------------------------------
// Function Prototypes
//
// =---------------------------------------------------------------------------
_result Startup_Daemon           ( );
_result Shutdown_Daemon          ( void );
_result Setup_Listening_Socket   ( uint_16 port, listen_info& li );
void    Write_Server_Message     ( char* p_msg );
_result Open_Databases           ( const char* Data_Directory );
_result Close_Databases          ( void );
_result Open_Single_Database     ( db_metastructure* p_db, const char* path, const char* dbname );
_result Close_Single_Database    ( db_metastructure* p_db, const char* dbname );
_result Begin_Daemon_Thread      ( void );
_result Find_Client_By_Username  ( const char* username, 
                                   uint_32& found_recnum );

// Useful for daemon statistics
//
const char* Daemon_Start_DateTime     ( void );

// Health reporting functions
//
bool Report_Listening_Health   ( bool verbose, const listen_info& li );
bool Report_Server_Stats       ( bool verbose );
void Report_Server_Connections ( );

#if UNIX
void* IDPCD_Thread_Proc ( void* linfo );
#elif WIN32
void IDPCD_Thread_Proc ( void* linfo );
#endif

_result Process_Listening ( listen_info& li );
_result Process_Busy ( void );
_result Process_Heartbeat ( void );

#endif // __STARTUP_DAEMON_H__
