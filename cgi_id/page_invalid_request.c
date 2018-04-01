 // =---------------------------------------------------------------------------
// p a g e _ i n v a l i d _ r e q u e s t . c
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

#include "cgi_id_subsystems.h"
#include "pages.h"

int Page_Invalid_Request ( )
{
   SS_HTML_Start_Page ( "RE Invalid Page Request" );
   Write_User_Link_Bar ( );

   SS_HTML_Heading_Tag ( "Rail Empires: Iron Dragon", 1 );
   SS_HTML_Heading_Tag ( "Invalid Page Request!", 2 );
   
   SS_HTML_Write ( "<p>Go to: " );
   Write_User_Anchor ( "Homepage", "Rail Empires: Iron Dragon Homepage" );
   SS_HTML_Write ( "</p>" );
   
   Write_Generic_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

