// =---------------------------------------------------------------------------
// c g i _ i d _ s u b s y s t e m s . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    subsystem interface definitions
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE          WHO   REVISION
//   -----------   ---   --------------------------------------------------------
//   2000 Jan 18   nic   Created.
//

#include "standard_types.h"
#include "cgi_id_definitions.h"

#ifndef __cplusplus
typedef int bool;
#endif

//
// Subsystem Net
//

#include "net_subsystem.h"

// 
// Subsystem SS_File
//

#include "file_io_subsystem.h"

// 
// Subsystem SS_Port (generic utility fns)
//

#include "port_subsystem.h"

//
// Subsystem SS_Message
//

typedef enum MSGCHANNELS
{
   i=0
} MSGCHANNELS;

void SS_Message_Write ( MSGCHANNELS Channel, const char* format, ... );

// 
// Subsystem SS_HTML
//

int  SS_Set_NewLine_Writes    ( int newline_writes );
int  SS_HTML_Write            ( const char* format, ... );
int  SS_HTML_WriteP           ( const char* format, ... );
int  SS_HTML_Table_Width      ( int New_Width );
void SS_HTML_Set_Stylesheet   ( const char* stylesheet );
void SS_HTML_Start_Page       ( const char* title );
void SS_HTML_End_Page         ( );
void SS_HTML_Start_XML_Page   ( );
void SS_HTML_End_XML_Page     ( );
void SS_HTML_Heading_Tag      ( const char* heading, int level );
void SS_HTML_Table_Tag        ( );
void SS_HTML_Table_Tag_Ex     ( const char* Attributes );
void SS_HTML_End_Table_Tag    ( );
void SS_HTML_Input_Text       ( const char* Name, const char* Attributes );
void SS_HTML_Input_TextArea   ( const char* Name, int cols, int rows );
void SS_HTML_Input_Password   ( const char* Name, const char* Attributes );
void SS_HTML_Input_Hidden     ( const char* Name, const char* Value );
void SS_HTML_Submit           ( const char* Value );
void SS_HTML_Form             ( const char* Action );
void SS_HTML_End_Form         ( );
void SS_HTML_Image            ( const char* Image_Path, const char* Alt );
void SS_HTML_Start_Bullet      ( );
void SS_HTML_Write_Bullet      ( const char* Item );
void SS_HTML_End_Bullet        ( );
void SS_HTML_Encode_Symbols   ( char* p_buf, int bufsize, int replace_brackets );

//
// Subsystem Database
//

#include "database_subsystem.h"

const char* SS_HTML_Mailto ( const char* email );



