// =---------------------------------------------------------------------------
// p a g e _ a t h e n t i c a t e . c
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
//   00 Feb 11   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "pages.h"

// =---------------------------------------------------------------------------
// P a g e _ A t h e n t i c a t e
// =---------------------------------------------------------------------------
int Page_Authenticate ( )
{
   SS_HTML_Start_Page ( "RE Authentication" );
   SS_HTML_Heading_Tag ( "RE Authentication", 1 );

   Page_Body_Authenticate ( );

   Write_Generic_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ B o d y _ A t h e n t i c a t e
// =---------------------------------------------------------------------------
int Page_Body_Authenticate ( )
{
   SS_HTML_WriteP ( "Please enter your network game Username and Password and click submit." );
   
   SS_HTML_Form ( CGI_ID_SCRIPT );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"Page\" value=\"Main_Menu\">" );
   
      SS_HTML_Table_Tag ( );
         
         SS_HTML_Write ( "<TR>" );
            SS_HTML_Write ( "<TH align=right>Server</TH>" );
            SS_HTML_Write ( "<TD>%s</TD>", Page_Info.Server_Name );
         SS_HTML_Write ( "</TR>" );

         SS_HTML_Write ( "<TR>" );
            SS_HTML_Write ( "<TH align=right>Username</TH>" );
            SS_HTML_Write ( "<TD><INPUT size=30 type=\"text\" name=\"Username\" value=\"\"></TD>" );
         SS_HTML_Write ( "</TR>" );

         SS_HTML_Write ( "<TR>" );
            SS_HTML_Write ( "<TH align=right>Password</TH>" );
            SS_HTML_Write ( "<TD><INPUT size=30 type=\"password\" name=\"Password\" value=\"\"></TD>" );
         SS_HTML_Write ( "</TR>" );

         SS_HTML_Write ( "<TR>" );
            SS_HTML_Write ( "<TH>&nbsp;</TH>" );
            SS_HTML_Write ( "<TD>" );
            SS_HTML_Submit ( "" );
            SS_HTML_Write ( "</TD>" );
         SS_HTML_Write ( "</TR>" );
      SS_HTML_End_Table_Tag ();

   SS_HTML_End_Form ( );

   return 0;
}
