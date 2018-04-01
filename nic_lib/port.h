// =---------------------------------------------------------------------------
// p o r t . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Platform-independent socket helper declarations
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 June 9   nic   Created.
//   99 Aug  5   
//

#ifndef __PORT_H__
#define __PORT_H__

#include <sys/types.h>
#include <sys/stat.h>

#if UNIX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#elif WIN32
#include <winsock2.h>   // For some error codes and not necessarily for Winsock2 functions, which we avoided.
#endif

#include <stdio.h>

// =---------------------------------------------------------------------------
// Types
//
#define interface struct

#if UNIX
   typedef int SOCKET;
   typedef int HANDLE;
   
   #define INVALID_SOCKET  (SOCKET)(~0)
   #define SOCKET_ERROR            (-1)
   #define MAX_PATH                 255
//   #define MAP_FAILED              ((caddr_t)-1)
#elif WIN32
   #define MAP_FAILED              (0)
#endif

#if UNIX

#define MAXFD 64

// =---------------------------------------------------------------------------
// typedefs
//
// ----------------------------------------------------------------------------
   extern "C" {
typedef void Sigfunc ( int );
   }

Sigfunc* signal ( int signo, Sigfunc* func );
Sigfunc* Signal ( int signo, Sigfunc* func );

_result Unix_Daemon_Init     ( const char* pname );

#endif // UNIX


int Port_Receive_Buffer    ( int sock, void* buf, int len, int flags=0 );
int Port_Receive_Buffer_Ex ( int sock, void* buf, int len, int flags=0 );
int Port_Send_Buffer       ( int sock, void* buf, int len, int flags=0 );
int Port_Send_Buffer_Ex    ( int sock, void* buf, int len, int flags=0 );

//
// Process Functions
//
int  Port_Process_Id   ( void );
void Port_Sleep        ( uint_32 ms /*milliseconds*/ );

//
// File I/O
//
int   Port_File_Create ( const char* filename );
int   Port_File_Open   ( const char* filename );
int   Port_File_Open_Append ( const char* filename );
int   Port_File_Read   ( int file, uint_08* p_data, uint_32 size, uint_32& size_read );
int   Port_File_Write  ( int file, uint_08* p_data, uint_32 size );
int   Port_File_Close  ( int file );
void* Port_File_Map    ( int file, HANDLE& hMap, uint_32 byte_size );
int   Port_File_Unmap  ( void* p_data, HANDLE& hMap, uint_32 byte_size );
int   Port_File_Stat   ( int file, struct stat* p_stat );
int   Port_File_Seek   ( int file, long offset, int origin );
int   Port_File_Flush  ( int file );

// Cool String Manipulation
//
char* Port_Strcpy_Len ( char* dest, const char * src, uint_32 len );

//
// Socket I/O
//

int Port_Sock_Close    ( SOCKET& sock );
int Port_Sock_Blocking ( SOCKET sock, bool blocking );
int Port_Sock_Print_IP ( char* p_string, uint_32 ip );
int Port_Sock_Print_IP ( char* p_string, sockaddr_in& addr );

//
// Misc
//
const char* Port_Get_DateTime ( void );

#endif // __PORT_H__





