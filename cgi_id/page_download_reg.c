// =---------------------------------------------------------------------------
// p a g e _ d o w n l o a d _ r e g. c
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
//   00 Mar 28   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "pages.h"
#include "srvstat_actions.h"


// =---------------------------------------------------------------------------
// P a g e _ D o w n l o a d _ R e g
// =---------------------------------------------------------------------------
int Page_Download_Reg ( )
{
   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Game Download Registration" );

   //SS_HTML_Heading_Tag ( "Rail Empires: Iron Dragon - Game Download Registration", 1 );
   SS_HTML_Heading_Tag ( "Game Download Registration", 1 );

   // Describe the page...
   //
   Write_ParaTable ( PT_STANDARD );
      SS_HTML_Write ( "Please fill out this form for us. If you fill out all fields with a red asterisk" );
      Write_Required_Field_Asterisk ( );
      SS_HTML_Write ( ", you wil receive a license file at the e-mail address you specify. This license file will fully unlock the game for 30 days (including online play!)" );
   Write_End_ParaTable ( );

   Write_ParaTable ( PT_STANDARD );
      SS_HTML_Write ( "If you do not fill out the required fields, the downloaded game can be enjoyed for only 30 turns and you cannot play online." );
   Write_End_ParaTable ( );

   // Write out the Registration Form
   //
   Write_Script_Form ( "Download" );
   SS_HTML_Input_Hidden ( "Direct", "1" );
   Write_User_Download_Form_Fields ( );
   SS_HTML_End_Form (  );

   //Write_Under_Construction ( );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

