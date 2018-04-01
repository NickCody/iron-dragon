// =---------------------------------------------------------------------------
// s y s t e m _ s e t t i n g s .h
//
// Defines the structure and helper functions for the system settings database
//

#ifndef __SYSTEM_SETTINGS_H__
#define __SYSTEM_SETTINGS_H__

#define SYSSET_NAME_BYTES     32
#define SYSSET_VALUE_BYTES    160
#define SYSSET_CLASS_BYTES     1
#define SYSSET_STATE_BYTES     1
#define SYSSET_SLUSH          62
//                          ____
//                           256 (must be 256 bytes total)

typedef enum SYSSET_CLASS
{
   SYSSET_CLASS_NONE    = 0
} SYSSET_CLASS;

typedef enum SYSSET_STATE
{
   SYSSET_STATE_NORMAL   = 0,
   SYSSET_STATE_INACTIVE = 1,
   SYSSET_STATE_DELETED  = 2
} SYSSET_STATE;

#if UNIX
   #define PACKED __attribute__((packed))
#elif WIN32
   #pragma pack ( push ,1 )
   #define PACKED
#endif

typedef struct db_sysset
{
   char     Name  [ SYSSET_NAME_BYTES  ];
   char     Value [ SYSSET_VALUE_BYTES ];
   uint_08  Class;
   uint_08  State;   // 0 normal, 1 inactive, 2 deleted
   uint_08  Slush [ SYSSET_SLUSH ];
} db_sysset;

#if UNIX
#undef PACKED
#elif WIN32
   #pragma pack ( pop )
#endif

// Database Queries
//
int SysSet_Get_Value ( const char* Name, char* Value );
int SysSet_Set_Value ( const char* Name, char* Value, int Add_If_Not_Found );
int Write_SysSet_Table ( );

int SysSet_Get_Integer_Value ( const char* Name );
void SysSet_Set_Integer_Value ( const char* Name, int Value, int Add_If_Not_Found );

#endif // __SYSTEM_SETTINGS_H__