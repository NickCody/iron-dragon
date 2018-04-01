// =---------------------------------------------------------------------------
// m e s s a g e_ s u b s y s t e m . c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements a debug messaging subsystem
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE         WHO   REVISION
//   -----------  ---   --------------------------------------------------------
//   2000 Jan 20  nic   Created.
//


#include "cgi_id_subsystems.h"

void SS_Message_Write ( MSGCHANNELS Channel, const char* format, ... );
