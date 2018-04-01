// =---------------------------------------------------------------------------
// h e a r t b e a t . h
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Oct 4    nic   Created.
//

#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <time.h>

enum HEARTBEAT_STATE
{
   HB_PRIMAL      = 0,
   HB_WAITING     = 1,
   HB_RECEIVED    = 2,
   HB_RECEIVEDANY = 3
};

// =---------------------------------------------------------------------------
// (global) timecode
//
// =---------------------------------------------------------------------------
class timecode
{
public:
   timecode()
   {
      time ( &Time_Start );
   }

   ~timecode()
   {
   }

   bool Is_Expired ( timecode& Time_End )
   {
      return Time_Start > Time_End.Time_Start;

   }

   void Advance_Time ( long Seconds )
   {
      Time_Start += Seconds;
   }

protected:
   time_t Time_Start;

};

#endif // __HEARTBEAT_H__
