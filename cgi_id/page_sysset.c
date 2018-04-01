// =---------------------------------------------------------------------------
// p a g e _ s y s s e t. c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the HTML interface to the system settings database
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Jan 24   nic   Created.
//

#include <stdlib.h>
#include "cgi_id_subsystems.h"
#include "pages.h"
#include "cgic.h"
#include "databases.h"

typedef struct SysSet_Form
{
   char Name  [ VARIABLE_BYTES ];
   char Value [ VARIABLE_BYTES ];
} SysSet_Form;

// =---------------------------------------------------------------------------
// P a g e _ S y s S e t
//
// =---------------------------------------------------------------------------
int Page_SysSet ( )
{
   int         ret = -1;
   char        Set [ VARIABLE_BYTES ];
   SysSet_Form frm;

   SS_HTML_Start_Page  ( "RE System Settings" );
   SS_HTML_Heading_Tag ( "RE System Settings", 1 );

   if ( cgiFormNotFound != cgiFormStringNoNewlines ( "Set", Set, VARIABLE_BYTES) )
   {
      if ( atoi(Set) == 1 )
      {
         frm.Name[0] = 0; frm.Value[0] = 0;

         if ( cgiFormNotFound != cgiFormStringNoNewlines ( "frmName",  frm.Name,  VARIABLE_BYTES) &&
              cgiFormNotFound != cgiFormStringNoNewlines ( "frmValue", frm.Value, VARIABLE_BYTES) )
         {
            if ( strlen(frm.Name) )
               ret = SysSet_Set_Value ( frm.Name, frm.Value, 1 );
         }
      }

      if ( ret != 0 )
         SS_HTML_WriteP ( "We failed to change or add Name[%s] to Value[%s].", frm.Name, frm.Value );
   }

   ret = Write_SysSet_Table ( );

   // Now, write out the form so the user can add or change a setting
   //
   Write_Script_Form ( "SysSet" );
   SS_HTML_Input_Hidden ( "Set", "1" );

      Write_ParaTable ( PT_STANDARD );
      SS_HTML_WriteP ( "To <b>add</b> a record, use a unique setting name. To <b>delete</b> a record set its value to <i>(delete)</i>." );
      Write_End_ParaTable();
      SS_HTML_Table_Tag_Ex ( "width=\"500\"" );
         
         SS_HTML_Write ( "<tr>" );
         SS_HTML_Write ( "<th>Setting Name (%d chars max)</th>", (int)SYSSET_NAME_BYTES );
         SS_HTML_Write ( "<th colspan=\"2\">Value (%d chars max)</th>", (int)SYSSET_VALUE_BYTES );
         SS_HTML_Write ( "</tr>" );


         SS_HTML_Write ( "<tr>" );
         SS_HTML_Write ( "<td>" );
            SS_HTML_Input_Text ( "frmName", "size=\"32\"" );
         SS_HTML_Write ( "</td>" );
         SS_HTML_Write ( "<td>" );
            SS_HTML_Input_Text ( "frmValue", "size=\"48\"" );
         SS_HTML_Write ( "</td>" );
         SS_HTML_Write ( "<td>" );
            SS_HTML_Submit ( "" );
         SS_HTML_Write ( "</td>" );
         SS_HTML_Write ( "</tr>" );
      SS_HTML_End_Table_Tag ( );

   SS_HTML_End_Form ( );

   Write_Admin_Footer ( );
   SS_HTML_End_Page ( );

   return 0;
}
