
// =---------------------------------------------------------------------------
// p a g e _ o r d e r . c
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
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
//   00 Jul 22   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "pages.h"
#include "db_system_settings.h"

// =---------------------------------------------------------------------------
// P a g e _ O r d e r
// =---------------------------------------------------------------------------
int Page_Order ( )
{
   char Link [ 256 ];
   char Download_Filename [ SYSSET_VALUE_BYTES ];
   char* Preorder_Link = "https://edenstudios.safeserver.com/Merchant2/merchant.mv?Screen=CTGY&Store_Code=Edn&Category_Code=ID";

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Ordering Instructions" );
   SS_HTML_Heading_Tag ( "Ordering Instructions", 1 );

   Write_ParaTable ( PT_STANDARD );

   SS_HTML_WriteP ( "Thank you for your interest in the Rail Empires: Iron Dragon computer game." );

   SS_HTML_WriteP ( "Rail Empires: Iron Dragon is is a downloadable product so the purchase "
                    "process is a bit different from normal. The process is relatively simple -- "
                    "you download the demo, fill out the order form, and insert the license file "
                    "in the proper directory once you receive it via email. For those who want all "
                    "the gory details, however, read on. We even suggest you print out this "
                    "instruction page before proceeding so you do not miss any steps." );

   SS_HTML_Heading_Tag ( "Step 1: Download and install the demo.", 2);

   Format_Script_Link ( Link, "Download" );

   SS_HTML_WriteP ( "The latest version of the demo can be downloaded <a href=\"%s\">here</a>."
                    "When you install the demo, you will be asked to designate your "
                    "username. You may choose any name you wish (although we reserve the right to "
                    "bar access to the Iron Dragon Connection Zone of any players with offensive "
                    "or obscene names).", Link );

   SysSet_Get_Value ( "Public_Demo_Filename", Download_Filename );

   SS_HTML_WriteP ( "If you have downloaded a demo file at some time prior to ordering, please "
                    "make sure you have the most recent version of the demo. The proper version is "
                    "labeled \"%s\". Prior versions will not support online play even if fully licensed.", 
                    Download_Filename );

   SS_HTML_Heading_Tag ( "Step 2: Fill out the order form.", 2 );

   SS_HTML_WriteP ( "This process begins when you click continue below. Be sure to check the menu "
                    "selection for no shipping charges. " );

   SS_HTML_Heading_Tag ( "Step 3: Receive license files.", 2 );

   SS_HTML_WriteP ( "We will process your credit card order within one business day."
		    " Once the order has cleared, we will email you a note with a permanent "
		    " license file attachment. ");

   SS_HTML_WriteP ( "If you wish to pay by some other means, we will email you once we receive "
                    "your payment, and attach a copy of a permanent license file." );

   SS_HTML_WriteP ( "Simply download the attachment (the license file) to the directory that you "
                    "stored your Rail Empires: Iron Dragon demo (the default is c:\\Program "
                    "Files\\Eden Studios). Once this is done, you will be granted access to all "
                    "features of the game. " );

   SS_HTML_WriteP ( "If you have further questions about the game, the user interface or online "
                    "play, please consult the Rail Empires: Iron Dragon User Manual on the Support "
                    "page of www.irondragon.org." );

   SS_HTML_WriteP ( "That's it. Enjoy your new game." );

   SS_HTML_WriteP ( "<a href=\"%s\">Continue...</a>", Preorder_Link );

   Write_End_ParaTable ( );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}
