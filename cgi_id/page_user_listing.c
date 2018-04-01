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

#include "cgi_id_subsystems.h"
#include "net_messages_admin.h"
#include "srvstat_actions.h"
#include "pages.h"
#include "users.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// P a g e _ U s e r _ L i s t i n g
//
// =---------------------------------------------------------------------------
int Page_User_Listing ( )
{
   int ret;
   int pagenum;
   char buf [ VARIABLE_BYTES ];
   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "pagenum",  buf, VARIABLE_BYTES) )
       pagenum = 0;
   else
       pagenum = atoi(buf);

   SS_HTML_Start_Page ( "RE User Listing" );
   SS_HTML_Heading_Tag ( "RE User Listing", 1 );

   if ( 0&& User_Is_Administrator(Page_Info.Username) )
   {
      Write_Script_Form ( "Mass_User_Options" );
      SS_HTML_Table_Tag ( );
         SS_HTML_Write ( "<tr><td valign=\"middle\">Mass User Options</td>" );
         SS_HTML_Write ( "<td valign=\"middle\">" );
         SS_HTML_Submit ( 0 );
         SS_HTML_Write ( "</td></tr>" );
      SS_HTML_End_Table_Tag ( );
      SS_HTML_End_Form ( );
   }
   
   // We reply on the action to report any error messages
   //
   ret = Action_Enumerate_Users ( pagenum );

   Write_Admin_Footer    ( );
   SS_HTML_End_Page  ( );

   return ret;
}