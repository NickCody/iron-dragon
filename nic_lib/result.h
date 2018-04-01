// =---------------------------------------------------------------------------
// r e s u l t . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the group class for maintaining groups of client
//    connections
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Sep 5   nic   Created.
//

#ifndef __RESULT_H__
#define __RESULT_H__

// =-----------------------------------
// Enumerations
//
enum _result
{  
   RS_OK                 = 0,     // Ok
   
   RS_ERR                = -1,    // Unspecified Error
   RS_EXCEPTION          = -2,    // Caught exception, reason unknown
   
   RS_NOTIMPL            = -10,  // Not implemented
   RS_NOCREATEWINDOW     = -11,  // Window could not be created
   RS_BADINIT            = -12,  // Bad initialization
   RS_BADFILE            = -13,  // Bad file specification, could not open file
   RS_BADMAP             = -14,  // Could not map file into memory
   RS_BADINDEX           = -15,  // Bad index passed into array
   RS_NOMEMORY           = -16,  // No more memory
   RS_BADSIGNAL          = -17,  // Could not install signal handler
   RS_BADCLIENTCOMM      = -18,  // Client failed to communicate properly
   RS_NODATA             = -19,  // No Data
   RS_NOTOPEN            = -20,  // Database/file/socket is not open
   RS_ALREADYOPEN        = -21,  // Database/file/socket is already open
   RS_POOLFULL           = -22,  // The Pool (Connection/Group) is Full

   RS_BADSOCKET          = -50,  // Bad socket handle as argument
   RS_BADSOCKETCOMM      = -51,  // Bad socket communication
   RS_BADWINSOCK         = -52,  // Winsock is bad, not initialized
   RS_BADADDRESS         = -53,  // The supplied address is bad

   RS_ITEMINVECTOR       = -100, // Item was already in vector

   RS_PLACEHOLDER        = -1000
} ;

// =---------------------------------------------------------------------------
// Macros & Helpers
//
bool RSUCCEEDED ( _result r );
bool RFAILED ( _result r );
bool NONZERO ( int c );
const char* Result_String ( _result rs );

#define RRETURNONFAILURE(res) if ( RFAILED(res) ) return res


#endif // __RESULT_H__

