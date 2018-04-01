// =---------------------------------------------------------------------------
// p a g e _ e d i t _ u s e r _ r e s u l t . c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
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
//   00 Jan 26   nic   Created.
//


#include <stdlib.h>
#include "cgi_id_subsystems.h"
#include "srvstat_actions.h"
#include "pages.h"
#include "cgic.h"
#include "license_manager.h"
#include "databases.h"


// =---------------------------------------------------------------------------
// Types
// =---------------------------------------------------------------------------
typedef enum Edit_User_Command
{
   EUC_SEND_LIC         = 1,
   EUC_RECV_PAYMENT     = 2,
   EUC_PAYMENT_FAILED   = 3
} Edit_User_Command;

// =---------------------------------------------------------------------------
// Forward Declarations
// =---------------------------------------------------------------------------
int Action_Process_Edit_User_Command ( int Command, nm_user_record* p_ur );

// =---------------------------------------------------------------------------
// P a g e _ E d i t _ U s e r _ R e s u l t
// =---------------------------------------------------------------------------
int Page_Edit_User_Result  ( )
{
   int            ret;
   int            Show_User_Details = 1;
   nm_user_record ur;
   user_form      uf;
   char           Operation[VARIABLE_BYTES];

   SS_Port_ZeroMemory ( &ur, sizeof(nm_user_record) );
   SS_Port_ZeroMemory ( &uf, sizeof(user_form) );

   SS_HTML_Start_Page  ( "RE Edit User" );
   SS_HTML_Heading_Tag ( "RE Edit User", 1 );

   // Gather Fields...
   //
   ret = Action_Gather_User_Fields ( &uf, 0 /*enforce required*/ );

   if ( ret == 0 )
   {
      // Check password validity
      //
      ret = Action_Verify_Password ( uf.password1, uf.password2, 1 /*allow blank*/, 1 /*verbose*/ );
   
      if ( ret == 0 )
      {
         User_Form_To_Record ( &uf, &ur, 0 /*merge*/ );

         // Some of the commands modify the user record, so process the commands before calling 
         // Action_Modify_User_By_Recnum
         //
         if ( cgiFormNotFound != cgiFormStringNoNewlines ( "send_lic", Operation, VARIABLE_BYTES ) )
         {
            nm_user_record ur_actual;
            SS_Port_ZeroMemory ( &ur_actual, sizeof(nm_user_record) );

            // if password is blank, we're not modifying it so we need to look up
            // the user...
            //
            if ( !strlen(uf.password1) )
            {
               ret = Action_Lookup_User ( &ur_actual, uf.username );
            }
            else
            {
               memcpy ( &ur_actual, &ur, sizeof(nm_user_record) );
               ret = 0;
            }

            if ( ret == 0 )
               ret = Action_Process_Edit_User_Command ( EUC_SEND_LIC, &ur_actual );

         }
         //else if ( cgiFormNotFound != cgiFormStringNoNewlines ( "recv_payment", Operation, VARIABLE_BYTES ) )
         //{
         //   ret = Action_Process_Edit_User_Command ( EUC_RECV_PAYMENT, &ur );
         //}
         else if ( cgiFormNotFound != cgiFormStringNoNewlines ( "payment_failed", Operation, VARIABLE_BYTES ) )
         {
            // ret should always fail so we won't edit user record
            //
            ret = Action_Process_Edit_User_Command ( EUC_PAYMENT_FAILED, &ur );

            // prevent user details from showing, Action_Process_Edit_User_Command would
            // have already displayed a payment failed form...
            //
            Show_User_Details = 0;
         }

         // Only update the user record on success
         //
         if ( ret == 0 )
            ret = Action_Modify_User_By_Recnum ( &ur, atoi(uf.recnum) );

         if ( ret != 0 )
            SS_HTML_WriteP ( "Failed to update the user record." );
      }
      else
         SS_HTML_WriteP ( "The specified password did not match or was otherwise invalid." );
   }
   else
      SS_HTML_WriteP ( "Error gathering user form data." );

   if ( Show_User_Details )
      Edit_User_Body ( &ur, atoi(uf.recnum) );

   Write_Admin_Footer    ( );
   SS_HTML_End_Page ( );

   return ret;
}

// =---------------------------------------------------------------------------
// A c t i o n _ P r o c e s s _ E d i t _ U s e r _ C o m m a n d
//
// =---------------------------------------------------------------------------
int Action_Process_Edit_User_Command ( int Command, nm_user_record* p_ur )
{
   int      ret = 0;
   uint_08  Lic_Data [ 4096 ];
   char     Lic_Filename[256];
   int      Permit = PERMIT_NONE;
   //char     Name[80];
   //char     Sales_Email [ SYSSET_VALUE_BYTES ];

   switch ( Command )
   {
      // Send License File to User, if Requested
      //
      case EUC_SEND_LIC:
      {
         switch ( p_ur->record.registration )
         {
            case DBREGISTRATION_NONE:
            {
               SS_HTML_WriteP ( "No license file can be sent because the user is not registered." );
               Permit = PERMIT_NONE;  
               break;
            }
            case DBREGISTRATION_EVALREG:     Permit = PERMIT_ANONYMOUS_EVAL; break; // should be registered eval, but it doesn't work
            case DBREGISTRATION_REGISTERED:  Permit = PERMIT_REGISTERED_FULL; break;
         }

         if ( Permit != PERMIT_ANONYMOUS_EVAL )
         {
            ret = Generate_License_File ( p_ur->record.fname, p_ur->record.lname, p_ur->record.username, p_ur->record.password, Permit, Lic_Data, Lic_Filename );

            if ( ret != 0 )
            {
               SS_HTML_WriteP ( "Failed to generate a license file for the user." );
            }
            else
            {
               ret = Send_License_File ( p_ur->record.email, Lic_Filename, Lic_Data );

               if ( ret == 0 )
                  SS_HTML_WriteP ( "The license file to %s was sent successfully.", p_ur->record.email );
               else
                  SS_HTML_WriteP ( "Failed to send the license file to %s!", p_ur->record.email );
            }

         }
         break;
      }

      /*
      case EUC_RECV_PAYMENT:
      {
         // Update the user record
         //
         p_ur->record.registration = DBREGISTRATION_REGISTERED;
      
         // Send the License File...
         //
         Action_Process_Edit_User_Command ( EUC_SEND_LIC, p_ur );
      
         SS_HTML_WriteP ( "The user was registered successfully." );
      
         break;
      }
     
      case EUC_PAYMENT_FAILED:
      {
         
         // setting ret to -1 will prevent the user record from updating
         ret = -1;

         SS_HTML_Form ( "Payment_Failed" );
            SS_HTML_Table_Tag ( );

            SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "  <th colspan=\"2\">Payment Failed Notification</th>" );
            SS_HTML_Write ( "</tr>" );

            // From
            SysSet_Get_Value ( "Sales_Department", Sales_Email );

            SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "  <th>From</th>" );
            SS_HTML_Write ( "  <td> <input type=\"text\" name=\"from\" value=\"%s\"> </td>", Sales_Email );
            SS_HTML_Write ( "</tr>" );

            // To
            sprintf ( Name, "%s %s", p_ur->record.fname, p_ur->record.lname );
            SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "  <th>To</th>" );
            SS_HTML_Write ( "  <td> <input type=\"text\" name=\"to\" value=\"%s\"> </td>", Name );
            SS_HTML_Write ( "</tr>" );
            
            // E-mail
            SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "  <th>E-mail</th>" );
            SS_HTML_Write ( "  <td> <input type=\"text\" name=\"email\" value=\"%s\"> </td>", p_ur->record.email );
            SS_HTML_Write ( "</tr>" );

            // Message
            //
            SS_HTML_Write ( "<tr>" );
            SS_HTML_Write ( "  <th>Message</th>" );
            SS_HTML_Write ( "  <td> <textarea rows=\"7\" name=\"message\" cols=\"50\">%s</textarea></td>", "This is a test" );
            SS_HTML_Write ( "</tr>" );

            // Submit
            SS_HTML_Write ( "<tr><td>&nbsp;</td><td>" );
               SS_HTML_Submit ( 0 );
            SS_HTML_Write ( "</td></tr>" );

            SS_HTML_End_Table_Tag ( );
         SS_HTML_End_Form ( );

         SS_HTML_WriteP ( "Hit Go! once you are satisfied with the <i>Payment Failed</i> notification message." );
         break;
      }

      */

      default:
         // Unknown command
         ret = -1;
         break;
   }

   return ret;
}