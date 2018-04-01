// =---------------------------------------------------------------------------
// p a g e _ e d i t _ u s e r . c
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
#include "srvstat_actions.h"
#include "pages.h"
#include "net_messages_admin.h"
#include "cgic.h"
#include <stdlib.h>

// =---------------------------------------------------------------------------
// P a g e _ E d i t _ U s e r
//
// =---------------------------------------------------------------------------
int Page_Edit_User ( )
{
   char           EditUserRecnum [VARIABLE_BYTES];
   int            Recnum;
   char*          Page_Name  = "RE Edit User";
   int            ret;
   nm_user_record ur;

   SS_HTML_Start_Page  ( Page_Name    );
   SS_HTML_Heading_Tag ( Page_Name, 1 );

   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "EditUserRecnum",  EditUserRecnum, VARIABLE_BYTES) )
   {
      ret = -1;
      SS_HTML_WriteP ( "The user index was not specified." );
   }
   else
   {
      Recnum = atoi(EditUserRecnum);
      ret = Action_Lookup_User_By_Recnum ( &ur, Recnum );

      if ( ret != 0 )
      {
         SS_HTML_WriteP ( "The user [%d] could not be retrieved due to some error.", Recnum );
      }
      else
      {
         Edit_User_Body ( &ur, Recnum );
         SS_HTML_WriteP ( "Leave the password fields blank if you do not wish to change the password." );
      }
   }


   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );
   
   return ret;
}

// =---------------------------------------------------------------------------
// E d i t _ U s e r _ B o d y
// =---------------------------------------------------------------------------
int Edit_User_Body ( nm_user_record* p_ur, int Recnum )
{
   // Write out the form
   //
   Write_Script_Form ( "Edit_User_Result" );

   Write_User_Inputs ( p_ur, Recnum );
   Write_User_Commands ( );

   SS_HTML_End_Form ( );

   return 0;
}
