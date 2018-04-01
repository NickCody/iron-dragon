//
// bugrep_actions.c
//

#include <stdlib.h>
#include "cgi_id_subsystems.h"
#include "cgic.h"
#include "bugrep_actions.h"
#include "connection_zone.h"
#include "pages.h"
#include "databases.h"
#include "users.h"

char Unknown_String [ 80 ];
char Bug_Icon_Link[LINK_BYTES] = "<IMG src=\"/styles/images/bug_icon.gif\" alt=\"Bug\">";

// =---------------------------------------------------------------------------
// A c t i o n _ G a t h e r _ B u g _ F i e l d s
//
// Try and gather as many fields from the cgi environment as we can. 
// The caller is responsible for making sense of the results.
// =---------------------------------------------------------------------------

int Action_Gather_Bug_Fields ( bug_form* p_form )
{
   SS_Port_ZeroMemory ( p_form, sizeof(bug_form) );

   cgiFormStringNoNewlines ( "bugUsername"   , p_form->BugUsername, VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugEmail"      , p_form->Email      , VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugVersion"    , p_form->Version    , VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugOS"         , p_form->OS         , VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugBugType"    , p_form->Bug_Type   , VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugPriority"   , p_form->Priority   , VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugCategory"   , p_form->Category   , VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "bugStatus"     , p_form->Status     , VARIABLE_BYTES );
   cgiFormString           ( "bugSummary"    , p_form->Summary    , SUMMARY_BYTES  );
   cgiFormString           ( "bugDescription", p_form->Description, DESCRIPTION_BYTES );
   cgiFormString           ( "bugSteps"      , p_form->Repro_Steps, REPRO_STEPS_BYTES );
   cgiFormString           ( "bugComments"   , p_form->Comments   , COMMENTS_BYTES );
   cgiFormString           ( "bugResolution" , p_form->Resolution , RESOLUTION_BYTES );

   SS_Port_Get_DateTime_String ( p_form->Date_Reported );

   cgiFormStringNoNewlines ( "Bugid"      , p_form->Bugid      , VARIABLE_BYTES );

   // Crop the dangerous ones
   //
   p_form->BugUsername   [USERNAME_BYTES -1] = '\0';
   p_form->Email         [EMAIL_BYTES    -1] = '\0';
   p_form->Version       [VERSION_BYTES  -1] = '\0';

   // Form variables need to be properly escaped ( " goes to &quot, etc)
   //

   SS_HTML_Encode_Symbols   ( p_form->Summary    , SUMMARY_BYTES    , 0 );
   SS_HTML_Encode_Symbols   ( p_form->Description, DESCRIPTION_BYTES, 0 );
   SS_HTML_Encode_Symbols   ( p_form->Repro_Steps, REPRO_STEPS_BYTES, 0 );
   SS_HTML_Encode_Symbols   ( p_form->Comments,    COMMENTS_BYTES   , 0 );
   SS_HTML_Encode_Symbols   ( p_form->Resolution,  RESOLUTION_BYTES , 0 );

   return 0;
}

// =---------------------------------------------------------------------------
// A c t i o n _ A d d _ B u g 
//
// Bugbase protection is achieved through disallowing multiple opens
// We open bugbase, add the bug, and close the bugbase.
// =---------------------------------------------------------------------------
int Action_Add_Bug ( bug_form* p_form, bug_record* p_rec, uint_32* p_bugid )
{
   db_metastructure ms;
   int ret;

   *p_bugid = 0xFFFFFFFF;

   ret = Bug_Form_To_Record ( p_form, p_rec, 0 /*merge*/ );
   if ( ret != 0 ) SS_HTML_WriteP ( "Invalid CGI data from form!" );
   
   if ( ret == 0 )
   {
      SS_DB_Initialize_MetaStructure ( &ms, sizeof(bug_record), 0, 0 );

      // Try to open the bugbase
      //
      ret = SS_DB_Create_Open ( &ms, "", DB_Name_Bugbase );
      if ( ret != 0 ) SS_HTML_WriteP ( "Failed to open/create the database!" );

      if ( ret == 0 )
      {
         ret = SS_DB_Add_Record ( &ms, (uint_08*)p_rec, p_bugid );
         if ( ret != 0 ) SS_HTML_WriteP ( "Failed to add the record!" );

         SS_DB_Close ( &ms );
      }
   }

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ L o o k u p _ B u g 
// =---------------------------------------------------------------------------
int Action_Lookup_Bug ( bug_record* p_rec, uint_32 bugnum )
{
   db_metastructure ms;
   int         ret;

   SS_DB_Initialize_MetaStructure ( &ms, sizeof(bug_record), 0, 0 );

   ret = SS_DB_Create_Open ( &ms, "", DB_Name_Bugbase );
   if ( ret != 0 ) SS_HTML_WriteP ( "Failed to open the database!" );
   
   if ( ret == 0 )
   {
      ret = SS_DB_Get_Record ( &ms, bugnum, (uint_08*)p_rec );

      SS_DB_Close ( &ms );
   }

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ U p d a t e _ B u g
// =---------------------------------------------------------------------------
int Action_Update_Bug ( bug_record* p_rec, uint_32 bugnum )
{
   db_metastructure ms;
   int ret;

   SS_DB_Initialize_MetaStructure ( &ms, sizeof(bug_record), 0, 0 );

   ret = SS_DB_Create_Open ( &ms, "", DB_Name_Bugbase );
   if ( ret != 0 ) SS_HTML_WriteP ( "Failed to open the database!" );
   
   if ( ret == 0 )
   {
      ret = SS_DB_Set_Record ( &ms, bugnum, (uint_08*)p_rec );

      SS_DB_Close ( &ms );
   }

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ E n u m e r a t e _ B u g s
// =---------------------------------------------------------------------------
int Action_Bug_Summary ( uint_08 Status )
{
   db_metastructure ms;
   bug_record  rec;
   uint_32     rec_count;
   int         ret;
   sint_32     i;

   SS_DB_Initialize_MetaStructure ( &ms, sizeof(bug_record), 0, 0 );

   // Try to open the bugbase
   //
   ret = SS_DB_Create_Open ( &ms, "", DB_Name_Bugbase );
   if ( ret != 0 ) SS_HTML_WriteP ( "Failed to open the database!" );

   if ( ret == 0 )
   {
      rec_count = SS_DB_Get_Record_Count ( &ms );

      //SS_HTML_WriteP ( "There are a total of %d bugs in the bugbase.", rec_count );

      SS_HTML_Table_Tag ();

      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<th>BugID</th><th>Date Reported</th><th>Version</th><th>Priority</th><th>Category</th><th>Status</th><th>Summary</th>" );
      SS_HTML_Write ( "</tr>" );


      // reverse count
      for ( i = (sint_32)rec_count-1; i >= 0 ; i-- )
      //for ( i = 0; i < rec_count; i++ )
      {
         ret = SS_DB_Get_Record ( &ms, i, &rec );
         
         if ( ret != 0 ) 
            SS_HTML_WriteP ( "Failed to get record #%d, continuing...", i );
         else if ( Status == rec.Status ) 
            Write_Bug_Summary_Row ( &rec, i );

      }

      SS_HTML_End_Table_Tag ();

      SS_DB_Close ( &ms );
   }

   return ret;
}

// =---------------------------------------------------------------------------
// W r i t e _ B u g _ C o n f i r m a t i o n _ T a b l e
//
// Precondition: form field variables have already been escaped
//
// =---------------------------------------------------------------------------

int Write_Bug_Confirmation_Table ( bug_form* p_form )
{
   int      Old_Width   = SS_HTML_Table_Width( 500 );
   uint_08  os          = (uint_08)atoi(p_form->OS);
   uint_08  bugtype     = (uint_08)atoi(p_form->Bug_Type);
   uint_08  priority    = (uint_08)atoi(p_form->Priority);
   uint_08  category    = (uint_08)atoi(p_form->Category);

   SS_HTML_Table_Tag ( );
   SS_HTML_Table_Width( Old_Width );

   SS_HTML_Write ( "<tr><th nowrap> Reporter Username</th> <td> %s &nbsp; </td> </tr>" , p_form->BugUsername );
   SS_HTML_Write ( "<tr><th nowrap> Reporter E-mail  </th> <td> %s &nbsp; </td> </tr>" , p_form->Email );
   SS_HTML_Write ( "<tr><th nowrap> Version          </th> <td> %s &nbsp; </td> </tr>" , p_form->Version );
   SS_HTML_Write ( "<tr><th nowrap> Date Reported    </th> <td> %s &nbsp; </td> </tr>" , p_form->Date_Reported );
   SS_HTML_Write ( "<tr><th nowrap> OS               </th> <td> %s &nbsp; </td> </tr>" , Get_OS_String(os) );
   SS_HTML_Write ( "<tr><th nowrap> BugType          </th> <td> %s &nbsp; </td> </tr>" , Get_BugType_String(bugtype) );
   SS_HTML_Write ( "<tr><th nowrap> Priority         </th> <td> %s &nbsp; </td> </tr>" , Get_Priority_String(priority) );
   SS_HTML_Write ( "<tr><th nowrap> Category         </th> <td> %s &nbsp; </td> </tr>" , Get_Category_String(category) );
   SS_HTML_Write ( "<tr><th       > Summary          </th> <td> %s &nbsp; </td> </tr>" , p_form->Summary );
   SS_HTML_Write ( "<tr><th       > Description      </th> <td> %s &nbsp; </td> </tr>" , p_form->Description );
   SS_HTML_Write ( "<tr><th       > Repro Steps      </th> <td> %s &nbsp; </td> </tr>" , p_form->Repro_Steps );

   SS_HTML_End_Table_Tag ();

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ B u g _ F o r m _ V a r i a b l e s
//
// Precondition: form field variables have already been escaped on capture
//
// =---------------------------------------------------------------------------

int Write_Bug_Form_Variables ( bug_form* p_form )
{
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugUsername\"    value=\"%s\" >", p_form->BugUsername );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugEmail\"       value=\"%s\" >", p_form->Email       );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugVersion\"     value=\"%s\" >", p_form->Version     );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugOS\"          value=\"%s\" >", p_form->OS          );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugBugType\"     value=\"%s\" >", p_form->Bug_Type    );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugPriority\"    value=\"%s\" >", p_form->Priority    );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugCategory\"    value=\"%s\" >", p_form->Category    );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugSummary\"     value=\"%s\" >", p_form->Summary     );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugDescription\" value=\"%s\" >", p_form->Description );
   SS_HTML_Write ( "<INPUT type=\"hidden\" name=\"bugSteps\"       value=\"%s\" >", p_form->Repro_Steps );

   return 0;
}

// =---------------------------------------------------------------------------
// B u g _ F o r m _ T o _ R e c o r d
//
// Translates a bug_form structure (cgi form) to a bug_record structure (database record)
//
// If Merge is true, blank form fields will not be written to the record. See
//   SS_Port_Merge_Strcpy_Len and SS_Port_Merge_Ascii_To_Byte.
// =---------------------------------------------------------------------------
int Bug_Form_To_Record ( bug_form* p_form, bug_record* p_rec, uint_08 Merge )
{
   if ( !Merge )
      SS_Port_ZeroMemory ( p_rec, sizeof(bug_record) );

   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Username,      p_form->BugUsername,    USERNAME_BYTES      );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Email,         p_form->Email,          EMAIL_BYTES         );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Version,       p_form->Version,        VERSION_BYTES       );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Date_Reported, p_form->Date_Reported,  DATE_BYTES          );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Summary,       p_form->Summary,        SUMMARY_BYTES       );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Description,   p_form->Description,    DESCRIPTION_BYTES   );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Repro_Steps,   p_form->Repro_Steps,    REPRO_STEPS_BYTES   );

   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Resolution,    p_form->Resolution,     RESOLUTION_BYTES    );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->Comments,      p_form->Comments,       COMMENTS_BYTES      );

   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->OS       , p_form->OS        );
   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->Bug_Type , p_form->Bug_Type  );
   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->Priority , p_form->Priority  );
   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->Category , p_form->Category  );
   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->Deleted  , p_form->Deleted   );
   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->Status   , p_form->Status    );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ B u g _ D e t a i l _ T a b l e
// =---------------------------------------------------------------------------
int Write_Bug_Detail_Table ( bug_record* p_rec, uint_32 recnum )
{
   int  Old_Width = SS_HTML_Table_Width( 600 );
   char Link [ LINK_BYTES ];

   SS_HTML_Table_Tag ( );
   SS_HTML_Table_Width( Old_Width );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th colspan=2>" );
   SS_HTML_Write ( "    Bug [%d] %s", recnum, Get_Status_String(p_rec->Status) );
   SS_HTML_Write ( "  </th>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr><th align=right>Username            </th> <td>%s&nbsp;</td> </tr>" , p_rec->Username               );
   SS_HTML_Write ( "<tr><th align=right>Email               </th> <td>%s&nbsp;</td> </tr>" , SS_HTML_Mailto(p_rec->Email)  );
   SS_HTML_Write ( "<tr><th align=right>Version             </th> <td>%s&nbsp;</td> </tr>" , p_rec->Version                );
   SS_HTML_Write ( "<tr><th align=right>Date Reported       </th> <td>%s&nbsp;</td> </tr>" , p_rec->Date_Reported          );
   SS_HTML_Write ( "<tr><th align=right>OS                  </th> <td>%s&nbsp;</td> </tr>" , Get_OS_String ( p_rec->OS )   );
   SS_HTML_Write ( "<tr><th align=right>Bug Type            </th> <td>%s&nbsp;</td> </tr>" , Get_BugType_String(p_rec->Bug_Type) );
   SS_HTML_Write ( "<tr><th align=right>Priority            </th> <td>%s&nbsp;</td> </tr>" , Get_Priority_String(p_rec->Priority) );
   SS_HTML_Write ( "<tr><th align=right>Category            </th> <td>%s&nbsp;</td> </tr>" , Get_Category_String(p_rec->Category) );
   SS_HTML_Write ( "<tr><th align=right>Status              </th> <td>%s&nbsp;</td> </tr>" , Get_Status_String(p_rec->Status) );
   SS_HTML_Write ( "<tr><th align=right>Summary             </th> <td>%s&nbsp;</td> </tr>" , p_rec->Summary                );
   SS_HTML_Write ( "<tr><th align=right>Description         </th> <td>%s&nbsp;</td> </tr>" , p_rec->Description            );
   SS_HTML_Write ( "<tr><th align=right>Steps to Reproduce  </th> <td>%s&nbsp;</td> </tr>" , p_rec->Repro_Steps            );
   SS_HTML_Write ( "<tr><th align=right>Comments            </th> <td>%s&nbsp;</td> </tr>" , p_rec->Comments               );
   SS_HTML_Write ( "<tr><th align=right>Resolution          </th> <td>%s&nbsp;</td> </tr>" , p_rec->Resolution             );

   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<tr>" );

      SS_HTML_Write ( "<td>&nbsp;</td>" );
      SS_HTML_Write ( "<td>" );

      SS_HTML_Table_Tag ( );
      SS_HTML_Write ( "<tr>" );

      // Edit Bug Link
      //
      Format_Script_Link ( Link, "Edit_Bug" );
      SS_HTML_Write ( "    <td><A href=\"%s&Bugid=%d\">", Link, (int)recnum );
      SS_HTML_Write ( "      Edit Bug %d", (int)recnum );
      SS_HTML_Write ( "    </A></td>" );

      // Quick change from Open to Fixed
      //
      if ( p_rec->Status == BUGSTATUS_OPEN )
      {
         Format_Script_Link ( Link, "Edit_Bug" );
         //SS_HTML_Write ( "    <td><A href=\"%s&Bugid=%d;Status=%d\">", Link, (int)recnum, (int)BUGSTATUS_FIXED );
         SS_HTML_Write ( "    <td>" );
         SS_HTML_Write ( "      Mark as Fixed", (int)recnum );
         SS_HTML_Write ( "    </A></td>" );
      }

      SS_HTML_Write ( "</tr>" );
      SS_HTML_End_Table_Tag ( );

      SS_HTML_Write ( "</td>" );
      SS_HTML_Write ( "</tr>" );
   }

   SS_HTML_End_Table_Tag ();

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e_  B u g _ S u m m a r y _ R o w 
// =---------------------------------------------------------------------------
int Write_Bug_Summary_Row ( bug_record* p_rec, uint_32 bugid )
{
   char Short_Date   [11];
   char Bugid_Link   [LINK_BYTES];

   SS_Port_Strcpy_Len ( Short_Date, p_rec->Date_Reported, 11 );

   // Create the specified page/link to script
   //
   Format_Script_Link ( Bugid_Link, "Find_Bug" );

   SS_HTML_Write ( "<tr>" );

   // Bugid link
   //
   SS_HTML_Write ( "<td><A href=\"%s&Bugid=%d\">%04d</A></td>", Bugid_Link, (int)bugid, (int)bugid );

   // Date Reported, Version, Priority, Category, Status
   //
   SS_HTML_Write ( "<td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td width=300>%s</td>",
      Short_Date, 
      p_rec->Version,
      Get_Priority_String ( p_rec->Priority),
      Get_Category_String ( p_rec->Category),
      Get_Status_String   ( p_rec->Status ),
      p_rec->Summary );

   SS_HTML_Write ( "</tr>" );
   
   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ B u g _ I n p u t s
//
// p_rec must not be null, though its memory can be nulls
//
// Precondition: bug fields are properly escaped
// =---------------------------------------------------------------------------
int Write_Bug_Inputs ( bug_record* p_rec, uint_32 recnum )
{
   if ( recnum != 0xFFFFFFFF )
   {
      SS_HTML_Write ( "<input type=hidden name=\"Bugid\" value=\"%d\">", (int)recnum );
   }

   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Product</th>" );
   SS_HTML_Write ( "  <td><strong>Rail Empires: Iron Dragon</strong></td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Username</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=30 name=bugUsername value=\"%s\"></td>", p_rec->Username );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>E-Mail</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text name=bugEmail size=20 value=\"%s\"></td>", p_rec->Email );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Version</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=20 name=bugVersion value=\"%s\"></td>", p_rec->Version );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>OS</th>" );
   SS_HTML_Write ( "  <td>" );
                        Write_OS_Select ( p_rec->OS );
   SS_HTML_Write ( "  </td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Type</th>" );
   SS_HTML_Write ( "  <td>" );
                        Write_BugType_Select ( p_rec->Bug_Type );
   SS_HTML_Write ( "  </td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Category</th>" );
   SS_HTML_Write ( "  <td>" );
                        Write_Category_Select ( p_rec->Category );
   SS_HTML_Write ( "  </td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Priority</th>" );
   SS_HTML_Write ( "  <td>" );
                        Write_Priority_Select ( p_rec->Priority );
   SS_HTML_Write ( "  </td>" );
   SS_HTML_Write ( "</tr>" );

   if ( recnum != 0xFFFFFFFF )
   {
      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "  <th align=right>Status</th>" );
      SS_HTML_Write ( "  <td>" );
                           Write_Status_Select ( p_rec->Status );
      SS_HTML_Write ( "  </td>" );
      SS_HTML_Write ( "</tr>" );
   }

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Summary</th>" );
   SS_HTML_Write ( "  <td><INPUT type=text size=50 name=bugSummary value=\"%s\"></td>", p_rec->Summary );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Detailed Description</th>" );
   SS_HTML_Write ( "  <td><textarea rows=7 name=bugDescription cols=50>%s</textarea></td>", p_rec->Description );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <th align=right>Steps to Reproduce</th>" );
   SS_HTML_Write ( "  <td><textarea rows=7 name=bugSteps cols=50>%s</textarea></td>", p_rec->Repro_Steps );
   SS_HTML_Write ( "</tr>" );

   if ( recnum != 0xFFFFFFFF )
   {
      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "  <th align=right>Comments</th>" );
      SS_HTML_Write ( "  <td><textarea rows=7 name=bugComments cols=50>%s</textarea></td>", p_rec->Comments );
      SS_HTML_Write ( "</tr>" );

      SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "  <th align=right>Resolution</th>" );
      SS_HTML_Write ( "  <td><textarea rows=7 name=bugResolution cols=50>%s</textarea></td>", p_rec->Resolution );
      SS_HTML_Write ( "</tr>" );
   }

   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "  <td>&nbsp;</td>" );
   SS_HTML_Write ( "  <td>" );
                        SS_HTML_Submit( "Submit" );
   SS_HTML_Write ( "</td>" );
   SS_HTML_Write ( "</tr>" );

   SS_HTML_End_Table_Tag ( );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ O S _ S e l e c t
// =---------------------------------------------------------------------------
void Write_OS_Select ( uint_08 OS_Default )
{
   int i;
   SS_HTML_Write ( "<select size=1 name=bugOS>" );

   for ( i = BUGOS_START; i <= BUGOS_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)OS_Default) ? "selected" : "", Get_OS_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ B u g T y p e _ S e l e c t
// =---------------------------------------------------------------------------
void Write_BugType_Select ( uint_08 BugType_Default )
{
   int i;

   SS_HTML_Write ( "<select name=bugBugType size=1>" );

   for ( i = BUGTYPE_START; i <= BUGTYPE_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)BugType_Default) ? "Selected" : "", Get_BugType_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ C a t e g o r y _ S e l e c t
// =---------------------------------------------------------------------------
void Write_Category_Select ( uint_08 Category_Default )
{
   int i;

   SS_HTML_Write ( "<select name=bugCategory size=1>" );

   for ( i = BUGCAT_START; i <= BUGCAT_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)Category_Default) ? "selected" : "", Get_Category_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ P r i o r i t y _ S e l e c t
// =---------------------------------------------------------------------------
void Write_Priority_Select ( uint_08 Priority_Default )
{
   int i;

   SS_HTML_Write ( "<select name=bugPriority size=1>" );

   for ( i = BUGPRIORITY_START; i <= BUGPRIORITY_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)Priority_Default) ? "selected" : "", Get_Priority_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ S t a t u s _ S e l e c t
// =---------------------------------------------------------------------------
void Write_Status_Select ( uint_08 Status_Default )
{
   int i;

   SS_HTML_Write ( "<select name=bugStatus size=1>" );

   for ( i = BUGSTATUS_START; i <= BUGSTATUS_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)Status_Default) ? "selected" : "", Get_Status_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// G e t _ O S _ S t r i n g
// =---------------------------------------------------------------------------
const char* Get_OS_String ( uint_08 os )
{
   switch ( os )
   {
      case BUGOS_WINDOWS95     : return "Windows 95";
      case BUGOS_WINDOWS95OSR2 : return "Windows 95 OSR2";
      case BUGOS_WINDOWS98     : return "Windows 98";
      case BUGOS_WINDOWS98SE   : return "Windows 98 Second Edition";
      case BUGOS_WINDOWSNT4SP3 : return "Windows NT 4.0 Service Pack 3";
      case BUGOS_WINDOWSNT4SP4 : return "Windows NT 4.0 Service Pack 4";
      case BUGOS_WINDOWSNT4SP5 : return "Windows NT 4.0 Service Pack 5";
      case BUGOS_WINDOWSNT4SP6 : return "Windows NT 4.0 Service Pack 6";
      case BUGOS_WINDOWSNT2000 : return "Windows 2000 (Release)";
      default:
         sprintf ( Unknown_String, "(Unknown OS %d)", (int)os );
         return Unknown_String;
   }
}

// =---------------------------------------------------------------------------
// G e t _ S t a t u s _ S t r i n g
// =---------------------------------------------------------------------------
const char* Get_Status_String ( uint_08 status )
{
   switch ( status )
   {
      case BUGSTATUS_OPEN           : return "Open";
      case BUGSTATUS_FIXED          : return "Fixed";
      case BUGSTATUS_NOTABUG        : return "Not a bug";
      case BUGSTATUS_FUTURERELEASE  : return "Future Release";
      default: 
         sprintf ( Unknown_String, "(Unknown Status %d)", (int)status );
         return Unknown_String;
   }
}

// =---------------------------------------------------------------------------
// G e t _ B u g T y p e _ S t r i n g 
// =---------------------------------------------------------------------------
const char* Get_BugType_String ( uint_08 type )
{
   switch ( type )
   {
      case BUGTYPE_BUG        : return "Bug/Problem";
      case BUGTYPE_SUGGESTION : return "Suggestion";
      default: 
         sprintf ( Unknown_String, "(Unknown Bug Type %d)", (int)type );
         return Unknown_String;
   }
}

// =---------------------------------------------------------------------------
// G e t _ P r i o r i t y _ S t r i n g 
// =---------------------------------------------------------------------------
const char* Get_Priority_String ( uint_08 type )
{
   switch ( type )
   {
      case BUGPRIORITY_URGENT   : return "Urgent";
      case BUGPRIORITY_HIGH     : return "High";
      case BUGPRIORITY_MEDIUM   : return "Medium";
      case BUGPRIORITY_LOW      : return "Low";
      default: 
         sprintf ( Unknown_String, "(Unknown Priority %d)", (int)type );
         return Unknown_String;
   }
}

// =---------------------------------------------------------------------------
// G e t _ C a t e g o r y _S t r i n g
// =---------------------------------------------------------------------------
const char* Get_Category_String ( uint_08 type )
{
   switch ( type )
   {
      case BUGCAT_NETWORKING    : return "Networking";
      case BUGCAT_AI            : return "AI/Bots";
      case BUGCAT_GRAPHICS      : return "Graphics";
      case BUGCAT_SOUND         : return "Sound";
      case BUGCAT_GAMEPLAY      : return "Gameplay";
      case BUGCAT_INSTALL       : return "Install/Update";
      default: 
         sprintf ( Unknown_String, "(Unknown Category %d)", (int)type );
         return Unknown_String;
   }
}

// =---------------------------------------------------------------------------
// G e t _ B u g _ I c o n
// =---------------------------------------------------------------------------
const char* Get_Bug_Icon ( )
{
   return Bug_Icon_Link;
}

// =---------------------------------------------------------------------------
// A c t i o n _ S e n d _ N e w _ B u g _ M a i l 
// =---------------------------------------------------------------------------
int Action_Send_New_Bug_Mail ( bug_form* p_form, bug_record* p_rec, uint_32 recnum )
{
   int  ret = 0;
   char Short_Summary [ SUMMARY_BYTES + 20 ];
   char Message       [ sizeof(bug_form) ];
   char From          [ 64 ];
   char To            [ 64 ];
   char Reply_To      [ 64 ];
   char Bug_Link      [ LINK_BYTES ];
   char SMTP_Server   [SYSSET_VALUE_BYTES];

   SysSet_Get_Value ( "SMTP_Server", SMTP_Server );

   strcpy ( Short_Summary, "Bug Repository : " );
   SS_Port_Strcat ( Short_Summary, p_form->Summary );

   // From:
   //
   strcpy ( From, "ID Bug Repository <rail-tester@edenstudios.net>" );
   //strcpy ( From, "nic@primordia.com" );

   // To:
   strcpy ( To, "Rail-Tester Listserv <rail-tester@edenstudios.net>" );
   //strcpy ( From, "nic@primordia.com" );

   // Reply-To:
   strcpy ( Reply_To, "Rail-Tester Listserv <rail-tester@edenstudios.net>" );
   //strcpy ( From, "nic@primordia.com" );

   // Format the Message Proper
   strcpy ( Message, "Reply-To: " );

   SS_Port_Strcat ( Message, "%s\r\n", Reply_To );

   SS_Port_Strcat ( Message, "To: %s\r\n", To );

   SS_Port_Strcat ( Message, "A new bug has been entered into the Iron Dragon Bug Repository. This is an auto-notification.\r\n" );

   SS_Port_Strcat ( Message, "\r\nBUG ID: %04d\r\n", (int)recnum );

   SS_Port_Strcat ( Message, "\r\nREPORTER: %s\r\n", p_rec->Username );

   SS_Port_Strcat ( Message, "\r\nBUG DESCRIPTION:\r\n\r\n%s\r\n", p_form->Description );

   SS_Port_Strcat ( Message, "\r\nBUG REPRO STEPS:\r\n\r\n%s\r\n", p_form->Repro_Steps );

   SS_Port_Strcat ( Message, "\r\nVISIT THE REPOSITORY:\r\n\r\n" );

   sprintf ( Bug_Link, "%s?Page=Find_Bug&Bugid=%d&Username=railtester&Password=password", CGI_ID_SCRIPT_FULLPATH, (int)recnum );
   SS_Port_Strcat ( Message, "\r\nYou can see more details about this bug by visiting the bug repository and visiting this link:\r\n\r\n%s\r\n", Bug_Link, (int)recnum );

   SS_Port_Strcat ( Message, "\r\nHave a nice day, from the Bug Repository!" );

   ret = SS_Net_Send_Mail ( SMTP_Server, From, To, Short_Summary, Message );

   return ret;
}