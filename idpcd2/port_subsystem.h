// =---------------------------------------------------------------------------
// p o r t _ s u b s y s t e m . h
// 
//   (C) 1999-2001 - Martin R. Szinger, Nicholas Codignotto.
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
//

#ifndef __PORT_SUBSYSTEM_H__
#define __PORT_SUBSYSTEM_H__

#include "standard_types.h"

#ifdef __cplusplus
extern "C" 
{
#endif
int   SS_Port_Process_Id             ( void );
void  SS_Port_Sleep                  ( uint_32 ms /*milliseconds*/ );
char* SS_Port_Strcpy_Len             ( char* dest, const char * const src, uint_32 dest_size );
char* SS_Port_Strcat                 ( char* dest, const char* format, ... );
void  SS_Port_Get_DateTime_String    ( char* buf );
void  SS_Port_DateTime_To_DayMonThYear ( const char* DateTime, int* p_Day, int* p_Month, int* p_Year );
void  SS_Port_DateTime_To_Short_Date ( char* Short_Buf, const char* DateTime );
int   SS_Port_Month_To_Number        ( const char* Month );

void* SS_Port_AllocMem               ( int size );
void  SS_Port_FreeMem                ( void* block );
void  SS_Port_ZeroMemory             ( void* block, uint_32 size );

void SS_Port_Merge_Strcpy_Len        ( int merge, char* dest, const char* src, int Len );
void SS_Port_Merge_Ascii_To_Byte     ( int merge, uint_08* p_dest_byte, const char* src );
#ifdef __cplusplus
}
#endif

typedef struct Month_Struct
{
   char* Short_Month;
   char* Long_Month;
} Month_Struct;

extern Month_Struct Months[];

#if UNIX

#define MAXFD 64

// =---------------------------------------------------------------------------
// typedefs
//
// ----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" 
{
#endif
typedef void Sigfunc ( int );
Sigfunc* signal ( int signo, Sigfunc* func );
Sigfunc* Signal ( int signo, Sigfunc* func );
#ifdef __cplusplus
}
#endif

int Unix_Daemon_Init     ( const char* pname );

#endif // UNIX

#endif // __PORT_SUBSYSTEM_H__
