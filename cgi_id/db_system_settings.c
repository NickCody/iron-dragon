// =---------------------------------------------------------------------------
// s y s t e m _ s e t t i n g s . c
// 
//   (C) 2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//    For all functions, 0 means success, non-zero means failure
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
//   00 Apr 22   nic   Created.
//

#include <stdio.h>
#include <memory.h>
#include "cgi_id_subsystems.h"
#include "databases.h"
#include "pages.h"
#include "cgic.h"
#include <stdlib.h>

// =---------------------------------------------------------------------------
// S y s S e  t_  G e t _ V a l u e
//
// Returns:
// 0  : Success
// -1 : setting not found
// -2 : database error
//
// =---------------------------------------------------------------------------
int SysSet_Get_Value ( const char* Name, char* Value )
{
   db_metastructure ms;
   db_sysset        Record;
   uint_32          i;
   int              ret = -1; // -1 means not found

   if ( !Name || !Value ) return -1;

   memset ( Value, 0, SYSSET_VALUE_BYTES );

   SS_DB_Initialize_MetaStructure ( &ms, sizeof(db_sysset), 0, 0 );

   if ( -1 == SS_DB_Create_Open ( &ms, "", DB_Name_System_Settings ) )
      return -2;

   for ( i = 0; i < SS_DB_Get_Record_Count(&ms); i++ )
   {
      SS_DB_Get_Record ( &ms, i, &Record );

      if ( Record.State == SYSSET_STATE_DELETED )
         continue;

      if ( strcmp ( Record.Name, Name ) == 0 )
      {
         strcpy ( Value, Record.Value );
         ret = 0;
         break;
      }
   }

   SS_DB_Close ( &ms );

   return ret;
}

// =---------------------------------------------------------------------------
// S y s S e t _ S e t _ V a l u e
//
// Returns:
// 0  : Success
// -1 : setting not found
// -2 : database error
//
// =---------------------------------------------------------------------------
int SysSet_Set_Value ( const char* Name, char* Value, int Add_If_Not_Found )
{
   db_metastructure ms;
   db_sysset        Record;
   uint_32          i;
   int              ret = -1; // -1 means not found

   SS_DB_Initialize_MetaStructure ( &ms, sizeof(db_sysset), 0, 0 );

   if ( -1 == SS_DB_Create_Open ( &ms, "", DB_Name_System_Settings ) )
      return -2;

   for ( i = 0; i < SS_DB_Get_Record_Count(&ms); i++ )
   {
      // we change deleted records, too...

      SS_DB_Get_Record ( &ms, i, &Record );

      if ( strcmp ( Record.Name, Name ) == 0 )
      {
         if ( strlen(Value) == 0 )
            Record.State = SYSSET_STATE_INACTIVE;
         else
         {
            if ( strcmp(Record.Value, "(delete)") == 0 || strcmp(Record.Value, "(deleted)") == 0 )
               Record.State =  SYSSET_STATE_DELETED;
            else
               Record.State = SYSSET_STATE_NORMAL;
         }

         strcpy ( Record.Value, Value );
         SS_DB_Set_Record ( &ms, i, &Record );

         ret = 0;

         break;
      }
   }

   // If we didn't find the ame and we specified to add new, add new
   //
   if ( Add_If_Not_Found && ret == -1 /*not found*/ )
   {
      strcpy ( Record.Name,  Name );
      strcpy ( Record.Value, Value );
      Record.Class = SYSSET_CLASS_NONE;
      Record.State = SYSSET_STATE_NORMAL;
      if ( 0 != SS_DB_Add_Record ( &ms, &Record, &i ) )
         ret = -2;

      ret = 0;
   }  

   SS_DB_Close ( &ms );

   return ret;
}

// =---------------------------------------------------------------------------
// S y s S e t _ G e t _ I n t e g e r _ V a l u e
// =---------------------------------------------------------------------------
int SysSet_Get_Integer_Value ( const char* Name )
{
   char Value [SYSSET_VALUE_BYTES];

   if ( SysSet_Get_Value ( Name, Value ) == 0 )
   {
#if WIN32
      // TODO: do this for unix...
      strupr ( Value );
#endif

      // Do some error handling, mapping common poops to 0/1
      //

      if ( strcmp ( Value, "TRUE" )==0 || strcmp ( Value, "TRUE" )==0 )
         return 1;
      else if ( strcmp ( Value, "FALSE" )==0 || strcmp ( Value, "NO" )==0 )
         return 0;
      else
         return atoi(Value);
   }
   else
      return 0;
}

// =---------------------------------------------------------------------------
// S y s S e t _ S e t _ I n t e g e r _ V a l u e
// =---------------------------------------------------------------------------
void SysSet_Set_Integer_Value ( const char* Name, int Value, int Add_If_Not_Found )
{
   char Char_Value [SYSSET_VALUE_BYTES];

   sprintf ( Char_Value, "%d", Value );

   SysSet_Set_Value ( Name, Char_Value, Add_If_Not_Found );
}

// =---------------------------------------------------------------------------
// W r i t e _ S y s S e t _ T a b l e
// 0  : Success
// -2 : database error
// =---------------------------------------------------------------------------

int Write_SysSet_Table ( )
{
   db_metastructure ms;
   db_sysset        Record;
   uint_32          i;
   int              ret = 0;
   int              Num_Written = 0;

   SS_DB_Initialize_MetaStructure ( &ms, sizeof(db_sysset), 0, 0 );

   if ( -1 == SS_DB_Create_Open ( &ms, "", DB_Name_System_Settings ) )
      return -2;

   SS_HTML_WriteP ( "Here are the currently defined system settings:" );

   SS_HTML_Table_Tag_Ex ( "width=\"500\"" );

   SS_HTML_Write ( "<TR>" );
   SS_HTML_Write ( "<TH>Setting Name</TH>" );
   SS_HTML_Write ( "<TH>Value (%d chars max)</TD>", (int)SYSSET_VALUE_BYTES );
   SS_HTML_Write ( "</TR>" );

   for ( i = 0; i < SS_DB_Get_Record_Count(&ms); i++ )
   {
      SS_DB_Get_Record ( &ms, i, &Record );

      if ( Record.State == SYSSET_STATE_DELETED )
         continue;

      SS_HTML_Encode_Symbols ( Record.Name, SYSSET_NAME_BYTES, 1 );
      SS_HTML_Encode_Symbols ( Record.Value, SYSSET_VALUE_BYTES, 1 );

      Num_Written++;

      SS_HTML_Write ( "<TR>" );
      SS_HTML_Write ( "<TD>%s</TD>", Record.Name );

      if ( Record.State == SYSSET_STATE_INACTIVE )
         SS_HTML_Write ( "<TD><i>(inactive)</i></TD>" );
      else
         SS_HTML_Write ( "<TD>%s</TD>", Record.Value );

      SS_HTML_Write ( "</TR>" );
   }

   // Write out a row
   if ( !Num_Written )
      SS_HTML_Write ( "<TR><TD colspan=\"2\">There are no system settings defined.</TD></TR>" );
   else
      SS_HTML_Write ( "<TR><TD colspan=\"2\">%d System Settings found.</TD></TR>", Num_Written );

   SS_HTML_End_Table_Tag ( );

   // Clean up
   //
   SS_DB_Close ( &ms );

   return ret;
}
