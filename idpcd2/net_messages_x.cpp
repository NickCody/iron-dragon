
#include "PreComp.h"

#include "net_messages.h"
#include "net_messages_admin.h"
#include "net_messages_x.h"
#include "common.h"
#include "connection_idpcd.h"
#include "stack_tag.h"

uint_32 Report_Raw_Bytes = 0;
char Message_Name_Buf[1024];

// =---------------------------------------------------------------------------
// (global)
//
// =---------------------------------------------------------------------------
_result Scramble_Bytes( uint_08 *m, uint_16 m_len, uint_08 key )
{
   if (!key) return RS_OK;

   //uint_32  sval     = 0x31415918;
   uint_32  sval     = 0xEF5DBAF7;  // NickC 4/20/00 : Changed to protect security (accidental source code post)
   uint_08  bshf     = 0;

   key &= 0x1f;   // modulate the key to the range 0..31

   try
   {
      while (m_len--)
      {
         if (key) {sval = (sval << key) | (sval >> (32 - key));
                   key = 0;}

         *m++ ^= uint_08((sval >> bshf) & 0xff);

         if (bshf != 24) bshf += 8; else {bshf = 0; key = 1;}
      }
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Scramble_Bytes>" );
      return RS_EXCEPTION;
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (global) Print_Hex_Bytes
//
// Prints out a line of hexadecimal bytes, two characters wide, with a space
//    between each column. For a total of 3 characters of dest output per byte 
//    in src.
//
// =---------------------------------------------------------------------------
void Print_Hex_Bytes ( char* dest, uint_08* src, uint_32 cols )
{
   char* hex_codes = "0123456789ABCDEF";

   uint_32 col = 0;

   for ( col=0; col < cols; col++ )
   {
      // Put the last space in
      dest[col*3+2] = ' ';

      if ( src[col] == 0 )
      {
         dest[col*3]   = '_';
         dest[col*3+1] = '_';
      }
      else if ( src[col] > 31 && src[col] < 128 &&
                src[col] != '&' &&     // skip these special characters to our 
                src[col] != '<' &&     // output is XML-friendly
                src[col] != '>' )
      {
         dest[col*3]   = '.';
         dest[col*3+1] = src[col];
      }
      else
      {
         dest[col*3]   = hex_codes[ (src[col] & 0xF0) >> 4];
         dest[col*3+1] = hex_codes[ (src[col] & 0x0F)     ];
      }
   }

   dest[col*3] = 0;

}

// =---------------------------------------------------------------------------
// (global) M e s s a g e _ N a m e
//
// =---------------------------------------------------------------------------
const uint_08 * const Message_Name ( uint_08 Message_Type, uint_08 Message_Type_Ex )
{
   char* Msg = "";
   bool Extended = false;

   switch ( Message_Type )
   {
      case CZS_AUTHENTICATE_REQUEST : Msg = "CZS_AUTHENTICATE_REQUEST"; break;
      case CZS_JOIN_GROUP_REQUEST   : Msg = "CZS_JOIN_GROUP_REQUEST"; break;
      case CZS_QUIT_GROUP_REQUEST   : Msg = "CZS_QUIT_GROUP_REQUEST"; break;

      case ZCS_AUTHENTICATE_REPLY   : Msg = "ZCS_AUTHENTICATE_REPLY"; break;
      case ZCS_JOIN_GROUP_REPLY     : Msg = "ZCS_JOIN_GROUP_REPLY"; break;
      case ZCS_QUIT_GROUP_NOTIFY    : Msg = "ZCS_QUIT_GROUP_NOTIFY"; break;
      case ZCS_GROUP_MEMBER_STATUS  : Msg = "ZCS_GROUP_MEMBER_STATUS"; break;
      case ZCS_GAME_MODULE_STATUS   : Msg = "ZCS_GAME_MODULE_STATUS"; break;
      case ZCS_GAME_TYPE_STATUS     : Msg = "ZCS_GAME_TYPE_STATUS"; break;
      case ZCS_PLAYER_ASSIGN_STATUS : Msg = "ZCS_PLAYER_ASSIGN_STATUS"; break;
      case ZCS_GAME_OPTION_STATUS   : Msg = "ZCS_GAME_OPTION_STATUS"; break;
      case ZCS_START_GAME_NOTIFY    : Msg = "ZCS_START_GAME_NOTIFY"; break;
      case ZCS_GAME_GROUP_NOTIFY    : Msg = "ZCS_GAME_GROUP_NOTIFY"; break;

      case LZS_GAME_MODULE_REQUEST  : Msg = "LZS_GAME_MODULE_REQUEST"; break;
      case LZS_GAME_TYPE_ACTION     : Msg = "LZS_GAME_TYPE_ACTION"; break;
      case LZS_PLAYER_ASSIGN_ACTION : Msg = "LZS_PLAYER_ASSIGN_ACTION"; break;
      case LZS_GAME_OPTION_ACTION   : Msg = "LZS_GAME_OPTION_ACTION"; break;
      case LZS_START_GAME_ACTION    : Msg = "LZS_START_GAME_ACTION"; break;

 
      case CCH_TEXT                 : Msg = "CCH_TEXT"; break;
      case CCH_AUDIO                : Msg = "CCH_AUDIO"; break;
      case ANY_SMACKDOWN            : Msg = "ANY_SMACKDOWN"; break;
      case ANY_PING                 : Msg = "ANY_PING"; break;

      case PGA_START_GAME_NOTIFY    : Msg = "PGA_START_GAME_NOTIFY"; break;
      case PGA_INITIAL_FOREMAN      : Msg = "PGA_INITIAL_FOREMAN"; break;
      case PGA_GAME_ACTION          : Msg = "PGA_GAME_ACTION"; break;
      case PGA_GAME_UNDO            : Msg = "PGA_GAME_UNDO"; break;
      case PGA_QUIT_GAME            : Msg = "PGA_QUIT_GAME"; break;

      case LGA_PLAYER_REASSIGN      : Msg = "LGA_PLAYER_REASSIGN"; break;
      case LGA_SETUP_GAME_DATA      : Msg = "LGA_SETUP_GAME_DATA"; break;
      case LGA_START_GAME_ACTION    : Msg = "LGA_START_GAME_ACTION"; break;

      case ZLG_GAME_STATUS_REQUEST  : Msg = "ZLG_GAME_STATUS_REQUEST"; break;
      case LZG_GAME_STATUS_REPLY    : Msg = "LZG_GAME_STATUS_REPLY"; break;

      case 0xFF:
      {
         switch ( Message_Type_Ex )
         {
            case ACZ_INFO           : Msg = "ACZ_INFO"; break;
            case ACZ_NUMUSERS       : Msg = "ACZ_NUMUSERS"; break;
            case ACZ_GETUSERS       : Msg = "ACZ_GETUSERS"; break;
            case CZA_INFO           : Msg = "CZA_INFO"; break;
            case BIDIR_NUMUSERS     : Msg = "BIDIR_NUMUSERS"; break;
            case BIDIR_GETUSERS     : Msg = "BIDIR_GETUSERS"; break;
            case BIDIR_ADDUSER      : Msg = "BIDIR_ADDUSER"; break;
            case BIDIR_SETUSER      : Msg = "BIDIR_SETUSER"; break;
            default:
               Msg = "UNKNOWN ADMIN"; break;
         }

         Extended = true;
      }

      default:
         Msg = "UNKNOWN"; break;
   }

   /*
   if ( Extended )
      sprintf ( Message_Name_Buf, "%s,%d:%d", Msg, (int)Message_Type, (int)Message_Type_Ex );
   else
      sprintf ( Message_Name_Buf, "%s,%d", Msg, (int)Message_Type );

   return (const uint_08 *)Message_Name_Buf;
   */

   return (uint_08*)Msg;
};

// =---------------------------------------------------------------------------
// (global) M e s s a g e _ I D
//
// =---------------------------------------------------------------------------
const uint_08 * const Message_ID ( uint_08 Message_Type, uint_08 Message_Type_Ex )
{
   if ( Message_Type == 0xFF )
      sprintf ( Message_Name_Buf, "%d:%d", (int)Message_Type, (int)Message_Type_Ex );
   else
      sprintf ( Message_Name_Buf, "%d", (int)Message_Type );

   return (const uint_08 *)Message_Name_Buf;
};


// =---------------------------------------------------------------------------
// (global) R e p o r t _ M e s s a g e
//
// Reports the bloody details of the specified message
//
// =---------------------------------------------------------------------------
// =---------------------------------------------------------------------------
// (global) R e p o r t _ M e s s a g e
//
// Reports the bloody details of the specified message
//
// =---------------------------------------------------------------------------
void Report_Message ( connection_idpcd& Conn, message_package& pkg, bool Is_Scrambled, bool Is_Send, bool Did_Succeed )
{
   uint_08 Message_Type    = pkg.p_data[0];
   uint_08 Message_Type_Ex = pkg.p_data[1];  // valid only if Message_Type is 0xFF
   Scramble_Bytes ( &Message_Type,    1, pkg.hdr.Scramble_Key );
   Scramble_Bytes ( &Message_Type_Ex, 1, pkg.hdr.Scramble_Key ); 

   // Set up mesage tag
   char Attribs[128];
   sprintf ( Attribs, "name=\"%s\" direction=\"%s\"", Message_Name (Message_Type,Message_Type_Ex), Is_Send ? "send" : "receive" );

   if ( Sys.Get_Active_MsgChannels() & CHANNEL_MESSAGE_DETAIL )
   {
      char datestr[32];
      SS_Port_Get_DateTime_String ( datestr );

      stacktag Msg ( CHANNEL_MESSAGE_DETAIL, "message", Attribs );

      Sys.Message ( CHANNEL_MESSAGE_DETAIL, "<handle>%s</handle>",         Conn.Get_Auth_User_Handle() );
      Sys.Message ( CHANNEL_MESSAGE_DETAIL, "<datetime>%s</datetime>",     datestr );
      Sys.Message ( CHANNEL_MESSAGE_DETAIL, "<msgid>%s</msgid>",           Message_ID (Message_Type,Message_Type_Ex) );
      Sys.Message ( CHANNEL_MESSAGE_DETAIL, "<socket>%d</socket>",         (int)Conn.Get_Socket() );
      Sys.Message ( CHANNEL_MESSAGE_DETAIL, "<ip>%s</ip>",                 Conn.Get_IP_String() );

      // We use exception handling here because we don't want to have
      // debugging writes (which tend to be buggy since they are not
      //  updated very well) to have their exceptions caught elsewhere.
      //
      if ( Report_Raw_Bytes )
         Report_Message_Bytes ( pkg, Is_Scrambled );
      else
         Report_Message_Specifics ( pkg, Is_Scrambled );
   }
   else
   {
      Sys.Message ( CHANNEL_ACTIVITYLOG, "<message %s handle=\"%s\" />", Attribs, Conn.Get_Auth_User_Handle() );
   }


}

// =---------------------------------------------------------------------------
// (global) R e p o r t _ M e s s a g e _ S p e c i f i c s
//
// Reports the bloody details of the specified message
//
// =---------------------------------------------------------------------------
void Report_Message_Specifics ( message_package& pkg, bool Is_Scrambled )
{
   // Store Message Type and unscramble local variable
   //
   uint_16 Message_Length = NM_Payload_Length( &pkg.hdr );

   if ( Is_Scrambled )
      Scramble_Bytes ( pkg.p_data, Message_Length, pkg.hdr.Scramble_Key );

   uint_08 Message_Type = pkg.p_data[0];

   Sys.Message_Indent ( 2 );

   switch ( Message_Type )
   {
      case CZS_AUTHENTICATE_REQUEST :
      {
         nm_czs_authenticate_request* p_msg = (nm_czs_authenticate_request*)pkg.p_data;

         uint_16 port = Make_Hardware_Word(p_msg->Inbound_Port);

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "User_Handle[%s]",         (char*)p_msg->User_Handle );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Default_Player_Name[%s]", (char*)p_msg->Default_Player_Name );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Inbound_Port[%d,0x%x]",   (int  )port, (int)port );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Password[*****]" ); //,            (char*)p_msg->Password );
      
         break;
      }

      case CZS_JOIN_GROUP_REQUEST   :
      {
         nm_czs_join_group_request* p_msg = (nm_czs_join_group_request*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Game_Module[%d]",     (int)p_msg->Game_Module );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_Name[%s]",      (char*)p_msg->Group_Name );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_Password[*****]" ); //,  (char*)p_msg->Group_Password );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Is_Ranked[%s]",  p_msg->Is_Ranked ? "yes" : "no" );

         break;
      }

      case CZS_QUIT_GROUP_REQUEST           :
      {
         //nm_czs_quit_group_request* p_msg = (nm_czs_quit_group_request*)pkg.p_data;
         break;
      }


      case ZCS_AUTHENTICATE_REPLY   :
      {
         nm_zcs_authenticate_reply* p_msg = (nm_zcs_authenticate_reply*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Client_UID[%d]",            (int)Make_Hardware_Word(p_msg->Client_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Made_Inbound_Connect[%d]",  (int)p_msg->Made_Inbound_Connect );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "CZ_Message[%s]",            (char*)p_msg->CZ_Message );

         break;
      }

      case ZCS_JOIN_GROUP_REPLY  :
      {
         nm_zcs_join_group_reply* p_msg = (nm_zcs_join_group_reply*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]",        (int)Make_Hardware_Word(p_msg->Group_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Reply_Code[%d]",       (int)p_msg->Reply_Code );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Assigned_Slot_ID[%d]", (int)p_msg->Assigned_Slot_ID );

         char ip[IP_STRING_BYTES];
         SS_Net_Print_IP ( ip, *(long*)p_msg->GL_IP_Address );

         uint_16 port = Make_Hardware_Word(p_msg->GL_IP_Port);

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "GL_IP_Address[%s]",    ip );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "GL_IP_Port[%d,0x%x]",    (int)port, (int)port );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Is_Ranked[%s]",    p_msg->Is_Ranked ? "yes" : "no" );

         break;
      }

      case ZCS_QUIT_GROUP_NOTIFY      :
      {
         nm_zcs_quit_group_notify* p_msg = (nm_zcs_quit_group_notify*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]", (int)Make_Hardware_Word(p_msg->Group_UID) );

         break;
      }

      case ZCS_GROUP_MEMBER_STATUS  :
      {
         nm_zcs_group_member_status* p_msg = (nm_zcs_group_member_status*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]",             (int)Make_Hardware_Word(p_msg->Group_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Client_UID[%d]",            (int)Make_Hardware_Word(p_msg->Client_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Slot_ID[%d]",               (int)p_msg->Slot_ID );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Is_Slot_Filled[%d]",        (int)p_msg->Is_Slot_Filled );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Supported_Modules[%d]",     (int)Make_Hardware_Word(p_msg->Supported_Modules) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "User_Handle[%s]",           (char*)p_msg->User_Handle );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Default_Player_Name[%s]",   (char*)p_msg->Default_Player_Name );
      
         break;
      }

      case ZCS_START_GAME_NOTIFY:
      {
         //nm_xxx_start_game* p_msg = (nm_xxx_start_game*)pkg.p_data;

         break;
      }

      case ZCS_GAME_GROUP_NOTIFY:
      {
         nm_zcs_game_group_notify* p_msg = (nm_zcs_game_group_notify*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]", (int)Make_Hardware_Word(p_msg->Group_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Number_Of_Members[%d]", (int)p_msg->Number_Of_Members );

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Game_Module[%d]", (int)p_msg->Game_Module );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Is_Ranked[%s]", p_msg->Is_Ranked ? "yes" : "no" );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_Name[%s]", p_msg->Group_Name );
         
         break;
      }

      case LZS_GAME_MODULE_REQUEST  :
      case ZCS_GAME_MODULE_STATUS   :
      {
         nm_xxs_game_module* p_msg = (nm_xxs_game_module*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Module_ID[%d]", (int)p_msg->Module_ID );
      
         break;
      }

      case LZS_GAME_TYPE_ACTION:
      case ZCS_GAME_TYPE_STATUS:
      {
         nm_xxs_game_type* p_msg = (nm_xxs_game_type*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Game_Slot_ID[%d]",   (int)p_msg->Game_Slot_ID);
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Save_Game_Name[%s]",  (char*)p_msg->Save_Game_Name );

         break;
      }

      case LZS_PLAYER_ASSIGN_ACTION:
      case ZCS_PLAYER_ASSIGN_STATUS:
      case LGA_PLAYER_REASSIGN:
      {
         nm_xxs_player_assign* p_msg = (nm_xxs_player_assign*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Player_ID[%d]",          (int)p_msg->Player_ID );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_Slot_ID[%d]",   (int)p_msg->Group_Slot_ID );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Color_ID[%d]",           (int)p_msg->Color_ID );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Name[%s]",               (char*)p_msg->Name );

         break;
      }

      case LZS_GAME_OPTION_ACTION:
      case ZCS_GAME_OPTION_STATUS:
      {
         nm_xxs_game_option* p_msg = (nm_xxs_game_option*)pkg.p_data;

         try
         {

            Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Num_Options[%d] = ", p_msg->Num_Options );

            for ( int i=0; i < p_msg->Num_Options; i++ )
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Option_ID[%2d] = %3d", (int)i, (int)p_msg->Option_Setting[i] );
         }
         catch ( ... )
         {
            Sys.Message ( CHANNEL_EXCEPTIONLOG, "<exception>Report_Message_Specifics</exception>, ZCS_GAME_OPTION_STATUS>" );
         }

         break;
      }

      case LZS_START_GAME_ACTION:
      {
         //nm_xxx_start_game* p_game = (nm_xxx_start_game*)pkg.p_data;

         break;
      }

      case CCH_TEXT:
      {
         nm_cch_text* p_msg = (nm_cch_text*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]",    (int)Make_Hardware_Word(p_msg->Group_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Client_UID[%d]",   (int)Make_Hardware_Word(p_msg->Client_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Text_String[%s]",  (char*)p_msg->Text_String );

         break;
      }

      case ANY_PING:
      {
         nm_any_ping* p_msg = (nm_any_ping*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Bounce_Count[%d]", (int)p_msg->Bounce_Count );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]",    (int)Make_Hardware_Word(p_msg->Group_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Client_UID[%d]",   (int)Make_Hardware_Word(p_msg->Client_UID) );

         break;
      }

      case ZLG_GAME_STATUS_REQUEST  :
      {
         //nm_zlg_game_status_request* p_msg = (nm_zlg_game_status_request*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "(no fields)" );
         break;
      }


      case LZG_GAME_STATUS_REPLY    :
      {
         nm_lzg_game_status_reply* p_msg = (nm_lzg_game_status_reply*)pkg.p_data;

         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Turn_Number[%d]",          (int)Make_Hardware_Word(p_msg->Turn_Number) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Group_UID[%d]",            (int)Make_Hardware_Word(p_msg->Group_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "GL_Client_UID[%d]",        (int)Make_Hardware_Word(p_msg->GL_Client_UID) );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "GL_Password[*****]" ); //,          p_msg->GL_Password);
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Number_Of_Players[%d]",    (int)p_msg->Number_Of_Players );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Winning_Player_ID[%d]",    (int)p_msg->Winning_Player_ID );
         Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Turn_Number[%d]",          (int)Make_Hardware_Word(p_msg->Turn_Number) );
   
         Sys.Message_Indent ( 2 );

         try
         {
            for ( int i=0; i < p_msg->Number_Of_Players; i++ )
            {
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "User_Handle[%s]",    p_msg->Player_Status[i].User_Handle  );
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Client_UID[%d]",     (int)Make_Hardware_Word(p_msg->Player_Status[i].Client_UID) );
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "GP[%d]",             (int)Make_Hardware_Word(p_msg->Player_Status[i].GP) );
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "GP_Earned[%d]",      (int)Make_Hardware_Word(p_msg->Player_Status[i].GP_Earned) );
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "MC_Connected[%d]",   (int)p_msg->Player_Status[i].MC_Connected );
               Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Current_Rank[%d]",   (int)p_msg->Player_Status[i].Current_Rank );
            }
         }
         catch ( ... )
         {
            Sys.Message ( CHANNEL_EXCEPTIONLOG, "<exception>Report_Message_Specifics</exception>, LZG_GAME_STATUS_REPLY>" );
         }  

         Sys.Message_Indent ( -2 );

         break;
      }

      default:
      {
         Report_Message_Bytes ( pkg, Is_Scrambled );
         break;
      }

   } // switch ( Message_Type )

   Sys.Message_Indent ( -2 );

   // Put the message right where it was...
   // 
   if ( Is_Scrambled )
      Scramble_Bytes ( pkg.p_data, Message_Length, pkg.hdr.Scramble_Key );

}

// =---------------------------------------------------------------------------
// (global) R e p o r t _ M e s s a g e _ B y t e s
//
// Reports the bloody details of the specified message in raw format
//
// =---------------------------------------------------------------------------

void Report_Message_Bytes ( message_package& pkg, bool Is_Scrambled )
{
   // Store Message Type and unscramble local variable
   //
   uint_16 Message_Length = Make_Hardware_Word ( pkg.hdr.Payload_Length );

   if ( Is_Scrambled )
      Scramble_Bytes ( pkg.p_data, Message_Length, pkg.hdr.Scramble_Key );

   //uint_08 Message_Type = pkg.p_data[0];

   Sys.Message ( CHANNEL_MESSAGE_DETAIL, "<raw>" );
   Sys.Message_Indent ( 2 );

   uint_32 byte = 0;
   while ( byte < Message_Length  )
   {
      char line[61];

      if ( Message_Length-byte < 20 )
         Print_Hex_Bytes ( line, &pkg.p_data[byte], Message_Length-byte );
      else
         Print_Hex_Bytes ( line, &pkg.p_data[byte], 20 );

      Sys.Message ( CHANNEL_MESSAGE_DETAIL, line );

      byte += 20;
   }

   Sys.Message_Indent ( -2 );
   Sys.Message ( CHANNEL_MESSAGE_DETAIL, "</raw>" );

   // Put the message right where it was...
   // 
   if ( Is_Scrambled )
      Scramble_Bytes ( pkg.p_data, Message_Length, pkg.hdr.Scramble_Key );
}

