// =---------------------------------------------------------------------------
// p a g e _ e d i t u s e r . c
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
//   00 Jan 24   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "pages.h"
#include "net_messages_admin.h"
#include "connection_zone.h"
#include "srvstat_actions.h"
#include "users.h"

// =---------------------------------------------------------------------------
// P a g e _ A d d _ U s e r
//
// =---------------------------------------------------------------------------
int Page_Add_User ( )
{
   nm_user_record ur;

   SS_Port_ZeroMemory ( &ur, sizeof(nm_user_record) );

   SS_HTML_Start_Page  ( "RE Add New User" );
   SS_HTML_Heading_Tag ( "RE Add New User", 1 );

   if ( !User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_WriteP ( "We're sorry, but only <B>administrators</B> can add users." );
   }
   else
   {
      // Write out the form
      //
      Write_Script_Form ( "Add_User_Result" );

      Write_User_Inputs ( &ur, 1 /*new user*/ );

      SS_HTML_End_Form ( );
   }

   Write_Admin_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}
