// =---------------------------------------------------------------------------
// p a g e _ e l e m e n t s . c
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


#include "cgic.h"
#include "cgi_id_subsystems.h"
#include "cgi_id_definitions.h"

#include "pages.h"
#include "connection_zone.h"
#include "bugrep_actions.h"
#include "databases.h"
#include "users.h"

const char* PT_STANDARD      = "classStandard";
const char* PT_STANDARDLEFT  = "classStandardLeft";
const char* PT_NEWS          = "classNews";
const char* PT_FAQ           = "classFAQ";

// =---------------------------------------------------------------------------
// W r i t e _ A d m i n  _ F o o t e r
//
// =---------------------------------------------------------------------------
void Write_Admin_Footer ( )
{
   char Link[LINK_BYTES];

   SS_HTML_Write ( "<p>" );

   SS_HTML_Table_Tag_Ex ( "width=\"500\"" );

   SS_HTML_Write ( "<tr>" );

      Format_Script_Link ( Link, "Main_Menu" );
      SS_HTML_Write ( "<td align=center><a href=\"%s\">Main Menu</a></td>", Link );

      Format_Script_Link ( Link, "Daemon_Stats" );
      SS_HTML_Write ( "<td align=center><a href=\"%s\">Daemon Stats</a></td>", Link );

      Format_Script_Link ( Link, "Bug_Repository" );
      SS_HTML_Write ( "<td align=\"center\"><table cellpadding=\"0\" cellspacing=\"0\"><tr>" );
      SS_HTML_Write ( "<td>%s</td><td><a href=\"%s\">Bug Repository</A></td>", Get_Bug_Icon(), Link );
      SS_HTML_Write ( "</tr></table></td>" );

      Format_Script_Link ( Link, "User_Listing" );
      SS_HTML_Write ( "<td align=center><a href=\"%s\">User Listing</A></td>", Link );
   
      if ( User_Is_Administrator ( Page_Info.Username ) )
      {
         Format_Script_Link ( Link, "Add_User" );
         SS_HTML_Write ( "<td align=center><a href=\"%s\">Add User</A></td>", Link );

         Format_Script_Link ( Link, "NewsAdmin" );
         SS_HTML_Write ( "<td align=center><a href=\"%s\">News Admin</A></td>", Link );

         Format_Script_Link ( Link, "FAQAdmin" );
         SS_HTML_Write ( "<td align=center><a href=\"%s\">FAQ Admin</A></td>", Link );

         Format_Script_Link ( Link, "SysSet" );
         SS_HTML_Write ( "<td align=center><a href=\"%s\">System Settings</A></td>", Link );
      }

      strcpy ( Link, CGI_ID_SCRIPT );
      SS_HTML_Write ( "<td align=center><a href=\"%s?Page=Authenticate\"><B>Logoff %s</B></A></td>", Link, Page_Info.Username );

   SS_HTML_Write ( "</tr>" );

   SS_HTML_End_Table_Tag ();

   Write_Generic_Footer ( );
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ F o o t e r
// =---------------------------------------------------------------------------
void Write_User_Footer ( )
{
   // TODO: Replace with stylesheets
   SS_HTML_Write ( "<font size=\"2\">" );
   Write_User_Link_Bar ( );
   SS_HTML_Write ( "</font>" );

   Write_Generic_Footer ( );
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ L i n k _ B a r
//
// =---------------------------------------------------------------------------
void Write_User_Link_Bar ( )
{
   char Page [ SYSSET_VALUE_BYTES ];

   SS_HTML_Write ( "<p>" );

   //Write_User_Anchor ( "Homepage", "Homepage" );
   //SS_HTML_Write ( "&nbsp;-&nbsp;" );

   if ( 0 == SysSet_Get_Value ( "Product_Description_FIlename", Page ) )
   {
      SS_HTML_Write ( "<a href=\"%s\">Product Description</a>", Page );
      SS_HTML_Write ( "&nbsp;-&nbsp;" );
   }

   Write_User_Anchor ( "News", "News" );
   SS_HTML_Write ( "&nbsp;-&nbsp;" );

   Write_User_Anchor ( "Download", "Demo" );
   SS_HTML_Write ( "&nbsp;-&nbsp;" );

   //Write_User_Anchor ( "Ranking", "Rankings" );
   //SS_HTML_Write ( "&nbsp;-&nbsp;" );

   if ( 0 == SysSet_Get_Value ( "Ranking_Page_URL", Page ) )
   {
      SS_HTML_Write ( "<a href=\"%s\">Rankings</a>", Page );
      SS_HTML_Write ( "&nbsp;-&nbsp;" );
   }


   Write_User_Anchor ( "Support", "Support" );
   SS_HTML_Write ( "&nbsp;-&nbsp;" );

   Write_User_Anchor ( "Order", "Ordering" );

   SS_HTML_Write ( "</p>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ G e n e r i c _ F o o t e r
//
// =---------------------------------------------------------------------------
void Write_Generic_Footer ( )
{
   SS_HTML_Write ( "<hr>" );

   if ( Page_Family == PF_USERPAGES )
   {
      SS_HTML_Table_Tag ( );

      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<td>" );
      SS_HTML_Write ( "    <p>" );
      SS_HTML_Write ( "    <a href=\"http://www.edenstudios.net\">" );
      SS_HTML_Write ( "        <IMG src=\"http://www.irondragon.org/styles/images/EdenLogo3.gif\"" );
      SS_HTML_Write ( "             alt=\"Eden Studios, Inc.\">" );
      SS_HTML_Write ( "    </a>" );
      SS_HTML_Write ( "    </p>" );
      SS_HTML_Write ( "</td>" );
      SS_HTML_Write ( "<td>" );
      SS_HTML_Write ( "    <p>" );
      SS_HTML_Write ( "    <a href=\"http://www.coolgames.com\">" );
      SS_HTML_Write ( "        <IMG src=\"http://www.irondragon.org/styles/images/MayfairLogo2.gif\"" );
      SS_HTML_Write ( "             alt=\"Mayfair Games, Inc.\">" );
      SS_HTML_Write ( "    </a>" );
      SS_HTML_Write ( "    </p>" );
      SS_HTML_Write ( "</td>" );

      // GO Forum link
      //

      SS_HTML_Write ( "<td>" );
      SS_HTML_Write ( "    <p>" );
      SS_HTML_Write ( "    <a href=\"http://www.gamingoutpost.com/forums/default.cfm?site=IronDragon\">" );
      SS_HTML_Write ( "        <IMG src=\"http://www.irondragon.org/go.gif\"" );
      SS_HTML_Write ( "             alt=\"Rail Empires on GO\">" );
      SS_HTML_Write ( "    </a>" );
      SS_HTML_Write ( "    </p>" );
      SS_HTML_Write ( "</td>" );

      // ZDDownloads.com
      //

      SS_HTML_Write ( "<td>" );
      SS_HTML_Write ( "    <p>" );
      SS_HTML_Write ( "    <a href=\"http://www.zdnet.com/downloads/stories/info/0,,001C0R,.html\">" );
      SS_HTML_Write ( "        <IMG src=\"http://www.irondragon.org/4star2000.gif\"" );
      SS_HTML_Write ( "             alt=\"ZD Net Downloads\">" );
      SS_HTML_Write ( "    </a>" );
      SS_HTML_Write ( "    </p>" );
      SS_HTML_Write ( "</td>" );

      SS_HTML_Write ( "</tr>" );

      SS_HTML_End_Table_Tag ( );
   }
   else if ( Page_Family == PF_ADMINPAGES )
   {
      SS_HTML_Write ( "<p>" );
      SS_HTML_Write ( "<a href=\"http://www.edenstudios.net\">" );
      SS_HTML_Write ( "    <IMG src=\"http://www.irondragon.org/styles/images/EdenLogo2.gif\"" );
      SS_HTML_Write ( "         alt=\"Eden Studios, Inc.\">" );
      SS_HTML_Write ( "</A>" );
      SS_HTML_Write ( "</p>" );
   }

   if ( Page_Family == PF_ADMINPAGES )
      SS_HTML_WriteP ( "ID Web Server Version <b>%s</b> at <b>%s</b> talking to ID Daemon at <b>%s</b>.", CGI_ID_VERSION, Web_Vars.cgiServerName, Page_Info.Server_Name );


   if ( Page_Family == PF_USERPAGES )
   {
      SS_HTML_WriteP ( "<font size=\"2\">Please read our <a href=\"http://www.edenstudios.net/privacy.htm\">Privacy Policy</a>.</font>" );
   }
   // For Testbed purposes only
   // 
   if ( SysSet_Get_Integer_Value ( "Show_HTML_Validation" ) )
   {
      // W3C HTML 4.0 Validation
      //
      SS_HTML_Write ( "<p>" );
      SS_HTML_Write ( "<a href=\"http://validator.w3.org/check/referer\">" );
      SS_HTML_Write ( "    <img src=\"http://www.irondragon.org/styles/images/html40.gif\" " );
      SS_HTML_Write ( "         alt=\"Valid HTML 4.0!\" height=31 width=88>" );
      SS_HTML_Write ( "</a>" );
      SS_HTML_Write ( "</p>" );
   }
}

// =---------------------------------------------------------------------------
// Format_Script_Link
// =---------------------------------------------------------------------------
void Format_Script_Link ( char* Link_Buf, const char* Page )
{
	sprintf ( Link_Buf, "%s?Page=%s;Username=%s;Password=%s",
      CGI_ID_SCRIPT,
      Page,
      Page_Info.Username, 
      Page_Info.Password );
}

// =---------------------------------------------------------------------------
// W r i t e _ S c r i p t _ F o r m 
// =---------------------------------------------------------------------------
void Write_Script_Form ( const char* Page )
{
   SS_HTML_Form ( CGI_ID_SCRIPT );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"Page\"        value=\"%s\">", Page                       );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"Username\"    value=\"%s\">", Page_Info.Username         );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"Password\"    value=\"%s\">", Page_Info.Password         );
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ A n c h o r
// =---------------------------------------------------------------------------
void Write_User_Anchor ( const char* Page, const char* Text )
{
   SS_HTML_Write ( "<a href=\"%s?Page=%s\">%s</A>", CGI_ID_SCRIPT, Page, Text );
}

// =---------------------------------------------------------------------------
// W r i t e _ U n d e r _ C o n s t r u c t i o n  
// =---------------------------------------------------------------------------
void Write_Under_Construction ( )
{
   SS_HTML_Image ( "/styles/images/under_cons.gif", "Under Construction" );
   SS_HTML_Heading_Tag ( "This page is currently under construction!", 3 );
}

// =---------------------------------------------------------------------------
// W r i t e _ R e q u i r e d _ F i e l d _ A s t e r i s k
// =---------------------------------------------------------------------------
void Write_Required_Field_Asterisk ( )
{
   if ( Page_Family == PF_USERPAGES )
      SS_HTML_Write ( "<IMG src=\"/styles/images/redasterisk_onblack.gif\" alt=\"Required Field\">" );
   else
      SS_HTML_Write ( "<IMG src=\"/styles/images/redasterisk_onwhite.gif\" alt=\"Required Field\">" );
}

// =---------------------------------------------------------------------------
// Write_Row_Input
// =---------------------------------------------------------------------------
void Write_Row_Input ( const char* Header, const char* Input_Value, int Required )
{
   SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<Th align=\"right\">%s</th>", Header );
      
      SS_HTML_Write ( "<td>" );
      SS_HTML_Input_Text ( Input_Value, 0 );

      if ( Required )
         Write_Required_Field_Asterisk ( );

      SS_HTML_Write ( "</td>" );
   SS_HTML_Write ( "</tr>" );
}

// =---------------------------------------------------------------------------
// Write_Row_Input_Password
// =---------------------------------------------------------------------------
void Write_Row_Input_Password ( const char* Header, const char* Input_Value, int Required )
{
   SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<Th align=\"right\">%s</th>", Header );
      
      SS_HTML_Write ( "<td>" );
      SS_HTML_Input_Password ( Input_Value, 0 );

      if ( Required )
         Write_Required_Field_Asterisk ( );

      SS_HTML_Write ( "</td>" );
   SS_HTML_Write ( "</tr>" );
}

// =---------------------------------------------------------------------------
// Write_ParaTable
// =---------------------------------------------------------------------------
void Write_ParaTable ( const char* Class )
{
   SS_HTML_Write ( "<table class=\"%s\" cellspacing=\"0\">", Class );
   SS_HTML_Write ( "<tr><td class=\"%s\">", Class );
}

// =---------------------------------------------------------------------------
// Write_ParaTable_Ex
// =---------------------------------------------------------------------------
void Write_ParaTable_Ex ( const char* Class, const char* TABLE_Extra, const char* TD_Extra )
{
   SS_HTML_Write ( "<table class=\"%s\" %s cellspacing=\"0\">", Class, TABLE_Extra ? TABLE_Extra : "" );

   SS_HTML_Write ( "<tr><td class=\"%s\" %s>", Class, TD_Extra ? TD_Extra : "" );

}

// =---------------------------------------------------------------------------
// W r i t e _ E n d _ P a r a T a b l e
// =---------------------------------------------------------------------------
void Write_End_ParaTable ( )
{
   SS_HTML_Write ( "</td></tr>" );
   SS_HTML_End_Table_Tag ( );
}

// =---------------------------------------------------------------------------
// L o a d _ A n d _ W r i t e _ H T M L _ D o c u m e n t
//
// Loads the html document at system-defined path and with parameter filename
// Strips out all outside <BODY> tag, discarding <BODY> tag element as well
// Path is assumed to end in /
// =---------------------------------------------------------------------------
int Load_And_Write_HTML_Document ( const char* filename )
{
   char  Path [ SYSSET_VALUE_BYTES ];
   char* Source_File  = 0;
   char* Dest_File    = 0;
   int   File_Handle  = 0;
   struct stat File_Stats;

   if ( 0 != SysSet_Get_Value ( "HTML_Fragment_Path", Path ) )
      Path[0] = 0;

   strcat ( Path, filename );

   File_Handle = SS_File_IO_Open ( Path, 0 /*readonly*/ );
   
   if ( File_Handle == -1 /*error*/ ) return -1;

   SS_Port_ZeroMemory ( &File_Stats, sizeof(struct stat) );
   SS_File_IO_Stat ( File_Handle, &File_Stats );

   //if ( File_Stats.st_size <= 0 || File_Stats.st_size > MAX_HTML_FRAGMENT

   return -1;
}