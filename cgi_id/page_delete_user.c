// =---------------------------------------------------------------------------
// p a g e _ d e l e t e _ u s e r . c
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
//   00 Feb 03   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "srvstat_actions.h"
#include "pages.h"
#include "net_messages_admin.h"
#include "cgic.h"
#include <stdlib.h>

// =---------------------------------------------------------------------------
// P a g e _ D e l e t e _ U s er
//
// =---------------------------------------------------------------------------
int Page_Delete_User ( )
{
   char DelUserRecnum [ VARIABLE_BYTES ];
   int  Recnum = -1;
   int  ret = -1;


   SS_HTML_Start_Page ( "RE Delete User Results" );
   SS_HTML_Heading_Tag ( "RE Delete User Results", 1 );

   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "DelUserRecnum",  DelUserRecnum, VARIABLE_BYTES) )
   {
      SS_HTML_WriteP ( "The user record number was not specified." );
   }
   else
   {
      Recnum = atoi(DelUserRecnum);
      ret = Action_Delete_User_By_Recnum ( Recnum );

      if ( ret == 0 )
      {
         SS_HTML_WriteP ( "The user [%d] was successfully removed.", Recnum );
         ret = Action_Enumerate_Users (0);
      }
      else
      {
         SS_HTML_WriteP ( "The user [%d] could <I>not</I> be removed.", Recnum );
      }
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return ret;
}
