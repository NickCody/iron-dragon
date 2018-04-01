// =---------------------------------------------------------------------------
// p a g e _ a d d _ b u g _ r e s u l t . c
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

#include "cgi_id_subsystems.h"
#include "bugrep_actions.h"
#include "pages.h"
#include "connection_zone.h"
#include "databases.h"

// =---------------------------------------------------------------------------
// P a g e _ A d d _ B u g_ C o n f i r m
//
// =---------------------------------------------------------------------------
int Page_Add_Bug_Confirm ( )
{
   int                        ret;
   //nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;
   bug_form                   report;
 
   SS_HTML_Start_Page ( "RE Bug/Suggestion Confirmation" );
   SS_HTML_Heading_Tag ( "RE Bug/Suggestion Confirmation", 1 );

   if ( Page_Info.Server_Sock > 0 )
   {
      ret = Action_Gather_Bug_Fields ( &report );

      if ( ret == 0 /*success*/ )
      {
         SS_HTML_WriteP ( "Hit your browsers back button if you made a mistake." );

         ret = Write_Bug_Confirmation_Table ( &report );

         if ( ret == 0 )
         {
            // Confirmation Form
            //
            Write_Script_Form ( "Add_Bug_Result" );
            Write_Bug_Form_Variables ( &report );
            SS_HTML_Submit ( "" );
            //SS_HTML_Write ( "<INPUT type=submit value=\"Submit\">" );
            SS_HTML_End_Form ( );
         }
      }

      if ( ret != 0 )
      {
         SS_HTML_WriteP ( "We experienced some kind of failure." );
      }
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return 0; // Success
}

// =---------------------------------------------------------------------------
// P a g e _ A d d _ B u g _ R e s u l t
//
// =---------------------------------------------------------------------------
int Page_Add_Bug_Result ( )
{
   int         ret   = 0;
   uint_32     bugid = 0;
   bug_form    report;
   bug_record  bugrec;

   SS_HTML_Start_Page ( "RE Bug/Suggestion Result" );
   SS_HTML_Heading_Tag ( "RE Bug/Suggestion Result", 1 );

   ret = Action_Gather_Bug_Fields ( &report );

   if ( ret == 0 /*success*/ )
   {
      ret = Action_Add_Bug ( &report, &bugrec /*outparam*/ , &bugid );

      if ( ret == 0 )
      {
         SS_HTML_WriteP ( "We succeeded in adding your bug/suggestion. The reference number is %d.", bugid );
         SS_HTML_WriteP ( "You will be notified when the bug is resolved. Have a nice day!" );

         // Notify the Rail-Tester listserv if this is a real bug (not testbed bug)
         //
         if ( SysSet_Get_Integer_Value ( "Notify_Rail-Tester_On_Bug" ) )
         {
            ret = Action_Send_New_Bug_Mail ( &report, &bugrec, bugid );
            if ( ret == 0 ) SS_HTML_WriteP ( "The Rail-Tester Listserv was notified successfully." );
            else            SS_HTML_WriteP ( "<STRONG>We failed to notify the Rail-Tester Listserv!</STRONG>" );
         }
      }
      else
      {
         SS_HTML_WriteP ( "We <I>failed</I> to add this bug:", bugid );

         ret = Write_Bug_Confirmation_Table ( &report );
         
         // Give the user a chance to try again
         //
         Write_Script_Form ( "Add_Bug_Result" );
         Write_Bug_Form_Variables ( &report );
         SS_HTML_Submit ( "" );
         SS_HTML_Write ( "(try again)" );
         SS_HTML_End_Form ( );
      }
   }

   if ( ret != 0 )
   {
      SS_HTML_WriteP ( "We experienced some kind of failure" );
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return 0; // Success
}