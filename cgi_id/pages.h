
#ifndef __PAGES_H__
#define __PAGES_H__

#include "net_messages_admin.h"

// =---------------------------------------------------------------------------
// End-User Pages
//

int  Page_Homepage         ( );
int  Page_Product_Description ( );
int  Page_Reviews          ( );
int  Page_Forums           ( );
int  Page_Invalid_Request  ( );
int  Page_News             ( );
int  Page_Ranking          ( );
int  Page_Rank_Calculator  ( );
int  Page_Rank_Calculator_Results ( );
int  Page_FAQ              ( );
int  Page_Order            ( );
int  Page_Download         ( );
int  Page_Download_Reg     ( );
int  Page_Miva             ( );
int  Page_Request_Beta_License ( );
int  Send_License_File     ( const char* email, const char* Filename, uint_08* Lic_Data );
int  Generate_License_File ( const char* fname,
                             const char* lname,
                             const char* username, 
                             const char* password,
                             int         Permit,
                             uint_08*    Lic_Data, 
                             const char* Lic_Filename );

int  Page_Support          ( );

int Write_System_Requirements ( );
int Write_Download_Link    ( int Type );

typedef enum DOWNLOAD_TYPES
{
   DT_REGISTEREDEVAL       = 1,
   DT_ANONYMOUSEVAL        = 2,
   DT_BETADOWNLOAD         = 3
} DOWNLOAD_TYPES;

// =---------------------------------------------------------------------------
// Page Output Functions
//
int  Page_Main_Menu        ( );
int  Page_Authenticate     ( ); int Page_Body_Authenticate  ( );
int  Page_Daemon_Stats     ( );
int  Page_Log_Monitor      ( );

// =---------------------------------------------------------------------------
// User Admin Pages
int  Page_User_Listing      ( );
int  Page_Edit_User         ( );
int  Page_Mass_User_Options ( );
int  Edit_User_Body         ( nm_user_record* p_ur, int Recnum );
int  Page_Delete_User       ( );
int  Page_Add_User          ( );
int  Page_Web_Stats         ( );
int  Page_Add_User_Result   ( );
int  Page_Edit_User_Result  ( );
int  Page_Admin_Doc_Request ( );
// =---------------------------------------------------------------------------
// Bug Report Pages
//
int Page_Bug_Repository    ( );
int Page_Add_Bug           ( );
int Page_Add_Bug_Confirm   ( );
int Page_Add_Bug_Result    ( );
int Page_List_Bugs         ( );
int Page_Find_Bug          ( );
int Page_Edit_Bug          ( );
int Page_Edit_Bug_Result   ( );

// =---------------------------------------------------------------------------
// News Administration Pages
//
int Page_News_Admin        ( );

int Page_Body_Request_Summary_Report_Fields ( );
int Page_Body_Request_Bug_Search_Criteria   ( );

// =---------------------------------------------------------------------------
// FAQ Admin Pages
//
int Page_FAQ_Admin         ( );

// =---------------------------------------------------------------------------
// System Settings Pages
// 
int  Page_SysSet           ( );


// =---------------------------------------------------------------------------
// CGI-ID-specific Helpers
//
void Write_Admin_Footer    ( );
void Write_User_Footer     ( );
void Write_User_Link_Bar   ( );

void Write_Generic_Footer  ( );
void Format_Script_Link    ( char* Link_Buf, const char* Page );
void Write_Script_Form     ( const char* Page );
void Write_User_Anchor     ( const char* Page, const char* Text );
void Write_Under_Construction ( );

void Write_Required_Field_Asterisk ( );

void Write_Row_Input       ( const char* Header, const char* Input_Value, int Required );
void Write_Row_Input_Password ( const char* Header, const char* Input_Value, int Required );

// =---------------------------------------------------------------------------
// ParaTable Methods
//
void Write_ParaTable       ( const char* Class );
void Write_ParaTable_Ex    ( const char* Class, const char* TABLE_Extra, const char* TD_Extra );
void Write_End_ParaTable   ( );

extern const char* PT_STANDARD;
extern const char* PT_STANDARDLEFT;
extern const char* PT_NEWS;
extern const char* PT_FAQ;

#endif // __PAGES_H__

