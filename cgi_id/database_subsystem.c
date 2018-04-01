// =---------------------------------------------------------------------------
// b u g b a s e .c
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//    Implements a generic database based on an array of structures stored on 
//    disk and accessed via memory-mapped files
//
//    For all functions, 0 means success, non-zero means failure
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
//   00 Apr 23   nic   Created. (Happy Easter)
//

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <sys/stat.h>
#include "database_subsystem.h"
#include "file_io_subsystem.h"
#include "port_subsystem.h"

#if UNIX
   #include <sys/mman.h>
#endif

// =---------------------------------------------------------------------------
// S S _ D B _ I n i t i a l i z e _ M e t a S t r u c t u r e
// 
// We accept a zero record size here so don't assume the params are valid,
// they are sometimes used to initialize the structure to an empty state
//
// =---------------------------------------------------------------------------
int SS_DB_Initialize_MetaStructure ( db_metastructure* p_ms, 
                                     uint_32           rs, 
                                     uint_32           as,
                                     uint_32           hs 
                                   )
{
   if ( !p_ms ) return -1;
   if ( !as ) as = 10 * rs;         // default expansion size 10 records
   if ( !hs ) hs = sizeof(uint_32);

   p_ms->cbSize            = sizeof(db_metastructure);
   p_ms->record_size       = rs;
   p_ms->db_win32_filemap  = 0;     // Unused in UNIX, but requires in subsystem params
   p_ms->p_header          = 0;       
   p_ms->db_file           = 0;
   p_ms->p_record_count    = 0;
   p_ms->alloc_size        = as;
   p_ms->header_size       = hs;

   SS_Port_ZeroMemory ( &p_ms->db_stat, sizeof(struct stat) );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ C r e a t e
// =---------------------------------------------------------------------------
int SS_DB_Create ( db_metastructure* p_ms, const char* path, const char* filename )
{
   int      res           = 0;
   uint_32  writ          = 0;
   uint_08* initial_bytes = 0;
   char     fullpath [256];

   if ( !SS_DB_Is_Initialized(p_ms) ) return -1;
   if ( SS_DB_Is_Open(p_ms) ) return -1;
   
   // TODO: Find out why write permissions do not take for
   // 'group' and 'other'

   SS_File_Full_Path ( fullpath, path, filename );

   p_ms->db_file = SS_File_IO_Create ( fullpath );
   
   if ( p_ms->db_file == -1 )
   {
      res = -1;
   }
   else
   {
      initial_bytes = (uint_08*)SS_Port_AllocMem ( p_ms->alloc_size );

      if ( !initial_bytes )
      {
         res = -1;
      }
      else
      {
         SS_Port_ZeroMemory ( initial_bytes, p_ms->alloc_size );
         
         writ = SS_File_IO_Write ( p_ms->db_file, initial_bytes, p_ms->alloc_size );
	 
         SS_Port_FreeMem ( initial_bytes );
         initial_bytes = 0;
      
         // if we did not write out 4 bytes, something went wrong and we need
         // to report the error here
         //
         if ( writ == p_ms->alloc_size ) res = 0;
         else res = -1;

         SS_File_IO_Close ( p_ms->db_file );
         p_ms->db_file = 0;
      }
   }

   return res;
}

// =---------------------------------------------------------------------------
// S S _ D B _ O p e n
//
// Returns -1 on error, such as when the database could not be opened
// or the memory mapping failed
//
// =---------------------------------------------------------------------------
int SS_DB_Open ( db_metastructure* p_ms, const char* path, const char* filename )
{
   int res;
   char fullpath[256];

   if ( !SS_DB_Is_Initialized(p_ms) ) return -1;
   if ( SS_DB_Is_Open(p_ms) ) return -1;

   SS_File_Full_Path ( fullpath, path, filename );

   p_ms->db_file = SS_File_IO_Open ( fullpath, 0 /*readonly*/ );
   
   if ( p_ms->db_file == -1 )
   {
   // #if WIN32
   //       Report_Win32LastError ( "Database_Open_Error", GetLastError() );
   // #endif          
   //       return RS_BADFILE;
   //   SS_HTML_WriteP ( "Failure to open: path=%s, filename=%s, fullpath=%s",
   //      path, filename, fullpath );

      return -1;
   }

   // we use db_stat to figure out the actual file size

   res = SS_File_IO_Stat ( p_ms->db_file, &p_ms->db_stat );

   if ( res == -1 )
   {
      //SS_HTML_WriteP ( "SS_File_IO_Stat failed" );
      return -1;
   }

   p_ms->p_header = (uint_08*)
      SS_File_IO_Map ( p_ms->db_file, &p_ms->db_win32_filemap, p_ms->db_stat.st_size );
   
   if ( p_ms->p_header == (uint_08*)MAP_FAILED )
   {
      //SS_HTML_WriteP ( "SS_File_IO_Map failed: p_ms->db_file=%d, p_ms->db_win32_filemap=%d",
      //   (int)p_ms->db_file, (int)p_ms->db_win32_filemap );

      SS_File_IO_Close ( p_ms->db_file );
      p_ms->db_file = 0;
      return -1;
   }
   
   // the first 32-bits of every database contains the number
   // of records

   p_ms->p_record_count = (uint_32*)p_ms->p_header;

   // reset pointers to the records
   //
   p_ms->p_records = p_ms->p_header + SS_DB_Header_Size ( p_ms );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ C r e a t e _ O p e n
//
// Returns -1 on error, see SS_DB_Open
//
// =---------------------------------------------------------------------------
int SS_DB_Create_Open ( db_metastructure* p_ms, const char* path, const char* filename )
{
   char fullpath[256 /*MAX_PATH*/];
   int  file;
   int  ret;

   if ( !SS_DB_Is_Initialized(p_ms) ) return -1;
   if ( SS_DB_Is_Open(p_ms) ) return -1;

   SS_File_Full_Path ( fullpath, path, filename );

   file = SS_File_IO_Open ( fullpath, 0 /*readonly*/ ); // O_RDONLY
   
   if ( file != -1 )
   {
      SS_File_IO_Close ( file );
   }
   else
   {
      ret = SS_DB_Create ( p_ms, path, filename );
      if ( ret != 0 ) return ret;
   }

   return SS_DB_Open ( p_ms, path, filename );
}

// =---------------------------------------------------------------------------
// S S _ D B _ C l o s e
// =---------------------------------------------------------------------------
int SS_DB_Close ( db_metastructure* p_ms )
{
   if ( !SS_DB_Is_Open(p_ms) ) return -1;

   SS_File_IO_Unmap ( p_ms->p_header, p_ms->db_win32_filemap, p_ms->db_stat.st_size );

   p_ms->p_header = 0;

   SS_File_IO_Close ( p_ms->db_file );
   
   SS_DB_Initialize_MetaStructure ( p_ms, 0, 0, 0 );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ I s _ O p e n
// =---------------------------------------------------------------------------
int SS_DB_Is_Open ( db_metastructure* p_ms )
{
   if ( !SS_DB_Is_Initialized(p_ms) ) return 0;

   return   ( p_ms->cbSize == sizeof(db_metastructure) )
         && ( p_ms->p_header > 0 ) 
         && ( p_ms->db_file > 0 );
}

// =---------------------------------------------------------------------------
// S S _ D B _ I s _ I n i t i a l i z e d
// =---------------------------------------------------------------------------
int SS_DB_Is_Initialized ( db_metastructure* p_ms )
{
   return p_ms->cbSize == sizeof(db_metastructure);
}

// =---------------------------------------------------------------------------
// S S _ D B _ H e a d e r _ S i z e
// =---------------------------------------------------------------------------
int SS_DB_Header_Size ( db_metastructure* p_ms )
{
   if ( !SS_DB_Is_Initialized(p_ms) ) return -1;
   return p_ms->header_size;
}

// =---------------------------------------------------------------------------
// S S _ D B _ G e t _ R e c o r d _ S i z e
// =---------------------------------------------------------------------------
uint_32 SS_DB_Get_Record_Size ( db_metastructure* p_ms )
{
   if ( !SS_DB_Is_Initialized(p_ms) ) return 0xFFFFFFFF;
   return p_ms ? p_ms->record_size : 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ G e t _ R e c o r d _ C o u n t
// =---------------------------------------------------------------------------
uint_32 SS_DB_Get_Record_Count ( db_metastructure* p_ms )
{
   if ( !SS_DB_Is_Open(p_ms) ) return 0xFFFFFFFF;
   return p_ms ? *p_ms->p_record_count : 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ S e t _ R e c o r d _ C o u n t
// =---------------------------------------------------------------------------
uint_32 SS_DB_Set_Record_Count ( db_metastructure* p_ms, uint_32 count )
{
   if ( !SS_DB_Is_Open(p_ms) ) return 0xFFFFFFFF;
   return p_ms ? *p_ms->p_record_count = count : 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ C l e a r _ D a t a b a s e
// =---------------------------------------------------------------------------
int SS_DB_Clear_Database ( db_metastructure* p_ms, int fSecure_Wipe )
{
   if ( !SS_DB_Is_Open(p_ms) ) return -1;

   if ( fSecure_Wipe ) SS_Port_ZeroMemory ( p_ms->p_header, p_ms->db_stat.st_size );
   else SS_DB_Set_Record_Count ( p_ms, 0 );

   return -1;
}

// =---------------------------------------------------------------------------
// S S _ D B _ E x p a n d _ f i l e
// Returns:
// -1 : general failure on expansion, file error, etc.
// =---------------------------------------------------------------------------
int SS_DB_Expand_File ( db_metastructure* p_ms, uint_32 expand_bytes )
{
   int      ret = 0;
   uint_08* p_slush = 0;

   if ( !SS_DB_Is_Open(p_ms) ) return -1;

   // first we have to unmap the file bacause we need
   // to map the newly allocated file area afterwards
   
   ret = SS_File_IO_Unmap ( p_ms->p_header, p_ms->db_win32_filemap, p_ms->db_stat.st_size );

   if ( ret == -1 ) return -1;
   
   // create a buffer to use in expanding the file
   // then expand the file

   p_slush = (uint_08*)SS_Port_AllocMem ( expand_bytes );
   
   if ( !p_slush ) return -1;

   SS_Port_ZeroMemory ( p_slush, expand_bytes );
   
   ret = SS_File_IO_Seek  ( p_ms->db_file, 0, SEEK_END );
   ret = SS_File_IO_Write ( p_ms->db_file, p_slush, expand_bytes );

   SS_Port_FreeMem ( p_slush );
   p_slush = 0;

   if ( ret != (int)expand_bytes ) return -1;
   
   // update our notion of how big the file is

   SS_File_IO_Stat ( p_ms->db_file, &p_ms->db_stat );
   
   // now we can remap the file based on the new size
   
   p_ms->p_header = (uint_08*)
      SS_File_IO_Map ( p_ms->db_file, &p_ms->db_win32_filemap, p_ms->db_stat.st_size );
   
   if ( p_ms->p_header == (uint_08*)MAP_FAILED )
   {
      SS_File_IO_Close ( p_ms->db_file );
      p_ms->db_file = 0;
      return -1;
   }
   
   // reset where record count gets its data from
   //
   p_ms->p_record_count = (uint_32*)p_ms->p_header;

   // reset pointers to the records
   //
   p_ms->p_records = p_ms->p_header + SS_DB_Header_Size ( p_ms );

   return 0;
}


// =---------------------------------------------------------------------------
// S S _ D B _ G e t _ R e c o r d
//
// Copies the entire record into the user's buffer
// The user's buffer needs to be pre-allocated to  exactly
// Record_Size bytes or more
// =---------------------------------------------------------------------------
int SS_DB_Get_Record ( db_metastructure* p_ms, uint_32 recnum, void* p_rec )
{
   if ( !SS_DB_Is_Open(p_ms) ) return -1;
   if ( !p_rec ) return -1;
   if ( recnum >= SS_DB_Get_Record_Count(p_ms) ) return -1;

   memcpy ( p_rec, p_ms->p_records + recnum*p_ms->record_size, p_ms->record_size );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ G e t _ R e c o r d
//
// Gets a quick reference to the specified record.
//
// Returns 0 on success, -1 on error.
//
// =---------------------------------------------------------------------------
int SS_DB_Get_Record_Ref  ( db_metastructure* p_ms, uint_32 recnum, void** pp_rec )
{
   if ( !SS_DB_Is_Open(p_ms) ) return -1;
   if ( !pp_rec ) return -1;
   if ( recnum >= SS_DB_Get_Record_Count(p_ms) ) return -1;

   *pp_rec = (void*)(p_ms->p_records + recnum*p_ms->record_size);

   return 0;
}

// =---------------------------------------------------------------------------
// S e t _ R e c o r d
//
// Copies the entire record from users buffer to database
// Record_Size bytes or more
// =---------------------------------------------------------------------------
int SS_DB_Set_Record ( db_metastructure* p_ms, uint_32 recnum, void* p_rec )
{
   if ( !p_rec ) return -1;
   if ( recnum >= SS_DB_Get_Record_Count(p_ms) ) return -1;

   memcpy ( p_ms->p_records + recnum*p_ms->record_size, p_rec, p_ms->record_size );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ D e l e t e _ R e c o r d
//
// =---------------------------------------------------------------------------
int SS_DB_Delete_Record ( db_metastructure* p_ms, uint_32 recnum )
{
   if ( !SS_DB_Is_Open(p_ms) ) return -1;
   if ( recnum >= SS_DB_Get_Record_Count(p_ms) ) return -1;

   if ( recnum < SS_DB_Get_Record_Count(p_ms)-1 )
   {
      memcpy ( p_ms->p_records + p_ms->record_size*recnum, 
               p_ms->p_records + p_ms->record_size*(recnum+1), 
               p_ms->record_size * (SS_DB_Get_Record_Count(p_ms)-recnum-1) );
   }

   SS_DB_Set_Record_Count ( p_ms, SS_DB_Get_Record_Count(p_ms) - 1 );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ A d d _ R e c o r d
//
// Returns:
// -1 : generic error (bad params, etc.)
// -2 : database error
// =---------------------------------------------------------------------------
int SS_DB_Add_Record ( db_metastructure* p_ms, void* p_rec, uint_32* p_recnum )
{
   int ret;

   if ( !SS_DB_Is_Open(p_ms) ) return -2;
   if ( !p_rec ) return -1;
   if ( !p_recnum ) return -1;

   // if the addition of one record (Record_Size()) plus the total
   // space used by the header and all records is less than the 
   //
   if ( SS_DB_Header_Size(p_ms) + (SS_DB_Get_Record_Count(p_ms)+1) * p_ms->record_size > (uint_32)p_ms->db_stat.st_size )
   {
      ret = SS_DB_Expand_File ( p_ms, p_ms->alloc_size );
      if ( ret != 0 ) return -2;
   }

   memcpy ( p_ms->p_records + p_ms->record_size*SS_DB_Get_Record_Count(p_ms), p_rec, p_ms->record_size );
   *p_recnum = SS_DB_Get_Record_Count(p_ms);

   SS_DB_Set_Record_Count ( p_ms, SS_DB_Get_Record_Count(p_ms) + 1 );

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ D B _ F l u s h 
//
// =---------------------------------------------------------------------------
int SS_DB_Flush ( db_metastructure* p_ms )
{
   if ( !SS_DB_Is_Open(p_ms) ) return -2;

   return SS_File_IO_Flush_Mapped ( p_ms->p_header, 0 );
}

