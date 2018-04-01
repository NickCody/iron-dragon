
// =---------------------------------------------------------------------------
// p a g e _ r a n k i n g . c
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
//   00 Mar 23   nic   Created.
//

#include <stdlib.h>
#include <stdio.h>
#include "cgic.h"
#include "cgi_id_subsystems.h"
#include "pages.h"
#include "databases.h"
#include "connection_zone.h"
#include "site_error_codes.h"
#include "rank_calc.h"

// =---------------------------------------------------------------------------/
// Forward Decl
//
int Write_Rankings_Table ( int Num_Ranked );

// =---------------------------------------------------------------------------
// Page_Ranking
// =---------------------------------------------------------------------------
int Page_Ranking ( )
{
   db_metastructure  ms;
   int               ret;
   char              Rank_Calc_Link[256];
   char              date[33];

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Player Ranking" );
   Write_User_Link_Bar ( );
   SS_HTML_Heading_Tag ( "Player Rankings", 1 );

   SS_Port_Get_DateTime_String ( date );

   // Write Intro
   //
   Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "Welcome to Phase II of the Rail Empires: Iron Dragon Player Ranking Beta! More information on online play "
                       "and ranked play options will appear in a coming FAQ. " ); // See <a href=\"#notes\">Notes...</a>
      SS_HTML_WriteP ( "All honor the top ranks! Good luck in all your play!" );
      SS_HTML_Write ( "<p align=\"center\">Last Update %s<br>", date );
   Write_End_ParaTable ( );

   // Write News Articles, if any
   //
   ret = SS_DB_Initialize_MetaStructure ( &ms, sizeof(db_news_articles), 0, 0 );
   ret = ret == 0 ? SS_DB_Create_Open ( &ms, "", DB_Name_News_Articles ) : -2;

   if ( ret == 0 )
   {
      Write_End_User_Article_Summaries ( NA_CLASS_RANKINGNEWS, &ms );
      SS_DB_Close ( &ms );
   }
   else
   {
      Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "Some kind of error occurred while accessing the news database. Please try again later." );
      Write_End_ParaTable ( );
   }

   // Write Rankings Table
   //
   if ( Write_Rankings_Table ( 0 ) != 0 )
   {
      Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "<hr>" );
      SS_HTML_WriteP ( "The Rankings Server experienced a problem retrieving the standings, Code %s. Please try again later.", SITECODE_FAILEDTOGETUSERS );
      SS_HTML_WriteP ( "<hr>" );
      Write_End_ParaTable ( );
   }


   // Write Notes
   //
   /*
   SS_HTML_WriteP  ( "<a name=\"notes\">Notes:</a>" );

   Write_ParaTable_Ex ( PT_STANDARD, "", "style=\"{font-size: small;}\"" );
      SS_HTML_WriteP ( "Ranked play is not available in the limited-play demo. It may only "
                       "be accessed from the fully licensed version of Rail Empires: Iron Dragon, and "
                       "is still in beta form. Look for another month of ranked play testing before "
                       "we finalize that portion of the game. We will also be awarding a prize for "
                       "the highest ranked player during the beta period (look for more on that later "
                       "as well)." );
   Write_End_ParaTable ( );
   */

   // Write Rank Calculator Link
   //
   SS_HTML_Heading_Tag ( "Rank Calculator", 2 );

   Write_ParaTable ( PT_STANDARD );
      Format_Script_Link ( Rank_Calc_Link, "Rank_Calculator" );
      SS_HTML_WriteP ( "Have you ever wanted to know how many points were at stake for a given set of players? "
                       "Use the <a href=\"%s\">Rank Calculator</a> to find out!", Rank_Calc_Link );
   Write_End_ParaTable ( );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// Write_Rankings_Table
//
// Ask the server for the top Num_Ranked ranked players, 0 means all rankings
// the returned list is always pre-sorted by the server
//
// =---------------------------------------------------------------------------
int Write_Rankings_Table ( int Num_Ranked )
{
   int               ret, Num_Written;
   uint_32           start;
   uint_32           end;
   uint_32           i;
   nm_bidir_numusers numusr;
   nm_bidir_getusers getusr;
   nm_user_record    user_rec;
   nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;

   char* Top_Ten_Style = "{text-align:center; background-color: #222299}";
   char* Normal_Style = "{text-align:center;}";
   char* Use_Style = Normal_Style;

   // Connect to the server
   //
   Page_Info.Server_Sock = CZ_Connect_And_Authenticate ( "NickAdmin", "4Rtu79", p_auth_reply );

   // Ask server how many users there are
   //
   numusr.Message_Type    = 0xFF;
   numusr.Message_Type_Ex = BIDIR_NUMUSERS;
   memset ( numusr.Num_Users, 0, 4 );

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&numusr, sizeof(numusr) );
   if ( ret != 0 )
      return -1;

   // Get the servers response : There are X users
   //
   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&numusr, sizeof(numusr) );
   if ( ret != 0 )
      return -1;

   end = ntohl( *(uint_32*)numusr.Num_Users );

   // Ask server for specified range of users
   //
   getusr.Message_Type           = 0xFF;
   getusr.Message_Type_Ex        = BIDIR_GETUSERS;              
   *(uint_32*)getusr.Start_User  = 0;
   *(uint_32*)getusr.End_User    = Num_Ranked ? Num_Ranked : htonl(end-1);
   getusr.Rank_Sorted            = 1;

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&getusr, sizeof(getusr) );
   if ( ret != 0 )
      return -1;

   // Now, get the server's user list header
   //
   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&getusr, sizeof(getusr) );
   if ( ret != 0 )
      return -1;

   // Now, get each user and print results
   //
   start = ntohl ( *(uint_32*)getusr.Start_User );
   end   = ntohl ( *(uint_32*)getusr.End_User );

   // Write out a table with a row for each user
   //
   SS_HTML_Table_Tag_Ex ( "class=\"classCoolOutline\" cellspacing=\"1\" cellpadding=\"4\"" );

   // Write out the table's header row
   //
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "<TH align=\"center\">Rank</TH>" );
   SS_HTML_Write ( "<TH align=\"center\">Name</TH>" );
   SS_HTML_Write ( "<TH align=\"center\">Rating</TH>" );
   SS_HTML_Write ( "<TH align=\"center\">Wins</TH>" );
   SS_HTML_Write ( "<TH align=\"center\">Losses</TH>" );
   //SS_HTML_Write ( "<TH align=\"center\">Disconnects</TH>" );
   SS_HTML_Write ( "</TR>" );
   
   Num_Written = 0;

   // For each user, write a table row
   //
   for ( i=start; i <= end; i++ )
   {
      ret = SS_Net_Receive_Buffer_Ex ( Page_Info.Server_Sock, &user_rec, sizeof(nm_user_record), 0 );
      
      if ( ret != sizeof(nm_user_record) )
      {
         SS_HTML_Write ( "(whoa! reading user record of erroneous %d size)<BR>", ret );
         break;
      }

      // Only display users with non-zero rankings
      //
      if ( ntohs(user_rec.record.rating) > 0 )
      {
         Num_Written++;

         SS_HTML_Write ( "<TR>" );

            Use_Style = ( i-start < 10 ) ? Top_Ten_Style : Normal_Style;

            SS_HTML_Write ( "<TD style=\"%s\">%d</TD>",     Use_Style, i-start+1 );
            SS_HTML_Write ( "<TD style=\"%s\">%s%c</TD>",   Use_Style, user_rec.record.fname, user_rec.record.lname[0] );
            SS_HTML_Write ( "<TD style=\"%s\"><b>%d</b></TD>",     Use_Style, (int)ntohs(user_rec.record.rating) );
            SS_HTML_Write ( "<TD style=\"%s\">%d</TD>",     Use_Style, (int)ntohs(user_rec.record.wins) );
            SS_HTML_Write ( "<TD style=\"%s\">%d</TD>",     Use_Style, (int)ntohs(user_rec.record.losses) );
            //SS_HTML_Write ( "<TD style=\"%s\">%d</TD>",     Use_Style, (int)ntohs(user_rec.record.disconnects) );

         SS_HTML_Write ( "</TR>" );
      }
   }

   if ( !Num_Written )
   {
         SS_HTML_Write ( "<TR>" );

            SS_HTML_Write ( "<TD colspan=\"6\" style=\"Top_Ten_Style\">There are no ratings to report.</TD>" );

         SS_HTML_Write ( "</TR>" );
   }

   // Finish off the table
   SS_HTML_End_Table_Tag ();

   return 0;
}


// =---------------------------------------------------------------------------
// P a g e _ R a n k _ C a l c u l a t o r
//
// =---------------------------------------------------------------------------
int Page_Rank_Calculator ( )
{
   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Ranking Calculator" );
   Write_User_Link_Bar ( );
   SS_HTML_Heading_Tag ( "Ranking Calculator", 1 );

   Write_Script_Form ( "Rank_Calculator_Results" );

      SS_HTML_Write ( "<table cellspacing=\"1\" cellpadding=\"5\">" );

         SS_HTML_Write ( "<tr><td colspan=\"2\">" );
            SS_HTML_Start_Bullet ( );
               SS_HTML_Write_Bullet ( "The -1 means, no player is assigned here." );
               SS_HTML_Write_Bullet ( "Only the results for PLAYER ONE will be calculated." );
            SS_HTML_End_Bullet ( );
         SS_HTML_Write ( "</td></tr>" );

         SS_HTML_Write ( "<tr><th align=\"right\">Player</th><th>Rating</th></tr>" );
         SS_HTML_Write ( "<tr><td align=\"right\">PLAYER ONE</td><td><INPUT type=\"text\" name=\"uf_Player1\" value=\"-1\"> </td></tr>" );
         SS_HTML_Write ( "<tr><td align=\"right\">Player 2</td><td><INPUT type=\"text\" name=\"uf_Player2\" value=\"-1\"> </td></tr>" );
         SS_HTML_Write ( "<tr><td align=\"right\">Player 3</td><td><INPUT type=\"text\" name=\"uf_Player3\" value=\"-1\"> </td></tr>" );
         SS_HTML_Write ( "<tr><td align=\"right\">Player 4</td><td><INPUT type=\"text\" name=\"uf_Player4\" value=\"-1\"> </td></tr>" );
         SS_HTML_Write ( "<tr><td align=\"right\">Player 5</td><td><INPUT type=\"text\" name=\"uf_Player5\" value=\"-1\"> </td></tr>" );
         SS_HTML_Write ( "<tr><td align=\"right\">Player 6</td><td><INPUT type=\"text\" name=\"uf_Player6\" value=\"-1\"> </td></tr>" );

         SS_HTML_Write ( "<tr><td>&nbsp;</td><td>" );
            SS_HTML_Submit ( "" );
         SS_HTML_Write ( "</td></tr>" );

      SS_HTML_Write ( "</table>" );

   SS_HTML_End_Form ( );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}


// =---------------------------------------------------------------------------
// P a g e _ R a n k _ C a l c u l a t o r _ R e s u l t s
//
// =---------------------------------------------------------------------------
int Page_Rank_Calculator_Results ( )
{
   char   Back_Link[ 256 ];
   int    ranks    [ RATING_MAXPLAYERS ];
   char   Variable [ VARIABLE_BYTES ];
   char   Value    [ VARIABLE_BYTES ];
   int    i, j;
   float  Prob;
   int    New_Rank_Win, New_Rank_Lose;

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Ranking Calculator Results" );
   Write_User_Link_Bar ( );
   SS_HTML_Heading_Tag ( "Ranking Calculator Results", 1 );

   Rating_Clear_Array ( ranks );

   for ( i=1, j=0; i <= 6; i++ )
   {
      sprintf ( Variable, "uf_Player%d", i );
      if ( cgiFormNotFound != cgiFormStringNoNewlines ( Variable, Value, VARIABLE_BYTES) )
      {
         ranks[j] = atoi ( Value );
         if ( ranks[j] < 0 ) continue;

         j++;
      }
   }

   Prob          = Rating_Resolve_Probability ( ranks );
   New_Rank_Win  = Rating_Adjust ( ranks[0], Prob, 1 );
   New_Rank_Lose = Rating_Adjust ( ranks[0], Prob, 0 );

   SS_HTML_Write ( "<table>" );
   SS_HTML_Write ( "<tr><th colspan=\"2\">Results of Imaginary %d player game</th</tr>", j );
   SS_HTML_Write ( "<tr><td>Player 1 Win Probability</td><td>%3.2f percent</td>", Prob * 100.0f );
   SS_HTML_Write ( "<tr><td>Starting Rank</td><td>%d</td>", ranks[0] );
   SS_HTML_Write ( "<tr><td>Rank if game is WON</td><td>%d</td>", New_Rank_Win );
   SS_HTML_Write ( "<tr><td>Rank if game is LOST</td><td>%d</td>", New_Rank_Lose );
   SS_HTML_Write ( "</table>" );

   Format_Script_Link ( Back_Link, "Rank_Calculator" );
   SS_HTML_WriteP ( "Back to the <a href=\"%s\">Ranking Calculator</a>.", Back_Link );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}
