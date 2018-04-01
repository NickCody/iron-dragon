// =---------------------------------------------------------------------------
// h t m l _ s u b s y s t e m . c
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
//   DATE         WHO   REVISION
//   -----------  ---   --------------------------------------------------------
//   2000 Jan 20  nic   Created.
//

#ifdef UNIX
#include <stdarg.h>
#endif

#include "cgi_id_subsystems.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// Define constants
#define WRITE_BUFFER_BYTES    4096
#define DEF_CELLPADDING       3
#define DEF_CELLSPACING       1
#define DEF_TABLEALIGN        "center"
#define DEF_TABLEWIDTH        -1          // no specific width, blank

#define STYLESHEET_BYTES      64

// =---------------------------------------------------------------------------
// Define Local Variables
//
char Write_Buffer[WRITE_BUFFER_BYTES];
char Write_BufferP[WRITE_BUFFER_BYTES];
int  Table_Attr_CellPadding   = DEF_CELLPADDING;
int  Table_Attr_CellSpacing   = DEF_CELLSPACING;
int  Table_Attr_Width         = DEF_TABLEWIDTH;

char MailTo_Temp [ VARIABLE_BYTES ];

char Active_Stylesheet[STYLESHEET_BYTES];

#ifdef _DEBUG
int  NewLine_Writes = 1;
#else
int  NewLine_Writes = 0;
#endif

// =---------------------------------------------------------------------------
// S S _ H T M L _ W r i t e
//
// Takes a variable-length argument list and formats it using cgiOut
//
// =---------------------------------------------------------------------------
int SS_Set_NewLine_Writes ( int newline_writes )
{
   int temp = NewLine_Writes;
   NewLine_Writes = newline_writes;
   return temp;
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ W r i t e
//
// Takes a variable-length argument list and formats it using cgiOut
//
// =---------------------------------------------------------------------------
int SS_HTML_Write ( const char* format, ... )
{
   va_list v1;
   va_start ( v1, format );
   vsprintf ( Write_Buffer, (char*)format, v1 );
   va_end ( v1 );

   if ( NewLine_Writes )
      strcat ( Write_Buffer, "\n" );

   return fprintf ( cgiOut, Write_Buffer );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ W r i t e P
//
// Same as SS_HTML_Write, but encloses argument in <p>
//
// =---------------------------------------------------------------------------
int SS_HTML_WriteP ( const char* format, ... )
{
   int ret;
   va_list v1;

   va_start ( v1, format );
   vsprintf ( Write_BufferP, (char*)format, v1 );
   va_end ( v1 );

#ifdef _DEBUG
   strcat ( Write_BufferP, "\n" );
#endif

   SS_HTML_Write ( "<p>" );
   ret = fprintf ( cgiOut, Write_BufferP );
   SS_HTML_Write ( "</p>" );

   return ret;
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ S e t _ S t y l e s h e e t
//
// =---------------------------------------------------------------------------
void SS_HTML_Set_Stylesheet ( const char* stylesheet )
{
   if ( stylesheet && strlen(stylesheet) )
      strcpy ( Active_Stylesheet, stylesheet );
   else
      strcpy ( Active_Stylesheet, "/styles/default.css" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ S t a r t _ P a g e
//
// =---------------------------------------------------------------------------
void SS_HTML_Start_Page ( const char* title )
{
   cgiHeaderContentType("text/html");

   // For Strict HTML 4.0
   //
   SS_HTML_Write ( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">" );

   // For Strict XHTML
   //
   //SS_HTML_Write ( "<!DOCTYPE html" );
   //SS_HTML_Write ( "   PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" );
   //SS_HTML_Write ( "   \"DTD/xhtml1-strict.dtd\">" );

   SS_HTML_Write ( "<html>" );
   SS_HTML_Write ( "<head>" );
   SS_HTML_Write ( "<title>%s</title>", title );

   if ( Active_Stylesheet && Active_Stylesheet[0] )
      SS_HTML_Write ( "<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">", Active_Stylesheet );

   //SS_HTML_Write ( "<style fprolloverstyle>A:hover {color: #FF0000; text-decoration: overline}</style>" );
   SS_HTML_Write ( "</head>" );

   SS_HTML_Write ( "<body>" );
   SS_HTML_Write ( "<p><a href=\"%s\">", CGI_ID_SCRIPT );
   SS_HTML_Write ( "<img src=\"/styles/images/IronDraginRE.gif\" alt=\"Rail Empires: Iron Dragon\">" );
   SS_HTML_Write ( "</a></p>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ E n d _ P a g e
// =---------------------------------------------------------------------------
void SS_HTML_End_Page ( )
{
   SS_HTML_Write ( "</body>" );
   SS_HTML_Write ( "</html>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ S t a r t _ X M L _ P a g e
//
// =---------------------------------------------------------------------------
void SS_HTML_Start_XML_Page ( )
{
   cgiHeaderContentType("text/html");

   SS_HTML_Write ( "<?xml version=\"1.0\" ?>" );

   // For Strict HTML 4.0
   //
   //SS_HTML_Write ( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">" );

   SS_HTML_Write ( "<idpcd>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ E n d _ X M L _ P a g e
// =---------------------------------------------------------------------------
void SS_HTML_End_XML_Page ( )
{
   SS_HTML_Write ( "</idpcd>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ H e a d i n g _ T a g
// =---------------------------------------------------------------------------
void SS_HTML_Heading_Tag ( const char* heading, int level )
{
   SS_HTML_Write ( "<h%d>%s</H%d>\n", level, heading, level );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ T a b l e _ T a g
// =---------------------------------------------------------------------------
void SS_HTML_Table_Tag ( )
{
   if ( Table_Attr_Width == -1 )
      SS_HTML_Write ( "<table cellpadding=\"%d\" cellspacing=\"%d\">",
         Table_Attr_CellPadding, Table_Attr_CellSpacing );
   else
      SS_HTML_Write ( "<table cellpadding=\"%d\" cellspacing=\"%d\" width=\"%d\">",
         Table_Attr_CellPadding, Table_Attr_CellSpacing, Table_Attr_Width );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ T a b l e _ T a g _ E x
// =---------------------------------------------------------------------------
void SS_HTML_Table_Tag_Ex ( const char* Attributes )
{
   SS_HTML_Write ( "<table %s>", Attributes );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ E n d _ T a b l e _ T a g 
// =---------------------------------------------------------------------------
void SS_HTML_End_Table_Tag ( )
{
   SS_HTML_Write ( "</table>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ T a b l e _ W i d t h
// =---------------------------------------------------------------------------
int SS_HTML_Table_Width ( int New_Width )
{
   int temp = Table_Attr_Width;
   Table_Attr_Width = New_Width;
   return temp;
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ M a i l t o
// =---------------------------------------------------------------------------
const char* SS_HTML_Mailto ( const char* email )
{
   sprintf ( MailTo_Temp, "<A href=\"mailto:%s\">%s</A>", email, email );
   return MailTo_Temp;
}


// =---------------------------------------------------------------------------
// S S _ H T M L _ F o r m
// =---------------------------------------------------------------------------
void SS_HTML_Form ( const char* Action ) 
{
   SS_HTML_Write ( "<form method=\"post\" action=\"%s\">", Action );
   SS_HTML_Write ( "<p>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ E n d _ F o r m
// =---------------------------------------------------------------------------
void SS_HTML_End_Form ( ) 
{
   //SS_HTML_Write ( "</p>" );
   SS_HTML_Write ( "</form>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ I n p u t _ T e x t
// =---------------------------------------------------------------------------
void SS_HTML_Input_Text ( const char* Name, const char* Attributes )
{
   SS_HTML_Write ( "<input type=\"text\" name=\"%s\" %s>", Name,
      Attributes ? Attributes : "" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ I n p u t _ T e x t A r e a
// =---------------------------------------------------------------------------
void SS_HTML_Input_TextArea ( const char* Name, int cols, int rows )
{
   SS_HTML_Write ( "<textarea cols=\"%d\" rows=\"%d\" name=\"%s\"></textarea>", 
      cols, rows, Name );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ I n p u t _ P a s s w o r d
// =---------------------------------------------------------------------------
void SS_HTML_Input_Password ( const char* Name, const char* Attributes )
{
   SS_HTML_Write ( "<input type=\"password\" name=\"%s\" %s>", Name,
      Attributes ? Attributes : "" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ I n p u t _ H i d d e n
// =---------------------------------------------------------------------------
void SS_HTML_Input_Hidden ( const char* Name, const char* Value )
{
   SS_HTML_Write ( "<input type=\"hidden\" name=\"%s\" value=\"%s\">", Name, Value );
}

// =---------------------------------------------------------------------------
// SS_HTML_Submit
// =---------------------------------------------------------------------------
void SS_HTML_Submit ( const char* Value )
{
   // This gets rid of warning...
   //
   Value = 0;
   //SS_HTML_Write ( "<INPUT type=submit value=%s>", Value );

   if ( Page_Family == PF_USERPAGES )
      SS_HTML_Write ( "<input src=\"/styles/images/Goblk.gif\" type=\"image\">" );
   else
      SS_HTML_Write ( "<input src=\"/styles/images/Go.gif\" type=\"image\">" );
}

// =---------------------------------------------------------------------------
// SS_HTML_Image
// =---------------------------------------------------------------------------
void SS_HTML_Image ( const char* Image_Path, const char* Alt )
{
   SS_HTML_WriteP ( "<img src=\"%s\" alt=\"%s\">", Image_Path, Alt );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ S t a r t _ B u l l e t
// =---------------------------------------------------------------------------
void SS_HTML_Start_Bullet ( )
{
   SS_HTML_Write ( "<p><ul>" );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ W ri t e _ B u l l e t
// =---------------------------------------------------------------------------
void SS_HTML_Write_Bullet ( const char* Item )
{
   SS_HTML_Write ( "<li>%s</li>", Item );
}

// =---------------------------------------------------------------------------
// S S _ H T M L _ E n  d _ B u l l e t
// =---------------------------------------------------------------------------
void SS_HTML_End_Bullet ( )
{
   SS_HTML_Write ( "</ul></p>" );
}


// =---------------------------------------------------------------------------
// S S _ H T M L _ E n c o d e _ S y m b o l s
//
// Sucks in the input p_buf and writes back into it p_buf
// 
// Replaces < with &lt;    (removed, since it caused conflicts wuth crlf to <BR> conversion
//          > with &gt;    (removed, since it caused conflicts wuth crlf to <BR> conversion
//          & with &amp;
//          " with &quot;
//          ' with &apos;  (removed, IE doesn't seem to expand it)
//           cr/lf with <br>
//
// =---------------------------------------------------------------------------
void SS_HTML_Encode_Symbols ( char* p_buf, int bufsize, int replace_brackets )
{
   char* p_tmp     = 0;
   char* p_buf_ptr = p_buf;
   char* p_tmp_ptr = 0;

   p_tmp = (char*)SS_Port_AllocMem ( bufsize );
   
   if ( !p_tmp ) return;

   SS_Port_ZeroMemory ( p_tmp, bufsize );

   p_tmp_ptr = p_tmp;

   // While our incoming buffer is not at the end and out
   // outgoing buffer hasn't overfilled...
   //
   while ( (*p_buf_ptr) && (p_tmp_ptr < p_tmp + bufsize) )
   {
      switch ( *p_buf_ptr )
      {
         case 13:
            // skip
         case 10:
            if ( p_tmp_ptr + 4 < p_tmp + bufsize )
               strcat ( p_tmp_ptr, "<BR>" );
            p_tmp_ptr += 4;
            break;

         case '&' :
            if ( p_tmp_ptr + 5 < p_tmp + bufsize )
               strcat ( p_tmp_ptr, "&amp;" );
            p_tmp_ptr += 5;
            break;

         case '\"' :
            if ( p_tmp_ptr + 6 < p_tmp + bufsize )
               strcat ( p_tmp_ptr, "&quot;" );
            p_tmp_ptr += 6;
            break;

         //case '\'' :
         //   strcat ( p_tmp_ptr, "&apos;" );
         //   p_tmp_ptr += 6;
         //   break;

         case '<' :

            if ( replace_brackets )
            {
               if ( p_tmp_ptr + 4 < p_tmp + bufsize )
                  strcat ( p_tmp_ptr, "&lt;" );
               p_tmp_ptr += 4;

               break;
            }

            // else fall through

         case '>' :

            if ( replace_brackets )
            {
               if ( p_tmp_ptr + 4 < p_tmp + bufsize )
                  strcat ( p_tmp_ptr, "&gt;" );
               p_tmp_ptr += 4;
               break;
            }

            // else fall through

         default:
            if ( p_tmp_ptr + 1 < p_tmp + bufsize )
               *p_tmp_ptr = *p_buf_ptr;
            p_tmp_ptr++;
            break;
      }

      p_buf_ptr++;
   }

   strcpy ( p_buf, p_tmp );

   SS_Port_FreeMem ( p_tmp );
}

