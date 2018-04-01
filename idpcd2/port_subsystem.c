// =---------------------------------------------------------------------------
// p o r t  _ s u b s y s t e m . h
// 
//   (C) 1999-2001 - Martin R. Szinger, Nicholas Codignotto.
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
//   DATE         WHO   REVISION
//   -----------  ---   --------------------------------------------------------
//   2000 Jan 20  nic   Created.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "port_subsystem.h"

#include <time.h>

#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


#if UNIX

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <signal.h>
#endif // UNIX

// =---------------------------------------------------------------------------
// Local/Externl Definitions
//
// =---------------------------------------------------------------------------
Month_Struct Months[12] = { {"Jan", "January"},
                            {"Feb", "February"},
                            {"Mar", "March"},
                            {"Apr", "April"},
                            {"May", "May"},
                            {"Jun", "June"},
                            {"Jul", "July"},
                            {"Aug", "August"},
                            {"Sep", "September"},
                            {"Oct", "October"},
                            {"Nov", "November"},
                            {"Dec", "December"} };

// =---------------------------------------------------------------------------
// S S _ P o r t _ S t r c p y _ L e n 
//
// Useful for copying fixed-length strings that comes with padded nulls to end
//
char* SS_Port_Strcpy_Len ( char* dest, const char * const src, uint_32 dest_size )
{
   char*   res = 0;
   uint_32 copy_len = strlen(src);
   
   if ( copy_len > dest_size-1 )
      copy_len = dest_size-1;

   memcpy ( dest, src, copy_len );
	dest[copy_len] = '\0';

   return res;
}

// =---------------------------------------------------------------------------
// Combines strcat with sprintf
// =---------------------------------------------------------------------------
char* SS_Port_Strcat ( char* dest, const char* format, ... )
{
   int len = strlen(dest);

   va_list v1;
   va_start ( v1, format );
   vsprintf ( &dest[len], (char*)format, v1 );
   va_end ( v1 );

   return dest;
}

// =---------------------------------------------------------------------------
// (global) S S _ P o r t _ P r o c e s s _ I d
//
int SS_Port_Process_Id ( void )
{
#if UNIX

   return getpid();

#elif WIN32

   // Not very meaningful in Win32, since the handle is only valid in 
   // the context of this process, usually -1. DuplicateHandle is needed to
   // get at a process id that is valid in the context of other processes.
   return (int)GetCurrentProcess();

#endif
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ S l e e p
//
void SS_Port_Sleep ( uint_32 ms /*milliseconds*/ )
{
#if UNIX
   usleep ( ms * 1000 /*usleep is in microseconds!*/ );
#elif WIN32
   Sleep ( ms );
#endif
}


// =---------------------------------------------------------------------------
// P o r t _ G e t _ D a t e T i m e _ S t r i n g
// buf needs to be at leats 32 bytes long
//
//           1         2         3
// 0123456789012345678901234567890
// Tue May 02 09:59:08 2000 GMT
// =---------------------------------------------------------------------------
void SS_Port_Get_DateTime_String ( char* buf )
{
   char*       out = 0;
   struct tm*  gmt = 0;
   time_t      ltime;

#if WIN32
   _tzset();
#else
   tzset();
#endif

   time( &ltime );
    
   gmt = gmtime( &ltime );

   out = asctime( gmt );

   out[24] = 0;

   sprintf( buf, "%s GMT", out );
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ D a t e T i m e _ T o _ D a y M o n t h Y e a r 
//           1         2
// 012345678901234567890123456789
// Wed May 10 09:58:44 2000 GMT 
// =---------------------------------------------------------------------------
void SS_Port_DateTime_To_DayMonThYear ( const char* DateTime, int* p_Day, int* p_Month, int* p_Year )
{
   char Day   [ 3 ];
   char Month [ 4 ]; // 3-letter month, May, Jun, Jul
   char Year  [ 5 ]; // Y2k compliant!

   SS_Port_Strcpy_Len ( Day,   &DateTime[ 8], 3 /*includes null*/ );
   SS_Port_Strcpy_Len ( Month, &DateTime[ 4], 4 /*includes null*/ );
   SS_Port_Strcpy_Len ( Year,  &DateTime[20], 5 /*includes null*/ );

   *p_Day   = atoi(Day);
   *p_Month = SS_Port_Month_To_Number ( Month );
   *p_Year  = atoi(Year);
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ D a t e T i m e _ T o _ S h o r t _ D a t e
// =---------------------------------------------------------------------------
void  SS_Port_DateTime_To_Short_Date ( char* Short_Buf, const char* DateTime )
{
   int Day_I, Month_I, Year_I;

   // We might have a blank input string
   //
   if ( strlen(DateTime) == 0 ) { Short_Buf[0] = 0; return; }

   // We might have a short-date input string
   //
   if ( strlen(DateTime) < 20 ) { strcpy(Short_Buf, DateTime ); return; }

   // Regular processing...
   //
   SS_Port_DateTime_To_DayMonThYear ( DateTime, &Day_I, &Month_I, &Year_I );

   if ( Day_I   < 1 || Day_I   > 31 ) { Short_Buf[0] = 0; return; }
   if ( Month_I < 1 || Month_I > 12 ) { Short_Buf[0] = 0; return; }

   sprintf ( Short_Buf, "%d/%d/%d", Month_I, Day_I, Year_I );
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ M o n t h _ T o _ N u m b e r
//
// Returns: 1-12 : Month number
//          0    : error!
//
// =---------------------------------------------------------------------------
int SS_Port_Month_To_Number ( const char* Month )
{
   int i;
   for ( i=0; i < 12; i++ )
   {
      if ( strncmp(Months[i].Short_Month, Month, 3 ) == 0 )
         return i+1;
   }

   return 1 /*Jan default*/;
}


// =---------------------------------------------------------------------------
// S S _ P o r t _ A l l o c M e m 
// =---------------------------------------------------------------------------
void* SS_Port_AllocMem ( int size )
{
   return malloc ( (size_t)size );
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ F r e e M e m
//
// =---------------------------------------------------------------------------
void SS_Port_FreeMem ( void* block )
{
   free ( block );
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ Z e r o M e m o r y
// =---------------------------------------------------------------------------
void SS_Port_ZeroMemory ( void* block, uint_32 size )
{
   memset ( block, 0, size );
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ M e r g e _ S t r c p y _ L e n
// =---------------------------------------------------------------------------
void SS_Port_Merge_Strcpy_Len ( int merge, char* dest, const char* src, int Len )
{
   if ( !merge || strlen(src) )
      SS_Port_Strcpy_Len ( dest, src, Len );
}

// =---------------------------------------------------------------------------
// S S _ P o r t _ M e r g e _ A s c i i _ T  o_ B y t e
// =---------------------------------------------------------------------------
void SS_Port_Merge_Ascii_To_Byte ( int merge, uint_08* p_dest_byte, const char* src )
{
   if ( !merge || strlen(src) )
      *p_dest_byte = (uint_08)atoi(src);
}

#if UNIX

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
int Unix_Daemon_Init ( const char* pname )
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
   
   return 0;
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
#endif
