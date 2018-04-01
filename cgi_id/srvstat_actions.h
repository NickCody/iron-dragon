#ifndef __ID_ACTION_H__
#define __ID_ACTION_H__

#include "net_messages_admin.h"

typedef struct user_form
{
   char    recnum             [VARIABLE_BYTES];
   char    username           [VARIABLE_BYTES];
   char    password1          [VARIABLE_BYTES];
   char    password2          [VARIABLE_BYTES];
   char    fname              [VARIABLE_BYTES];
   char    lname              [VARIABLE_BYTES];
   char    email              [VARIABLE_BYTES];
   char    connects           [VARIABLE_BYTES];
   char    lastgroup_uid      [VARIABLE_BYTES];
   char    status             [VARIABLE_BYTES];
   char    registration       [VARIABLE_BYTES];
   char    address1           [VARIABLE_BYTES];
   char    address2           [VARIABLE_BYTES];
   char    city               [VARIABLE_BYTES];
   char    state_province     [VARIABLE_BYTES];
   char    zip_code           [VARIABLE_BYTES];
   char    country_code       [VARIABLE_BYTES];
   char    rating             [VARIABLE_BYTES];
   char    wins               [VARIABLE_BYTES];
   char    losses             [VARIABLE_BYTES];
} user_form;

int  Action_Get_Stats            ( nm_cza_info*   p_info );
int  Action_Get_Log_Filename     ( nm_cza_getlog* p_getlog );
int  Action_Enumerate_Users      ( int page );
int  Action_Enumerate_Users_Callback ( int (*Callback)(nm_user_record* p_ur), int Write_Back );

void Write_User_List_Navigation  (int page, int count);

// Mass-user callbacks
//
int  MU_Reset_Rating_Callback    ( nm_user_record* p_ur );

int  Action_Add_User             ( user_form* p_uf, nm_user_record* p_usr, uint_08* p_Success_Code );   // assumes fields for user are provided by http variables
int  Action_Modify_User_By_Recnum( nm_user_record* p_usr, uint_32 Recnum );            // assumes fields for user are provided by http variables
int  Action_Lookup_User          ( nm_user_record* p_rec, const char* username );
int  Action_Lookup_User_By_Recnum( nm_user_record* p_rec, uint_32 Recnum );
int  Action_Delete_User_By_Recnum( uint_32 Recnum );

int  Action_Verify_Password      ( const char* Password1, const char* Password2, int Allow_Blank, int Verbose );

void Write_User                  ( nm_user_record* p_ur );
int  Write_User_Inputs           ( nm_user_record* p_ur, uint_32 Recnum );
int  Write_User_Commands         ( );
int  Action_Gather_User_Fields   ( user_form* p_uf, int Enforce_Required );
void Write_User_Download_Form_Fields ( );
int  User_Form_To_Record         ( user_form* p_form, nm_user_record* p_rec, uint_08 Merge );
void Write_User_Status_Select    ( uint_08 Default_Status );
void Write_User_Registration_Select ( int Default_Registration );

const char* Get_User_Status_String ( uint_08 Status );
const char* Get_User_Registration_String ( uint_08 Registration );

void Dump_User_Form ( user_form* p_uf );

#endif // __ID_ACTION_H__

