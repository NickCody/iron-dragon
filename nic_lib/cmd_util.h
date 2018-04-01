// =---------------------------------------------------------------------------
// u t i l . h
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
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Apr 03   nic   Created.
//


#ifndef __UTIL_H__
#define __UTIL_H__

#define MAX_ARGS        10

void Process_Command_Line ( int& argc, char**& argv, const int max_args );
void Destroy_Command_Line ( int& argc, char**& argv, const int max_args );
bool Translate_Shortcut   ( char* file_name );
void Strip_Character      ( char* str, char strip );
bool DibBlt               ( HDC hdc, sint_32 x0, sint_32 y0, sint_32 dx, sint_32 dy, 
                            HANDLE hdib, sint_32 x1, sint_32 y1, sint_32 rop );


#endif // __UTIL_H_
