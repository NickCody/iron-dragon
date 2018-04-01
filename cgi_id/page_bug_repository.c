// =---------------------------------------------------------------------------
// p a g e _ b u g _ r e p o s i t o r y
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
//   00 Feb 12   nic   Created.
//


#include "cgi_id_subsystems.h"

#include "net_messages_admin.h"
#include "pages.h"

// =---------------------------------------------------------------------------
// P a g e _ B u g _ R e p o s i t o r y
//

int Page_Bug_Repository ( )
{
   char Link [ LINK_BYTES ];

   SS_HTML_Start_Page ( "RE Bug Repository" );
   SS_HTML_Heading_Tag ( "RE Bug Repository", 1 );

   SS_HTML_WriteP ( "Please choose an option from the choices below:" );

   // Add New Bug
   //
   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Enter a New Bug</TH>" );
   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "Add_Bug" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );

   // Summary Report
   //
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Generate a Summary Report</TH>" );
   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "List_Bugs" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );

   // Find Bug
   //
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Get Bug Details</TH>" );
   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "Find_Bug" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );

   SS_HTML_End_Table_Tag ( );

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return 0;
}


#if 0
   // New Bug
   SS_HTML_Table_Tag ( );
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Enter a New Bug</TH>" );
   SS_HTML_Write ( "    <TD colspan=2>" );
                           Format_Script_Link ( Link, "Add_Bug" );
                           SS_HTML_Write ( "<a href=\"%s\"><IMG src=\"/styles/images/Go.gif\" alt=\"Go!\"></a>", Link );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );
   SS_HTML_End_Table_Tag ( );

   // Bug Summary
   Write_Script_Form ( "List_Bugs" );
   SS_HTML_Table_Tag ( );
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Generate a Summary Report</TH>" );
   SS_HTML_Write ( "    <TD>" );
   SS_HTML_Write ( "       <table border=0 cellspacing=0 cellpadding=3>" );
   SS_HTML_Write ( "         <TR>" );
   SS_HTML_Write ( "           <TD>Bug Status</TD>" );
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

   // Find Bug
   //
   Write_Script_Form ( "Find_Bug" );
   SS_HTML_Table_Tag ( );
   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "    <TH align=right>Bug Details</TH>" );
   SS_HTML_Write ( "    <TD>" );
   SS_HTML_Write ( "       Bug ID:" );
                           SS_HTML_Input_Text ( "Bugid", 0 );
   SS_HTML_Write ( "       <BR>" );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "    <TD>" );
                           SS_HTML_Submit ( "" );
   SS_HTML_Write ( "    </TD>" );
   SS_HTML_Write ( "</TR>" );
   SS_HTML_End_Table_Tag ( );
   SS_HTML_End_Form ( );

#endif