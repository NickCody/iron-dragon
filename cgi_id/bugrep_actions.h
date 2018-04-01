#ifndef __BUGREP_ACTIONS_H__
#define __BUGREP_ACTIONS_H__

#include "db_bugbase.h"

typedef struct bug_form
{
   // New bug Fields, used when editing and for new bug reports
   //
   char     BugUsername  [VARIABLE_BYTES];
   //char     Password     [VARIABLE_BYTES];
   char     Email        [VARIABLE_BYTES];
   char     Version      [VARIABLE_BYTES];
   char     Date_Reported[VARIABLE_BYTES];
   char     OS           [VARIABLE_BYTES];
   char     Bug_Type     [VARIABLE_BYTES];
   char     Priority     [VARIABLE_BYTES];
   char     Category     [VARIABLE_BYTES];
   char     Summary      [SUMMARY_BYTES];
   char     Description  [DESCRIPTION_BYTES];
   char     Repro_Steps  [REPRO_STEPS_BYTES];

   // Existing Bug Fields, used only when editing an existing bug
   //
   char     Bugid        [VARIABLE_BYTES];
   char     Status       [VARIABLE_BYTES];
   char     Deleted      [VARIABLE_BYTES];
   char     Comments     [COMMENTS_BYTES];
   char     Resolution   [RESOLUTION_BYTES];

} bug_form;

int Action_Gather_Bug_Fields     ( bug_form* p_form );
int Action_Add_Bug               ( bug_form* p_form, bug_record* p_rec, uint_32* p_bugid );
int Action_Lookup_Bug            ( bug_record* p_rec, uint_32 bugnum );
int Action_Update_Bug            ( bug_record* p_rec, uint_32 bugnum );
int Action_Bug_Summary           ( uint_08 Status );

int Write_Bug_Confirmation_Table ( bug_form* p_form  );
int Write_Bug_Form_Variables     ( bug_form* p_form  );
int Action_Send_New_Bug_Mail     ( bug_form* p_form, bug_record* p_rec, uint_32 recnum );
int Write_Bug_Detail_Table       ( bug_record* p_rec, uint_32 recnum );
int Write_Bug_Summary_Row        ( bug_record* p_rec, uint_32 recnum );
int Write_Bug_Inputs             ( bug_record* p_rec, uint_32 recnum );

void Write_OS_Select             ( uint_08 OS_Default       );
void Write_BugType_Select        ( uint_08 BugType_Default  );
void Write_Category_Select       ( uint_08 Category_Default );
void Write_Priority_Select       ( uint_08 Priority_Default );
void Write_Status_Select         ( uint_08 Status_Default   );

int         Bug_Form_To_Record   ( bug_form* p_form, bug_record* p_rec, uint_08 Merge );
const char* Get_OS_String        ( uint_08 os      );
const char* Get_Status_String    ( uint_08 status  );
const char* Get_BugType_String   ( uint_08 type    );
const char* Get_Priority_String  ( uint_08 type    );
const char* Get_Category_String  ( uint_08 type    );

// Miscellany
//
const char* Get_Bug_Icon         ( );

#endif // __BUGREP_ACTIONS_H__