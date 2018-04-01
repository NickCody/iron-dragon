// =---------------------------------------------------------------------------
// i d p c d _ g r o u p . h
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
//   99 Jun 15   nic   Created.
//   99 Oct 13   nic   Forcibly removed the oracle from office.

#ifndef __IDPCDGROUP_H__
#define __IDPCDGROUP_H__

#include "connection_idpcd.h"
#include "idpcd_defaults.h"

// Group Status Enum
enum GROUPSTATUS
{
   GS_PRIMAL   = 1,      // Completely empty, ready for initialization
   GS_READY    = 2,      // Ready to receive a group leader, not available for
                         //  consideration of subsequent leader wanna-bes
   GS_OPEN     = 3,      // Group is open, ready for (more) players
   GS_CLOSED   = 4       // Group is closed, game has started
};

struct ranked_client
{
   uint_16        uid;        // machine byte-order
   client_record* p_clirec;
};

// =---------------------------------------------------------------------------
// (global) g r o u p
//
// =---------------------------------------------------------------------------

class idpcd_group //: public lockable_object
{
   friend class group_pool;

public:
   idpcd_group ( bool Lobby, bool Dummy = false );
   ~ idpcd_group( );

   // =------------------------------------------
   // (public) Initialize_Group
   //
   // Performs a pretty brainless initalization of
   // the group object. No intelligent freeing
   // of possible existing data or such logic
   // =------------------------------------------
   void Clear_Group ( bool Lobby, bool Dummy = false );

   // =------------------------------------------
   // (public) Initialize_Group
   // Performs a true pre-connection signup initialization
   // of the grou object
   // =------------------------------------------
   void Initialize_Group ( const char * const Name, 
                           const char * const Password, 
                           uint_08 game_module, 
                           bool ranked );

   // =------------------------------------------
   // (public) A d d _ C o n n e c t i o n
   //
   // Adds the specified connection to the first empty slot
   // =------------------------------------------
   _result Add_Connection ( uint_16 PoolIdx, uint_08& Slot_ID );

   // =------------------------------------------
   // (public) G e t _ G r o u p _ C o n n e c t i o n
   //
   // Returns the connection associated with the specified group position
   //
   // =------------------------------------------
   connection_idpcd& Get_Group_Connection ( uint_16 GroupIdx );

   // =------------------------------------------
   // (public) R e m o v e _ C o n n e c t i o n
   //
   // Removes the group's reference to specified conn
   // (group may delete itself if leader/last connection
   // =------------------------------------------
   _result Remove_Connection ( uint_16 PoolIdx, bool Quit_Requested );

   // =------------------------------------------
   // B r o a d c a s t _ T o _ T a r g e t s
   // 
   // =------------------------------------------
   _result Broadcast_To_Targets ( message_package& pkg, bool Include_GL = true );

   // =------------------------------------------
   // (public) H a n d l e _ Leader _ M e s s a g e
   //
   // Handles messages that generally affect the group such as game options, etc
   // these messages are generally reflected to all other members of the group
   // =------------------------------------------
   _result Handle_Leader_Message ( connection_idpcd& sender, message_package& pkg );

   _result Handle_Game_Module_Request ( message_package& pkg );
   _result Handle_Game_Type_Action ( message_package& pkg );
   _result Handle_Game_Option_Action ( message_package& pkg );
   _result Handle_Player_Assign_Action ( message_package& pkg );
   _result Handle_Start_Game_Action ( message_package& pkg );
   _result Handle_Game_Status_Reply ( message_package& pkg );

   uint_08 Initialize_Ratings            ( int*           rg_ratings, 
                                           player_status* rg_statuses, 
                                           uint_08        Focus_Player, 
                                           uint_08        Num_Players );

   void    Resolve_Endgame_Ratings       ( nm_lzg_game_status_reply* p_msg, bool Write_Back = true );
   _result Validate_Game_Status_Reply    ( nm_lzg_game_status_reply* p_msg );
   client_record* Client_RecPtr_From_UID ( uint_16 uid );

   // =------------------------------------------
   // (public) S e n d _ G r o u p _ M e m b e r _ S t a t u s
   //
   // =------------------------------------------
   _result Broadcast_Group_Member_Status ( uint_08 Slot,
                                           uint_08 Filled,
                                           connection_idpcd& conn_about );
   
   _result Send_All_Summaries ( connection_idpcd& conn_dest, uint_08 Slot );

   _result Send_Player_Assign_Status_Summary ( connection_idpcd& conn_dest );
   _result Send_Group_Member_Status_Summary ( connection_idpcd& conn_dest );
   _result Send_Option_Status ( connection_idpcd& conn_dest );
   _result Send_Game_Type_Status ( connection_idpcd& conn_dest );
   _result Send_Module_Status ( connection_idpcd& conn );
   
   _result Send_Group_Notify ( connection_idpcd& conn, idpcd_group* p_grp = 0 );
   _result Broadcast_Group_Notify ( nm_zcs_game_group_notify* p_ggn );
   
   _result Initialize_Group_Notify ( nm_zcs_game_group_notify* p_ggn );


   bool Is_Dummy ( )
   {
      return Dummy;
   }

   // =------------------------------------------
   // (public) R e p o r t _ G r o u p
   // =------------------------------------------
   void Report_Group ( void );


   bool                  Is_Game_Leader   ( connection_idpcd& conn_leader );
   bool                  Is_Full          ( void );
   bool                  Is_Joinable      ( void );
   bool                  Is_Lobby         ( void ) { return Group_UID == 1; }
   bool                  Is_Ranked        ( void ) { return Ranked; }
   GROUPSTATUS           Get_Status       ( void ) { return Status; }

   connection_idpcd&     Get_Game_Leader  ( void ) { return Get_Group_Connection(0); }

   const char * const    Get_Group_Name ( ) { return Group_Name; }
   const char * const    Get_Group_Password ( ) { return Group_Password; }
   uint_32               Get_Ref_Count    ( void ) const { return Ref_Count; }

   //uint_08               Get_Game_Type    ( void ) const { return Game_Type; }
   uint_16               Get_Group_UID    ( void ) { return Group_UID; }

   uint_08               Get_Module_ID    ( void ) { return Module_ID; }
   bool                  Get_Module_Option( uint_08 option_id );

   void                  Set_Module_ID    ( uint_08 mi ) { Module_ID = mi; }
   void                  Zero_Module_Options ( void );

   bool                  Is_Assigned_Connection ( uint_16 GroupConnIdx );
   uint_16               Group_To_Busy_Pool_Index ( uint_16 GroupConnIdx );

protected:
   // =------------------------------------------
   // D i s b a n d
   // Kicks all connections out of the group, resets group to primal
   //
   _result Disband ( );
   _result ObliterateGroup ( ); // disconnect all clients immediately

   // Object State
   //
   uint_16           Group_UID; // 1 is lobby
   //uint_08           Game_Type;
   char              Group_Name[GROUP_NAME_BYTES];
   char              Group_Password[GROUP_PASSWORD_BYTES];
   uint_32           Ref_Count;
   uint_08           Num_Slots;
   GROUPSTATUS       Status;
   bool              Ranked;
   bool              Dummy;

   //connection_idpcd** rgConnections;
   uint_16*           rgConnection_Indices;

   ranked_client*     rg_Ranked_Clients;

   // Player Assignments
   uint_08           rgComputerSlot   [MAX_PLAYERS_PER_GAME]; // points to index into rgConnections
   uint_08           rgColor_ID       [MAX_PLAYERS_PER_GAME];
   char              rgName           [MAX_PLAYERS_PER_GAME][PLAYER_NAME_BYTES];

   // LZS_GAME_MODULE_REQUEST
   //
   uint_08           Module_ID;
   uint_08           Module_Options[OPTION_DATA_BYTES];
   uint_08           Num_Options;

   // LZS_GAME_TYPE_ACTION
   //
   //uint_08           Saved_Game;
   uint_08           Game_Slot_ID;     // 0 for new, 1+ for GL's saved game
   uint_08           Saved_Game_Name[SAVE_GAME_NAME_BYTES];

   // LZA_GAME_STATUS_REPLY
   //
   nm_lzg_game_status_reply Ranked_Status;
   uint_08           Cum_Ranked_XOR_Key;
};

#endif // __IDPCDGROUP_H__

