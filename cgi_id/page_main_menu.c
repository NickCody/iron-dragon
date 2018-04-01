// =---------------------------------------------------------------------------
// p a g e _ m a i n _ m e n u . c
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

#include <stdio.h>
#include "cgi_id_subsystems.h"
#include "bugrep_actions.h"
#include "pages.h"
#include "connection_zone.h"
#include "users.h"
#include "db_system_settings.h"

// =---------------------------------------------------------------------------
// P a g e _ E d i t _ B u g
//
// =---------------------------------------------------------------------------
int Page_Main_Menu ( )
{
   char Link[LINK_BYTES];

   SS_HTML_Start_Page ( "RE Administration" );
   SS_HTML_Heading_Tag ( "RE Administration", 1 );

   SS_HTML_Table_Tag ( );

   // Daemon Stats Link
   //
   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "    <th align=right>See Daemon Statistics</th>" );
   SS_HTML_Write ( "    <td colspan=2>" );
                           Format_Script_Link ( Link, "Daemon_Stats" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </td>" );
   SS_HTML_Write ( "</tr>" );


   // News maintenance Link
   //

   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "    <th align=right>News Maintenance</th>" );
      SS_HTML_Write ( "    <td colspan=2>" );
                              Format_Script_Link ( Link, "NewsAdmin" );
                              SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
      SS_HTML_Write ( "    </td>" );
      SS_HTML_Write ( "</tr>" );
   }

   // FAQ maintenance Link
   //

   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "    <th align=right>FAQ Maintenance</th>" );
      SS_HTML_Write ( "    <td colspan=2>" );
                              Format_Script_Link ( Link, "FAQAdmin" );
                              SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
      SS_HTML_Write ( "    </td>" );
      SS_HTML_Write ( "</tr>" );
   }

   // User Listing Link
   //
   SS_HTML_Write ( "<TR>" );
   if ( User_Is_Administrator ( Page_Info.Username ) )
      SS_HTML_Write ( "    <TH align=right>User Maintenance</TH>" );
   else
      SS_HTML_Write ( "    <TH align=right>See a User Listing</TH>" );

   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "User_Listing" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );

   // System Settings Link
   //
   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<TR>" );
      SS_HTML_Write ( "    <TH align=right>System Settings</TH>" );
      SS_HTML_Write ( "    <TD colspan=2>" );
                              Format_Script_Link ( Link, "SysSet" );
                              SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
      SS_HTML_Write ( "    </TD>" );
      SS_HTML_Write ( "</TR>" );
   }

   // System Settings Link
   //
   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<TR>" );
      SS_HTML_Write ( "    <TH align=right>Log Monitor</TH>" );
      SS_HTML_Write ( "    <TD colspan=2>" );
                              Format_Script_Link ( Link, "LogMon" );
                              SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
      SS_HTML_Write ( "    </TD>" );
      SS_HTML_Write ( "</TR>" );
   }

   // Site Documentation
   //
   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<TR>" );
      SS_HTML_Write ( "    <TH align=right>Site Documentation</TH>" );
      SS_HTML_Write ( "    <TD colspan=2>" );
                              //Format_Script_Link ( Link, "Admin_Doc_Request" );
                              sprintf ( Link, "/admin_docs/EdenWebsite.htm" );
                              SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
      SS_HTML_Write ( "    </TD>" );
      SS_HTML_Write ( "</TR>" );
   }

   // Bug Repository Link
   //

   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>%s Visit the Bug Repository</TH>", Get_Bug_Icon() );
   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "Bug_Repository" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );

   // Beta Download Area
   //
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Beta Download</TH>" );
   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "Download;Direct=3" /*DT_BETADOWNLOAD*/ );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );

   // We do this only if this system setting is defined
   //
   if ( SysSet_Get_Integer_Value ( "Beta_Tester_License_Request" ) )
   {
      // License File Mailing
      //
      SS_HTML_Write ( "<TR>" );
      SS_HTML_Write ( "    <TH align=right>Request a License File</TH>" );
      SS_HTML_Write ( "    <TD colspan=2>" );
                              Format_Script_Link ( Link, "Request_Beta_License" );
                              SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
      SS_HTML_Write ( "    </TD>" );
      SS_HTML_Write ( "</TR>" );
   }

   SS_HTML_End_Table_Tag ();

   Write_Admin_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}
