// =---------------------------------------------------------------------------
// c o m m o n . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Common types, constants, etc.
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
// 
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 May 19   nic   Created.
//

// We are NOT using precompiled headers for this file...

#include "PreComp.h"

extern MessageHelper Sys;

#include "../nic_lib/common.h"
#include "message.h"


// =---------------------------------------------------------------------------
// Constants
//
// =---------------------------------------------------------------------------
const char* EMPTYSTRING = "";

/*
const uint_16 CHANNEL_DEBUG  = MSGCHANNELS::CHANNEL_DEBUG; 
const uint_16 CHANNEL_DEBUG1 = MSGCHANNELS::CHANNEL_DEBUG1;
const uint_16 CHANNEL_DEBUG2 = MSGCHANNELS::CHANNEL_DEBUG2;
const uint_16 CHANNEL_DEBUG3 = MSGCHANNELS::CHANNEL_DEBUG3;
const uint_16 CHANNEL_DEBUG4 = MSGCHANNELS::CHANNEL_DEBUG4;
const uint_16 CHANNEL_DEBUG5 = MSGCHANNELS::CHANNEL_DEBUG5;
const uint_16 CHANNEL_DEBUG6 = MSGCHANNELS::CHANNEL_DEBUG6;
const uint_16 CHANNEL_DEBUG7 = MSGCHANNELS::CHANNEL_DEBUG7;
const uint_16 CHANNEL_DEBUG8 = MSGCHANNELS::CHANNEL_DEBUG8;
const uint_16 CHANNEL_DEBUG9 = MSGCHANNELS::CHANNEL_DEBUG9;
const uint_16 CHANNEL_NORMAL = MSGCHANNELS::CHANNEL_NORMAL;
*/

#if 0
// =---------------------------------------------------------------------------
// ( globals ) M e s s a g e
// 
// For UNIX: If we are running as a Daemon process, then we have no controlling 
// terminal, thus, we need to output messages to some kind of system log.
// If we are *not* a daemon, we just do a printf to the console
//
// For Win32: Write to the LogView window
// =---------------------------------------------------------------------------
void Message ( MSGCHANNELS channel, const char* format, ... )
{
   if ( p_Msg_Helper )
   {
      p_Msg_Helper->Message ( 
   }

   va_list v1;
   va_start ( v1, format );
   vsprintf ( Message_Buffer, (char*)format, v1 );
   va_end ( v1 );

#if UNIX
   if ( Run_As_Daemon )
   {
      //if ( Indent_Value > 0 && Message_With_Newline_Last )
      //   syslog ( LOG_DEBUG, Indent_String );

      //syslog ( LOG_DEBUG, Message_Buffer );
   }
   else
#endif
   {
      #if WIN32LOGVIEW

      if ( channel & Active_MsgChannel_Mask )
      {
         if ( Indent_Value > 0 && Message_With_Newline_Last )
            MessageView.Message ( channel, Indent_String );

         MessageView.Message ( channel, "%s\r\n", Message_Buffer );
      }

      #elif WIN32MESSAGEBOX

      if ( channel & Active_MsgChannel_Mask )
         MessageBox ( GetDesktopWindow(), Message_Buffer, "App Message", MB_OK );

      #else // for win32 and unix

      if ( channel & Active_MsgChannel_Mask )
      {
         if ( Indent_Value > 0 && Message_With_Newline_Last )
            printf ( Indent_String );

         printf ( "%s\n", Message_Buffer );
      }

      #endif
   }

   Message_With_Newline_Last = true;
}

// =---------------------------------------------------------------------------
// ( globals ) M e s s a g e _ N o N e w l i n e
// 
// =---------------------------------------------------------------------------
void Message_NoNewline ( MSGCHANNELS channel, const char* format, ... )
{
   lock_helper save_ferris ( Message_Locker );

   va_list v1;
   va_start ( v1, format );
   vsprintf ( Message_Buffer, (char*)format, v1 );
   va_end ( v1 );

#if UNIX
   if ( Run_As_Daemon )
   {
      //if ( Indent_Value > 0 && Message_With_Newline_Last )
      //   syslog ( LOG_DEBUG, Indent_String );

      //syslog ( LOG_DEBUG, Message_Buffer );
   }
   else
#endif
   {
      
      #if WIN32LOGVIEW

      if ( channel & Active_MsgChannel_Mask )
      {
         if ( Indent_Value > 0 && Message_With_Newline_Last )
            MessageView.Message ( channel, Indent_String );

         MessageView.Message ( channel, "%s", Message_Buffer );
      }

      #elif WIN32MESSAGEBOX

      if ( channel & Active_MsgChannel_Mask )
         MessageBox ( GetDesktopWindow(), Message_Buffer, "App Message", MB_OK );

      #else // For Win32 and UNIX

      if ( channel & Active_MsgChannel_Mask )
      {
         if ( Indent_Value > 0 && Message_With_Newline_Last )
            printf ( Indent_String );

         printf ( "%s", Message_Buffer );
      }

#endif
   }

   Message_With_Newline_Last = false;
}

// =---------------------------------------------------------------------------
// ( global ) M e s s a g e _ I n d e n t
//
// =---------------------------------------------------------------------------
int Message_Indent ( int Delta )
{
   if ( (Indent_Value + Delta) > MAX_INDENT )
      return Indent_Value;

   Indent_Value += Delta;

   for ( int i=0; i < Indent_Value; i++ )
   {
      Indent_String[i] = ' ';
   }

   if ( Indent_Value >= 0 )
      Indent_String[Indent_Value] = '\0';

   return Indent_Value;
}
#endif // #if 0

// =---------------------------------------------------------------------------
// ( global ) E r r o r _ S t r i n g
//
// =---------------------------------------------------------------------------
const char* Error_String ( int code )
{
   return strerror ( code );
}

// =---------------------------------------------------------------------------
// ( global ) S o c k e t _ E r r o r _ S t r i n g
//
// =---------------------------------------------------------------------------
const char* Socket_Error_String ( int errcode )
{
   switch ( errcode )
   {

#if UNIX

      default                       : return "Unknown socket error";;

#elif WIN32

      case  WSAEINTR                : return "WSAEINTR";
      case  WSAEBADF                : return "WSAEBADF";
      case  WSAEACCES               : return "WSAEACCES";
      case  WSAEFAULT               : return "WSAEFAULT";
      case  WSAEINVAL               : return "WSAEINVAL";
      case  WSAEMFILE               : return "WSAEMFILE";
      case  WSAEWOULDBLOCK          : return "WSAEWOULDBLOCK";
      case  WSAEINPROGRESS          : return "WSAEINPROGRESS";
      case  WSAEALREADY             : return "WSAEALREADY";
      case  WSAENOTSOCK             : return "WSAENOTSOCK";
      case  WSAEDESTADDRREQ         : return "WSAEDESTADDRREQ";
      case  WSAEMSGSIZE             : return "WSAEMSGSIZE";
      case  WSAEPROTOTYPE           : return "WSAEPROTOTYPE";
      case  WSAENOPROTOOPT          : return "WSAENOPROTOOPT";
      case  WSAEPROTONOSUPPORT      : return "WSAEPROTONOSUPPORT";
      case  WSAESOCKTNOSUPPORT      : return "WSAESOCKTNOSUPPORT";
      case  WSAEOPNOTSUPP           : return "WSAEOPNOTSUPP";
      case  WSAEPFNOSUPPORT         : return "WSAEPFNOSUPPORT";
      case  WSAEAFNOSUPPORT         : return "WSAEAFNOSUPPORT";
      case  WSAEADDRINUSE           : return "WSAEADDRINUSE";
      case  WSAEADDRNOTAVAIL        : return "WSAEADDRNOTAVAIL";
      case  WSAENETDOWN             : return "WSAENETDOWN";
      case  WSAENETUNREACH          : return "WSAENETUNREACH";
      case  WSAENETRESET            : return "WSAENETRESET";
      case  WSAECONNABORTED         : return "WSAECONNABORTED";
      case  WSAECONNRESET           : return "WSAECONNRESET";
      case  WSAENOBUFS              : return "WSAENOBUFS";
      case  WSAEISCONN              : return "WSAEISCONN";
      case  WSAENOTCONN             : return "WSAENOTCONN";
      case  WSAESHUTDOWN            : return "WSAESHUTDOWN";
      case  WSAETOOMANYREFS         : return "WSAETOOMANYREFS";
      case  WSAETIMEDOUT            : return "WSAETIMEDOUT";
      case  WSAECONNREFUSED         : return "WSAECONNREFUSED";
      case  WSAELOOP                : return "WSAELOOP";
      case  WSAENAMETOOLONG         : return "WSAENAMETOOLONG";
      case  WSAEHOSTDOWN            : return "WSAEHOSTDOWN";
      case  WSAEHOSTUNREACH         : return "WSAEHOSTUNREACH";
      case  WSAENOTEMPTY            : return "WSAENOTEMPTY";
      case  WSAEPROCLIM             : return "WSAEPROCLIM";
      case  WSAEUSERS               : return "WSAEUSERS";
      case  WSAEDQUOT               : return "WSAEDQUOT";
      case  WSAESTALE               : return "WSAESTALE";
      case  WSAEREMOTE              : return "WSAEREMOTE";
      case  WSASYSNOTREADY          : return "WSASYSNOTREADY";
      case  WSAVERNOTSUPPORTED      : return "WSAVERNOTSUPPORTED";
      case  WSANOTINITIALISED       : return "WSANOTINITIALISED";
      case  WSAEDISCON              : return "WSAEDISCON";
      case  WSAENOMORE              : return "WSAENOMORE";
      case  WSAECANCELLED           : return "WSAECANCELLED";
      case  WSAEINVALIDPROCTABLE    : return "WSAEINVALIDPROCTABLE";
      case  WSAEINVALIDPROVIDER     : return "WSAEINVALIDPROVIDER";
      case  WSAEPROVIDERFAILEDINIT  : return "WSAEPROVIDERFAILEDINIT";
      case  WSASYSCALLFAILURE       : return "WSASYSCALLFAILURE";
      case  WSASERVICE_NOT_FOUND    : return "WSASERVICE_NOT_FOUND";
      case  WSATYPE_NOT_FOUND       : return "WSATYPE_NOT_FOUND";
      case  WSA_E_NO_MORE           : return "WSA_E_NO_MORE";
      case  WSA_E_CANCELLED         : return "WSA_E_CANCELLED";
      case  WSAEREFUSED             : return "WSAEREFUSED";
      default                       : return "Unknown Winsock error";;

#endif

   }
}

// =---------------------------------------------------------------------------
// ( global ) Debug_Message
//
// If DEBUG is defined, Debug_Message maps to Message, otherwise it maps
//   to this empty implementation
// =---------------------------------------------------------------------------

#ifndef DEBUG
void _Debug_Message ( const char* format, ...  )
{
   // no implementation
}
#endif

#if WIN32
// =---------------------------------------------------------------------------
// ( global, Win32 only ) Report_GetLastError
//
// If DEBUG is defined, Debug_Message maps to Message, otherwise it maps
//   to this empty implementation
// =---------------------------------------------------------------------------
void Report_Win32LastError ( const char* Prefix, DWORD dwError )
{
   LPVOID lpMsgBuf;
   FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dwError,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
      0,
      NULL );

   // Display the string.
   Sys.Message( CHANNEL_DEBUG, "%s : Code<%d>, Text<%s>", Prefix, (int)dwError, lpMsgBuf );

   // Free the buffer.
   LocalFree( lpMsgBuf );
}
#endif
