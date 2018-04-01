// =---------------------------------------------------------------------------
// n e w s _ a r t i c l e s . h
// 
//   (C) 2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
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
//   00 Apr 26   nic   Created.
//

#ifndef __NEWS_ARTICLES_H__
#define __NEWS_ARTICLES_H__

typedef enum NEWSARTICLES
{
   NEWSARTICLES_TITLE_BYTES       =   120,
   NEWSARTICLES_AUTHOR_BYTES      =    30,
   NEWSARTICLES_BODY_BYTES        = 16384,
   NEWSARTICLES_PUBLISHDATE_BYTES =    32,
   NEWSARTICLES_EXPIREDATE_BYTES  =    32,
   NEWSARTICLES_CLASS_BYTES       =     1,
   NEWSARTICLES_STATE_BYTES       =     1,
   NEWSARTICLES_SLUSH_BYTES       =  1024
                               //   _____
                               //   17624 bytes
} NEWSARTICLES;

typedef enum NA_CLASS
{
   NA_CLASS_ALL             = -1,
   NA_CLASS_START           =  0,
   NA_CLASS_STANDARDNEWS    =  0,
   NA_CLASS_RANKINGNEWS     =  1,
   NA_CLASS_PRODNOTES       =  2,
   NA_CLASS_END             =  2
} NA_CLASS;

typedef enum NA_STATE
{
   NA_STATE_START    = 0,
   NA_STATE_NORMAL   = 0,
   NA_STATE_INACTIVE = 1,
   NA_STATE_DELETED  = 2,
   NA_STATE_END      = 2
} NA_STATE;

#if UNIX
   #define PACKED __attribute__((packed))
#elif WIN32
   #pragma pack ( push ,1 )
   #define PACKED
#endif

typedef struct db_news_articles
{
   char     Title          [ NEWSARTICLES_TITLE_BYTES       ];
   char     Author         [ NEWSARTICLES_AUTHOR_BYTES      ];
   char     Body           [ NEWSARTICLES_BODY_BYTES        ];
   char     Publish_Date   [ NEWSARTICLES_PUBLISHDATE_BYTES ];
   char     Expire_Date    [ NEWSARTICLES_EXPIREDATE_BYTES  ];
   uint_08  Class;
   uint_08  State;         // 0 normal, 1 inactive, 2 deleted
   uint_08  Slush          [ NEWSARTICLES_SLUSH_BYTES       ];
} db_news_articles;

#if UNIX
#undef PACKED
#elif WIN32
   #pragma pack ( pop )
#endif

int  Write_Article_Summaries           ( int Class, db_metastructure* p_ms );
int  Write_End_User_Article_Summaries  ( int Class, db_metastructure* p_ms );
int  Write_News_Article_Inputs         ( );
int  Action_Gather_News_Article_Fields ( db_news_articles* p_rec );
void Write_News_Class_Select           ( uint_08 Class_Default );
void Write_News_State_Select           ( uint_08 Class_Default );
const char* Get_News_Class_String      ( uint_08 nc );
const char* Get_News_State_String      ( uint_08 ns );


#endif // __NEWS_ARTICLES_H__
