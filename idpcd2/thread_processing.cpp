// =---------------------------------------------------------------------------
// p r o c e s s i n g . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Processing Thread main loop. startup_daemon.h contains most of our useful 
//    declarations
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

// =---------------------------------------------------------------------------
// Include our standard headers
//
// =---------------------------------------------------------------------------

#include "PreComp.h"

#include "idpcd_defaults.h"
#include "startup_daemon.h"
#include "connection_pool.h"
#include "idpcd_defaults.h"
#include "idpcd_group.h"
#include "heartbeat.h"

timecode Last_Heartbeat_Check_Time;

// =---------------------------------------------------------------------------
// (global) P r o c e s s i n g _ T h r e a d _ P r o c
//
// =---------------------------------------------------------------------------
_result Process_Busy ( void )
{
   _result res;
   int     ret;
   int     max_desc;
   fd_set  fd_reads;
   fd_set  fd_writes;

   res = 
      Busy_Pool.Initialize_Multiplex_Descriptors ( 
         fd_reads, fd_writes, max_desc );

   // If no connections need transer (usually no connections) then repeat loop
   //
   if ( max_desc == 0 ) return RS_OK;

   // specify a 1 second timeout, we do this because the listening thread
   // may have added or removed a connection while we sit here waiting for
   // socket activity
   //
   timeval tv;
   tv.tv_sec  = 1;
   tv.tv_usec = 0;

   ret = select ( max_desc+1, &fd_reads, &fd_writes, 0, &tv );

   if ( ret == -1 )
   {
      #if UNIX
      int err = errno;
      #elif WIN32
      int err = WSAGetLastError ( );
      #endif

      Sys.Message ( CHANNEL_DEBUG, "Process_Busy<select> : %s", Socket_Error_String ( err ) );

      return RS_OK;
   }

   // Even though select may have returned no ready descriptors, we might
   // have pending auxilliary transfers, such as reverse connect queries pending
   // these are handled in Process_Pending_Transfers
   //
   res = Busy_Pool.Process_Pending_Transfers ( fd_reads, fd_writes );
   RRETURNONFAILURE(res);

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (global) P r o c e s s _ H e a r t b e a t
//
// =---------------------------------------------------------------------------
_result Process_Heartbeat ( void )
{
   timecode Current_Time;
   timecode Expired_Time;

   // Calculate the scheduled expired time (last + expire)
   Expired_Time = Last_Heartbeat_Check_Time;
   Expired_Time.Advance_Time ( HEARTBEAT_CHECK_EXPIRATION_SECONDS );

   if ( Current_Time.Is_Expired ( Expired_Time ) )
   {
      Last_Heartbeat_Check_Time = Current_Time;
      return Busy_Pool.Process_Heartbeat ( );
   }

   return RS_OK;
}
