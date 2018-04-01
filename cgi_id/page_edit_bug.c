// =---------------------------------------------------------------------------
// p a g e _ e d i t _ b u g .c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
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
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Feb 07   nic   Created.
//

#include <stdlib.h>
#include "cgi_id_subsystems.h"
#include "bugrep_actions.h"
#include "pages.h"
#include "connection_zone.h"
#include "cgic.h"

int Page_Body_Request_Bugid ( );

// =---------------------------------------------------------------------------
// P a g e _ E d i t _ B u g
//
// =---------------------------------------------------------------------------
int Page_Edit_Bug ( )
{
   char        Bugid_Variable [ VARIABLE_BYTES ];
   uint_32     recnum;
   bug_record  bugrec;
   int         ret = 0;

   SS_Port_ZeroMemory ( &bugrec, sizeof(bug_record) );

   SS_HTML_Start_Page ( "RE Edit Bug" );
   SS_HTML_Heading_Tag ( "RE Edit Bug", 1 );

   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "Bugid", Bugid_Variable, VARIABLE_BYTES ) )
   {
      ret = Page_Body_Request_Bugid ( );
   }
   else
   {
      SS_HTML_WriteP ( "Please enter the bug details and hit the Go! button" );
   
      Write_Script_Form ( "Edit_Bug_Result" );

      // Lookup the bug they requested
      recnum = (uint_32)atoi(Bugid_Variable);
      Action_Lookup_Bug ( &bugrec, recnum );

      // Generate the form variables for them
      Write_Bug_Inputs ( &bugrec, recnum );

      SS_HTML_End_Form (  );
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return ret;
}


// =---------------------------------------------------------------------------
// P a g e _ B o d y _ R e q u e s t _ B u g i d
// =---------------------------------------------------------------------------
int Page_Body_Request_Bugid ( )
{
   SS_HTML_WriteP ( "Please specify the Bug ID." );
   
   Write_Script_Form ( "Edit_Bug" );

   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Bug Number</TH>" );
   SS_HTML_Write ( "    <TD>" );
                           SS_HTML_Input_Text ( "Bugid", 0 );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "    <TD>" );
                           SS_HTML_Submit ( "" );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );
   
   SS_HTML_End_Table_Tag ( );

   SS_HTML_End_Form ( );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ E d i t _ B u g _ R e s u l t
// =---------------------------------------------------------------------------
int Page_Edit_Bug_Result ( )
{
   int         ret = 0;
   uint_32     recnum;
   bug_form    bugform;
   bug_record  bugrec;
   char        Bugid_Variable [ VARIABLE_BYTES ];
   char        Link [ LINK_BYTES ];

   SS_HTML_Start_Page ( "RE Edit Bug Results" );
   SS_HTML_Heading_Tag ( "RE Edit Bug Results", 1 );

   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "Bugid", Bugid_Variable, VARIABLE_BYTES ) )
   {
      SS_HTML_WriteP ( "The Bug ID was not specified." );
   }
   else
   {
      ret = Action_Gather_Bug_Fields ( &bugform );
      
      if ( ret == 0 )
      {
         recnum = (uint_32)atoi(Bugid_Variable);
         Bug_Form_To_Record ( &bugform, &bugrec, 0 );

         ret = Action_Update_Bug ( &bugrec, recnum );

         if ( ret != 0 )
            SS_HTML_WriteP ( "There was an error updating the database." );
         else
         {
            Format_Script_Link ( Link, "Find_Bug" );
            strcat ( Link, "&Bugid=" );
            strcat ( Link, Bugid_Variable );
            SS_HTML_WriteP ( "Your modification to <A href=\"%s\">Bug %04d</A> was successful.", Link, (int)recnum );
         }
      }
      else
      {
         SS_HTML_WriteP ( "All required fields were not entered!" );
      }
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return ret;
}