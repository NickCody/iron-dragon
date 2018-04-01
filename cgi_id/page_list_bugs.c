// =---------------------------------------------------------------------------
// p a g e _ l i s t _ b u g s .c
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

#include <stdlib.h>
#include <stdio.h>
#include "cgi_id_subsystems.h"
#include "bugrep_actions.h"
#include "pages.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// P a g e _ L i s t _ B u g s
//
// =---------------------------------------------------------------------------
int Page_List_Bugs ( )
{
   int      ret = 0;
   char     GenHead[256];
   char     DateTime[32];
   char     Bug_Status[VARIABLE_BYTES];
   uint_08  Status = 0;

   SS_HTML_Start_Page ( "Iron Dragon Bug Summary Report" );
   SS_HTML_Heading_Tag ( "Iron Dragon Bug Summary Report", 1 );

   // Check that the user specified bug status, otherwise ask them for it
   //
   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "Bug_Status", Bug_Status, VARIABLE_BYTES ) )
   {
      Page_Body_Request_Summary_Report_Fields ( );
   }
   else
   {
      Status = (uint_08)atoi(Bug_Status);

      SS_Port_Get_DateTime_String ( DateTime );
      sprintf ( GenHead, "Report Generated on %s", DateTime );
      SS_HTML_Heading_Tag ( GenHead, 3 );

      ret = Action_Bug_Summary ( Status );
   }

   if ( ret != 0 )
   {
      SS_HTML_WriteP ( "Failed to Generate a bug summary." );
   }

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return ret;
}


// =---------------------------------------------------------------------------
// P a g e _ B o d y _ R e q u e s t _ S u m m a r y _ R e p o r t _ F i e l d s
// =---------------------------------------------------------------------------
int Page_Body_Request_Summary_Report_Fields ( )
{
   SS_HTML_WriteP ( "Please pick the type of report." );

   Write_Script_Form ( "List_Bugs" );
   SS_HTML_Table_Tag ( );
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Bug Status</TH>" );
   SS_HTML_Write ( "    <TD>" );
   SS_HTML_Write ( "       <table border=0 cellspacing=0 cellpadding=3>" );
   SS_HTML_Write ( "         <TR>" );
   SS_HTML_Write ( "           <TD>" );
   SS_HTML_Write ( "             <SELECT size=1 name=Bug_Status>" );
   SS_HTML_Write ( "               <option value=0>Open</option>" );
   SS_HTML_Write ( "               <option value=1>Fixed</option>" );
   SS_HTML_Write ( "               <option value=2>Not a bug</option>" );
   SS_HTML_Write ( "               <option value=3>Future release</option>" );
   SS_HTML_Write ( "             </SELECT>" );
   SS_HTML_Write ( "           </TD>" );
   SS_HTML_Write ( "       </TABLE>" );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "    <TD>" );
                           SS_HTML_Submit ( "" );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );
   SS_HTML_End_Table_Tag ( );
   SS_HTML_End_Form ( );

   return 0;
}

