// =---------------------------------------------------------------------------
// r e s u l t . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
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
//   99 Aug 10   nic   Created.
//


#include "PreComp.h"
#include "result.h"

// =---------------------------------------------------------------------------
// ( global ) RSUCCEEDED, RFAILED, NONZERO
//
// To test _result values
//
// =---------------------------------------------------------------------------
bool RSUCCEEDED ( _result r )
{
   return r >= 0;
}

bool RFAILED ( _result r )
{
   return r < 0;
}

bool NONZERO ( int c )
{
   return c != 0;
}

// =---------------------------------------------------------------------------
// ( global ) R e s u l t _ S t r i n g
//
// =---------------------------------------------------------------------------
const char* Result_String ( _result rs )
{
   switch ( rs )
   {
      case RS_OK:             return  "Status OK";
      case RS_EXCEPTION:      return  "Caught exception, reason unknown";
      case RS_ERR:            return  "Unspecified Error";
      case RS_NOTIMPL:        return  "Not implemented";
      case RS_NOCREATEWINDOW: return  "Window could not be created";
      case RS_BADINIT:        return  "Bad initialization";
      case RS_BADSOCKET:      return  "Bad socket handle as argument";
      case RS_BADSOCKETCOMM:  return  "Bad socket communication";
      case RS_BADFILE:        return  "Bad file specification, could not open file";
      case RS_BADMAP:         return  "Could not map file into memory";
      case RS_BADINDEX:       return  "Bad index passed into array";
      case RS_NOMEMORY:       return  "No more memory";
      case RS_NOTOPEN:        return  "Database/file/socket is not open";
      case RS_ALREADYOPEN:    return  "Database/file/socket is already open";
      case RS_POOLFULL:       return  "The Pool (Connection/Group) is Full";

      case RS_BADSIGNAL:      return  "Could not install signal handler";
      case RS_BADCLIENTCOMM:  return  "Client failed to communicate properly";
      case RS_BADWINSOCK:     return  "Winsock is bad, not initialized";
      case RS_BADADDRESS:     return  "The supplied address is bad";

      case RS_ITEMINVECTOR:   return  "Item was already in vector";

      case RS_NODATA:         return  "No Data";

      default:
         return "Status code is unrecognized";
   }
}

