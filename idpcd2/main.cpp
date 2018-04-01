// =---------------------------------------------------------------------------
// m a i n . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Main Iron Dragon Server Daemon source file
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//    - Routines for many UNIX-specific functions are available in 
//      the POSIX folder found in the MSVC folder tree.
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 May 19   nic   Created.
//

// =---------------------------------------------------------------------------
// Include our standard headers
//
// =---------------------------------------------------------------------------

#include "PreComp.h"

#include <stdio.h>
#include "idpcd_defaults.h"
#include "startup_daemon.h"
#include "command_line.h"
#include "idpcd_defaults.h"
#include "version.h"

File_Sink Logfile_Sink;
MessageHelper Sys ( Logfile_Sink );

#if UNIX
extern bool Quiet_Mode;
#endif

// =---------------------------------------------------------------------------
// Forward Declarations
//
// =---------------------------------------------------------------------------
void Display_Usage      ( void );
bool Parse_Command_Line ( int argc, char** argv );

// =---------------------------------------------------------------------------
// (global) m a i n
//
// =---------------------------------------------------------------------------

int main( int argc, char** argv )
{
   // if we fail, user gave us invalid arguments
   //
   if ( !Parse_Command_Line ( argc, argv ) ) 
      return 0;

   #if UNIX
   if ( !Quiet_Mode )
   #endif
      printf ( "idpcd daemon version %s\n\n", IDPCD_FILE_VERSION );

   // if a config file was specified on command line, parse it
   //
   if ( !Parse_Configuration ( Config_Filename ) )
      printf ( "Failed to find config options in file: %s, using default options.\n", Config_Filename );

   if ( Run_As_Daemon )
   {

      #if UNIX
      if ( !Quiet_Mode )
      #endif
         printf ( "Starting as daemon process with pre-fork pid of %d\n", SS_Port_Process_Id() );

      if ( RFAILED((_result)Unix_Daemon_Init("idpcd")) )
      {
         #if UNIX
         if ( !Quiet_Mode )
         #endif
            printf ( "Could not start as a daemon. Aborting.\n" );
         return -1;
      }
   }
   else 
   {
      #if UNIX
      if ( !Quiet_Mode )
      #endif
         printf ( "Starting as shell process with pid %d\n", SS_Port_Process_Id() );
   }

   //
   // If we are a daemon, no printf() will be seen at the terminal from here on out...
   //

   // Start the daemon
   //
   if ( RFAILED(Startup_Daemon( ) ) )
   {
      Logfile_Sink.Close ( );
   }
   else
   {
      Logfile_Sink.Close ( );
   }

   return 0;
}











