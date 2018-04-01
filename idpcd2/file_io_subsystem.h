
#ifndef __FILE_IO_SUBSYSTEM_H__
#define __FILE_IO_SUBSYSTEM_H__

#include "standard_types.h"

#if UNIX
   //#define MAX_PATH                 256
   
   //#ifndef MAP_FAILED
   //   #define MAP_FAILED              ((caddr_t)-1)
   //#endif

#elif WIN32
   #define MAP_FAILED              (0)
#endif

#include <sys/types.h>
#include <sys/stat.h>

// 
// Subsystem SS_File
//

#ifdef __cplusplus
extern "C" 
{
#endif
int   SS_File_IO_Create        ( const char* filename );
int   SS_File_IO_Open          ( const char* filename, int Read_Only );
int   SS_File_IO_Open_Append   ( const char* filename );
int   SS_File_IO_Read          ( int file, uint_08* p_data, uint_32 size, uint_32* p_size_read );
int   SS_File_IO_Write         ( int file, uint_08* p_data, uint_32 size );
int   SS_File_IO_Close         ( int file );
int   SS_File_IO_Stat          ( int file, struct stat* p_stat );
int   SS_File_IO_Seek          ( int file, long offset, int origin );
int   SS_File_IO_Flush         ( int file );
int   SS_File_IO_Flush_Mapped  ( void* p_data, uint_32 byte_size );

void* SS_File_IO_Map           ( int file, void** pp_hMap, uint_32 byte_size );
int   SS_File_IO_Unmap         ( void* p_data, void* hMap, uint_32 byte_size );
char* SS_File_Full_Path        ( char* fullpath, const char* path, const char* filename );
#ifdef __cplusplus
}
#endif

#endif // __FILE_IO_SUBSYSTEM_H__


