// =---------------------------------------------------------------------------
// p a g e _ h o m e p a g e . c
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
//   00 Mar 22   nic   Created.
//

#include <stdio.h>
#include "cgi_id_subsystems.h"
#include "pages.h"
#include "databases.h"

int Write_WebRing_Fragment ( );


// =---------------------------------------------------------------------------
// P a g e _ H o m e p a g e
//
// =---------------------------------------------------------------------------
int Page_Homepage ( )
{
   SS_HTML_Start_Page ( "Eden's Rail Empires: Iron Dragon Homepage" );

   Write_User_Link_Bar ( );

   SS_HTML_Heading_Tag ( "Welcome to Eden&#146;s Rail Empires: Iron Dragon Homepage", 1 );

   
   SS_HTML_Image ( "http://edenstudios.net/irondragon/Idtest2.jpg", "Rail Empires: Iron Dragon" );

   Write_WebRing_Fragment ( );

   Write_Generic_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ F o r u m s
//
// =---------------------------------------------------------------------------
int Page_Forums ( )
{
   SS_HTML_Start_Page ( "Eden's Rail Empires: Iron Dragon Forums" );

   Write_User_Link_Bar ( );

   SS_HTML_Heading_Tag ( "Welcome to Eden&#146;s Rail Empires: Iron Dragon Forums", 1 );

   SS_HTML_Table_Tag ( );

   // GO Forum link
   //

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "    <td align=\"center\">" );
   SS_HTML_Write ( "        <p>" );
   SS_HTML_Write ( "        <a href=\"http://www.gamingoutpost.com/forums/default.cfm?site=IronDragon\">" );
   SS_HTML_Write ( "            <IMG src=\"http://www.irondragon.org/go.gif\"" );
   SS_HTML_Write ( "                 alt=\"Rail Empires on GO\">" );
   SS_HTML_Write ( "        </a>" );
   SS_HTML_Write ( "        </p>" );
   SS_HTML_Write ( "    </td>" );
   SS_HTML_Write ( "    <td>Talk about Rail Empires at the Gaming Outpost</td>" );
   SS_HTML_Write ( "</tr>" );


   // Yahoo Clubs link
   //

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "    <td align=\"center\">" );
   SS_HTML_Write ( "    <p>" );
   SS_HTML_Write ( "    <a href=\"http://clubs.yahoo.com/clubs/irondragonrailempires\">" );
   SS_HTML_Write ( "        <IMG src=\"http://www.irondragon.org/yahoo_clubs.gif\"" );
   SS_HTML_Write ( "             alt=\"Rail Empires Yahoo! Club\">" );
   SS_HTML_Write ( "    </a>" );
   SS_HTML_Write ( "    </p>" );
   SS_HTML_Write ( "    </td>" );
   SS_HTML_Write ( "    <td>Talk about Rail Empires at Yahoo! Clubs</td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_End_Table_Tag ( );

   SS_HTML_Write ( "<br><br><br><br>" );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ W e b R i n g _ F r a g m e n t
//
// =---------------------------------------------------------------------------
int Write_WebRing_Fragment ( )
{

   SS_HTML_Write ( "<!-- Begin Railroad Software Site Ring Ring HTML Fragment -->" );
   SS_HTML_Write ( "<center><p><table bgcolor=white border=1 bordercolor=red><tr><td valign=top>" );
   SS_HTML_Write ( "<center>" );
   SS_HTML_Write ( "<A href=\"http://pub8.bravenet.com/sitering/nav.php?usernum=653050463&action=list&siteid=34986\">" );
   SS_HTML_Write ( "  <img src=\"http://www.railwaystation.com/images/rss01.gif\" border=0 " );
   SS_HTML_Write ( "  alt=\"Railroad Software Site Ring\" WIDTH=350 HEIGHT=81></a><br><br>" );
   SS_HTML_Write ( "[<A HREF=\"http://pub8.bravenet.com/sitering/nav.php?usernum=653050463&action=prev&siteid=34986\" TARGET=\"_top\">Reverse</A>]" );
   SS_HTML_Write ( "[<A HREF=\"http://pub8.bravenet.com/sitering/nav.php?usernum=653050463&action=next&siteid=34986\" TARGET=\"_top\">Forward</A>]" );
   SS_HTML_Write ( "[<A HREF=\"http://pub8.bravenet.com/sitering/nav.php?usernum=653050463&action=random&siteid=34986\" TARGET=\"_top\">Random</A>]" );
   SS_HTML_Write ( "[<A HREF=\"http://pub8.bravenet.com/sitering/nav.php?usernum=653050463&action=list&siteid=34986\" TARGET=\"_top\">List</A>]" );
   SS_HTML_Write ( "[<A HREF=\"http://www.railwaystation.com/sitering.html\" TARGET=\"_top\">Join</A>]</center></td></tr> </table></center><p>" );
   SS_HTML_Write ( "<!-- End Railroad Software Site Ring HTML Fragment -->" );

#if 0
   SS_HTML_Write ( "<!-- Begin Railroad Software Web Ring HTML Fragment -->\n" );
   SS_HTML_Write ( "<center>\n" );
   SS_HTML_Write ( "<p>\n" );
   SS_HTML_Write ( "<table bgcolor=white border=1 bordercolor=red><tr><td valign=top>\n" );
   SS_HTML_Write ( "<center>\n" );
   SS_HTML_Write ( "<a href=\"http://www.webring.org/cgi-bin/webring?ring=railsoft&list\"><img src=\"http://www.railwaystation.com/images/rsw01.gif\" border=0 alt=\"Railroad Software Webring\"></a><br>\n" );
   SS_HTML_Write ( "<br>\n" );
   SS_HTML_Write ( "[<A HREF=\"http://www.webring.org/cgi-bin/webring?ring=railsoft&id=20&prev\" TARGET=\"_top\">Reverse</A>]\n" );
   SS_HTML_Write ( "[<A HREF=\"http://www.webring.org/cgi-bin/webring?ring=railsoft&id=20&next\" TARGET=\"_top\">Forward</A>]\n" );
   SS_HTML_Write ( "[<A HREF=\"http://www.webring.org/cgi-bin/webring?random&ring=railsoft\" TARGET=\"_top\">Random</A>]\n" );
   SS_HTML_Write ( "[<A HREF=\"http://www.webring.org/cgi-bin/webring?ring=railsoft&id=20&next5\" TARGET=\"_top\">Next 5</A>]\n" );
   SS_HTML_Write ( "[<A HREF=\"http://www.webring.org/cgi-bin/webring?ring=railsoft&list\" TARGET=\"_top\">List</A>]\n" );
   SS_HTML_Write ( "[<A HREF=\"http://www.railwaystation.com/webring.html\" TARGET=\"_top\">Join</A>]\n" );
   SS_HTML_Write ( "</center>\n" );
   SS_HTML_Write ( "</td>\n" );
   SS_HTML_Write ( "</tr>\n" );
   SS_HTML_Write ( "</table>\n" );
   SS_HTML_Write ( "</center>\n" );
   SS_HTML_Write ( "<p>\n" );
   SS_HTML_Write ( "<!-- End Railroad Software Web Ring HTML Fragment -->\n" );
#endif
   return 0;
}


// =---------------------------------------------------------------------------
// P a g e _ P r o d u c t _ D e s c r i p t i o n
//
// =---------------------------------------------------------------------------
int Page_Product_Description ( )
{
   char user_manual_bullet_line        [512];
   char user_manual_filename           [256];
   const char* adobe_url = "http://www.adobe.com/products/acrobat/readermain.html";

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon Product Description" );

   Write_User_Link_Bar ( );

   SS_HTML_Heading_Tag ( "Rail Empires: Iron Dragon", 1 );

   Write_ParaTable_Ex ( PT_STANDARD, "style=\"{width: 550px;}\"", "style=\"{font-size: small;}\"" );

   // Write Intro tidbits
   //
   SS_HTML_WriteP ( "<i>Since the signing of the Great Peace, it has been a time of splendor and progress. With the passing of war, trade has emerged as the great enterprise, the means for those of the New and Old World to travel, explore and grow prosperous. You are a new breed -- a rail baron. You must merge magic and metal to produce a new type of transport, the Iron Dragon. Using Dwarf, Elf and even Troll foremen, you must build a rail network across the two continents that outshines the others. You will carry the gems, gold, wands, pipeweed, dragons and more to those that demand them. Your riches will know no limit -- unless you are crushed by your rivals!</i>" );
   SS_HTML_WriteP ( "<b>Rail Empires: Iron Dragon</b> is a computer strategy game for up to six players. The objective is to build an extensive rail network across the fantastic world of Darwinia, use a powerful blend of dragon and steel to convey goods across those lines, and amass the greatest fortune in the shortest time period." );
   SS_HTML_WriteP ( "Rail Empires: Iron Dragon faithfully recreates the gaming experience of Mayfair Games' popular Empire Builder Series game, Iron Dragon. The computer game enhances the fun with new art, animations, computer play aids, on-line play and much more." );

   // Write Features Tidbit
   //
   SS_HTML_Heading_Tag ( "Features", 2 );

   SS_HTML_Start_Bullet ( );
      SS_HTML_Write_Bullet ( "Choice of new, full color art and map, or classic art and map" );
      SS_HTML_Write_Bullet ( "Event-driven and ambient animations to delight and amaze" );
      SS_HTML_Write_Bullet ( "Pleasing music and helpful sound effects to enhance play" );
      SS_HTML_Write_Bullet ( "Quick start rules that get experienced players or newcomers playing in minutes" );
      SS_HTML_Write_Bullet ( "Any combination of hot seat, computer or on-line opponent play" );
      SS_HTML_Write_Bullet ( "Ranked on-line play for a true test of your skills (beta)" );
      SS_HTML_Write_Bullet ( "Computer aids for finding cities, locating loads, building rails and moving trains" );
      SS_HTML_Write_Bullet ( "Over 40 rule option choices all with automatic defaults" );
      SS_HTML_Write_Bullet ( "Extensive user manual covering all aspects of the game and interface" );
      SS_HTML_Write_Bullet ( "Small file sizes for quick downloading and simple system demands" );
   SS_HTML_End_Bullet ( );

   Write_System_Requirements ( );

   SS_HTML_Heading_Tag ( "More Information", 2 );
   SS_HTML_Start_Bullet ( );

      if ( 0 == SysSet_Get_Value ( "User_Manual_Filename", user_manual_filename ) )
      {
         sprintf ( user_manual_bullet_line, "<a href=\"%s\">User Manual <sup>*</sup></a>", user_manual_filename );
         SS_HTML_Write_Bullet ( user_manual_bullet_line );
      }

      SS_HTML_Write_Bullet ( "<a href=\"http://www.irondragon.org/art.htm\">Art</a>" );
      SS_HTML_Write_Bullet ( "<a href=\"http://www.irondragon.org/gallery/index.htm\">Screenshots</a>" );
      SS_HTML_Write_Bullet ( "<a href=\"http://www.irondragon.org/cgi-bin/cgi_id.exe?Page=Download\">Demo Download</a>" );
      SS_HTML_Write_Bullet ( "<font color=\"white\">Price: $35.00 USD</font>" );

   SS_HTML_End_Bullet ( );

   SS_HTML_Write ( "<p style=\"{text-align:center; font-size:small;}\">" );
      SS_HTML_Write ( "* Requires Adobe <a href=\"%s\">Acrobat Reader</a> Software. ", adobe_url );
      SS_HTML_Write ( "Also, you can right-click on the link to save the PDF file to your hard drive.<br>Feel free to print it for your own personal use. Enjoy!" );
   SS_HTML_Write ( "</p>" );

   Write_End_ParaTable ( );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// Page_Reviews
//
// =---------------------------------------------------------------------------
int Page_Reviews ( )
{
}