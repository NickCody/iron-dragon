// =---------------------------------------------------------------------------
// p a g e _ f a q . c
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
//   00 Sep 02   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "pages.h"
#include "databases.h"

// =---------------------------------------------------------------------------
// P a g e _ F A Q 
// =---------------------------------------------------------------------------
int Page_FAQ ( )
{
   db_metastructure  ms;
   int               ret = 0;

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - FAQ" );
   Write_User_Link_Bar ( );
   SS_HTML_Heading_Tag ( "FAQ", 1 );

   ret = SS_DB_Initialize_MetaStructure ( &ms, sizeof(db_faq_entry), 0, 0 );
   ret = ret == 0 ? SS_DB_Create_Open ( &ms, "", DB_FAQ_Entries ) : -2;

   if ( ret == 0 )
   {
      Write_End_User_FAQ_Summaries ( &ms );

      SS_DB_Close ( &ms );
   }
   else
   {
      Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "Some kind of error occurred while accessing the FAQ database. Please try again later." );
      Write_End_ParaTable ( );
   }
  
   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}


