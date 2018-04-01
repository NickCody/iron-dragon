// =---------------------------------------------------------------------------
// p a g e _ a d d _ b u g . c
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
//   00 Feb 12   nic   Created.
//

#include <stdlib.h>
#include <stdio.h>
#include "cgi_id_subsystems.h"
#include "net_messages_admin.h"
#include "srvstat_actions.h"
#include "pages.h"
#include "bugrep_actions.h"

// =---------------------------------------------------------------------------
// P a g e _ A d d _ B u g
//
// =---------------------------------------------------------------------------
int Page_Add_Bug ( )
{
   bug_record     bugrec;
   nm_user_record userrec;
   int            ret;
   //char           Email[32];
   //char           Name_String[80];

   SS_Port_ZeroMemory ( &bugrec, sizeof(bug_record) );

   // Fill in user's e-mail address if we can
   //
   ///*
   ret = Action_Lookup_User ( &userrec, Page_Info.Username );

   if ( ret == 0 )
   {
      strcpy ( bugrec.Email, userrec.record.email );
      //sprintf ( Name_String, "%s %s", userrec.fname, userrec.lname );
   }
   else
   {
      strcpy ( bugrec.Email, "" );
      //strcpy ( Name_String, "" );
   }

   strcpy ( bugrec.Username, Page_Info.Username );

   // Set dropdowns to their defaults
   bugrec.OS        = BUGOS_WINDOWS98SE;
   bugrec.Bug_Type  = BUGTYPE_BUG;
   bugrec.Status    = BUGSTATUS_OPEN;
   bugrec.Priority  = BUGPRIORITY_MEDIUM; 
   bugrec.Category  = BUGCAT_GAMEPLAY;

   strcpy ( bugrec.Description, "Please type in a general description of the bug and any notes you have about the bug. We will cut off anything over 4000 characters." );
   strcpy ( bugrec.Repro_Steps, "Please enumerate the exact steps (if possible) to reproduce the problem. Leave this blank if you are submitting a suggestion. We will cut off anything over 2000 characters." );

   // Write out the bug submittal form...
   //
   SS_HTML_Start_Page ( "RE Bug Submittal Form" );
   SS_HTML_Heading_Tag ( "RE Bug Submittal Form", 1 );

   SS_HTML_WriteP ( "Please enter the bug details and hit the Go! button" );
   Write_Script_Form ( "Add_Bug_Confirm" );
   
   Write_Bug_Inputs ( &bugrec, 0xFFFFFFFF );

   SS_HTML_End_Form (  );

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return 0;
}