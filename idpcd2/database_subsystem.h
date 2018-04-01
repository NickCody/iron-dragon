
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "standard_types.h"
#include <sys/stat.h>

typedef struct db_metastructure
{
   uint_32     cbSize;              // set to size of db_metastructure
   uint_32     record_size;         // size of each record, in bytes
   void*       db_win32_filemap;    // used for Port_File subsystem memory mapped files
   uint_08*    p_header;            // pointer to header, start of entire database
   uint_08*    p_records;           // pointer to header, start of entire database
   int         db_file;             // file handle to database
   struct stat db_stat;             // used for file_size under Win32/Port_File subsystem
   uint_32*    p_record_count;      // pointer to the record count uint_32 in the header
   uint_32     alloc_size;          // default file grow size as recs are added
   uint_32     header_size;         // size of header, before first record
} db_metastructure;

#ifdef __cplusplus
extern "C" 
{
#endif
int      SS_DB_Initialize_MetaStructure ( db_metastructure* p_ms, 
                                          uint_32           rs,
                                          uint_32           as,
                                          uint_32           hs );

int      SS_DB_Create           ( db_metastructure* p_ms, const char* path, const char* filename );
int      SS_DB_Open             ( db_metastructure* p_ms, const char* path, const char* filename );
int      SS_DB_Create_Open      ( db_metastructure* p_ms, const char* path, const char* filename );
int      SS_DB_Close            ( db_metastructure* p_ms );
int      SS_DB_Is_Open          ( db_metastructure* p_ms );
int      SS_DB_Is_Initialized   ( db_metastructure* p_ms );
int      SS_DB_Header_Size      ( db_metastructure* p_ms );
uint_32  SS_DB_Get_Record_Count ( db_metastructure* p_ms );
uint_32  SS_DB_Get_Record_Size  ( db_metastructure* p_ms );
uint_32  SS_DB_Set_Record_Count ( db_metastructure* p_ms, uint_32 count );
int      SS_DB_Clear_Database   ( db_metastructure* p_ms, int  fErase );
int      SS_DB_Expand_File      ( db_metastructure* p_ms, uint_32 expand_bytes );
int      SS_DB_Get_Record       ( db_metastructure* p_ms, uint_32 recnum, void* p_rec );
int      SS_DB_Get_Record_Ref   ( db_metastructure* p_ms, uint_32 recnum, void** pp_rec );
int      SS_DB_Set_Record       ( db_metastructure* p_ms, uint_32 recnum, void* p_rec );
int      SS_DB_Delete_Record    ( db_metastructure* p_ms, uint_32 recnum );
int      SS_DB_Add_Record       ( db_metastructure* p_ms, void* p_rec, uint_32* p_recnum );
int      SS_DB_Flush            ( db_metastructure* p_ms );

#ifdef __cplusplus
}
#endif

#endif // __DATABASE_H__

