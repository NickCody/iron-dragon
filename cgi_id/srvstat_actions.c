
#include "cgi_id_subsystems.h"
#include "net_messages.h"
#include "net_messages_admin.h"
#include "srvstat_actions.h"
#include "connection_zone.h"
#include "users.h"
#include "pages.h"

#include "cgic.h"

#include <stdlib.h>                 // for atoi

char Unknown_User_String [ 80 ];

// =---------------------------------------------------------------------------
// A c t i o n _ G e t _ S t a t s
//
// Returns 0 on success, -1 on error
//
// =---------------------------------------------------------------------------
int Action_Get_Stats ( nm_cza_info* p_info )
{
   int               ret;
   nm_acz_info       query_info;
   nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;

   if ( Page_Info.Server_Sock == -1 ) return -1;

   SS_HTML_WriteP ( "[Server Welcome Message]<BR><I>\"%s\"</I>", p_auth_reply->CZ_Message );

   // Query Info Admin Message
   //
   query_info.Message_Type    = 0xFF;
   query_info.Message_Type_Ex = ACZ_INFO;

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&query_info, sizeof(query_info) );
   if ( ret != 0 )
   {
      SS_HTML_Write ( "(failed to get server stats)<BR>" );
      return -1;
   }

   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)p_info, sizeof(nm_cza_info) );
   if ( ret != 0 )
   {
      SS_HTML_Write ( "(failed to get server stats)<BR>" );
      return -1;
   }

   return 0;
}

// =---------------------------------------------------------------------------
// A c t i o n _ G e t _ L o g _ F  i l e n a m e
//
//
// =---------------------------------------------------------------------------
int Action_Get_Log_Filename ( nm_cza_getlog* p_getlog )
{
   int               ret;
   nm_acz_getlog     query_info;

   if ( Page_Info.Server_Sock == -1 ) return -1;

   // Query Info Admin Message
   //
   query_info.Message_Type    = 0xFF;
   query_info.Message_Type_Ex = ACZ_GETLOG;

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&query_info, sizeof(query_info) );
   if ( ret != 0 )
      return ret;

   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)p_getlog, sizeof(nm_cza_getlog) );
   if ( ret != 0 )
      return ret;

   return 0;
}

// =---------------------------------------------------------------------------
// A c t i o n _ E n u m e r a t e _ U s e r s
//
// Port is in network byte order
//
// =---------------------------------------------------------------------------
int Action_Enumerate_Users (int page)
{
   int               ret;
   uint_32           start;
   uint_32           end;
   uint_32           count;
   uint_32           i;
   nm_bidir_numusers numusr;
   nm_bidir_getusers getusr;
   nm_user_record    user_rec;
   char              Link[LINK_BYTES];

   //nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;

   // Ask server how many users there are
   //
   numusr.Message_Type    = 0xFF;
   numusr.Message_Type_Ex = BIDIR_NUMUSERS;
   memset ( numusr.Num_Users, 0, 4 );

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&numusr, sizeof(numusr) );
   if ( ret != 0 )
   {
      SS_HTML_Write ( "(failed to ask for user count)<BR>" );
      return -1;
   }

   // Get the servers response : There are X users
   //
   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&numusr, sizeof(numusr) );
   if ( ret != 0 )
   {
      SS_HTML_Write ( "(failed to get user count)<BR>" );
      return -1;
   }

   count = ntohl( *(uint_32*)numusr.Num_Users );

   if (page != -1) {
       start = page*USER_LIST_PAGE_SIZE;
       end   = (page+1)*USER_LIST_PAGE_SIZE-1;
       end   = (end <= (count-1)) ? end : (count-1); // min(end, count-1); //
   }
   else {
       start = 0;
       end = count-1;
   }

   // Ask server for specified range of users
   //
   getusr.Message_Type           = 0xFF;
   getusr.Message_Type_Ex        = BIDIR_GETUSERS;              
   *(uint_32*)getusr.Start_User  = htonl(start);
   *(uint_32*)getusr.End_User    = htonl(end);
   getusr.Rank_Sorted            = 0;

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&getusr, sizeof(getusr) );
   if ( ret != 0 )
   {
      SS_HTML_Write ( "(failed to ask for user range)<BR>" );
      return -1;
   }

   // Now, get the server's user list header
   //
   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&getusr, sizeof(getusr) );
   if ( ret != 0 )
   {
      SS_HTML_Write ( "(failed to get user list header)<BR>" );
      return -1;
   }

   if (page != -1) {
       SS_HTML_Write ( "<p>There are %d users in the database.<br>", (int)count );
       SS_HTML_Write ("Page %d of %d</p>", page+1, count/USER_LIST_PAGE_SIZE+1);
   }
   else {
       SS_HTML_WriteP("This is a complete list of all %d users.", (int)count);
   }

   Write_User_List_Navigation(page, count);

   // Write out a table with a row for each user
   //
   SS_HTML_Table_Tag ( );

   // Write out the table's header row
   //
   SS_HTML_Write ( "<tr>" );
   SS_HTML_Write ( "<TH>Index</TH>" );
   SS_HTML_Write ( "<TH>Last Name</TH>" );
   SS_HTML_Write ( "<TH>Name</TH>" );
   SS_HTML_Write ( "<TH>Username</TH>" );
   SS_HTML_Write ( "<TH>E-Mail</TH>" );
   SS_HTML_Write ( "<TH>Rank</TH>" );

   // These two fields are only for administrators
   //
   if ( User_Is_Administrator ( Page_Info.Username ) )
   {
      SS_HTML_Write ( "<TH align=right>&nbsp</TH>" );
      SS_HTML_Write ( "<TH align=right>&nbsp</TH>" );
   }

   SS_HTML_Write ( "</tr>" );
   
   // For each user, write a table row
   //
   for ( i=start; i <= end; i++ )
   {
      ret = SS_Net_Receive_Buffer_Ex ( Page_Info.Server_Sock, &user_rec, sizeof(nm_user_record), 0 );
      
      if ( ret != sizeof(nm_user_record) )
      {
         SS_HTML_Write ( "(whoa! reading user record of erroneous %d size)<BR>", ret );
         continue;
      }

      SS_HTML_Write ( "<tr>" );
      
      SS_HTML_Write ( "<TD align=\"center\">%d</td>", ntohl(*(uint_32*)user_rec.index) );
      SS_HTML_Write ( "<td>%s, %s</td>", user_rec.record.lname, user_rec.record.fname );
      SS_HTML_Write ( "<td>%s</td>", user_rec.record.fname      );
      SS_HTML_Write ( "<td>%s</td>", user_rec.record.username   );

      SS_HTML_Write ( "<td><A href=\"mailto:%s\">%s</A></td>", user_rec.record.email, user_rec.record.email );
      SS_HTML_Write ( "<td align=\"right\">%d</td>", (int)ntohs(user_rec.record.rating) );

      // These two fields are only for administrators
      //
      if ( User_Is_Administrator ( Page_Info.Username ) )
      {
         Format_Script_Link ( Link, "Edit_User" );
         SS_HTML_Write ( "<td><A href=\"%s;EditUserRecnum=%d\">Edit</A></td>", Link, i );

         Format_Script_Link ( Link, "Delete_User" );
         SS_HTML_Write ( "<td><A href=\"%s;DelUserRecnum=%d\">Delete</A></td>", Link, i );
      }

      SS_HTML_Write ( "</tr>" );
   }

   // Finish off the table
   SS_HTML_End_Table_Tag ();

   Write_User_List_Navigation(page, count);

   return 0;
}

// =---------------------------------------------------------------------------
void Write_User_List_Navigation(int page, int count)
{
    int firstpage = 0;
    int lastpage = count/USER_LIST_PAGE_SIZE;
    char all[120];
    char first[120];
    char prev[120];
    char next[120];
    char last[120];
    char temp[120];
    char templink[120];

    Format_Script_Link ( templink, "User_Listing" );
    sprintf(temp, ";pagenum=-1");
    strcat(templink, temp);
    sprintf(all, "<a href=\"%s\">Complete List</a>", templink);
    
    Format_Script_Link ( templink, "User_Listing" );
    sprintf(temp, ";pagenum=%d", firstpage);
    strcat(templink, temp);
    sprintf(first, "<a href=\"%s\">first</a>", templink);

    Format_Script_Link ( templink, "User_Listing" );
    sprintf(temp, ";pagenum=%d", page-1);
    strcat(templink, temp);
    sprintf(prev, "<a href=\"%s\">prev</a>", templink);

    Format_Script_Link ( templink, "User_Listing" );
    sprintf(temp, ";pagenum=%d", page+1);
    strcat(templink, temp);
    sprintf(next, "<a href=\"%s\">next</a>", templink);

    Format_Script_Link ( templink, "User_Listing" );
    sprintf(temp, ";pagenum=%d", lastpage);
    strcat(templink, temp);
    sprintf(last, "<a href=\"%s\">last</a>", templink);

    if (firstpage == page) {
        strcpy(first, "first");
        strcpy(prev, "prev");
    }

    if (lastpage == page) {
        strcpy(next, "next");
        strcpy(last, "last");
    }

    SS_HTML_Table_Tag ( );
    SS_HTML_Write ( "<tr>"  );
    SS_HTML_Write ( "<td>"  );
    SS_HTML_Write (  all  );
    SS_HTML_Write ( "</td>" );
    SS_HTML_Write ( "<td>"  );
    SS_HTML_Write (  first  );
    SS_HTML_Write ( "</td>" );
    SS_HTML_Write ( "<td>"  );
    SS_HTML_Write (  prev   );
    SS_HTML_Write ( "</td>" );
    SS_HTML_Write ( "<td>"  );
    SS_HTML_Write (  next   );
    SS_HTML_Write ( "</td>" );
    SS_HTML_Write ( "<td>"  );
    SS_HTML_Write (  last   );
    SS_HTML_Write ( "</td>" );
    SS_HTML_Write ( "</tr>" );
    SS_HTML_End_Table_Tag ();
}

// =---------------------------------------------------------------------------
// A c t i o n _ A d d _ U s e r 
//
// assumes fields for user are provided by http variables
//
// Returns the server code part of nm_bidir_user
//
// =---------------------------------------------------------------------------
int Action_Add_User ( user_form* p_uf, nm_user_record* p_usr, uint_08* p_Success_Code )
{
   nm_user_record*   p_user_rec;
   nm_bidir_user*    p_user_msg;
   uint_08*          p_total_message;
   int               ret;
   uint_16           msg_len;

   // Check that passwords are valid
   //
   if ( 0 != Action_Verify_Password ( p_uf->password1, p_uf->password2, 1 /*allow blank*/, 1 /*verbose*/ ) )
      return -1;
   else
      strcpy ( p_usr->record.password, p_uf->password1 );

   if ( User_Form_To_Record ( p_uf, p_usr, 0 /*not used yet*/ ) == -1 )
   {
      SS_HTML_WriteP ( "Not all required fields were filled out. Please try again." );
      return -1;
   }

   *(uint_32*)p_usr->index = 0;

   // Construct the combined message
   //
   msg_len = sizeof(nm_user_record) + sizeof(nm_bidir_user);
   
   p_total_message = (uint_08*)SS_Port_AllocMem ( msg_len );

   if ( !p_total_message )
   {
      SS_HTML_Write ( "(local failure)" );
      return -1;
   }

   p_user_msg = (nm_bidir_user*) p_total_message;
   p_user_rec = (nm_user_record*) ( p_total_message + sizeof(nm_bidir_user) );

   // Copy from passed in mamory to our local memory
   //
   memcpy ( p_user_rec, p_usr, sizeof(nm_user_record) );

   // Initialize the bidir user struct
   //
   p_user_msg->Message_Type      = 0xFF;
   p_user_msg->Message_Type_Ex   = BIDIR_ADDUSER;

   // Lets be pessimistic
   //
   ret             = -1; // not a bool, 0 means success, -1 failure
   *p_Success_Code = 0xFF;

   // Add the user, see if successful
   //
   ret = CZ_Send_Message ( Page_Info.Server_Sock, p_total_message, msg_len );

   if ( ret == 0 )
   {
      ret = CZ_Receive_Message ( Page_Info.Server_Sock, p_total_message, msg_len );
      
      if ( ret == 0 )
      {
         *p_Success_Code = p_user_msg->Success_Code;

         // Copy the server's version of the user back to the passed param
         memcpy ( p_usr, p_user_rec, sizeof(nm_user_record) );
         ret = 0;
      }
   }

   SS_Port_FreeMem ( p_total_message );

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ M o d i f y _ U s e r _ B y _ R e c n u m
//
// Assumes fields for user are provided by http variables
//
// =---------------------------------------------------------------------------
int Action_Modify_User_By_Recnum ( nm_user_record* p_usr, uint_32 Recnum )
{
   int               ret;
   nm_bidir_user*    p_msg_send;
   nm_user_record*   p_usr_send;
   uint_08*          p_send;
   
   p_send = (uint_08*)SS_Port_AllocMem ( sizeof(nm_bidir_user) + sizeof(nm_user_record) );
   if ( !p_send ) return -1;

   p_msg_send                    = (nm_bidir_user*)p_send;
   p_msg_send->Message_Type      = 0xFF;
   p_msg_send->Message_Type_Ex   = BIDIR_SETUSER;
   p_msg_send->Success_Code      = 0;
   *(uint_32*)p_msg_send->Recnum = htonl(Recnum);

   p_usr_send = (nm_user_record*) ( p_send + sizeof(nm_bidir_user) );
   memcpy ( p_usr_send, p_usr, sizeof(nm_user_record) );

   ret = CZ_Send_Message ( Page_Info.Server_Sock, p_send, sizeof(nm_bidir_user) + sizeof(nm_user_record) );

   if ( ret == 0 )
      ret = CZ_Receive_Message ( Page_Info.Server_Sock, p_send, sizeof(nm_bidir_user) + sizeof(nm_user_record) );

   SS_Port_FreeMem ( p_send );
   p_send = 0;

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ L o o k u p _ U s e r
//
// =---------------------------------------------------------------------------
int Action_Lookup_User ( nm_user_record* p_rec, const char* username )
{
   int                  ret;
   nm_bidir_lookupuser  lu;
   uint_08*             p_result;

   lu.Message_Type    = 0xFF;
   lu.Message_Type_Ex = BIDIR_LOOKUPUSER;
   strcpy ( lu.Username, username );

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&lu, sizeof(nm_bidir_lookupuser) );
   
   if ( ret == 0 )
   {
      p_result = (uint_08*)SS_Port_AllocMem ( sizeof(nm_bidir_lookupuser)+sizeof(nm_user_record) );
      if ( !p_result ) return -1;

      ret = CZ_Receive_Message ( Page_Info.Server_Sock, p_result, sizeof(nm_bidir_lookupuser)+sizeof(nm_user_record) );
      if ( ret == 0 )
         memcpy ( p_rec, p_result + sizeof(nm_bidir_lookupuser), sizeof(nm_user_record) );

      SS_Port_FreeMem ( p_result );

      p_result = 0;
   }

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ L o o k u p _ U s e r_ B y _ R e c n u m
//
// =---------------------------------------------------------------------------
int Action_Lookup_User_By_Recnum ( nm_user_record* p_rec, uint_32 Recnum )
{
   int               ret;
   nm_bidir_getusers gu;

   gu.Message_Type          = 0xFF;
   gu.Message_Type_Ex       = BIDIR_GETUSERS;
   *(uint_32*)gu.Start_User = htonl ( Recnum );
   *(uint_32*)gu.End_User   = htonl ( Recnum );
   gu.Rank_Sorted           = 0;

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&gu, sizeof(nm_bidir_getusers) );
   
   if ( ret == 0 )
   {
      // Now, get the server's user list header
      //
      ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&gu, sizeof(nm_bidir_getusers) );

      if ( ret != 0 )
      {
         SS_HTML_Write ( "(failed to get user list header)<BR>" );
         return -1;
      }
      
      ret = SS_Net_Receive_Buffer_Ex ( Page_Info.Server_Sock, p_rec, sizeof(nm_user_record), 0 );

      if ( ret != sizeof(nm_user_record) )
      {
         SS_HTML_Write ( "(whoa! reading user record of erroneous %d size)<BR>", ret );
         return -1;
      }
   }

   return 0;
}

// =---------------------------------------------------------------------------
// A c t i o n _ D e l e t e _ U s e r _ B y _ R e c n u m
//
// =---------------------------------------------------------------------------
int Action_Delete_User_By_Recnum ( uint_32 Recnum )
{
   SS_HTML_WriteP ( "While trying to delete user [%d], we remembered that we forgot to implement Action_Delete_User_By_Recnum functionality. Coming soon!", Recnum );
   return -1;
}

// =---------------------------------------------------------------------------
// A c t i o n _ V e r i f y _ P a s s w o r d 
//
//  0 : passwords ok
// -1 : passwords do not match
// -2 : passwords too short
// =---------------------------------------------------------------------------
int Action_Verify_Password ( const char* Password1, 
                             const char* Password2, 
                             int         Allow_Blank, 
                             int         Verbose )
{
   // Blank password processing
   //
   if ( Allow_Blank )
   {
      if ( Password1[0] == 0 || Password2[0] == 0 )
         return 0;
   }
   else
   {
      if ( Password1[0] == 0 || Password2[0] == 0 )
      {
         if ( Verbose )
            SS_HTML_WriteP ( "Blank passwords are not allowed." );

         return -1;
      }
   }

   // Compare passwords
   //
   if ( strcmp(Password1, Password2) == 0 )
   {
      if ( strlen(Password1) < MIN_PASSWORD_LENGTH )
      {
         if ( Verbose )
            SS_HTML_WriteP ( "Password was too short, please pick one with at least %d characters.", (int)MIN_PASSWORD_LENGTH );
         return -2;
      }

      return 0;
   }
   else
   {
      if ( Verbose )
         SS_HTML_WriteP ( "Passwords do not match. Please try again." );

      return -1;
   }
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r
//
// =---------------------------------------------------------------------------
void Write_User ( nm_user_record* p_ur )
{
   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<tr><td>Username   </td> <td>%s</td>       </tr>", p_ur->record.username  );
   SS_HTML_Write ( "<tr><td>Password   </td> <td>(hidden)</td> </tr>"                         );
   SS_HTML_Write ( "<tr><td>First Name </td> <td>%s</td>       </tr>", p_ur->record.fname     );
   SS_HTML_Write ( "<tr><td>Last Name  </td> <td>%s</td>       </tr>", p_ur->record.lname     );
   SS_HTML_Write ( "<tr><td>E-mail     </td> <td>%s</td>       </tr>", p_ur->record.email     );
   SS_HTML_Write ( "<tr><td>Index      </td> <td>%d</td>       </tr>", ntohl(*(uint_32*)p_ur->index) );

   SS_HTML_End_Table_Tag ();
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ I n p u t s
//
// p_rec must not be null, though its memory can be nulls
//
// =---------------------------------------------------------------------------
int Write_User_Inputs ( nm_user_record* p_ur, uint_32 Recnum )
{
   if ( !p_ur ) return -1;

   // We assume we're in a form and <p> is active present
   SS_HTML_Write ( "<input type=\"hidden\" name=\"uf_Recnum\" value=\"%d\">", Recnum );

   SS_HTML_Write ( "<table cellspacing=\"2\" cellpadding=\"0\">" );
      SS_HTML_Write ( "<tr>" );

      SS_HTML_Write ( "<td valign=\"top\">" );
         SS_HTML_Write ( "<table cellspacing=\"0\" cellpadding=\"5\">" );
            SS_HTML_Write ( "<tr><TH align=right>First Name       </TH> <td><INPUT type=\"text\"     name=\"uf_FirstName\"        value=\"%s\"> </td></tr>",  p_ur->record.fname    );
            SS_HTML_Write ( "<tr><TH align=right>Last Name        </TH> <td><INPUT type=\"text\"     name=\"uf_LastName\"        value=\"%s\"> </td></tr>",  p_ur->record.lname    );
            SS_HTML_Write ( "<tr><TH align=right>Email            </TH> <td><INPUT type=\"text\"     name=\"uf_Email\"        value=\"%s\"> </td></tr>",  p_ur->record.email    );

            if ( Recnum == 0xFFFFFFFF /*New User*/ )
            {
               SS_HTML_Write ( "<tr><TH align=right>Username      </TH> <td><INPUT type=\"text\"     name=\"uf_Username\"  value=\"\"> </td></tr>" );
            }
            else
            {
               SS_HTML_Write ( "<tr><TH align=right>Current Username</TH><td>%s</td></tr>", p_ur->record.username );
               SS_HTML_Write ( "<tr><TH align=right>New Username    </TH><td><INPUT type=\"text\"    name=\"uf_Username\"  value=\"%s\"> </td></tr>",  p_ur->record.username );
            }

            SS_HTML_Write ( "<tr><TH align=\"right\">Password       </TH> <td><INPUT type=\"password\" name=\"uf_Password1\"   value=\"\">   </td></tr>" );
            SS_HTML_Write ( "<tr><TH align=\"right\">Password       </TH> <td><INPUT type=\"password\" name=\"uf_Password2\"   value=\"\">   </td></tr>" );
            SS_HTML_Write ( "<tr><TH align=\"right\">Rating         </TH> <td><INPUT type=\"text\" name=\"uf_Rating\"  value=\"%d\"> </td></tr>", (int)ntohs(p_ur->record.rating) );
            SS_HTML_Write ( "<tr><TH align=\"right\">Wins           </TH> <td><INPUT type=\"text\" name=\"uf_Wins\"  value=\"%d\"> </td></tr>", (int)ntohs(p_ur->record.wins) );
            SS_HTML_Write ( "<tr><TH align=\"right\">Losses         </TH> <td><INPUT type=\"text\" name=\"uf_Losses\"  value=\"%d\"> </td></tr>", (int)ntohs(p_ur->record.losses) );

            // Status
            SS_HTML_Write ( "<tr>" );
               SS_HTML_Write ( "<th align=\"right\">Status</th>" );
               SS_HTML_Write ( "<td>" );
                  Write_User_Status_Select ( p_ur->record.status );
               SS_HTML_Write ( "</td>" );
            SS_HTML_Write ( "</tr>" );

            // Registration
            SS_HTML_Write ( "<tr>" );
               SS_HTML_Write ( "<th align=\"right\">Registration</th>" );
               SS_HTML_Write ( "<td>" );
                  Write_User_Registration_Select ( p_ur->record.registration );
               SS_HTML_Write ( "</td>" );
            SS_HTML_Write ( "</tr>" );
         SS_HTML_End_Table_Tag ();
      SS_HTML_Write ( "</td>" );

      SS_HTML_Write ( "<td valign=\"top\">" );
         SS_HTML_Write ( "<table cellspacing=\"0\" cellpadding=\"5\">" );
            SS_HTML_Write ( "<tr><TH align=\"right\">Address        </TH> <td><INPUT type=\"text\"     name=\"uf_Address1\"    value=\"%s\"> </td></tr>", p_ur->record.address1 );
            SS_HTML_Write ( "<tr><TH align=\"right\">Address        </TH> <td><INPUT type=\"text\"     name=\"uf_Address2\"    value=\"%s\"> </td></tr>", p_ur->record.address2 );
            SS_HTML_Write ( "<tr><TH align=\"right\">City           </TH> <td><INPUT type=\"text\"     name=\"uf_City\"        value=\"%s\"> </td></tr>", p_ur->record.city );
            SS_HTML_Write ( "<tr><TH align=\"right\">State/Province </TH> <td><INPUT type=\"text\"     name=\"uf_State\"       value=\"%s\"> </td></tr>", p_ur->record.state_province );
            SS_HTML_Write ( "<tr><TH align=\"right\">Zip Code       </TH> <td><INPUT type=\"text\"     name=\"uf_Zip\"         value=\"%s\"> </td></tr>", p_ur->record.zip_code );
            SS_HTML_Write ( "<tr><TH align=\"right\">Country        </TH> <td><INPUT type=\"text\"     name=\"uf_Country\"     value=\"%s\"> </td></tr>", p_ur->record.country_code );
            SS_HTML_Write ( "<tr><TH align=\"right\">Authenticated Connects       </TH> <td><INPUT type=\"text\" name=\"uf_Connects\"      value=\"%d\"> </td></tr>", (int)ntohl(p_ur->record.connects) );
            //SS_HTML_Write ( "<tr><TH align=\"right\">LastGroupUID   </TH> <td><INPUT type=\"text\" name=\"uf_LastGroupUID\"  value=\"%d\"> </td></tr>", (int)ntohs(p_ur->record.lastgroup_uid) );
         SS_HTML_End_Table_Tag ();
      SS_HTML_Write ( "</td>" );
      
      SS_HTML_Write ( "</tr>" );

      SS_HTML_Write ( "<tr><td colspan=\"2\" style=\"{text-align:center;}\">" );
      SS_HTML_Submit( "" );
      SS_HTML_Write ( "</td></tr>" );
   SS_HTML_End_Table_Tag ();

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ C  o m m a n d s
int Write_User_Commands ( )
{
   SS_HTML_Write ( "<p>" );
   SS_HTML_Write ( "  <input type=\"submit\" name=\"send_lic\" value=\"Send License File\"> " );
   //SS_HTML_Write ( "  <input type=\"submit\" name=\"recv_payment\" value=\"Received Payment\"> " );
   //SS_HTML_Write ( "  <input type=\"submit\" name=\"payment_failed\" value=\"Payment Failed\"> " );
   SS_HTML_Write ( "</p>" );

   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ D o w n l o a d _ F o r m _ F i e l d s
// =---------------------------------------------------------------------------
void Write_User_Download_Form_Fields ( )
{
   char FirstName [VARIABLE_BYTES];

   SS_HTML_Table_Tag ( );

   if ( cgiFormNotFound != cgiFormStringNoNewlines ( "uf_FirstName", FirstName, VARIABLE_BYTES) )
      SS_HTML_WriteP ( "Hey, we got a first name of [%s]!", FirstName );
   else
      SS_HTML_WriteP ( "Damn, no first name." );

   // Demo license key will be mailed if they fill this out
   //
   Write_Row_Input ( "First Name",        "uf_FirstName",    1 );
   Write_Row_Input ( "Last Name",         "uf_LastName",     1 );
   Write_Row_Input ( "E-mail",            "uf_Email",        1 );
   Write_Row_Input ( "Username",          "uf_Username",     1 );

   Write_Row_Input ( "Address",           "uf_Address1",     1 );
   Write_Row_Input ( "Address",           "uf_Address2",     0 );
   Write_Row_Input ( "City",              "uf_City",         1 );
   Write_Row_Input ( "State/Province",    "uf_State",        1 );
   Write_Row_Input ( "Zip Code",          "uf_Zip",          1 );
   Write_Row_Input ( "Country",           "uf_Country",      1 );

   // Submit
   //
   SS_HTML_Write ( "<tr>" );
      SS_HTML_Write ( "<td align=\"center\" colspan=\"2\">" );
      SS_HTML_Submit ( "Go!" );
      SS_HTML_Write ( "<td>" );
   SS_HTML_Write ( "</tr>" );
   
   SS_HTML_End_Table_Tag ( );
}


// =---------------------------------------------------------------------------
// A c t i o n _ G a t h e r _ U s e r  _ F i e l d s 
//
// p_rec must not be null, though its memory can be nulls
// TODO: encode data?
// =---------------------------------------------------------------------------
int Action_Gather_User_Fields ( user_form* p_uf, int Enforce_Required )
{
   if ( !p_uf ) return -1;

   memset ( p_uf, 0, sizeof(user_form) );

   // Required Fields
   //
   cgiFormStringNoNewlines ( "uf_Recnum",       (char*)p_uf->recnum,         VARIABLE_BYTES );

   cgiFormStringNoNewlines ( "uf_FirstName",    (char*)p_uf->fname,          VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_LastName",     (char*)p_uf->lname,          VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Username",     (char*)p_uf->username,       VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Email",        (char*)p_uf->email,          VARIABLE_BYTES );
   
   cgiFormStringNoNewlines ( "uf_Password1",    (char*)p_uf->password1,      VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Password2",    (char*)p_uf->password2,      VARIABLE_BYTES );

   cgiFormStringNoNewlines ( "uf_Connects",     (char*)p_uf->connects,       VARIABLE_BYTES );
   //cgiFormStringNoNewlines ( "uf_LastGroupUID", (char*)p_uf->lastgroup_uid,  VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Status",       (char*)p_uf->status,         VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Registration", (char*)p_uf->registration,   VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Rating",       (char*)p_uf->rating,         VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Wins",         (char*)p_uf->wins,           VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Losses",       (char*)p_uf->losses,         VARIABLE_BYTES );
   
   cgiFormStringNoNewlines ( "uf_Address1",     (char*)p_uf->address1,       VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Address2",     (char*)p_uf->address2,       VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_City",         (char*)p_uf->city,           VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_State",        (char*)p_uf->state_province, VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Zip",          (char*)p_uf->zip_code,       VARIABLE_BYTES );
   cgiFormStringNoNewlines ( "uf_Country",      (char*)p_uf->country_code,   VARIABLE_BYTES );

   // Crop to realistic limits
   //
   p_uf->fname[MAX_CLIENT_FNAME-1] = 0;
   p_uf->lname[MAX_CLIENT_LNAME-1] = 0;
   p_uf->username[MAX_CLIENT_USERNAME-1] = 0;
   p_uf->email[MAX_CLIENT_EMAIL-1] = 0;
   p_uf->address1[ADDRESS1_LEN-1] = 0;
   p_uf->address2[ADDRESS1_LEN-1] = 0;
   p_uf->city[CITY_LEN-1] = 0;
   p_uf->state_province[CITY_LEN-1] = 0;
   p_uf->zip_code[STATEPROVINCE_LEN-1] = 0;
   p_uf->country_code[COUNTRYCODE_LEN-1] = 0;

   // Now verify completion of form
   //
   if ( Enforce_Required )
   {
      char Fields[256];
      Fields[0] = '\0';

      if ( !strlen ( p_uf->fname          ) ) strcat ( Fields, "first name," );
      if ( !strlen ( p_uf->lname          ) ) strcat ( Fields, "last name," );
      if ( !strlen ( p_uf->username       ) ) strcat ( Fields, "username," );
      if ( !strlen ( p_uf->email          ) ) strcat ( Fields, "e-mail," );
      if ( !strlen ( p_uf->address1       ) ) strcat ( Fields, "address," );
      if ( !strlen ( p_uf->city           ) ) strcat ( Fields, "city," );
      if ( !strlen ( p_uf->state_province ) ) strcat ( Fields, "state/province," );
      if ( !strlen ( p_uf->country_code   ) ) strcat ( Fields, "country code," );
      if ( !strlen ( p_uf->zip_code       ) ) strcat ( Fields, "zip code," );

      if ( Fields[0] )
      {
         SS_HTML_WriteP ( "You missed some required fields: %s", Fields );
         return -1;
      }
   }

   return 0;
}

// =---------------------------------------------------------------------------
// U s e r _ F o r m _ T o _ R e c o r d
//
// Translates a user_form structure (cgi form) to a nm_user_record structure (database record)
//
// If Merge is true, blank form fields will not be written to the record. See
//   SS_Port_Merge_Strcpy_Len and SS_Port_Merge_Ascii_To_Byte.
// =---------------------------------------------------------------------------
int User_Form_To_Record ( user_form* p_form, nm_user_record* p_rec, uint_08 Merge )
{
   if ( !Merge )
      memset ( &p_rec->record, 0, sizeof(client_record) );

   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.fname ,          p_form->fname,             MAX_CLIENT_FNAME     );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.lname,           p_form->lname,             MAX_CLIENT_LNAME     );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.username,        p_form->username,          MAX_CLIENT_USERNAME  );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.email,           p_form->email,             MAX_CLIENT_EMAIL     );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.password,        p_form->password1,         MAX_CLIENT_PASSWORD  );

   p_rec->record.connects      = htonl ( atoi(p_form->connects)      );
   p_rec->record.lastgroup_uid = htons ( (sint_16)atoi(p_form->lastgroup_uid) );
   p_rec->record.rating        = htons ( (sint_16)atoi(p_form->rating) );
   p_rec->record.wins          = htons ( (sint_16)atoi(p_form->wins) );
   p_rec->record.losses        = htons ( (sint_16)atoi(p_form->losses) );

   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->record.status,            p_form->status          );
   SS_Port_Merge_Ascii_To_Byte ( Merge, &p_rec->record.registration,      p_form->registration    );

   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.address1,        p_form->address1,          ADDRESS1_LEN         );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.address2,        p_form->address2,          ADDRESS2_LEN         );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.city,            p_form->city,              CITY_LEN             );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.state_province,  p_form->state_province,    STATEPROVINCE_LEN    );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.zip_code,        p_form->zip_code,          ZIPCODE_LEN          );
   SS_Port_Merge_Strcpy_Len ( Merge, p_rec->record.country_code,    p_form->country_code,      COUNTRYCODE_LEN      );
   
   return 0;
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ S t a t u s _ S e l e c t
// =---------------------------------------------------------------------------
void Write_User_Status_Select ( uint_08 Default_Status )
{
   int i;

   SS_HTML_Write ( "<select name=uf_Status size=1>" );

   for ( i=DBCLIENTSTATUS_START; i <= DBCLIENTSTATUS_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)Default_Status) ? "selected" : "", Get_User_Status_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// W r i t e _ U s e r _ R e g i s t r at i o n _ S e l e c t
// =---------------------------------------------------------------------------
void Write_User_Registration_Select ( int Default_Registration )
{
   int i;

   SS_HTML_Write ( "<select name=uf_Registration size=1>" );

   for ( i=DBREGISTRATION_START; i <= DBREGISTRATION_END; i++ )
      SS_HTML_Write ( "<option value=%d %s>%s</option>", i, (i == (int)Default_Registration) ? "selected" : "", Get_User_Registration_String((uint_08)i) );

   SS_HTML_Write ( "</select>" );
}

// =---------------------------------------------------------------------------
// G e t _ U s e r _ S t a t u s _ S t r i n g
// =---------------------------------------------------------------------------
const char* Get_User_Status_String ( uint_08 Status )
{
   switch ( Status )
   {
      case DBCLIENTSTATUS_NORMAL    : return "Normal";
      case DBCLIENTSTATUS_DELETED   : return "Deleted";

      default:
         sprintf ( Unknown_User_String, "(Unknown Status %d)", (int)Status );
         return Unknown_User_String;
   }
}

// =---------------------------------------------------------------------------
// G e t _ U s e r _ R e g i s t r a t i o n _ S t r i n g
// =---------------------------------------------------------------------------
const char* Get_User_Registration_String ( uint_08 Registration )
{
   switch ( Registration )
   {
      case DBREGISTRATION_NONE        : return "None";
      case DBREGISTRATION_EVALREG     : return "Registered for Evaluation";
      case DBREGISTRATION_REGISTERED  : return "Fully Registered";

      default:
         sprintf ( Unknown_User_String, "(Unknown Registration Code %d)", (int)Registration );
         return Unknown_User_String;
   }
}

// =---------------------------------------------------------------------------
// D u m p _ U s e r _ F o r m 
// =---------------------------------------------------------------------------
void Dump_User_Form ( user_form* p_uf )
{
   if ( !p_uf )
      return;

   SS_HTML_Write ( "uf_Recnum       = %s<BR>", (char*)p_uf->recnum         );
   SS_HTML_Write ( "uf_FirstName    = %s<BR>", (char*)p_uf->fname          );
   SS_HTML_Write ( "uf_LastName     = %s<BR>", (char*)p_uf->lname          );
   SS_HTML_Write ( "uf_Username     = %s<BR>", (char*)p_uf->username       );
   SS_HTML_Write ( "uf_Email        = %s<BR>", (char*)p_uf->email          );
   SS_HTML_Write ( "uf_Password1    = %s<BR>", (char*)p_uf->password1      );
   SS_HTML_Write ( "uf_Password2    = %s<BR>", (char*)p_uf->password2      );
   SS_HTML_Write ( "uf_Connects     = %s<BR>", (char*)p_uf->connects       );
   //SS_HTML_Write ( "uf_LastGroupUID = %s<BR>", (char*)p_uf->lastgroup_uid  );
   SS_HTML_Write ( "uf_Status       = %s<BR>", (char*)p_uf->status         );
   SS_HTML_Write ( "uf_Registration = %s<BR>", (char*)p_uf->registration   );
   SS_HTML_Write ( "uf_Address1     = %s<BR>", (char*)p_uf->address1       );
   SS_HTML_Write ( "uf_Address2     = %s<BR>", (char*)p_uf->address2       );
   SS_HTML_Write ( "uf_City         = %s<BR>", (char*)p_uf->city           );
   SS_HTML_Write ( "uf_State        = %s<BR>", (char*)p_uf->state_province );
   SS_HTML_Write ( "uf_Zip          = %s<BR>", (char*)p_uf->zip_code       );
   SS_HTML_Write ( "uf_Country      = %s<BR>", (char*)p_uf->country_code   );
}

// =---------------------------------------------------------------------------
// Action_Enumerate_Users_Callback
//
// Port is in network byte order
//
// =---------------------------------------------------------------------------
int Action_Enumerate_Users_Callback ( int (*Callback)(nm_user_record* p_ur), int Write_Back )
{
   int               ret;
   uint_32           start;
   uint_32           end;
   uint_32           i;
   nm_bidir_numusers numusr;
   nm_bidir_getusers getusr;
   nm_user_record    user_rec;
   //char              Link[LINK_BYTES];

   //nm_zcs_authenticate_reply* p_auth_reply = (nm_zcs_authenticate_reply*)Page_Info.Raw_Auth_Reply;

   // Ask server how many users there are
   //
   numusr.Message_Type    = 0xFF;
   numusr.Message_Type_Ex = BIDIR_NUMUSERS;
   memset ( numusr.Num_Users, 0, 4 );

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&numusr, sizeof(numusr) );
   if ( ret != 0 )
      return -1;

   // Get the servers response : There are X users
   //
   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&numusr, sizeof(numusr) );
   if ( ret != 0 )
      return -1;

   end = ntohl( *(uint_32*)numusr.Num_Users );

   // Ask server for specified range of users
   //
   getusr.Message_Type           = 0xFF;
   getusr.Message_Type_Ex        = BIDIR_GETUSERS;              
   *(uint_32*)getusr.Start_User  = 0;
   *(uint_32*)getusr.End_User    = htonl(end-1);
   getusr.Rank_Sorted            = 0;

   ret = CZ_Send_Message ( Page_Info.Server_Sock, (uint_08*)&getusr, sizeof(getusr) );
      return -1;

   // Now, get the server's user list header
   //
   ret = CZ_Receive_Message ( Page_Info.Server_Sock, (uint_08*)&getusr, sizeof(getusr) );
      return -1;

   // Now, get each user and print results
   //
   start = ntohl ( *(uint_32*)getusr.Start_User );
   end   = ntohl ( *(uint_32*)getusr.End_User );

   // For each user, invoke callback
   //
   for ( i=start; i <= end; i++ )
   {
      ret = SS_Net_Receive_Buffer_Ex ( Page_Info.Server_Sock, &user_rec, sizeof(nm_user_record), 0 );
      
      if ( ret != sizeof(nm_user_record) )
         return -1;

      (*Callback) ( &user_rec );
   }

   return 0;
}