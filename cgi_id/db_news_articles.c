// =---------------------------------------------------------------------------
// n e w s _ a r t i c l e s .c
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

#include "cgi_id_subsystems.h"
#include "databases.h"
#include "pages.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// F o r w a r d   D e c l a r a t i o n s
// =---------------------------------------------------------------------------
int Action_Gather_News_Article_Fields ( db_news_articles* p_rec );


// =---------------------------------------------------------------------------
// Local Variables
// =---------------------------------------------------------------------------
char Unknown_News_String  [ 80 ];
char Unknown_State_String [ 80 ];

// =---------------------------------------------------------------------------
// W r i t e _ A r t i c l e _ S  u m m a r i e s
//
// A class specification of -1 should be specified to ignore class and enumarate
// all normal-state articles
//
// Caller is responsible for opening and closing the database.
//
// 0  : Success
// -2 : database error
// =---------------------------------------------------------------------------
int Write_Article_Summaries ( int Class, db_metastructure* p_ms )
{
   db_news_articles Record;
   int              Num_Written;
   int              i, Count;
   char             Link[256];
   char             Short_Date[32];

   if ( !p_ms || !SS_DB_Is_Open ( p_ms ) ) return -2;

   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "<TH>Date</TH>" );
   SS_HTML_Write ( "<TH style=\"{width: 400px;}\">Title</TH>" );

   SS_HTML_Write ( "<TH>Author</TH>" );
   SS_HTML_Write ( "<TH>Class</TH>" );
   SS_HTML_Write ( "<TH>State</TH>" );
   SS_HTML_Write ( "<TH>&nbsp;</TH>" );

   SS_HTML_Write ( "</TR>" );

   Num_Written = 0;
   Count = SS_DB_Get_Record_Count(p_ms);
   for ( i = Count-1; i >= 0; i-- )
   {
      if ( 0 != SS_DB_Get_Record ( p_ms, i, &Record ) )
         continue;

      if ( Class != NA_CLASS_ALL && Record.Class != Class )
         continue;

      Num_Written++;

      SS_Port_DateTime_To_Short_Date ( Short_Date, Record.Publish_Date );
   
      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<td>%s</td>", Short_Date );
      SS_HTML_Write ( "<td style=\"{width: 400px;}\">%s</td>", Record.Title );
      
      SS_HTML_Write ( "<td>%s</td>", Record.Author );
      SS_HTML_Write ( "<td>%s</td>", Get_News_Class_String(Record.Class) );
      SS_HTML_Write ( "<td>%s</td>", Get_News_State_String(Record.State) );

      Format_Script_Link ( Link, "NewsAdmin" );
      SS_HTML_Write ( "<td><a href=\"%s;Direct=2;Recnum=%d\">Edit</a></td>", Link, (int)i );

      SS_HTML_Write ( "</tr>" );
   }

   if ( !Num_Written )
      SS_HTML_Write ( "<tr><td colspan=\"5\">There are no articles at this time.</td></tr>" );

   SS_HTML_End_Table_Tag ( );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ E n d _ U s e r _ A r t i c l e _ S u m m a r i e s
// =---------------------------------------------------------------------------
int Write_End_User_Article_Summaries ( int Class, db_metastructure* p_ms )
{
   db_news_articles Record;
   int              Num_Written;
   int              i;
   int              Count;

   if ( !p_ms || !SS_DB_Is_Open ( p_ms ) ) return -2;

   Write_ParaTable ( PT_NEWS );

   Num_Written = 0;
   Count = SS_DB_Get_Record_Count(p_ms);
   for ( i = Count-1; i >= 0 ; i-- )
   {
      if ( 0 != SS_DB_Get_Record ( p_ms, i, &Record ) )
         continue;

      if ( Record.State == NA_STATE_DELETED || Record.State == NA_STATE_INACTIVE)
         continue;

      if ( Class != NA_CLASS_ALL && Record.Class != Class )
         continue;

      Num_Written++;

      SS_HTML_WriteP ( "%s, <i><b>%s</b></i> - %s&nbsp;", Record.Publish_Date, Record.Title, Record.Body );

   }

   Write_End_ParaTable ( );

   return 0;
}


// =---------------------------------------------------------------------------
// W r i t e _ N e w s _ A r t i c l e _ I n p u t s 
// =---------------------------------------------------------------------------
int Write_News_Article_Inputs ( db_news_articles* p_rec )
{
   char Short_Date[32];

   if ( !p_rec ) return -1;

   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Title</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=60 name=\"newsTitle\" value=\"%s\"></td>", p_rec->Title );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Author</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=30 name=\"newsAuthor\" value=\"%s\"></td>", p_rec->Author );
   SS_HTML_Write ( "</tr>" );

   SS_Port_DateTime_To_Short_Date ( Short_Date, p_rec->Publish_Date );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Publish Date</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=30 name=\"newsPublish_Date\" value=\"%s\"></td>", Short_Date );
   SS_HTML_Write ( "</tr>" );

   SS_Port_DateTime_To_Short_Date ( Short_Date, p_rec->Expire_Date );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Expiration Date</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=30 name=\"newsExpire_Date\" value=\"%s\">&nbsp; (Leave blank for no expiration date.)</td>", Short_Date );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Body</th>" );
   SS_HTML_Write ( "  <td><textarea rows=12 name=newsBody cols=\"80\">%s</textarea></td>", p_rec->Body );
   SS_HTML_Write ( "</tr>" );


   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Class</th>" );
   SS_HTML_Write ( "  <td>" );
   Write_News_Class_Select ( p_rec->Class );
   SS_HTML_Write ( "  </td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>State</th>" );
   SS_HTML_Write ( "  <td>" );
   Write_News_State_Select ( p_rec->State );
   SS_HTML_Write ( "  </td>" );
   SS_HTML_Write ( "</tr>" );
   SS_HTML_Write ( "<tr>" );

   SS_HTML_Write ( "  <th></th>" );
   SS_HTML_Write ( "  <td>" );
   SS_HTML_Submit ( "" );
   SS_HTML_Write ( "</td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_End_Table_Tag ( );

   return 0;
}

// =---------------------------------------------------------------------------
// A c t i o n _ G a t h e r _ N e w s _ A r t i c l e _ F i e l d s
// =---------------------------------------------------------------------------
int Action_Gather_News_Article_Fields ( db_news_articles* p_rec )
{
   char     Class [ VARIABLE_BYTES ];
   char     State [ VARIABLE_BYTES ];

   SS_Port_ZeroMemory ( p_rec, sizeof(db_news_articles) );

   cgiFormStringNoNewlines ( "newsTitle",        p_rec->Title,        NEWSARTICLES_TITLE_BYTES );
   cgiFormStringNoNewlines ( "newsAuthor",       p_rec->Author,       NEWSARTICLES_AUTHOR_BYTES );
   cgiFormStringNoNewlines ( "newsBody",         p_rec->Body,         NEWSARTICLES_BODY_BYTES );
   cgiFormStringNoNewlines ( "newsPublish_Date", p_rec->Publish_Date, NEWSARTICLES_PUBLISHDATE_BYTES );
   cgiFormStringNoNewlines ( "newsExpire_Date",  p_rec->Expire_Date,  NEWSARTICLES_EXPIREDATE_BYTES );

   if ( !strlen ( p_rec->Publish_Date ) )
      SS_Port_Get_DateTime_String ( p_rec->Publish_Date );

   Class[0] = 0; State[0] = 0;
   cgiFormStringNoNewlines ( "newsState",  State,  VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "newsClass",  Class,  VARIABLE_BYTES );

   p_rec->State = NA_STATE_NORMAL;
   p_rec->Class = NA_CLASS_STANDARDNEWS;
   SS_Port_Merge_Ascii_To_Byte ( 1 /*merge*/, &p_rec->State, State );
   SS_Port_Merge_Ascii_To_Byte ( 1 /*merge*/, &p_rec->Class, Class );

   // Form variables need to be properly escaped ( " goes to &quot, etc)
   //

   SS_HTML_Encode_Symbols   ( p_rec->Title,   NEWSARTICLES_TITLE_BYTES , 0 );
   SS_HTML_Encode_Symbols   ( p_rec->Author,  NEWSARTICLES_AUTHOR_BYTES, 0 );
   SS_HTML_Encode_Symbols   ( p_rec->Body,    NEWSARTICLES_BODY_BYTES  , 0 );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ N e w s _ C l a s s _ S e l e c t
// =---------------------------------------------------------------------------
void Write_News_Class_Select ( uint_08 Class_Default )
{
   int i;
   SS_HTML_Write ( "<select size=1 name=newsClass>" );

   for ( i = NA_CLASS_START; i <= NA_CLASS_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)Class_Default) ? "selected" : "", Get_News_Class_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ N e w s _ S t a t e _ S e l e c t
// =---------------------------------------------------------------------------
void Write_News_State_Select ( uint_08 State_Default )
{
   int i;
   SS_HTML_Write ( "<select size=1 name=newsState>" );

   for ( i = NA_STATE_START; i <= NA_STATE_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)State_Default) ? "selected" : "", Get_News_State_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// G e t _ N e w s _ C l a s s _ S t  ri n  g
// =---------------------------------------------------------------------------
const char* Get_News_Class_String ( uint_08 nc )
{
   switch ( nc )
   {
      case NA_CLASS_STANDARDNEWS  : return "Standard News";
      case NA_CLASS_RANKINGNEWS   : return "Ranking News";
      case NA_CLASS_PRODNOTES     : return "Production Notes";

      default:
         sprintf ( Unknown_News_String, "(Unknown News Class %d)", (int)nc );
         return Unknown_News_String;
   }
}


// =---------------------------------------------------------------------------
// G e t _ N e w s _ S t a t e _ S t r i n  g
// =---------------------------------------------------------------------------
const char* Get_News_State_String ( uint_08 ns )
{
   switch ( ns )
   {
      case NA_STATE_NORMAL   : return "Normal";
      case NA_STATE_INACTIVE : return "Inactive";
      case NA_STATE_DELETED  : return "Deleted";

      default:
         sprintf ( Unknown_News_String, "(Unknown News State %d)", (int)ns );
         return Unknown_State_String;
   }
}
