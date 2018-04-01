// =---------------------------------------------------------------------------
// p a g e _ A d m i n _ D o c _ R e q u e s t .c
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
//   00 May 21   nic   Created.
//


#include "cgi_id_subsystems.h"
#include "users.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// P a g e _ A d m i n _  D o c _ R e q u e s t
// =---------------------------------------------------------------------------
int Page_Admin_Doc_Request ( )
{
   char URL[256];
   int ret = 0;

   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      sprintf ( URL, "/admin_docs/EdenWebsite.htm" );
      cgiHeaderLocation ( URL );
      cgiHeaderContentType("text/html");
   }
   else
   {
      ret = -1;
   }

   return ret;
}