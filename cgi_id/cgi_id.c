// =---------------------------------------------------------------------------
// c g i _ i d . c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Main Modile ENtry point : provides cgiMain
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jun 23   nic   Created.
//

#include <stdlib.h>
#include <string.h>

#include "cgi_id_subsystems.h"
#include "cgic.h"
#include "pages.h"
#include "connection_zone.h"
#include "databases.h"
#include "users.h"

// =---------------------------------------------------------------------------
// Types
// =---------------------------------------------------------------------------
typedef int (* Page_Function)();

typedef struct page_redirect
{
    char           Page[VARIABLE_BYTES];
    Page_Function Fn;
} page_redirect;

// =---------------------------------------------------------------------------
// Variables
// =---------------------------------------------------------------------------
cgi_Variables     Web_Vars;
common_page_info  Page_Info;

// These web pages require authentication
page_redirect     Admin_Lookup[] = {
                                    {"Daemon_Stats",           Page_Daemon_Stats},
                                    {"Main_Menu",              Page_Main_Menu},
                                    {"Authenticate",           Page_Authenticate},
                                    {"Web_Stats",              Page_Web_Stats},
                                    {"Request_Beta_License",   Page_Request_Beta_License},

                                    // User Pages
                                    {"User_Listing",           Page_User_Listing},
                                    {"Mass_User_Options",      Page_Mass_User_Options},
                                    {"Add_User",               Page_Add_User},
                                    {"Edit_User",              Page_Edit_User},
                                    {"Delete_User",            Page_Delete_User},
                                    {"Add_User_Result",        Page_Add_User_Result},
                                    {"Edit_User_Result",       Page_Edit_User_Result},

                                    // Bug Repository Pages
                                    {"Bug_Repository",         Page_Bug_Repository},
                                    {"Add_Bug",                Page_Add_Bug},
                                    {"Add_Bug_Confirm",        Page_Add_Bug_Confirm},
                                    {"Add_Bug_Result",         Page_Add_Bug_Result},
                                    {"List_Bugs",              Page_List_Bugs},
                                    {"Find_Bug",               Page_Find_Bug},
                                    {"Edit_Bug",               Page_Edit_Bug},
                                    {"Edit_Bug_Result",        Page_Edit_Bug_Result},

                                    // News Maintenance Pages
                                    {"NewsAdmin",              Page_News_Admin},

                                    // FAQ Maintenance Pages
                                    {"FAQAdmin",               Page_FAQ_Admin},

                                    // System Settings Pages
                                    {"SysSet",                 Page_SysSet},

                                    // Log Monitor
                                    {"LogMon",                 Page_Log_Monitor},

                                    // Site Documentation
                                    {"Admin_Doc_Request",      Page_Admin_Doc_Request},

                                    {"",                 0 } 
                                  };

// These web pages require no authentication
//
page_redirect     User_Lookup[] = {
                                    {"Homepage",            Page_Homepage},
                                    {"Product_Description", Page_Product_Description},
                                    {"News",                Page_News},
                                    {"Ranking",             Page_Ranking},
                                    {"Rank_Calculator",     Page_Rank_Calculator},
                                    {"Rank_Calculator_Results", Page_Rank_Calculator_Results},
                                    {"FAQ",                 Page_FAQ},
                                    {"Support",             Page_Support},
                                    {"Forums",              Page_Forums},
                                    {"Download",            Page_Download},
                                    {"Order",               Page_Order},
                                    //{"Download_Reg",      Page_Download_Reg},
                                    {"Miva",                Page_Miva},
                                    {"",                    0 } 
                                  };

int Page_Family;

char* DB_Name_System_Settings = "irondragon/systemsettings.db";
char* DB_Name_Bugbase         = "irondragon/bugbase.db";
char* DB_Name_News_Articles   = "irondragon/news_articles.db";
char* DB_Name_Rankings        = "irondragon/rankings.db";
char* DB_FAQ_Entries          = "irondragon/faq.db";

// =---------------------------------------------------------------------------
// Forward Decl
// =---------------------------------------------------------------------------
int            Capture_Web_Server_Variables   ( );
int            Page_Bad_Authentication        ( );
int            Page_Administrators_Only       ( );
int            Capture_Authentication         ( );
int            Authenticate_User              ( );
void           Prepare_User_Pages             ( );
void           Prepare_Admin_Pages            ( );

Page_Function  Find_Page                      ( const char* Page, page_redirect* p_redirect );

// =---------------------------------------------------------------------------
// c g i M a i n
//
// =---------------------------------------------------------------------------
int cgiMain()
{
   char  Page [VARIABLE_BYTES];
   Page_Function p_page = 0;
   int ret = 0;

   Capture_Web_Server_Variables ( );

   // Set up some initial attributes
   //
   Prepare_User_Pages ( );

   // Determine which page is requested and go there
   //
   if ( cgiFormNotFound != cgiFormStringNoNewlines ( "Page", Page, VARIABLE_BYTES) )
   {
      // lookup user pages (no authentication) first
      //
      if ( p_page = Find_Page ( Page, User_Lookup ) )
      {
         ret = (*p_page)();
      }
      // if that fails then lookup admin pages (after authentication)
      //
      else if ( p_page = Find_Page ( Page, Admin_Lookup ) )
      {
         Prepare_Admin_Pages ( );

         // First, authenticate user or redirect them to authenticate page
         // 
         if ( Capture_Authentication ( ) == 0 )
         {

            if ( Authenticate_User ( ) == -1 )
            {
               ret = Page_Bad_Authentication ( );
            }
            else
            {
               if ( User_Is_Back_End_Operator ( Page_Info.Username ) )
                  ret = (*p_page)();
               else
                  ret = Page_Administrators_Only ( );
            }

         }
         else
         {
            ret = Page_Authenticate ( );
         }
      }
      else
      {
         ret = Page_Invalid_Request ( );
      }
   }
   else
   {
      // This is our default page in case the user made an erroneous query
      // or they simply hit /cgi-bin/cgi_id.exe
      //
      ret = Page_Homepage ( );
   }

   if ( Page_Info.Server_Sock )
   {
      SS_Net_Close_Socket ( Page_Info.Server_Sock );
      Page_Info.Server_Sock = 0;
   }

   return ret;
}

// =---------------------------------------------------------------------------
// Find_Page
//
// =---------------------------------------------------------------------------
Page_Function Find_Page ( const char* Page, page_redirect* p_redirect )
{
   int i = 0;

   while ( p_redirect[i].Fn )
   {
      if ( strcmp( Page, p_redirect[i].Page ) == 0 )
         return p_redirect[i].Fn;

      i++;
   }

   return 0;
}

// =---------------------------------------------------------------------------
// Prepare_User_Pages
//
// =---------------------------------------------------------------------------
void Prepare_User_Pages ( )
{
   SS_HTML_Set_Stylesheet ( "http://www.irondragon.org/styles/user.css" );
   Page_Family = PF_USERPAGES;
}

// =---------------------------------------------------------------------------
// Prepare_Admin_Pages
//
// =---------------------------------------------------------------------------
void Prepare_Admin_Pages ( )
{
   SS_HTML_Set_Stylesheet ( "http://www.irondragon.org/styles/default.css" );
   Page_Family = PF_ADMINPAGES;
}

// =---------------------------------------------------------------------------
// C a p t u r e _ W e b _ S e r v e r _ V a r i a b l e s
//
// =---------------------------------------------------------------------------
int Capture_Web_Server_Variables ( )
{
   char SysSet_Buffer [SYSSET_VALUE_BYTES];

   // Initialize Web variables
   //
   Web_Vars.cgiServerSoftware    = cgiServerSoftware  ;
   Web_Vars.cgiServerName        = cgiServerName      ;
   Web_Vars.cgiGatewayInterface  = cgiGatewayInterface;
   Web_Vars.cgiServerProtocol    = cgiServerProtocol  ;
   Web_Vars.cgiServerPort        = cgiServerPort      ;
   Web_Vars.cgiRequestMethod     = cgiRequestMethod   ;
   Web_Vars.cgiPathInfo          = cgiPathInfo        ;
   Web_Vars.cgiPathTranslated    = cgiPathTranslated  ;
   Web_Vars.cgiScriptName        = cgiScriptName      ;
   Web_Vars.cgiQueryString       = cgiQueryString     ;
   Web_Vars.cgiRemoteHost        = cgiRemoteHost      ;
   Web_Vars.cgiRemoteAddr        = cgiRemoteAddr      ;
   Web_Vars.cgiAuthType          = cgiAuthType        ;
   Web_Vars.cgiRemoteUser        = cgiRemoteUser      ;
   Web_Vars.cgiRemoteIdent       = cgiRemoteIdent     ;
   Web_Vars.cgiContentType       = cgiContentType     ;
   Web_Vars.cgiAccept            = cgiAccept          ;
   Web_Vars.cgiUserAgent         = cgiUserAgent       ;

   // Initialize Page_Info
   //
   Page_Info.Port = 0;
   Page_Info.Server_Name[0] = '\0';
   Page_Info.Username[0] = '\0';
   Page_Info.Password[0] = '\0';
   Page_Info.Server_Sock = -1;

   SS_Port_ZeroMemory ( Page_Info.Raw_Auth_Reply, 1024 );

   if ( 0 != SysSet_Get_Value ( "Server_Name", SysSet_Buffer ) )
      strcpy ( SysSet_Buffer, "localhost" );
   SS_Port_Strcpy_Len ( Page_Info.Server_Name, SysSet_Buffer, SERVER_NAME_BYTES );

   if ( 0 != SysSet_Get_Value ( "Server_Port", SysSet_Buffer ) )
      Page_Info.Port = htons((uint_16)44099 );
   else
      Page_Info.Port = htons((uint_16)atoi(SysSet_Buffer));


   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ B a d _ A u t h e n t i c a t i o n
// =---------------------------------------------------------------------------
int Page_Bad_Authentication ( )
{
   nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;

   SS_HTML_Start_Page ( "Invalid Authentication" );
   SS_HTML_Heading_Tag ( "Invalid Authentication", 1 );

   SS_HTML_Write ( "<p>" );
   SS_HTML_Table_Tag ();
   SS_HTML_Write ( "<TR><TD align=center><STRONG>REASON</STRONG></TD></TR>", p_auth_reply->CZ_Message );
   SS_HTML_Write ( "<TR><TD><I>%s</I></TD></TR>", p_auth_reply->CZ_Message );
   SS_HTML_End_Table_Tag ();
   SS_HTML_Write ( "</p>" );

   Page_Body_Authenticate ( );

   Write_Generic_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}

// =---------------------------------------------------------------------------
// P a g e _ A d m i ni s t r a t o r s _ O n l y
// =---------------------------------------------------------------------------
int Page_Administrators_Only ( )
{
   SS_HTML_Start_Page ( "Administrators Only" );
   SS_HTML_Heading_Tag ( "Administrators Only", 1 );

   Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "You have tried to access a page available only to administrators." );
      Write_User_Anchor ( "", "Rail Empires: Iron Dragon Home" );
   Write_End_ParaTable ( );

   Write_Generic_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}
// =---------------------------------------------------------------------------
// C a p t u r e _ A u t h e n t i c a t i o n 
// =---------------------------------------------------------------------------
int Capture_Authentication ( )
{
   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "Username", Page_Info.Username, VARIABLE_BYTES ) ) return -1;
   if ( cgiFormNotFound == cgiFormStringNoNewlines ( "Password", Page_Info.Password, VARIABLE_BYTES ) ) return -1;

   return 0;
}

// =---------------------------------------------------------------------------
// A u t h e n t i c a t e _ U s e r
// =---------------------------------------------------------------------------
int Authenticate_User ( )
{
   nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;
   
   // Authenticate user if they actually specified a username/password
   //
   Page_Info.Server_Sock = CZ_Connect_And_Authenticate ( 
      Page_Info.Username, Page_Info.Password, p_auth_reply );

   if ( Page_Info.Server_Sock <= 0 )
      return -1;

   return 0;
}

