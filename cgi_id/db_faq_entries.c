// =---------------------------------------------------------------------------
// d b _ f a q _ e n t r i e s . c
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
//   00 Sep  1   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "databases.h"
#include "pages.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// Local Storage for Error Strings
// =---------------------------------------------------------------------------
char Unknown_FAQ_State_String [ 80 ];

// =---------------------------------------------------------------------------
// W r i t e _ F A Q _ E n t r y _ S u m m a r i e s
//
// Caller is responsible for opening and closing the database.
//
// 0  : Success
// -2 : database error
// =---------------------------------------------------------------------------
int Write_FAQ_Entry_Summaries ( db_metastructure* p_ms )
{
   db_faq_entry     Record;
   int              Num_Written;
   int              i, Count;
   char             Link[256];

   if ( !p_ms || !SS_DB_Is_Open ( p_ms ) ) return -2;

   SS_HTML_Table_Tag_Ex ( "width=500" );

   SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<th>Question</th>" );
      SS_HTML_Write ( "<th>State</th>" );
      SS_HTML_Write ( "<th>&nbsp;</th>" );
   SS_HTML_Write ( "</tr>" );

   Num_Written = 0;
   Count = SS_DB_Get_Record_Count(p_ms);
   for ( i = Count-1; i >= 0; i-- )
   {
      if ( 0 != SS_DB_Get_Record ( p_ms, i, &Record ) )
         continue;

      Num_Written++;

      SS_HTML_Write ( "<tr>" );
     
         SS_HTML_Write ( "<td>%s</td>", Record.Question );
         SS_HTML_Write ( "<td>%s</td>", Get_FAQ_State_String(Record.State) );

         Format_Script_Link ( Link, "FAQAdmin" );
         SS_HTML_Write ( "<td><a href=\"%s;Direct=2;Recnum=%d\">Edit</a></td>", Link, (int)i );

      SS_HTML_Write ( "</tr>" );
   }

   if ( !Num_Written )
      SS_HTML_Write ( "<tr><td colspan=\"5\">There are no FAQ entries at this time.</td></tr>" );

   SS_HTML_End_Table_Tag ( );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ E n d _ U s e r _ F A Q _ S u m m a r i e s
// =---------------------------------------------------------------------------
int Write_End_User_FAQ_Summaries ( db_metastructure* p_ms )
{
   db_faq_entry  Record;
   int           Num_Written;
   int           i;
   int           Count;

   if ( !p_ms || !SS_DB_Is_Open ( p_ms ) ) return -2;

   Num_Written = 0;
   Count = SS_DB_Get_Record_Count(p_ms);
   for ( i = Count-1; i >= 0 ; i-- )
   {
      if ( 0 != SS_DB_Get_Record ( p_ms, i, &Record ) )
         continue;

      if ( Record.State == FAQ_STATE_DELETED || Record.State == FAQ_STATE_INACTIVE)
         continue;

      Num_Written++;


      Write_ParaTable ( PT_FAQ );
         SS_HTML_Write ( "<b>Question:</b> %s<br><br>", Record.Question );
         SS_HTML_Write ( "<b>Answer:</b> %s<br><br>", Record.Answer );
         SS_HTML_Write  ( "<br><center>~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-~-=-</center><br>" );
         //SS_HTML_Write  ( "<hr>" );
      Write_End_ParaTable ( );
   }


   return 0;
}


// =---------------------------------------------------------------------------
// W r i t e _ F A Q _ E n t r y _ I n p u t s 
// =---------------------------------------------------------------------------
int Write_FAQ_Entry_Inputs ( db_faq_entry* p_rec )
{
   if ( !p_rec ) return -1;

   //SS_HTML_Table_Tag_Ex ( "style=\"{width:700px;}\"");
   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Question</th>" );
   SS_HTML_Write ( "  <td><textarea rows=\"12\" name=\"faqQuestion\" cols=\"80\">%s</textarea></td>", p_rec->Question );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Answer</th>" );
   SS_HTML_Write ( "  <td><textarea rows=\"12\" name=\"faqAnswer\" cols=\"80\">%s</textarea></td>", p_rec->Answer );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>State</th>" );
   SS_HTML_Write ( "  <td>" );
   Write_FAQ_State_Select ( p_rec->State );
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
// A c t i o n _ G a t h e r _ F A Q _ E n t r y _ F i e l d s
// =---------------------------------------------------------------------------
int Action_Gather_FAQ_Entry_Fields ( db_faq_entry* p_rec )
{
   char     State [ VARIABLE_BYTES ];

   SS_Port_ZeroMemory ( p_rec, sizeof(db_faq_entry) );

   cgiFormStringNoNewlines ( "faqQuestion",   p_rec->Question,  FAQQUESTION_BYTES );
   cgiFormStringNoNewlines ( "faqAnswer",     p_rec->Answer,    FAQANSWER_BYTES   );

   State[0] = 0;
   cgiFormStringNoNewlines ( "faqState",  State,  VARIABLE_BYTES );

   p_rec->State = FAQ_STATE_NORMAL;
   SS_Port_Merge_Ascii_To_Byte ( 1 /*merge*/, &p_rec->State, State );

   // Form variables need to be properly escaped ( " goes to &quot, etc)
   //

   //SS_HTML_Encode_Symbols   ( p_rec->Question,  FAQQUESTION_BYTES, 0 );
   //SS_HTML_Encode_Symbols   ( p_rec->Answer,    FAQANSWER_BYTES,   0 );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ F A Q _ S t a t e _ S e l e c t
// =---------------------------------------------------------------------------
void Write_FAQ_State_Select ( uint_08 State_Default )
{
   int i;
   SS_HTML_Write ( "<select size=1 name=faqState>" );

   for ( i = FAQ_STATE_START; i <= FAQ_STATE_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)State_Default) ? "selected" : "", Get_FAQ_State_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// G e t _ F A Q _ S t a t e _ S t r i n  g
// =---------------------------------------------------------------------------
const char* Get_FAQ_State_String ( uint_08 ns )
{
   switch ( ns )
   {
      case FAQ_STATE_NORMAL   : return "Normal";
      case FAQ_STATE_INACTIVE : return "Inactive";
      case FAQ_STATE_DELETED  : return "Deleted";

      default:
         sprintf ( Unknown_FAQ_State_String, "(Unknown FAQ State %d)", (int)ns );
         return Unknown_FAQ_State_String;
   }
}
