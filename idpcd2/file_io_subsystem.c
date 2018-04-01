// =---------------------------------------------------------------------------
// f i l e _ i o _ s u b s y s t e m . h
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
#include <string.h>
#include <time.h>
#include "file_io_subsystem.h"
#include "port_subsystem.h"

#if UNIX

   #include <unistd.h>
   #include <fcntl.h>
   #include <sys/ioctl.h>
   #include <sys/mman.h>

#elif WIN32

   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>

#endif

// =---------------------------------------------------------------------------
//
// S S _ F i l e _ I O _ C r e a t e
//
int SS_File_IO_Create ( const char* filename )
{
#if UNIX

   return open ( filename, O_CREAT | O_EXCL | O_RDWR, 0666 );

#elif WIN32
   int hFile = 0;

   hFile = (int)CreateFile( filename, 
                            GENERIC_WRITE | GENERIC_READ, 
                            FILE_SHARE_READ, 
                            NULL,
                            CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

   if ( hFile == (int)INVALID_HANDLE_VALUE )
      return -1;
   else
      return hFile;

#endif
}

// =---------------------------------------------------------------------------
//
// S S _ F i l e _ I O _ O p e n
//
// Returns -1 on error
//
int SS_File_IO_Open ( const char* filename, int Read_Only )
{
#if UNIX
   return open ( filename, Read_Only ? O_RDONLY : O_RDWR, 0666 );
#elif WIN32
   
   DWORD  dwAccess = GENERIC_READ;
   HANDLE hFile    = 0;

   if ( !Read_Only )
      dwAccess |= GENERIC_WRITE;

   hFile = CreateFile ( filename, 
                        dwAccess, 
                        FILE_SHARE_READ, 
                        0, 
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0 );

   if ( hFile == INVALID_HANDLE_VALUE )
      return -1;
   else
      return (int)hFile;
#endif
}

// =---------------------------------------------------------------------------
//
// S S _ F i l e _ I O _ O p e n _ A p p e n d
//
int SS_File_IO_Open_Append ( const char* filename )
{
#if UNIX
   return open ( filename, O_RDWR | O_CREAT | O_APPEND, 0666 );
#elif WIN32

   HANDLE hFile = 0;

   hFile = CreateFile ( filename, 
                        GENERIC_READ | GENERIC_WRITE, 
                        FILE_SHARE_READ, 
                        0, 
                        OPEN_ALWAYS, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0 );

   if ( hFile == INVALID_HANDLE_VALUE )
   {
      return -1;
   }
   else
   {
      SS_File_IO_Seek ( (int)hFile, 0, FILE_END );
      return (int)hFile;
   }
#endif
}

// =---------------------------------------------------------------------------
//
// S S _ F i l e _ I O _ R e a d
//

int SS_File_IO_Read ( int file, uint_08* p_data, uint_32 size, uint_32* p_size_read )
{
#if UNIX
   *p_size_read = read ( file, (void*)p_data, size );
#else
   ReadFile ( (HANDLE)file, p_data, size, p_size_read, 0 );
#endif

   return *p_size_read;
}

// =---------------------------------------------------------------------------
//
// S S _ F i l e _ I O _ W r i t e
//

int SS_File_IO_Write ( int file, uint_08* p_data, uint_32 size )
{
#if UNIX
   return write ( file, (void*)p_data, size );
#else
   DWORD dwWrite;
   WriteFile ( (HANDLE)file, p_data, size, &dwWrite, 0 );
   return dwWrite;
#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _I  O_  C l o s e
//
int SS_File_IO_Close ( int file )
{
#if UNIX
   return close ( file );
#elif WIN32
   return CloseHandle ( (HANDLE)file );
#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ I O _ S t a t
//
int SS_File_IO_Stat ( int file, struct stat* p_stat )
{
#if UNIX
   return fstat ( file, p_stat );
#elif WIN32
   DWORD file_size = GetFileSize ( (HANDLE)file, 0 );
   SS_Port_ZeroMemory ( p_stat, sizeof(struct stat) );
   p_stat->st_size = file_size;
   return 0;
#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ I O_  M a p
//
void* SS_File_IO_Map ( int file, void** pp_hMap, uint_32 byte_size )
{
   if ( !pp_hMap ) return MAP_FAILED;
#if UNIX
   return mmap ( 0, byte_size, PROT_READ | PROT_WRITE,
               MAP_SHARED, file, 0 );
#elif WIN32
   *pp_hMap = CreateFileMapping ( (HANDLE)file, 0, PAGE_READWRITE, 0, 0, 0 );
   if ( *pp_hMap == INVALID_HANDLE_VALUE )
   {
      return MAP_FAILED;
   }
   else
   {
      return MapViewOfFile ( *pp_hMap, FILE_MAP_WRITE, 0, 0, byte_size );
   }
#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ I O _ U n m a p
//
int SS_File_IO_Unmap ( void* p_data, void* hMap, uint_32 byte_size )
{
   if ( !hMap ) return 0;

#if UNIX

   return munmap ( (caddr_t)p_data, byte_size );

#elif WIN32
   byte_size = 0; // not needed for win32

   UnmapViewOfFile ( p_data );
   CloseHandle ( hMap );
   return 0;

#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ I O _ S e e k
//
int SS_File_IO_Seek ( int file, long offset, int origin )
{
#if UNIX
   return lseek ( file, offset, origin );
#elif WIN32
   return SetFilePointer ( (HANDLE)file, offset, 0, origin );
#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ I O _ F l u s h 
//
int SS_File_IO_Flush ( int file )
{
   file = 0;
#if UNIX
   return fflush ( (FILE*)file );
#elif WIN32
   return 0;
#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ I O _ F l u s h _ M a p p e d
//
// Returns  0 on success
//         -1 on failure
//

int SS_File_IO_Flush_Mapped  ( void* p_data, uint_32 byte_size )
{
	#if UNIX
	   return msync ( p_data, byte_size /*should be 0 for all*/, MS_SYNC );
	#elif WIN32
	   return FlushViewOfFile ( p_data, byte_size /*should be 0 for all*/ ) ? 0 : -1;
	#endif
}

// =---------------------------------------------------------------------------
// S S _ F i l e _ F u l l _ P a t h 
//
// creates a full path from path/filename components
//
char* SS_File_Full_Path ( char* fullpath, const char* path, const char* filename )
{
#if UNIX
   char* path_delimeter = "/";
#elif WIN32
   char* path_delimeter = "\\";
#endif
   int len;

   fullpath[0] = 0;

   if ( path )
   {
      len = strlen(path);
      if ( len > 0 )
      {
         strcat ( fullpath, path );
         if ( path[len-1] != path_delimeter[0] )
            strcat ( fullpath, path_delimeter );
      }
   }

   if ( filename )
   {
      len = strlen(filename);
      if ( len > 0 )
         strcat ( fullpath, filename );
   }

   return fullpath;
}
