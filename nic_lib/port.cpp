// =---------------------------------------------------------------------------
// s o c k e t _ p o r t _ u n i x . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    
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
//

#include "PreComp.h"

#include <stdio.h>
#include <time.h>
#include "port.h"
#include "common.h"
#include "message.h"

extern MessageHelper Sys;

char gmt_date[32];

#if UNIX

#include "standard_types.h"

#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

extern "C" void Signal_Child ( int x );

// =---------------------------------------------------------------------------
// (global) D a e m o n _ I n i t
//
// Compliments W. Richard Stevens, "UNIX Network Programming"
// ( We are not designed to be a daemon as launched by inetd
//   we make ourselves a daemon by fork()ing )
//
// TODO: Why do these signal handlers failto install?!
//
// =---------------------------------------------------------------------------
_result Unix_Daemon_Init ( const char* pname )
{
   pid_t pid;

   // Install signal handler for dead children processing
   //
   //if ( !Signal ( SIGCHLD, Signal_Child ) )
   //   return RS_BADSIGNAL;   

   if ( (pid=fork()) != 0 )
       exit(0);
   
   setsid();

   //if ( !Signal ( SIGHUP, SIG_IGN ) )
   //   return RS_BADSIGNAL;

   if ( (pid = fork()) != 0 )
       exit(0);
   
   //chdir ( "/" );

   umask ( 0 );
   
   for ( int i=0; i < MAXFD; i++ )
      close(i);
   
   return RS_OK;
}

// =---------------------------------------------------------------------------
// (global) S i g n a l
//
// returns the signal handler if successful, 0 otherwise
//
// Compliments W. Richard Stevens, "UNIX Network Programming"
// =---------------------------------------------------------------------------

Sigfunc* Signal ( int signo, Sigfunc* func )
{
   Sigfunc* sigfunc;
   
   if ( (sigfunc = signal(signo, func)) == SIG_ERR)
      return 0;
   
   return ( sigfunc );
}

// =---------------------------------------------------------------------------
// (global) s i g n a l
// 
// Compliments W. Richard Stevens, "UNIX Network Programming"
// =---------------------------------------------------------------------------

Sigfunc* signal ( int signo, Sigfunc* func )
{
   struct sigaction act, oact;
   
   act.sa_handler = func;
   sigemptyset ( &act.sa_mask );
   act.sa_flags = 0;
   
   if ( signo == SIGALRM )
   {
      #ifdef SA_INTERRUPT
      act.sa_flags |= SA_INTERRUPT;        // SunOS 4.x
      #endif
   }
   else
   {
      #ifdef SA_RESTART
      act.sa_flags |= SA_RESTART;          // SVR4, 4.4BSD
      #endif
   }
   
   if ( sigaction(signo, &act, &oact) < 0 )
      return ( SIG_ERR );
   
   return oact.sa_handler;
}

// =---------------------------------------------------------------------------
// (global signal handler) S i g n a l _ C h i l d
// =---------------------------------------------------------------------------
void Signal_Child ( int x ) 
{
   while ( 0 != waitpid( 0, 0, WNOHANG));
}


#endif // UNIX

// =---------------------------------------------------------------------------
// (global) P o r t _R e c e i v e _ B u f f e r
//
// This version doesn't loop until all data specified has been received
// See Port_Receive_Buffer_Ex for that
//
int Port_Receive_Buffer ( int sock, void* buf, int len, int flags )
{
   int ret = recv ( sock, (char*)buf, len, flags );

   if ( ret == SOCKET_ERROR )
   {
#if UNIX
   // TODO: need to handle zero reads / reset connections like we do in Win32
      return 0;
#elif WIN32
      int last_error = WSAGetLastError ( );
      switch ( last_error )
      {
         case WSAEWOULDBLOCK:

         case WSAESHUTDOWN:
         case WSAECONNRESET:     // server disconnected us?
            return 0;
      }
#endif
   }

   return ret;
}

// =---------------------------------------------------------------------------
// (global) P o r t _R e c e i v e _ B u f f e r _ E x
//
int Port_Receive_Buffer_Ex ( int sock, void* buf, int len, int flags )
{
   int ret_bytes = 0;
   while ( ret_bytes < len )
   {
      // NickC 8/27/99 : Changed len to len-ret_bytes to avoid overrunm
      int ret = recv ( sock, ((char*)buf) + ret_bytes, len-ret_bytes, flags );

      if ( ret == SOCKET_ERROR )
      {
#if UNIX
         // TODO: need to handle zero reads / reset connections like we do in Win32
         return 0;
#elif WIN32
         int last_error = WSAGetLastError ( );
         switch ( last_error )
         {
            case WSAESHUTDOWN:
            case WSAECONNRESET:     // server disconnected us?
               return 0;
         }
#endif
         return -1;
      }

      if ( ret == 0 )
         break;

      ret_bytes += ret;
   }

   return ret_bytes;
}

// =---------------------------------------------------------------------------
// (global) P o r t _ S e n d _ B u f f e r
//
// This version doe snot attempt to keep looping until send() sent all 
// requested bytes. For that see Port_Send_Buffer_Ex

int Port_Send_Buffer ( int sock, void* buf, int len, int flags )
{
   int ret = send ( sock, (char*)buf, len, flags );

   if ( ret == SOCKET_ERROR )
   {
#if UNIX
      // TODO: #error We need to implement UNIX error code for send()
      return 0;
#elif WIN32
      int last_error = WSAGetLastError ( );
      switch ( last_error )
      {
         case WSAESHUTDOWN:
         case WSAECONNRESET:     // server disconnected us?
            return 0;
      }
#endif
   }

   return ret;
}

// =---------------------------------------------------------------------------
// (global) P o r t _ S e n d _ B u f f e r _ E x
//
int Port_Send_Buffer_Ex ( int sock, void* buf, int len, int flags )
{
   int ret_bytes = 0;
   while ( ret_bytes < len )
   {
      int ret = send ( sock, ((char*)buf) + ret_bytes, len-ret_bytes, flags );

      if ( ret == SOCKET_ERROR )
      {
#ifdef WIN32
         int last_error = WSAGetLastError ( );
         switch ( last_error )
         {
            case WSAESHUTDOWN:
            case WSAECONNRESET:     // server disconnected us?
               return 0;
         }
#endif
         return -1;
      }

      // TODO: Makes sense for read, but I don't have much confidence in
      // blindly exiting here if send returns 0
      // perhaps sleep for a time and try again, or try 10 times?
      if ( ret == 0 )
         break;

      ret_bytes += ret;
   }

   return ret_bytes;
}

// =---------------------------------------------------------------------------
// (global) P o r t _ P r o c e s s _ I d
//
int Port_Process_Id ( void )
{
#if UNIX

   return getpid();

#elif WIN32

   // TODO: Not very meaningful in Win32, since the handle is only valid in 
   // the context of this process, usually -1. DuplicateHandle is needed to
   // get at a process id that is valid in the context of other processes.
   return (int)GetCurrentProcess();

#endif
}

// =---------------------------------------------------------------------------
// P o r t _ S l e e p
//
void Port_Sleep ( uint_32 ms /*milliseconds*/ )
{
#if UNIX
   // TODO: Do we have a Sleep that sleeps in milliseconds?
   usleep ( ms * 1000 );
#elif WIN32
   Sleep ( ms );
#endif
}

// =---------------------------------------------------------------------------
//
// P o r t _ F i l e _ C r e a t e
//

int Port_File_Create ( const char* filename )
{
#if UNIX

   return open ( filename, O_CREAT | O_EXCL | O_RDWR, 0666 );

#elif WIN32
   HANDLE hFile = CreateFile( filename, GENERIC_WRITE | GENERIC_READ,
                       0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY,
                       NULL );

   if ( hFile == INVALID_HANDLE_VALUE )
      return -1;
   else
      return (int)hFile;

#endif
}

// =---------------------------------------------------------------------------
//
// P o r t _ F i l e _ O p e n
//
int Port_File_Open ( const char* filename )
{
#if UNIX
   return open ( filename, O_RDWR, 0666 );
#elif WIN32
   HANDLE hFile = CreateFile ( filename, GENERIC_READ | GENERIC_WRITE,
      0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

   if ( hFile == INVALID_HANDLE_VALUE )
      return -1;
   else
      return (int)hFile;
#endif
}

// =---------------------------------------------------------------------------
//
// P o r t _ F i l e _ O p e n _ A p p e n d 
//
int Port_File_Open_Append ( const char* filename )
{
#if UNIX
   return open ( filename, O_RDWR | O_APPEND, 0666 );
#elif WIN32
   HANDLE hFile = CreateFile ( filename, GENERIC_READ | GENERIC_WRITE,
      0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

   Port_File_Seek ( (int)hFile, 0, FILE_END );

   if ( hFile == INVALID_HANDLE_VALUE )
      return -1;
   else
      return (int)hFile;
#endif
}

// =---------------------------------------------------------------------------
//
// P o r t _ F i l e _ R e a d
//

int Port_File_Read ( int file, uint_08* p_data, uint_32 size, uint_32& size_read )
{
#if UNIX
   size_read = read ( file, (void*)p_data, size );
#else
   // TODO: Is it safe to ignore WriteFile bool return and just use dwWrite?
   ReadFile ( (HANDLE)file, p_data, size, &size_read, 0 );
#endif

   return size_read;
}

// =---------------------------------------------------------------------------
//
// P o r t _ F i l e _ W r i t e
//

int Port_File_Write ( int file, uint_08* p_data, uint_32 size )
{
#if UNIX
   return write ( file, (void*)p_data, size );
#else
   // TODO: Is it safe to ignore WriteFile bool return and just use dwWrite?
   DWORD dwWrite;
   WriteFile ( (HANDLE)file, p_data, size, &dwWrite, 0 );
   return dwWrite;
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ F i l e _ C l o s e
//
int Port_File_Close ( int file )
{
#if UNIX
   return close ( file );
#elif WIN32
   return CloseHandle ( (HANDLE)file );
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ F i l e _ S t a t
//
int Port_File_Stat ( int file, struct stat* p_stat )
{
#if UNIX
   return fstat ( file, p_stat );
#elif WIN32
   DWORD file_size = GetFileSize ( (HANDLE)file, 0 );
   memset ( p_stat, 0, sizeof(struct stat) );
   p_stat->st_size = file_size;
   return 0;
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ F i l e _ M a p
//
void* Port_File_Map ( int file, HANDLE& hMap, uint_32 byte_size )
{
#if UNIX
   return mmap ( 0, byte_size, PROT_READ | PROT_WRITE,
               MAP_SHARED, file, 0 );
#elif WIN32
   hMap = CreateFileMapping ( (HANDLE)file, 0, PAGE_READWRITE, 0, 0, 0 );
   if ( hMap == INVALID_HANDLE_VALUE )
   {
      return MAP_FAILED; // idpc_common.h
   }
   else
   {
      return MapViewOfFile ( hMap, FILE_MAP_WRITE, 0, 0, byte_size );
   }
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ F i l e _ U n m a p
//
// TODO: Fix return values
//
int Port_File_Unmap ( void* p_data, HANDLE& hMap, uint_32 byte_size )
{
#if UNIX
   return munmap ( (caddr_t)p_data, byte_size );
#elif WIN32
   UnmapViewOfFile ( p_data );
   CloseHandle ( hMap );
   hMap = 0;
   return 0;
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ F i l e _ S e e k 
//
// TODO: Fix return values
//
int Port_File_Seek ( int file, long offset, int origin )
{
#if UNIX
   return lseek ( file, offset, origin );
#elif WIN32
   return SetFilePointer ( (HANDLE)file, offset, 0, origin );
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ F i l e _ F l u s h
//
int Port_File_Flush ( int file )
{
#if UNIX
   return 0; //return fflush ( (FILE*)file );
#elif WIN32
   return 0;
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ S t r c p y _ L e n
//
// Useful for copying fixed-length strings that comes with padded nulls to end
//
char* Port_Strcpy_Len ( char* dest, const char * const src, uint_32 dest_size )
{
   char* res = 0;

   try
   {
	   // TODO: Make sure this is correct here!
	   dest[dest_size-1] = '\0';
      //memset ( dest, 0, dest_size );
	   res = strncpy ( (char*)dest, (const char*)src, dest_size );
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_DEBUG, "Exception<Port_Strcpy_Len>" );
   }

   return res;
}

// =---------------------------------------------------------------------------
// P o r t _ S o c k _ C l o s e
//
// TODO: Fix return values
//
int Port_Sock_Close ( SOCKET& sock )
{
   if ( sock ) shutdown ( sock, 2 /*SD_BOTH*/);

   int ret = 0;

#if UNIX
   if ( sock ) ret = close( sock );
#elif WIN32
   ret = closesocket ( sock );
#endif

   sock = 0;

   return ret;
}

// =---------------------------------------------------------------------------
// P o r t _ S o c k _ B l o c k i n g
//
int Port_Sock_Blocking ( SOCKET sock, bool blocking )
{
   uint_32 uval = blocking ? 1 : 0;
#if UNIX
   return ioctl ( sock, FIONBIO, &uval );
#elif WIN32
   return ioctlsocket ( sock, FIONBIO, &uval );
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ S o c k _ P r i n t _ I P
//
// ip address is assumed to be in network-byte order
// =---------------------------------------------------------------------------
int Port_Sock_Print_IP ( char* p_string, uint_32 ip )
{
   return sprintf ( p_string, "%d.%d.%d.%d",
      //(ip >> 24), 
      //(ip & 0x00FFFFFF) >> 16,
      //(ip & 0x0000FFFF) >> 8,
      //(ip & 0x000000FF) );

      (int) ((ip & 0x000000FF)       ),
      (int) ((ip & 0x0000FFFF) >> 8  ),
      (int) ((ip & 0x00FFFFFF) >> 16 ),
      (int) ((ip >> 24)              ) );
}

int Port_Sock_Print_IP ( char* p_string, sockaddr_in& addr )
{
   #if UNIX
      Port_Sock_Print_IP ( p_string, addr.sin_addr.s_addr );
   #elif WIN32
      Port_Sock_Print_IP ( p_string, addr.sin_addr.S_un.S_addr );
   #endif

   return 0;
}

// =---------------------------------------------------------------------------
// P o r t _ G e t _ D a t e T i m e
//
// =---------------------------------------------------------------------------
const char* Port_Get_DateTime ( void )
{
   struct tm *gmt = 0;

#if WIN32
   _tzset();
#else
   tzset();
#endif

   time_t ltime;
   time( &ltime );
    
   gmt = gmtime( &ltime );

   char* out = asctime( gmt );

   try
   {
      out[24] = 0;
   }
   catch ( ... )
   {
      strcpy ( gmt_date, "date error" );
   }

   sprintf( gmt_date, "%s GMT", out );

   return gmt_date;
}

