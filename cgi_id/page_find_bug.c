// =---------------------------------------------------------------------------
// p a g e _ f i n d _ b u g .c
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

// =---------------------------------------------------------------------------
// P a g e _ F i n d _ B u g
//
// =---------------------------------------------------------------------------
int Page_Find_Bug ( )
{
   char        Form_Bugid[VARIABLE_BYTES];
   uint_32     Bugid;
   int         ret = 0;
   bug_record  bug;
   //nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;

   SS_HTML_Start_Page ( "RE Bug Details" );
   SS_HTML_Heading_Tag ( "RE Bug Details", 1 );

   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "Bugid", Form_Bugid, VARIABLE_BYTES ) )
   {
      ret = Page_Body_Request_Bug_Search_Criteria ( );
   }
   else
   {
      Bugid = (uint_32)atoi ( Form_Bugid );

      if ( Page_Info.Server_Sock > 0 )
      {
         ret = Action_Lookup_Bug ( &bug, Bugid );
   
         if ( ret == 0 )
         {
            Write_Bug_Detail_Table ( &bug, Bugid );
         }
         else
         {
            SS_HTML_WriteP ( "Could not locate Bug number %d.", (int)Bugid );
         }
      }
   }

   if ( ret != 0 )
      SS_HTML_WriteP ( "There was some error processing your request." );

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ B o d y _ R e q u e s t _ B u g _S e a r c h _ C r i t e r i a
// =---------------------------------------------------------------------------
int Page_Body_Request_Bug_Search_Criteria ( )
{
   SS_HTML_WriteP ( "Please specify search criteria." );
   
   Write_Script_Form ( "Find_Bug" );

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