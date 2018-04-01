
#ifndef __BUGBASE_H__
#define __BUGBASE_H__

#include "standard_types.h"

// Field Length Defines
//
#define USERNAME_BYTES       24
#define EMAIL_BYTES          32
#define VERSION_BYTES        24
#define SUMMARY_BYTES       120
#define DATE_BYTES           32
#define DESCRIPTION_BYTES  4000
#define REPRO_STEPS_BYTES  2000
#define RESOLUTION_BYTES    500
#define COMMENTS_BYTES      500
#define BUGSLUSH_BYTES      100

typedef enum BUGOS_TYPES
{
   BUGOS_START           = 0,
   BUGOS_WINDOWS95       = 0,
   BUGOS_WINDOWS95OSR2   = 1,
   BUGOS_WINDOWS98       = 2,
   BUGOS_WINDOWS98SE     = 3,
   BUGOS_WINDOWSNT4SP3   = 4,
   BUGOS_WINDOWSNT4SP4   = 5,
   BUGOS_WINDOWSNT4SP5   = 6,
   BUGOS_WINDOWSNT4SP6   = 7,
   BUGOS_WINDOWSNT2000   = 8,
   BUGOS_END             = 8
} BUGOS_TYPES;

typedef enum BUGTYPE_TYPES
{
   BUGTYPE_START         = 0,
   BUGTYPE_BUG           = 0,
   BUGTYPE_SUGGESTION    = 1,
   BUGTYPE_END           = 1
} BUGTYPE_TYPES;

typedef enum BUGPRIORITY_TYPES
{
   BUGPRIORITY_START         = 0,
   BUGPRIORITY_URGENT        = 0,
   BUGPRIORITY_HIGH          = 1,
   BUGPRIORITY_MEDIUM        = 2,
   BUGPRIORITY_LOW           = 3,
   BUGPRIORITY_END           = 3
} BUGPRIORITY_TYPES;

typedef enum BUGSTATUS_TYPES
{
   BUGSTATUS_START         = 0,
   BUGSTATUS_OPEN          = 0,
   BUGSTATUS_FIXED         = 1,
   BUGSTATUS_NOTABUG       = 2,
   BUGSTATUS_FUTURERELEASE = 3,
   BUGSTATUS_END           = 3
} BUGSTATUS_TYPES;

typedef enum BUGCAT_TYPES
{
   BUGCAT_START            = 0,
   BUGCAT_NETWORKING       = 0,
   BUGCAT_AI               = 1,
   BUGCAT_GRAPHICS         = 2,
   BUGCAT_SOUND            = 3,
   BUGCAT_GAMEPLAY         = 4,
   BUGCAT_INSTALL          = 5,
   BUGCAT_END              = 5
} BUGCAT_TYPES;

#if UNIX
   #define PACKED __attribute__((packed))
#elif WIN32
   #pragma pack ( push ,1 )
   #define PACKED
#endif


// Record Structure for a bug record
//
typedef struct bug_record
{
   // New bug fields
   //                                                  // user or eden field
   char     Username     [USERNAME_BYTES   ] PACKED;   // user : 
   char     Email        [EMAIL_BYTES      ] PACKED;   // user : 
   char     Version      [VERSION_BYTES    ] PACKED;   // user : 
   char     Date_Reported[DATE_BYTES       ] PACKED;   // user : 
   uint_08  OS                               PACKED;   // user : See above
   uint_08  Bug_Type                         PACKED;   // user : See above
   uint_08  QA_Status                        PACKED;   // eden : See above
   uint_08  Priority                         PACKED;   // user : See above
   uint_08  Category                         PACKED;   // user : See above
   char     Summary      [SUMMARY_BYTES    ] PACKED;   // user : 
   char     Description  [DESCRIPTION_BYTES] PACKED;   // user : 
   char     Repro_Steps  [REPRO_STEPS_BYTES] PACKED;   // eden :

   // Existing bug fields
   //
   uint_08  Deleted                          PACKED;   // eden : 0 - false, non-zero - true
   uint_08  Status                           PACKED;   // eden : 1 = open, 2 = fixed, 3=not a bug
   char     Comments     [COMMENTS_BYTES  ]  PACKED;   // for users to add their 2 cents.
   char     Resolution   [RESOLUTION_BYTES]  PACKED;   // for admins only

   // New Fields (Slush subtraction)
   //
   char     Slush        [BUGSLUSH_BYTES  ]  PACKED;

} bug_record;

#if UNIX
   #undef PACKED
#elif WIN32
   #pragma pack ( pop )
#endif


#endif // __BUGBASE_H__