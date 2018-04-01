// =---------------------------------------------------------------------------
// p a g e _ a d d  _ u s e r _ r e s u l t . c
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
//   00 Jan 26   nic   Created.
//


#include "cgi_id_subsystems.h"
#include "srvstat_actions.h"
#include "pages.h"
#include "srvstat_actions.h"

// =---------------------------------------------------------------------------
// P a g e _ A d d _ U s e r _ R e s u l t
//
// =---------------------------------------------------------------------------
int Page_Add_User_Result  ( )
{
   int            ret;
   uint_32        index;
   uint_08        Success_Code;
   nm_user_record user_rec;
   user_form      uf;

   SS_Port_ZeroMemory ( &uf, sizeof(user_form) );
   SS_Port_ZeroMemory ( &user_rec, sizeof(nm_user_record) );

   SS_HTML_Start_Page  ( "RE Add User Results" );
   SS_HTML_Heading_Tag ( "RE Add User Results", 1 );

   // Get user criteria from form submission
   //
   if ( Action_Gather_User_Fields ( &uf, 0 /*enforce required*/ ) == -1 )
   {
      SS_HTML_WriteP ( "Not all required fields were filled in." );
      return -1;
   }

   // Physically add the user
   //
   ret = Action_Add_User ( &uf, &user_rec, &Success_Code );

   if ( ret == -1 )
   {
      SS_HTML_WriteP ( "The user could not be added due to a serious error." );
   }
   else
   {

      index = ntohl(*(uint_32*)user_rec.index);

      if ( Success_Code == 0 )
      {
         SS_HTML_WriteP ( "The following user was added successfully to index %d.", index );
         Write_User ( &user_rec );
      }
      else if ( index != -1 )
      {
         SS_HTML_WriteP ( "The username was not unique. Here is the current user:" );
         Write_User ( &user_rec );
      }
      else
      {
         SS_HTML_WriteP ( "User could not be added. Perhaps your passwords did not match?" );
      }

   }

   Write_Admin_Footer ( );
   SS_HTML_End_Page ( );

   return ret;
}

