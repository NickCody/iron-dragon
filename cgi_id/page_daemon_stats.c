// =---------------------------------------------------------------------------
// p a g e _ d a e m o n _ s t a t s . c
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


#include "cgi_id_subsystems.h"

#include "net_messages_admin.h"
#include "srvstat_actions.h"
#include "pages.h"

// =---------------------------------------------------------------------------
// P a g e _ D a e m o n _ S t a t s
//

int Page_Daemon_Stats ( )
{
   nm_cza_info   info;
   nm_cza_getlog getlog;

   SS_HTML_Start_Page ( "RE Daemon Statistics" );

   SS_HTML_Heading_Tag ( "RE Daemon Statistics", 1 );

   if ( Action_Get_Stats ( &info ) == 0 )
   {
      SS_HTML_Table_Tag ( );

      SS_HTML_Write ( "<TR><TH colspan=2>LIVE SERVER INFO</TH></TR>" );
      SS_HTML_Write ( "<TR><TD> Server Name:        </TD> <TD> <B>%s</B> </TD></TR>", info.Server_Name );
      SS_HTML_Write ( "<TR><TD> Start Time:         </TD> <TD> <B>%s</B> </TD></TR>", info.Start_Time );
      SS_HTML_Write ( "<TR><TD> Active Connections: </TD> <TD> <B>%d</B> </TD></TR>", (int)ntohs(*(uint_16*)info.Active_Connections) );
      SS_HTML_Write ( "<TR><TD> Active Groups:      </TD> <TD> <B>%d</B> </TD></TR>", (int)ntohs(*(uint_16*)info.Active_Groups) );

      SS_HTML_Write ( "<TR><TH colspan=2>STATISTICS SINCE START TIME</TH></TR>" );

      SS_HTML_Write ( "<TR><TD> Total Connections:          </TD> <TD> <B>%d</B> </TD></TR>", (int)ntohl(*(uint_32*)info.Total_Connections) );
      SS_HTML_Write ( "<TR><TD> Total Authentications:      </TD> <TD> <B>%d</B> </TD></TR>", (int)ntohl(*(uint_32*)info.Total_Connections_Authenticated) );
      SS_HTML_Write ( "<TR><TD> Total Private Groups Formed:</TD> <TD> <B>%d</B> </TD></TR>", (int)ntohl(*(uint_32*)info.Total_Private_Groups) );
      SS_HTML_Write ( "<TR><TD> Total Open Groups Formed:   </TD> <TD> <B>%d</B> </TD></TR>", (int)ntohl(*(uint_32*)info.Total_Open_Groups) );
      SS_HTML_Write ( "<TR><TD> Total Ranked Groups Formed: </TD> <TD> <B>%d</B> </TD></TR>", (int)ntohl(*(uint_32*)info.Total_Ranked_Groups) );

      /*
      if ( Action_Get_Log_Filename ( &getlog ) == 0 )
      {
         SS_HTML_Write ( "<TR><TH colspan=2>MISCELLANEOUS</TH></TR>" );
         SS_HTML_Write ( "<TR><TD>Log Filename: </TD> <TD> <B>%s</B> </TD></TR>", getlog.Log_Filename );
      }
      */

      SS_HTML_End_Table_Tag ();

      SS_HTML_WriteP ( "Please note that all connection statistics include those connections made by this web page." );
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page  ( );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ L o g _ M o n i t o r
//

int Page_Log_Monitor ( )
{
   nm_cza_getlog getlog;
   char       buf[81];
   uint_32       read;
   int           handle;

   SS_HTML_Start_XML_Page ( );

   if ( Action_Get_Log_Filename ( &getlog ) == 0 )
   {
      handle = SS_File_IO_Open ( (const char *)getlog.Log_Filename, 1 /*readonly*/ );

      if ( handle != -1 )
      {
         int old_writes = SS_Set_NewLine_Writes ( 0 );

         do
         {
            SS_File_IO_Read ( handle, (uint_08*)buf, 80, &read );

            if ( read )
            {
               buf[read] = 0;
               SS_HTML_Write ( buf );
            }

         } while ( read == 80 );

         SS_Set_NewLine_Writes ( old_writes );

         SS_File_IO_Close ( handle );
      }
   }

   SS_HTML_End_XML_Page  ( );

   return 0;
}
