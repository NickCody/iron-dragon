#ifndef __CGI_ID_DEFINITIONS_H__
#define __CGI_ID_DEFINITIONS_H__

// =---------------------------------------------------------------------------
//
// Constants and Macro Definitions
//
// =---------------------------------------------------------------------------
#define CGI_ID_SCRAMBLE_KEY      0xE4
#define VARIABLE_BYTES           80
#define LINK_BYTES               1024
#define PF_UNDEFINEDPAGES        0
#define PF_USERPAGES             1
#define PF_ADMINPAGES            2
#define MIN_PASSWORD_LENGTH      6
#define MAX_HTML_FRAGMENT        (100*1024)
#define USER_LIST_PAGE_SIZE      100

#define CGI_ID_VERSION           "1.1.02162000"
#define CGI_ID_SCRIPT            "/cgi-bin/cgi_id.exe"
#define CGI_ID_SCRIPT_FULLPATH   "http://www.irondragon.org//cgi-bin/cgi_id.exe"

// =---------------------------------------------------------------------------
//
// struct cgi_Variables
//
// This struct is useful for allowing c files to access these
// variables, without the need to include cgic.h
//
// =---------------------------------------------------------------------------
typedef struct cgi_Variables
{
   const char* cgiServerSoftware;
   const char* cgiServerName;
   const char* cgiGatewayInterface;
   const char* cgiServerProtocol;
   const char* cgiServerPort;
   const char* cgiRequestMethod;
   const char* cgiPathInfo;
   const char* cgiPathTranslated;
   const char* cgiScriptName;
   const char* cgiQueryString;
   const char* cgiRemoteHost;
   const char* cgiRemoteAddr;
   const char* cgiAuthType;
   const char* cgiRemoteUser;
   const char* cgiRemoteIdent;
   const char* cgiContentType;
   const char* cgiAccept;
   const char* cgiUserAgent;
} cgi_Variables;

extern cgi_Variables Web_Vars;

// Page Information common for most web pages to pass around
//
typedef struct common_page_info
{

   char     Server_Name [VARIABLE_BYTES];
   uint_16  Port;
   int      Server_Sock;
   uint_08  Raw_Auth_Reply[1024];

   char     Username    [VARIABLE_BYTES];
   char     Password    [VARIABLE_BYTES];

} common_page_info;

extern common_page_info Page_Info;
extern int Page_Family;

extern char* DB_Name_System_Settings;
extern char* DB_Name_Bugbase;
extern char* DB_Name_News_Articles;
extern char* DB_Name_Rankings;
extern char* DB_FAQ_Entries;

#endif // __CGI_ID_DEFINITIONS_H__
