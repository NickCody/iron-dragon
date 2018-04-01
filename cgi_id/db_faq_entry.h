
#ifndef __DBFAQ_H__
#define __DBFAQ_H__

#include "standard_types.h"

// Field Length Defines
//
#define FAQQUESTION_BYTES    1024
#define FAQANSWER_BYTES     20480
#define FAQSTATE_BYTES          1
#define FAQSLUSH_BYTES       3071
//                          _____
//                          24576 TOTAL (must keep at 24576!!!)


typedef enum FAQ_STATE
{
   FAQ_STATE_START    = 0,
   FAQ_STATE_NORMAL   = 0,
   FAQ_STATE_INACTIVE = 1,
   FAQ_STATE_DELETED  = 2,
   FAQ_STATE_END      = 2
} FAQ_STATE;

#if UNIX
   #define PACKED __attribute__((packed))
#elif WIN32
   #pragma pack ( push ,1 )
   #define PACKED
#endif


// Record Structure for a bug record
//
typedef struct db_faq_entry
{
   char     Question [ FAQQUESTION_BYTES ]      PACKED;
   char     Answer   [ FAQANSWER_BYTES   ]      PACKED;
   char     Slush    [ FAQSLUSH_BYTES    ]      PACKED;
   uint_08  State                               PACKED;
} db_faq_entry;

#if UNIX
   #undef PACKED
#elif WIN32
   #pragma pack ( pop )
#endif

int  Write_FAQ_Entry_Summaries         ( db_metastructure* p_ms );
int  Write_End_User_FAQ_Summaries      ( db_metastructure* p_ms );
int  Write_FAQ_Entry_Inputs            ( );
int  Action_Gather_FAQ_Entry_Fields    ( db_faq_entry* p_rec );
void Write_FAQ_State_Select           ( uint_08 Class_Default );
const char* Get_FAQ_State_String      ( uint_08 ns );


#endif // __DBFAQ_H__
