// =---------------------------------------------------------------------------
// c o m m o n . h
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
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 May 19   nic   Created.
//

#ifndef __COMMON_H__
#define __COMMON_H__

#include "result.h"

// =---------------------------------------------------------------------------
// External variables
//
extern const char* EMPTYSTRING;        // defined in common.cpp
extern const char* Request_Strings[];  // defined in common.cpp

// =---------------------------------------------------------------------------
// Constants

#define REQ_START               0x00001000
#define REQ_NONE                (REQ_START +  0)
#define REQ_CLOSE               (REQ_START +  1)
#define REQ_CLIENT_LIST         (REQ_START +  2)
#define REQ_GROUP_LIST          (REQ_START +  3)
#define REQ_IDENTIFYING_INFO    (REQ_START +  4)
#define REQ_USERNAME            (REQ_START +  5)
#define REQ_PASSWORD            (REQ_START +  6)
#define REQ_LAST                (REQ_START +  7)

#define MESSAGE_SIZE             (1024*32-1)

// =---------------------------------------------------------------------------
// Message and Debug_Message - _Debug_Message is an empty function, does nada
//



//#ifdef DEBUG
//#define Debug_Message            Message
//#define Debug_Message_NoNewline  Message_NoNewline
//#else
//#define Debug_Message           _Debug_Message
//#define Debug_Message_NoNewline _Debug_Message
//#endif

//uint_32 Set_Active_MsgChannels   ( uint_32 new_channel_mask );
//uint_32 Get_Active_MsgChannels   ( void );

//void    Message                  ( MSGCHANNELS channel, const char* format, ... );
//void    Message_NoNewline        ( MSGCHANNELS channel, const char* format, ... );
//void  _Debug_Message           ( const char* format, ... );
//int     Message_Indent           ( int Delta );

//extern bool Message_With_Newline_Last;

// =---------------------------------------------------------------------------
// Various system error string functions, useful for debugging using Message()
//
const char* Error_String ( int code );
const char* Result_String ( _result rs );
const char* Request_String ( int req );
const char* Socket_Error_String ( int errcode );

#if WIN32
void Report_Win32LastError ( const char* Prefix, DWORD dwError );
#endif

// =---------------------------------------------------------------------------
// Win32-specific - POSIX functions we find useful in Windows
//
#if WIN32 || CYGNUS
extern "C" { int getopt( int argc, char *argv[], char *opstring ); }
extern "C" char *optarg;
extern "C" int optind;
#endif

#endif // __COMMON_H__










