// =---------------------------------------------------------------------------
// p a g e _ s u p p o r t . c
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
#include "cgi_id_subsystems.h"
#include "pages.h"
#include "databases.h"
#include "cgic.h"

typedef enum SUPPORT_PAGES
{
   SUPPORT_FORM_COMMENTS   = 1,
   SUPPORT_FORM_NEWLIC     = 2
} SUPPORT_PAGES;

// =---------------------------------------------------------------------------
// Forward Declarations
//
// =---------------------------------------------------------------------------
int Write_Support_Form ( int Form );
int Process_Support_Request ( int Process );

// =---------------------------------------------------------------------------
// P a g e _ S u p p o r t
//
// Arguments: Process : Means some support form was submitted for processing
//                      see Process_Support_Request. 
//            Form    : Means some support form was requested
//
// =---------------------------------------------------------------------------
int Page_Support ( )
{
   char faq_filename                [256];
   char user_manual_filename        [256];
   char strategy_guide_filename     [256];
   char PageArg                     [VARIABLE_BYTES];
   char Support_Email               [SYSSET_VALUE_BYTES];
   char Public_Demo_Update_Filename [SYSSET_VALUE_BYTES];
   char Public_Demo_Filename        [SYSSET_VALUE_BYTES];
   char Public_Demo_Update_Link     [256];
   const char* adobe_url = "http://www.adobe.com/products/acrobat/readermain.html";

   SS_Port_ZeroMemory ( faq_filename, 256 );
   SS_Port_ZeroMemory ( PageArg, VARIABLE_BYTES );

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Support" );
   Write_User_Link_Bar ( );
   SS_HTML_Heading_Tag ( "Support", 1 );

   Write_ParaTable ( PT_STANDARD );

   // Write public patch link if it is defined
   //
   if ( 0 == SysSet_Get_Value ( "Public_Demo_Update_Filename", Public_Demo_Update_Filename ) )
   {
      if ( strlen(Public_Demo_Update_Filename) )
      {
         sprintf ( Public_Demo_Update_Link, "<a href=\"http://www.irondragon.org/game/%s\">%s</a>", Public_Demo_Update_Filename, Public_Demo_Update_Filename );
         SS_HTML_Heading_Tag ( "Latest Patch", 2 );
         SS_HTML_WriteP ( "You can download the latest patch here: %s. This patch will update all released version of the game from 11/26/2000 (from idpc_1126_all.exe).", Public_Demo_Update_Link );

         // More info for users
         //
         if ( 0 == SysSet_Get_Value ( "Public_Demo_Filename", Public_Demo_Filename ) )
            if ( strlen(Public_Demo_Filename) )
               SS_HTML_WriteP ( "It is not necessary to patch the latest full demo download, %s, only previous versions such as idpc_0721_all.exe. %s contains everything.", Public_Demo_Filename, Public_Demo_Filename );
      }
   }

   // Write the Documentation tidbit
   //
   SS_HTML_Heading_Tag ( "Documentation", 2 );

   SS_HTML_Write ( "<p style=\"{text-align:center; font-size:small;}\">" );
      SS_HTML_Write ( "Requires Adobe <a href=\"%s\">Acrobat Reader</a> Software", adobe_url );
   SS_HTML_Write ( "</p>" );

   SS_HTML_Write ( "<p style=\"{text-align:center;}\">" );

      // Write user manual tidbit
      //
      if ( 0 == SysSet_Get_Value ( "User_Manual_Filename", user_manual_filename ) )
         SS_HTML_Write ( "<a href=\"%s\">User Manual</a><br><br>", user_manual_filename );

      // Write Strategy Guide tidbit
      //
      if ( 0 == SysSet_Get_Value ( "Strategy_Guide_Filename", strategy_guide_filename ) )
         SS_HTML_Write ( "<a href=\"%s\">Strategy Guide</a><br>", strategy_guide_filename );

   SS_HTML_Write ( "</p>" );

   SS_HTML_Write ( "<p style=\"{text-align:center; font-size:small;}\">" );
      SS_HTML_Write ( "You can right-click on the link to save the PDF file to your hard drive. Feel free to print it for your own personal use. Enjoy!" );
   SS_HTML_Write ( "</p>" );

   // Write FAQ Tidbit
   //
   SS_HTML_Heading_Tag ( "Frequently Asked Questions", 2 );
   SS_HTML_Write ( "<p style=\"{text-align:center;}\">" );
   Write_User_Anchor ( "FAQ", "Rail Empires: Iron Dragon FAQ" );
   SS_HTML_Write ( "</p>" );

   SS_HTML_Heading_Tag ( "Contact Us", 2 );

   // Write Support Tidbit
   //
   if ( 0 != SysSet_Get_Value ( "Support_Department", Support_Email ) )
      strcpy ( Support_Email, "eden@edenstudios.net" );
   SS_HTML_WriteP ( "Eden Studios offers e-mail support for Rail Empires: Iron Dragon. Please click here if you have any questions or problems: <a href=\"mailto:%s\">%s</a>.", Support_Email, Support_Email );

   // Write Forums Tidbit
   //
   SS_HTML_Heading_Tag ( "Discussion", 2 );

   SS_HTML_Write ( "<p style=\"{text-align:center;}\">" );
   Write_User_Anchor ( "Forums", "Forums" );
   SS_HTML_Write ( "</p>" );

   Write_End_ParaTable ( );

   SS_HTML_Write ( "<br><br>" );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ S y s t e m _ R e q u i r e m e n t s
// =---------------------------------------------------------------------------
int Write_System_Requirements ( )
{
   // Write out a blurb about the System Requirements
   //
   Write_ParaTable ( PT_STANDARD );
   SS_HTML_Heading_Tag ( "Minimum System Requirements", 3 );
   SS_HTML_Start_Bullet ( );
   SS_HTML_Write_Bullet ( "Pentium 60 or better CPU" );
   SS_HTML_Write_Bullet ( "16MB of RAM" );
   SS_HTML_Write_Bullet ( "Windows 95/98/NT4(SP3)/2000" );
   SS_HTML_Write_Bullet ( "A DirectX 3 or later compatible SVGA video card capable of running at 800x600 pixels in 16-bit color" );
   SS_HTML_Write_Bullet ( "A Microsoft compatible mouse" );
   SS_HTML_End_Bullet   ( );
   Write_End_ParaTable ( );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ D o w n l o a d _  L i n k
//
// Type:    1 - full download
//          0 - update download
//
// Return:  0 - success
//         -1 - error
//
// =---------------------------------------------------------------------------
int Write_Download_Link ( int Type )
{
   char Download_Full_Filename   [SYSSET_VALUE_BYTES];
   char Download_Update_Filename [SYSSET_VALUE_BYTES];
   char* Filename = Type == 1 ? Download_Full_Filename : Download_Update_Filename;
   char* Namen = Type == 1 ? "FULL" : "UPDATE";

   if ( 0 != SysSet_Get_Value ( "Download_Full_Filename",   Download_Full_Filename ) ||
        0 != SysSet_Get_Value ( "Download_Update_Filename", Download_Update_Filename ) 
      )
   {
      Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "An internal error occurred when accessing the download file. Please report this error to Eden Studios. We are sorry for any inconvenience. Your registration information has not been processed as a result." );
      Write_End_ParaTable ( );
      return -1;
   }

   // Skip the download if the filename ios blank
   //
   if ( strlen(Filename) == 0 )
      return 0;

   Write_ParaTable ( PT_STANDARD );

   SS_HTML_Write ( ">> Download the %s install: ", Namen );

   SS_HTML_Write ( "<b><a href=\"http://www.irondragon.org/game/%s\">", Filename );
   SS_HTML_Write ( Filename );
   SS_HTML_Write ( "</a></b>" );

   Write_End_ParaTable ( );

   return 0;
}
