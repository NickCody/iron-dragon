// =---------------------------------------------------------------------------
// p a g e _ c l i e n t _ l i s t i n g . c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Jan 23   nic   Created.
//

#include <stdlib.h>
#include "cgi_id_subsystems.h"
#include "net_messages_admin.h"
#include "srvstat_actions.h"
#include "pages.h"
#include "users.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// Page_Mass_User_Options
//
// =---------------------------------------------------------------------------

int New_Rating_Value = 1000;

int Page_Mass_User_Options ( )
{
   int   ret          = 0;
   int   Op_Processed = 0;

   char  CmdMassOp   [ VARIABLE_BYTES ];
   char  CmdValue    [ VARIABLE_BYTES ];

   // Mass-User Operations
   // 
   char* MASSOP_RESET_RATINGS = "Reset_Ratings";

   if ( !User_Is_Administrator ( Page_Info.Username ) )
      return -1;

   SS_HTML_Start_Page ( "Iron Dragon Mass User Options" );
   SS_HTML_Heading_Tag ( "Iron Dragon Mass User Options", 1 );

   // Results Page
   //
   if ( cgiFormNotFound != cgiFormStringNoNewlines ( "MassOp", CmdMassOp, VARIABLE_BYTES ) )
   {
      // User selected to reset all rankings
      //
      if ( strcmp ( CmdMassOp, MASSOP_RESET_RATINGS ) == 0 )
      {
         strcpy ( CmdValue, "Nothing" );

         New_Rating_Value = 0;

         if ( cgiFormNotFound != cgiFormStringNoNewlines ( "Value", CmdValue, VARIABLE_BYTES ) )
         {
            if ( New_Rating_Value > 32000 ) New_Rating_Value = 32000;
            else if ( New_Rating_Value < 0 ) New_Rating_Value = 0;
         }
         else
            New_Rating_Value = 1000;

         ret = Action_Enumerate_Users_Callback ( MU_Reset_Rating_Callback, 1 /*write back*/ );

         if ( ret == 0 )
            SS_HTML_WriteP ( "All user rankings are reset to <b>%d</b>, you specified <b>%s</b>.", (int)New_Rating_Value, CmdValue );
         else
            SS_HTML_WriteP ( "There was an error resetting user rankings to <b>%d</b> (you specified <b>%s</b>).", (int)New_Rating_Value, CmdValue );

         Op_Processed = 1;
      }

      if ( !Op_Processed )
         SS_HTML_WriteP ( "Your mass-user operation command was not understood." );

   }

   // Standard Options Page
   //
   if ( !Op_Processed )
   {
      Write_Script_Form ( "Mass_User_Options" );

      SS_HTML_Table_Tag ();

         SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "<th>Command</th>" );
            SS_HTML_Write ( "<td>" );
               SS_HTML_Input_Text ( "MassOp", 0 );
            SS_HTML_Write ( "</td>" );
         SS_HTML_Write ( "</tr>" );

         SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "<th>Value</th>" );
            SS_HTML_Write ( "<td>" );
               SS_HTML_Input_Text ( "Value", 0 );
            SS_HTML_Write ( "</td>" );
         SS_HTML_Write ( "</tr>" );

         SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "<td colspan=\"2\" align=\"center\">" );
               SS_HTML_Submit ( 0 );
            SS_HTML_Write ( "</td>" );
         SS_HTML_Write ( "</tr>" );
      SS_HTML_End_Table_Tag ( );
   }
   
   Write_Admin_Footer    ( );
   SS_HTML_End_Page  ( );

   return ret;
}

// =---------------------------------------------------------------------------
// MU_Reset_Rating_Callback 
//
int MU_Reset_Rating_Callback ( nm_user_record* p_ur )
{
   // youwsa, how do we  write this record back to the server efficiently?
   //
   //p_ur->record.rating = htons ( New_Rating_Value );
   p_ur = 0;
   return 0;
}
