// =---------------------------------------------------------------------------
// i d p c d _ g r o u p _ m e s s a g e s. c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the group class for maintaining groups of client
//    connections
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Jul 01   nic   Created. SPlit from idpcd_group.cpp
//

#include "PreComp.h"

#include "idpcd_defaults.h"
#include "connection_idpcd.h"
#include "idpcd_group.h"
#include "connection.h"
#include "connection_pool.h"
#include "startup_daemon.h"
#include "command_line.h"
#include "unique_id.h"
#include "group_pool.h"
#include "port_subsystem.h"
#include "stack_tag.h"
#include "rank_calc.h"

// =---------------------------------------------------------------------------
// =---------------------------------------------------------------------------
//
//
//
// SEND MESSAGES
//
//
//
// =---------------------------------------------------------------------------
// =---------------------------------------------------------------------------

// =---------------------------------------------------------------------------
// (public) B r o a d c a s t _ G r o u p _ M e m b e r _ S t a t u s
//
// Notify other parties that player joined them or left them...
//
// =---------------------------------------------------------------------------
_result idpcd_group::Broadcast_Group_Member_Status ( uint_08 Slot, uint_08 Filled, connection_idpcd& conn_about )
{
   nm_zcs_group_member_status gms;
   message_package pkg ( (uint_08*)&gms );
   NM_Write_Header( &pkg.hdr, sizeof(nm_zcs_group_member_status) );

   memset ( &gms, 0, sizeof(nm_zcs_group_member_status) );

   gms.Message_Type           = ZCS_GROUP_MEMBER_STATUS;
   gms.Group_UID[0]           = MSB ( Group_UID            );
   gms.Group_UID[1]           = LSB ( Group_UID            );
   gms.Client_UID[0]          = MSB ( conn_about.Get_UID() );
   gms.Client_UID[1]          = LSB ( conn_about.Get_UID() );
   gms.Slot_ID                = Slot;
   gms.Is_Slot_Filled         = Filled;
   gms.Supported_Modules[0]   = MSB( 1); /*Iron Dragon=bit1*/
   gms.Supported_Modules[1]   = LSB( 1);
   gms.Current_Player_Rank[0] = MSB( conn_about.Get_Current_Rank() );
   gms.Current_Player_Rank[1] = LSB( conn_about.Get_Current_Rank() );

   SS_Port_Strcpy_Len( (char*)gms.User_Handle, (char*)conn_about.Get_Auth_User_Handle(), USER_HANDLE_BYTES );
   SS_Port_Strcpy_Len( (char*)gms.Default_Player_Name, (char*)conn_about.Get_Auth_Default_Player_Name(), PLAYER_NAME_BYTES );

   // Scramble the eggs and send them out
   //
   _result res = Scramble_Bytes ( (uint_08*)&gms, sizeof(nm_zcs_group_member_status), IDPCD_SCRAMBLE_KEY );
   
   if ( RSUCCEEDED(res) )
      res = Broadcast_To_Targets ( pkg );

   return res;
}

// =---------------------------------------------------------------------------
// S e n d _ A l l _ S u m m a r i e s
//
_result idpcd_group::Send_All_Summaries ( connection_idpcd& conn_dest, uint_08 Slot )
{
   _result res = RS_OK;

   if ( !Is_Lobby() )
   {
      // Send the player module status
      //
      res = Send_Module_Status ( conn_dest );
      RRETURNONFAILURE(res);

      // Send the game's option bytes to client
      //
      res = Send_Option_Status ( conn_dest );
      RRETURNONFAILURE(res);

      // Send the game's type
      //
      res = Send_Game_Type_Status ( conn_dest );
      RRETURNONFAILURE(res);

      // Summarize player assignments
      //
      res = Send_Player_Assign_Status_Summary ( conn_dest );
      RRETURNONFAILURE(res);
   }

   // Notify other parties that player joined them...
   //
   res = Broadcast_Group_Member_Status ( Slot, 1 /*filled*/, conn_dest );
   RRETURNONFAILURE(res);

   //SS_Port_Sleep ( 2000 );

   // Summarize the group for the new grouper
   //
   res = Send_Group_Member_Status_Summary ( conn_dest );
   RRETURNONFAILURE(res);


   return res;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ P l a y e r _ A s s i g n _ S t a t u s _ S u m m a r y
//
// Sends player assign status for to conn_dest concerning conn_about
//
// =---------------------------------------------------------------------------
_result idpcd_group::Send_Player_Assign_Status_Summary ( connection_idpcd& conn_dest )
{
   nm_xxs_player_assign pas;
   message_package pkg ( (uint_08*)&pas );
   NM_Write_Header( &pkg.hdr, sizeof(nm_xxs_player_assign) );

   pas.Message_Type = ZCS_PLAYER_ASSIGN_STATUS;

   _result res = RS_OK;

   for ( int i=0; i < MAX_PLAYERS_PER_GAME; i++ )
   {
      if ( rgComputerSlot[i] )
      {
         // This is an error
         //
         if ( rgComputerSlot[i] != 128 && rgComputerSlot[i] >= MAX_PLAYERS_PER_GROUP )
         {
            Sys.Message ( CHANNEL_ERRORLOG, "Invalid Connection_ID<%d> in Computer_Slot<%d> in Group<%d>",
               (int)rgComputerSlot[i], (int)(i+1), (int)Group_UID );
            continue;
         }

         if ( rgComputerSlot[i] == 128 /*AI*/ || Is_Assigned_Connection ( rgComputerSlot[i] ) )
         {
            pas.Color_ID         = rgColor_ID[i];
            pas.Player_ID        = i+1;
            pas.Group_Slot_ID    = rgComputerSlot[i];

            SS_Port_Strcpy_Len ( pas.Name, rgName[i], PLAYER_NAME_BYTES );

            // Scramble Outgoing Message
            //
            res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), IDPCD_SCRAMBLE_KEY );

            res = conn_dest.Send_Message ( pkg );

            // Unscramble message
            //
            res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), IDPCD_SCRAMBLE_KEY );

         }
      }
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) I s _ A s s i g n e d _ C o n n e c t i o n
//
// =---------------------------------------------------------------------------
bool idpcd_group::Is_Assigned_Connection ( uint_16 GroupConnIdx )
{
   uint_16 ConnIdx = Group_To_Busy_Pool_Index ( GroupConnIdx );
   connection_idpcd& Conn = Busy_Pool.Get_Connection ( ConnIdx );

   return !Conn.Is_Dummy();
}

// =---------------------------------------------------------------------------
// (public) G r o u p _ T o _ B u s y _ P o o l _ I n d e x
//
// =---------------------------------------------------------------------------
uint_16 idpcd_group::Group_To_Busy_Pool_Index ( uint_16 GroupConnIdx )
{
   if ( GroupConnIdx < Num_Slots )
      return rgConnection_Indices[GroupConnIdx];
   else
      return 0xFFFF;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ G r o u p _ M e m b e r _ S t a t u s _ S u m m a r y
//
// Summarizes the group for the new grouper
//
// =---------------------------------------------------------------------------
_result idpcd_group::Send_Group_Member_Status_Summary ( connection_idpcd& conn_dest )
{
   _result res = RS_OK;

   nm_zcs_group_member_status gms;
   message_package pkg ( (uint_08*)&gms );
   NM_Write_Header( &pkg.hdr, sizeof(nm_zcs_group_member_status) );

   for ( int i=0; i < Num_Slots; i++ )
   {
      connection_idpcd& Conn = Get_Group_Connection ( i );
      if ( Conn.Is_Dummy() )
         continue;

      // For each non-null player other than conn_dest, send conn_dest summary
      //
      if ( &Conn != &conn_dest && Conn.Get_Lock_Status() == CS_BUSY )
      {
         memset ( &gms, 0, sizeof(nm_zcs_group_member_status) );

         gms.Message_Type          = ZCS_GROUP_MEMBER_STATUS;
         gms.Group_UID[0]          = MSB ( Group_UID );
         gms.Group_UID[1]          = LSB ( Group_UID );
         gms.Client_UID[0]         = MSB ( Conn.Get_UID() );
         gms.Client_UID[1]         = LSB ( Conn.Get_UID() );
         gms.Slot_ID               = i+1;
         gms.Is_Slot_Filled        = 1 /*filled*/;
         gms.Supported_Modules[0]  = MSB(1); /*Iron Dragon=bit1*/
         gms.Supported_Modules[1]  = LSB(1);
         gms.Current_Player_Rank[0] = MSB( Conn.Get_Current_Rank() );
         gms.Current_Player_Rank[1] = LSB( Conn.Get_Current_Rank() );

         SS_Port_Strcpy_Len( (char*)gms.User_Handle, (char*)Conn.Get_Auth_User_Handle(), USER_HANDLE_BYTES );
         SS_Port_Strcpy_Len( (char*)gms.Default_Player_Name, (char*)Conn.Get_Auth_Default_Player_Name(), PLAYER_NAME_BYTES );

         _result res = Scramble_Bytes ( (uint_08*)&gms, sizeof(nm_zcs_group_member_status), IDPCD_SCRAMBLE_KEY );
   
         if ( RSUCCEEDED(res) )
            res = conn_dest.Send_Message ( pkg );
      }

      if ( RFAILED(res) )
         break;
   }

   return res;
}

// =---------------------------------------------------------------------------
// Send_Group_Notify
//
_result idpcd_group::Send_Group_Notify ( connection_idpcd& conn, idpcd_group* p_grp )
{
   nm_zcs_game_group_notify ggn;
   
   if ( !p_grp ) p_grp = this;

   _result res = p_grp->Initialize_Group_Notify ( &ggn );
   RRETURNONFAILURE(res);

   message_package pkg ( (uint_08*)&ggn );
   NM_Write_Header( &pkg.hdr, sizeof(nm_zcs_game_group_notify) );

   res = Scramble_Bytes ( (uint_08*)&ggn, sizeof(nm_zcs_game_group_notify), IDPCD_SCRAMBLE_KEY );

   if ( RSUCCEEDED(res) )
      res = conn.Send_Message ( pkg );

   return res;
}

// =---------------------------------------------------------------------------
// B r o a d c a s t _ G r o u p _ N o t i f y
//
_result idpcd_group::Broadcast_Group_Notify ( nm_zcs_game_group_notify* p_ggn )
{
   message_package pkg ( (uint_08*)p_ggn );
   NM_Write_Header( &pkg.hdr, sizeof(nm_zcs_game_group_notify) );

   _result res = Scramble_Bytes ( (uint_08*)p_ggn, sizeof(nm_zcs_game_group_notify), IDPCD_SCRAMBLE_KEY );

   if ( RSUCCEEDED(res) )
      res = Broadcast_To_Targets ( pkg );

   return res;
}

// =---------------------------------------------------------------------------
// (public) I n i t i a l i z e _ G r o u p _ N o t i f y 
//
// =---------------------------------------------------------------------------
_result idpcd_group::Initialize_Group_Notify ( nm_zcs_game_group_notify* p_ggn )
{
   if ( !p_ggn ) return RS_ERR;

   p_ggn->Message_Type      = ZCS_GAME_GROUP_NOTIFY;
   p_ggn->Group_UID[0]      = MSB ( Get_Group_UID() );
   p_ggn->Group_UID[1]      = LSB ( Get_Group_UID() );
   p_ggn->Number_Of_Members = (uint_08)Get_Ref_Count();
   p_ggn->Game_Module       = Get_Module_ID();
   p_ggn->Is_Ranked         = Ranked;
   strcpy ( p_ggn->Group_Name, Get_Group_Name() );

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ M o d u l e _ S t a t u s
//
// =---------------------------------------------------------------------------
_result idpcd_group::Send_Module_Status ( connection_idpcd& conn )
{
   _result res = RS_OK;

   try
   {
      // Initialize message structure
      //
      nm_xxs_game_module gm;
      memset ( &gm, 0, sizeof(nm_xxs_game_module) );

      // Set message fields
      //
      gm.Message_Type = ZCS_GAME_MODULE_STATUS;
      gm.Module_ID    = Module_ID;

      // Package message structure
      //
      message_package pkg ( (uint_08*)&gm );
      NM_Write_Header ( &pkg.hdr, sizeof(nm_xxs_game_module) );

      // Scramble bytes
      res = Scramble_Bytes ( pkg.p_data, sizeof(nm_xxs_game_module), pkg.hdr.Scramble_Key );

      if ( RSUCCEEDED(res) )
         res = conn.Send_Message ( pkg );
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Send_Module_Status>" );
      res = RS_EXCEPTION;
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ O p t i o n _ S t a t u s
//
// Send the game's option bytes to client
//
// =---------------------------------------------------------------------------
_result idpcd_group::Send_Option_Status ( connection_idpcd& conn_dest )
{
   _result res = RS_OK;

   try
   {
      // Initialize message structure
      //
      int len = sizeof(nm_xxs_game_option) + Num_Options;
      uint_08* p_os_raw = new uint_08[len];
      nm_xxs_game_option* p_os = (nm_xxs_game_option*)p_os_raw;
      memset ( p_os, 0, len );

      // Set message fields
      //
      p_os->Message_Type = ZCS_GAME_OPTION_STATUS;
      p_os->Num_Options = Num_Options;
      memcpy ( p_os->Option_Setting, Module_Options, Num_Options );

      // Package message structure
      //
      message_package pkg ( (uint_08*)p_os );
      NM_Write_Header ( &pkg.hdr, len );

      // Scramble bytes
      res = Scramble_Bytes ( pkg.p_data, len, pkg.hdr.Scramble_Key );

      if ( RSUCCEEDED(res) )
         res = conn_dest.Send_Message ( pkg );

      delete [] p_os_raw;
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Send_Option_Status>" );
      res = RS_EXCEPTION;
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ G a m e _ T y p e _ S t a t u s
//
// Send the game's option bytes to client
//
// =---------------------------------------------------------------------------
_result idpcd_group::Send_Game_Type_Status ( connection_idpcd& conn_dest )
{
   _result res = RS_OK;

   try
   {
      // Initialize message structure
      //
      nm_xxs_game_type gts;
      memset ( &gts, 0, sizeof(nm_xxs_game_type) );

      // Set message fields
      //
      gts.Message_Type = ZCS_GAME_TYPE_STATUS;
      gts.Game_Slot_ID = Game_Slot_ID; // 0 for new, 1+ for GL's saved game
      SS_Port_Strcpy_Len ( gts.Save_Game_Name, (const char*)Saved_Game_Name, SAVE_GAME_NAME_BYTES );

      // Package message structure
      //
      message_package pkg ( (uint_08*)&gts );
      NM_Write_Header ( &pkg.hdr, sizeof(nm_xxs_game_type) );

      // Scramble bytes
      res = Scramble_Bytes ( pkg.p_data, sizeof(nm_xxs_game_type), pkg.hdr.Scramble_Key );

      if ( RSUCCEEDED(res) )
         res = conn_dest.Send_Message ( pkg );
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Send_Game_Type_Status>" );
      res = RS_EXCEPTION;
   }

   return res;
}

// =---------------------------------------------------------------------------
// =---------------------------------------------------------------------------
//
//
//
// RECEIVE MESSAGES
//
//
//
// =---------------------------------------------------------------------------
// =---------------------------------------------------------------------------


// =---------------------------------------------------------------------------
// (public) H a n d l e _ L e a d e r _ M e s s a g e
//
// Handles messages that generally affect the group such as game options, etc
// these messages are generally reflected to all other members of the group
//
// 'connection_idpcd leader' is assumed to be the leader.
//
// SPECNOTES: The spec says that if a player arrives right before GL_ sends module
//            status change, that the module status won't be changed and GL gets 
//            notify.
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Leader_Message ( connection_idpcd& leader, message_package& pkg )
{
   // Ignore this message if it is not sent by the leader
   //
   if ( !Is_Game_Leader(leader) )
      return RS_OK;

   uint_08 Message_Type = pkg.p_data[0];
   _result res = Scramble_Bytes ( &Message_Type, 1, pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   switch ( Message_Type )
   {
      case LZS_GAME_MODULE_REQUEST:
      {
         res = Handle_Game_Module_Request ( pkg );
         break;
      }

      case LZS_GAME_TYPE_ACTION:
      {
         res = Handle_Game_Type_Action ( pkg );
         break;
      }

      case LZS_PLAYER_ASSIGN_ACTION:
      {
         res = Handle_Player_Assign_Action ( pkg );
         break;
      }

      case LZS_GAME_OPTION_ACTION:
      {
         res = Handle_Game_Option_Action ( pkg );
         break;
      }

      case LZS_START_GAME_ACTION:
      {
         res = Handle_Start_Game_Action ( pkg );
         break;
      }
      
      case LZG_GAME_STATUS_REPLY:
      {
         res = Handle_Game_Status_Reply ( pkg );
         break;
      }
   }

   return RS_OK;
}


// =---------------------------------------------------------------------------
// (protected) H a n d l e _ G a m e _ M o d u l e _ R e q u e s t
//
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Game_Module_Request ( message_package& pkg )
{
   nm_xxs_game_module* p_msg = (nm_xxs_game_module*)pkg.p_data;

   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   // Quick check that each connection supports the new module type
   //
   bool supports = true;

   for ( int i=0; supports && (i < Num_Slots); i++ )
   {
      connection_idpcd& Conn = Get_Group_Connection ( i );
      if ( Conn.Is_Dummy() )
         continue;

      supports = Conn.Does_Support_Module ( p_msg->Module_ID );
   }

   // Update our module status ("new game")
   //
   if ( supports )
   {
      Module_ID = p_msg->Module_ID;
   }

   // Now send out a module status to everyone
   //
   p_msg->Message_Type = ZCS_GAME_MODULE_STATUS;

   res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );

   if ( RSUCCEEDED(res) )
      res = Broadcast_To_Targets ( pkg, true /*Include_GL?*/ );

   return res;
}

// =---------------------------------------------------------------------------
// (protected) H a n d l e _ G a m e _ T y p e _ A c t i o n
//
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Game_Type_Action ( message_package& pkg )
{
   nm_xxs_game_type* p_msg = (nm_xxs_game_type*)pkg.p_data;

   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   // Ignore if GL requested to load a saved game
   //
   //if ( p_msg->Is_Saved_Game )
   //   return RS_OK;

   // Store Current State
   //
   Game_Slot_ID = p_msg->Game_Slot_ID;
   SS_Port_Strcpy_Len( (char*)Saved_Game_Name, (const char*)p_msg->Save_Game_Name, SAVE_GAME_NAME_BYTES );

   // Send out to remaining players
   //
   p_msg->Message_Type = ZCS_GAME_TYPE_STATUS;
   res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   
   if ( RSUCCEEDED(res) )
      res = Broadcast_To_Targets ( pkg, true /*Include_GL?*/ );

   return res;
}

// =---------------------------------------------------------------------------
// (protected) H a n d l e _ G a m e _ O p t i o n _ A c t i o n
//
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Game_Option_Action ( message_package& pkg )
{
   nm_xxs_game_option* p_msg = (nm_xxs_game_option*)pkg.p_data;

   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   try
   {
      memcpy ( Module_Options, &p_msg->Option_Setting, p_msg->Num_Options );
      Num_Options = p_msg->Num_Options;
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Handle_Game_Option_Action>" );
      res = RS_EXCEPTION;
   }

   // Switch message from option action to option status
   //
   p_msg->Message_Type = ZCS_GAME_OPTION_STATUS;

   if ( RSUCCEEDED(res) )
   {
      // Send out to remaining players
      //
      res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );

      if ( RSUCCEEDED(res) )
         res = Broadcast_To_Targets ( pkg, false /*Include_GL?*/ );
   }

   return res;
}

// =---------------------------------------------------------------------------
// (protected) H a n d l e _ P l a y e r _ A s s i g n _ A c t i o n
//
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Player_Assign_Action ( message_package& pkg )
{
   nm_xxs_player_assign* p_msg = (nm_xxs_player_assign*)pkg.p_data;

   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   // Ignore the request if p_msg->Player_ID is out of range [1..MAX_PLAYERS_PER_GAME]
   //
   if ( !p_msg->Player_ID || p_msg->Player_ID > MAX_PLAYERS_PER_GAME )
      return RS_OK;

   // Store the information locally
   //
   if ( p_msg->Group_Slot_ID == 0 )
   {
      // Clear info for the specified Player_ID when we get 0 Computer_Slot_ID
      //
      rgColor_ID [ p_msg->Player_ID - 1 ] = 0;
      rgComputerSlot [ p_msg->Player_ID - 1 ] = 0;
      memset ( rgName [ p_msg->Player_ID - 1 ], 0, PLAYER_NAME_BYTES );
   }
   else
   {
      rgColor_ID [ p_msg->Player_ID - 1 ] = p_msg->Color_ID;
      rgComputerSlot [ p_msg->Player_ID - 1 ] = p_msg->Group_Slot_ID;
      SS_Port_Strcpy_Len ( rgName [ p_msg->Player_ID - 1 ], p_msg->Name, PLAYER_NAME_BYTES );
   }

   // Rescramble and send out to remaining players
   //
   p_msg->Message_Type = ZCS_PLAYER_ASSIGN_STATUS;
   res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );

   if ( RSUCCEEDED(res) )
      res = Broadcast_To_Targets ( pkg, true );

   return res;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ S t a r t _ G a m e _ A c t i o  n
//
//
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Start_Game_Action ( message_package& pkg )
{
   nm_xxx_start_game* p_msg = (nm_xxx_start_game*)pkg.p_data;

   // Unscramble bytes
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   Status = GS_CLOSED;

   // Rescramble message and send out!
   //
   p_msg->Message_Type = ZCS_START_GAME_NOTIFY;
   res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );

   if ( RSUCCEEDED(res) )
      res = Broadcast_To_Targets ( pkg, true );

   // The next few steps are similar to disband, but we do not want to send quit group
   // notifies. Therefore, I coded this by hand, yuck.


   // Make sure all clients no longer reference us
   //
   for ( int i=0; i < Num_Slots; i++ )
   {
      connection_idpcd& Conn = Get_Group_Connection ( i );
      if ( Conn.Is_Dummy() )
         continue;

      if ( Is_Ranked() && &Get_Game_Leader() == &Conn )
      {
         // do nothing
      }
      else
      {
         uint_16 Group_Index = Group_Pool.Get_Group_Index ( *this );
         Conn.Clear_Group_Reference ( Group_Index );
      }
   }

   // Notify the lobby that this group is gone
   //
   nm_zcs_game_group_notify ggn;
   Initialize_Group_Notify ( &ggn );
   ggn.Number_Of_Members = 0;
   Group_Pool.Get_Lobby().Broadcast_Group_Notify ( &ggn );

   if ( !Is_Ranked() )
   {
      // NickC 8/8/00 : Added this so we make this group available for reallocation
      //
      Clear_Group ( false );
   }

   // NickC : Removed so we rely on client to disconnect
   //if ( RSUCCEEDED(res) )
   //   Remove_All_Connections ( );

   return res;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ G a m e _ S t a t u s _ R e  p l y
//
//
// =---------------------------------------------------------------------------
_result idpcd_group::Handle_Game_Status_Reply ( message_package& pkg )
{
   nm_lzg_game_status_reply* p_msg = (nm_lzg_game_status_reply*)pkg.p_data;

   // Unscramble bytes
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   stacktag GameStatus ( CHANNEL_ACTIVITYLOG, "action", "name=\"game_status_reply\"" );

   int turn = (int)Make_Hardware_Word(p_msg->Turn_Number);

   Sys.Message ( CHANNEL_ACTIVITYLOG, "<group>%s</group>", Get_Group_Name() );
   Sys.Message ( CHANNEL_ACTIVITYLOG, "<turn>%d</turn>",   turn );
   Sys.Message ( CHANNEL_ACTIVITYLOG, "<results>%s</results>", p_msg->Winning_Player_ID ? "FINAL" : "interim" );

   // 1/18/00
   // increment disconnect count, only to be decremented
   // when final results are received

   // 1/19/00 disable this for now
   if (turn == TURN_OF_NO_RETURN)
   {
       stacktag NoReturn ( CHANNEL_CONNECTIONLOG, "rank_committed");
        for ( int i=0; i < p_msg->Number_Of_Players; i++ )
        {
            uint_16 client_uid = Make_Hardware_Word ( p_msg->Player_Status[i].Client_UID  );
            client_record* p_clirec = Client_RecPtr_From_UID ( client_uid );
            if (p_clirec)
            {
               stacktag Player ( CHANNEL_CONNECTIONLOG, "player");
                uint_16 disconnects = (uint_16)ntohs(p_clirec->disconnects);
                disconnects++;
               Sys.Message ( CHANNEL_CONNECTIONLOG, "<name value=\"%s\"/><disconnect newvalue=\"%d\"/>", p_clirec->username, (int)disconnects);
                p_clirec->disconnects = htons(disconnects);
            }
        }
   }

   // Validate_Game_Status_Reply will report any errors
   //
   res = Validate_Game_Status_Reply ( p_msg );

   if ( RSUCCEEDED(res) )
   {
      // 1. Cum_Ranked_XOR_Key ^ this_xor
      // 2. verify Cum_Ranked_XOR_Key ^ this = game_xor

      memcpy ( &Ranked_Status, p_msg, sizeof(nm_lzg_game_status_reply) );

      // Only resolve ratings if game is won (nonzero p_msg->Winning_Player_ID)
      //
      if ( p_msg->Winning_Player_ID )
         Resolve_Endgame_Ratings ( p_msg );
   }
   else 
   {
       Sys.Message ( CHANNEL_ACTIVITYLOG, "<error>%s</error>", Result_String(res) );
   }


   return res;
}

