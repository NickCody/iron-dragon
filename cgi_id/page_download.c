// =---------------------------------------------------------------------------
// p a g e _ d o w n l o a d . c
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
#include "cgic.h"
#include "srvstat_actions.h"
#include <stdlib.h>
#include "databases.h"
#include "connection_zone.h"
#include "license_manager.h"
#include "site_error_codes.h"
#include "users.h"

// =---------------------------------------------------------------------------
// Forward Declarations
//
// =---------------------------------------------------------------------------
int   Display_Direct_Download_Page ( int Download_Type );
int   Process_Miva_Registration    ( );

int    Capture_Authentication ( );
int    Authenticate_User      ( );
char * Auto_Generate_Password ( );
char * Auto_Generate_Username ( char* Username, const char* fname, const char* lname, int ordinal );

// =---------------------------------------------------------------------------
// P a g e _ D o w n l o a d
// =---------------------------------------------------------------------------
int Page_Download ( )
{
   int  ret = 0;
   char Public_Demo_Filename        [ SYSSET_VALUE_BYTES ];
   char Public_Demo_Update_Filename [ SYSSET_VALUE_BYTES ];
   char Download_Update_Filename    [ SYSSET_VALUE_BYTES ];
   char Download_Full_Filename      [ SYSSET_VALUE_BYTES ];
   char Beta_Download_Message       [ SYSSET_VALUE_BYTES ];
   char Download_Update_Link        [256];
   char Download_Full_Link          [256];
   char Public_Demo_Link            [256];
   char Public_Demo_Update_Link     [256];
   int  Download_Page_Hits;

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Demo" );
   Write_User_Link_Bar ( );
   
   // For beta testers, show them their special patch and beta message
   //
   if (    Capture_Authentication ( ) != -1
        && Authenticate_User ( ) != -1 
        && User_Is_Back_End_Operator ( Page_Info.Username ) )
   {
      //
      // Standard section for beta testers
      //

      SS_HTML_Heading_Tag ( "Beta Download", 1 );

      Write_ParaTable ( PT_STANDARD );

      if ( 0 == SysSet_Get_Value ( "Download_Full_Filename", Download_Full_Filename ) 
             && strlen(Download_Full_Filename) )
      {
         sprintf ( Download_Full_Link, "<a href=\"http://www.irondragon.org/game/%s\">%s</a>", Download_Full_Filename, Download_Full_Filename );
         SS_HTML_WriteP ( "The latest <b>tester</b> full install can be found here: %s.", Download_Full_Link  );
      }

      if ( 0 == SysSet_Get_Value ( "Download_Update_Filename", Download_Update_Filename ) 
             && strlen(Download_Update_Filename) )
      {
         sprintf ( Download_Update_Link, "<a href=\"http://www.irondragon.org/game/%s\">%s</a>", Download_Update_Filename, Download_Update_Filename );
         SS_HTML_WriteP ( "The latest <b>tester</b> patch can be found here: %s.", Download_Update_Link );
      }

      Write_End_ParaTable ( );
            
      SysSet_Get_Value ( "Beta_Download_Message", Beta_Download_Message );
      SS_HTML_WriteP ( Beta_Download_Message );
   }
   else
   {
      //
      // Standard section for the public
      //

      SS_HTML_Heading_Tag ( "Demo", 1 );

      // Intro text...
      //
      Write_ParaTable ( PT_STANDARD );
   
         SS_HTML_WriteP ( "Note that this demo installs the full game in demo format. "
                          "If you order the game, you will receive a license file that unlocks "
                          "the demo and allows for full unrestricted play." );

         SS_HTML_WriteP ( "Also, to curtail the size and time of the download, the program files include "
                          "only limited documentation. This should be sufficient for those who are "
                          "familiar with the Iron Dragon board game, or those who want to just dive "
                          "right in. For those who want to know everything about the game and then "
                          "some, a comprehensive User Manual is available." );

      Write_End_ParaTable ( );

      // Write Download Links
      //
      if ( 0 == SysSet_Get_Value ( "Public_Demo_Filename", Public_Demo_Filename ) )
      {
         sprintf ( Public_Demo_Link, "<a href=\"http://www.irondragon.org/game/%s\">%s</a>", Public_Demo_Filename, Public_Demo_Filename );

         Write_ParaTable ( PT_STANDARD );

         SS_HTML_Heading_Tag ( "Instructions", 2 );
         SS_HTML_WriteP ( "1. Download the demo <b>%s</b> to your desktop.", Public_Demo_Link );
         SS_HTML_WriteP ( "2. Double click the <b>%s</b> to install the demo.", Public_Demo_Filename );
         SS_HTML_WriteP ( "3. Follow the installation instructions." );
         SS_HTML_WriteP ( "4. Start playing!" );

         // Write public patch link if it is defined
         //
         if ( 0 == SysSet_Get_Value ( "Public_Demo_Update_Filename", Public_Demo_Update_Filename ) )
         {
            if ( strlen(Public_Demo_Update_Filename) )
            {
               sprintf ( Public_Demo_Update_Link, "<a href=\"http://www.irondragon.org/game/%s\">%s</a>", Public_Demo_Update_Filename, Public_Demo_Update_Filename );
               SS_HTML_Heading_Tag ( "Patch", 2 );
               SS_HTML_WriteP ( "You can download the latest patch here: %s. This patch will update all released version of the game from 11/26/2000 (from idpc_1126_all.exe).", Public_Demo_Update_Link );
            }
         }

         Write_End_ParaTable ( );

         // Update Page Counter
         //
         Download_Page_Hits = SysSet_Get_Integer_Value ( "Download_Page_Hits" );
         Download_Page_Hits++;
         SysSet_Set_Integer_Value ( "Download_Page_Hits", Download_Page_Hits, 1 );

      }
      else
      {
         Write_ParaTable ( PT_STANDARD );
         SS_HTML_WriteP ( "An error has occurred with the download link. Please try again later." );
         Write_End_ParaTable ( );
      }
   }

   SS_HTML_Write ( "<br><br>" );

   Write_System_Requirements ( );

   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return ret;
}

#if 0
// =---------------------------------------------------------------------------
// D i s p l a y _ D i r e c t _ D o w n l o a d _ P a g e
//
// Displays the download page and accept registration info
//
// Returns:
//  -1 on undefined or internal errors
//  -2 if beta download is attempted by a non-admin and non-beta-tester
// 
// =---------------------------------------------------------------------------
int Display_Direct_Download_Page ( int Download_Type )
{
   uint_08     Lic_Data [ 4096 ];
   char        Lic_Filename[256];
   user_form   uf;
   int         ret = 0;
   char        Beta_Download_Message [ SYSSET_VALUE_BYTES ];

   SS_Port_ZeroMemory ( &uf, sizeof(user_form) );
   SS_Port_ZeroMemory ( &Lic_Data, 4096 );
   SS_Port_ZeroMemory ( &Lic_Filename, 256 );

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Demo" );
   SS_HTML_Heading_Tag ( "Demo", 1 );
   Write_User_Link_Bar ( );

   // Registered Evaluation - License File is mailed (30-days, full access)
   //
   switch ( Download_Type )
   {
      case DT_REGISTEREDEVAL:
      {
         if ( 0 != Action_Gather_User_Fields ( &uf, 1 /*enforce required fields*/ ) )
         {
            Write_ParaTable ( PT_STANDARD );
            SS_HTML_Write ( "The information that you provided is not complete. Please hit your browser's back button and try again." );
            Write_End_ParaTable ( );
            break;
         }

         ret = Generate_License_File ( uf.fname, uf.lname, uf.username, Auto_Generate_Password(), PERMIT_REGISTERED_EVAL, Lic_Data, Lic_Filename );
         
         if ( ret != 0 )
         {
            Write_ParaTable ( PT_STANDARD );
            SS_HTML_WriteP ( "An error occurred while generating your license file. Please try again." );
            Write_End_ParaTable();
            break;
         }

         ret = Send_License_File ( /*uf.fname, uf.lname,*/ uf.email, Lic_Filename, Lic_Data );

         if ( ret != 0 )
         {
            Write_ParaTable ( PT_STANDARD );
            SS_HTML_WriteP ( "An error occurred while sending you the license file. Please check your e-mail address and try again. If the problem persists, please contact Eden Studios." );
            Write_End_ParaTable();
            break;
         }

         // Here, we directly bring them to the download
         //
         Write_ParaTable ( PT_STANDARD );
         SS_HTML_WriteP ( "Thank you! A registration file has been attached to an e-mail message and sent to the address you specified. Please copy this registration file to your Iron Dragon directory. Until then, you can enjoy the game for 30 turns. After you copy the file, the game will be fully unlocked for a 30-day trial period." );
         SS_HTML_WriteP ( "Rail Empires: Iron Dragon is currently in beta. The download is about 6.5MB and will take approximately 20 minutes to download using a 56k modem." );
         Write_End_ParaTable ( );

         Write_Download_Link ( 1 /*full download*/ );
         Write_Download_Link ( 0 /*update download*/ );

         Write_System_Requirements ( );

         break;
      }

      case DT_ANONYMOUSEVAL:
      {
         // No license file
         Write_ParaTable ( PT_STANDARD );
         SS_HTML_WriteP ( "Without registering, you will be able to enjoy Rail Empires: Iron Dragon for 30 game turns." );
         Write_End_ParaTable ( );

         Write_Download_Link ( 1 );

         Write_System_Requirements ( );

         break;
      }

      case DT_BETADOWNLOAD:
      {
         if ( Capture_Authentication() == 0 &&
              Authenticate_User ( ) == 0 )
         {
            if ( User_Is_Back_End_Operator ( Page_Info.Username ) )
            {
               SS_HTML_Heading_Tag ( "Beta Download Area", 2 );

               SS_HTML_WriteP ( "Go back to the <a href=\"%s?Page=Authenticate\">Development Backend</a>.", CGI_ID_SCRIPT );

               if ( 0 == SysSet_Get_Value ( "Beta_Download_Message", Beta_Download_Message ) )
               {
                  Write_ParaTable ( PT_STANDARD );
                  SS_HTML_WriteP ( Beta_Download_Message );
                  Write_End_ParaTable ( );
               }

               Write_Download_Link ( 0 );
               Write_Download_Link ( 1 );

               SS_HTML_WriteP ( "Thank you and have fun!<br><br>--The Iron Dragon Development Team" );

               Write_System_Requirements ( );

               break;
            }
         }
      }

      default:
      {
         Write_ParaTable ( PT_STANDARD );
         SS_HTML_WriteP ( "The download option that you specified was not recognized. Please hit your browser's Back button and try again." );
         Write_End_ParaTable ( );

         break;
      }
   }

   // For Miva Registration, we do not display any kind of a web page...
   //
   Write_User_Footer ( );
   SS_HTML_End_Page ( );

   return ret;
}
#endif

// =---------------------------------------------------------------------------
// P a g e _ M i v a
//
// =---------------------------------------------------------------------------
int Page_Miva ( )
{
   int            ret = 0;
   char           Username [ 64 /*USERNAME_BYTES =24*/ ];
   user_form      uf;
   nm_user_record usr;
   int            Username_Ordinal;
   uint_08        Code;
   int            Max_Reps;

   SS_Port_ZeroMemory ( &uf, sizeof(user_form) );
   SS_Port_ZeroMemory ( &usr, sizeof(nm_user_record) );

   // For Miva Registration, we do not display any kind of a web page...
   //
   if ( 0 != Action_Gather_User_Fields ( &uf, 0 /*enforce required fields*/ ) )
   {
      SS_HTML_Write ( "There was a small problem processing your account. Your payment method is not the reason for the failure. The registration engine failed to add your account to the game server. Please contact Eden Studios and give them the error code <b>%s</b>. Your order was processed successfully, but you need to contact Eden in order to get your license key.", SITECODE_NOMIVAUSERINFO );
      return -1;
   }

   // Now, add the user record
   //
   Code = 0xFE;      // duplicate user
   Max_Reps = 100;   // safety measure
   while ( Code == 0xFE /*duplicate user*/ && Max_Reps )
   {
      Username_Ordinal = 0;
      Auto_Generate_Username ( Username, uf.fname, uf.lname, Username_Ordinal );
   
      strcpy ( uf.username, Username );
      ret = Action_Add_User ( &uf, &usr, &Code );

      if ( ret == 0 )
         break;

      Max_Reps--;
   }
   
   return ret;
}

// =---------------------------------------------------------------------------
// G e n e r a t e _ L i c e n s e _ F i l e 
//
// =---------------------------------------------------------------------------
int Generate_License_File ( const char* fname,
                            const char* lname,
                            const char* username,
                            const char* password,
                            int         Permit,
                            uint_08*    Lic_Data, 
                            const char* Lic_Filename )
{
   ld_decoded  ldd;
   ld_encoded  lde;
   char const* filename = 0;
   char DateTime[32];
   int  Day, Month, Year;

   SS_Port_ZeroMemory ( &ldd, sizeof(ld_decoded) );
   SS_Port_ZeroMemory ( &lde, sizeof(ld_encoded) );

   // Generate a license file
   //
   ldd.Module = MODULE_IRON_DRAGON;

   switch ( Permit )
   {
      case PERMIT_NONE:
         strcpy ( ldd.Permit, "None" ); break;
      case PERMIT_CORRUPTED:
         strcpy ( ldd.Permit, "Corrupted" ); break;
      case PERMIT_ANONYMOUS_EVAL:
      case PERMIT_EXPIRED_EVAL:
      case PERMIT_REGISTERED_EVAL:
         strcpy ( ldd.Permit, "Evaluation" ); break;
      case PERMIT_REGISTERED_FULL:
         strcpy ( ldd.Permit, "Registered" ); break;
   }


   // Figure out date and set decoded data...
   //
   SS_Port_Get_DateTime_String ( DateTime );
   SS_Port_DateTime_To_DayMonThYear ( DateTime, &Day, &Month, &Year );

   ldd.Reg_Day   = (uint_08)( ((uint_16)Day)   & 0x00FF );
   ldd.Reg_Month = (uint_08)( ((uint_16)Month) & 0x00FF );
   ldd.Reg_Year  = (uint_08)( ((uint_16)Year-(uint_16)2000)  & 0x00FF );

   // Check to see that first+last is not greater than our smaller ldd.User_Name buffer
   // if it is, just write the user's first initial into the license file.
   if ( strlen(fname) + strlen(lname) >= 32 )
      sprintf ( ldd.Handle, "%c. %s", fname[0], lname );
   else
      sprintf ( ldd.Handle, "%s %s", fname, lname );

   strcpy ( ldd.F_Name, fname );
   strcpy ( ldd.L_Name, lname );
   strcpy ( ldd.Password, password );
   strcpy ( ldd.Handle, username );

   filename = Encrypt ( &ldd, &lde, 0, 1, Lic_Data );
   strcpy ( (char*)Lic_Filename, ( const char*)filename );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ R e q u e s t _ B e t a _ L i c e n s e 
//
// =---------------------------------------------------------------------------
int Page_Request_Beta_License ( )
{
   int            ret = -1;
   nm_user_record ur;
   uint_08        Lic_Data [ 4096 ];
   char           Lic_Filename[256];

   SS_HTML_Start_Page ( "Rail Empires: Iron Dragon - Beta License Request" );
   SS_HTML_Heading_Tag ( "Beta License Request", 1 );

   Write_ParaTable ( PT_STANDARD );

   if ( User_Is_Back_End_Operator ( Page_Info.Username ) )
   {
      ret = Action_Lookup_User ( &ur, Page_Info.Username );
      
      if ( ret == 0 )
      {
         ret = Generate_License_File ( ur.record.fname, ur.record.lname, ur.record.username, ur.record.password, PERMIT_REGISTERED_EVAL, Lic_Data, Lic_Filename );
         
         if ( ret == 0 )
         {
            ret = Send_License_File ( ur.record.email, Lic_Filename, Lic_Data );

            if ( ret == 0 )
            {
               SS_HTML_WriteP ( "The license file was sent to %s, you should receive it shortly.", ur.record.email );
            }
            else
            {
               SS_HTML_WriteP ( "We failed when trying to send an e-mail to %s, please contact Eden with error code <b>%s</b> and double check this e-mail address.", ur.record.email, SITECODE_FAILEDTOEMAIL );
            }
         }
         else
         {
            SS_HTML_WriteP ( "We failed to generate a license file because of unknown and potentially scary reasons. Please report this error to Eden and mention error code <b>%s</b>.", SITECODE_FAILEDTOGENLIC );
         }
      }
      else
      {
         SS_HTML_WriteP ( "There was an error looking you up. Please contact Eden about this problem. Please mention error code: <b>%s</b>.", SITECODE_FAILEDUSERLOOKUP );
      }
   }
   else
   {
      SS_HTML_WriteP ( "You cannot request a license file in this way. Please contact Eden Support." );

      ret = -1;
   }

   Write_End_ParaTable();

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return ret;
}

// =---------------------------------------------------------------------------
// A u t o_ G e n e r a t e _ P a s s w o r d 
// =---------------------------------------------------------------------------
char * Auto_Generate_Password ( )
{
   static char Valid_Chars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                                 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                                 'u', 'v', 'w', 'x', 'y', 'z' };

   static char Auto_Gen[7];
   int i;

   for ( i=0; i < 6; i++ )
   {
      Auto_Gen[i] = (char) ( (rand() % sizeof(Valid_Chars)) & 0xFF );
   }
   
   Auto_Gen[6] = 0;

   return Auto_Gen;
}

// =---------------------------------------------------------------------------
// A u t o _ G e n e r a t e _ U s e r n a m e
// =---------------------------------------------------------------------------
char * Auto_Generate_Username ( char* Username, const char* fname, const char* lname, int ordinal )
{
   if ( ordinal == 0 /*initial value, we try and give username w/o digits*/ )
      sprintf ( Username, "%c%s", fname[0], lname );
   else
      sprintf ( Username, "%c%s%d", fname[0], lname, ordinal );

   // Truncate at maximum
   //
   Username[23 /*USERNAME_BYTES-1*/] = 0;

   return Username;
}