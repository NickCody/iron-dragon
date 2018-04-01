/* --------------------------------------------------------------------------
   n e t _ m e s s a g e s . h

   Message definitions for everything.  Note that these are the "payloads",
   and that every message is preceded by a 4 byte "header".  The header is:

      uint_08  Payload data length MSB
      uint_08  Payload data length LSB
      uint_08  Scramble key
      uint_08  Sender ID

   The "length" field refers just to the payload length and does not count
   the 4 byte header.

   The scramble key is used by the net_socket to either scramble or
   unscramble the payload data bytes.  A key of 0 means no scramble.  The
   header is never scrambled, just the payload data.

   The sender ID is the net computer ID of the sender.  0 is always the
   connzection zone, 1 is the game leader, 2... are the others.

   . . .

   Each message payload is required to begin with a uint_08 field that
   specifies the message type.

   The payloads are designed to be bytewise-ordered; either byte sized data,
   or character strings.  The purpose here is to allow net messages to be sent
   without any wacky reordering functions, and to work across pltaforms...

   (c) 1999 by Martin R. Szinger and Nicholas Codignotto

   --------------------------------------------------------------------------

   Date        Who   Description
   ----------  ---   --------------------------------------------------------
   1999-11-25  MRS   Created (Eden's PC version of Mayfair's Iron Dragon).
   -------------------------------------------------------------------------- */

#ifndef  INC_NET_MESSAGES_H
#define  INC_NET_MESSAGES_H

/* -------------------------------------------------------------------------- */
#include "standard_types.h"

/* -------------------------------------------------------------------------- */
enum nm_type
{
   CLIENT_TO_CZ_SETUP_BASE          =   0,   CZS_AUTHENTICATE_REQUEST   =   1,
                                             CZS_JOIN_GROUP_REQUEST     =   2,
                                             CZS_QUIT_GROUP_REQUEST     =   3,

   CZ_TO_CLIENT_SETUP_BASE          =  32,   ZCS_AUTHENTICATE_REPLY     =  33,
                                             ZCS_JOIN_GROUP_REPLY       =  34,
                                             ZCS_QUIT_GROUP_NOTIFY      =  35,
                                             ZCS_GROUP_MEMBER_STATUS    =  36,
                                             ZCS_GAME_MODULE_STATUS     =  37,
                                             ZCS_GAME_TYPE_STATUS       =  38,
                                             ZCS_PLAYER_ASSIGN_STATUS   =  39,
                                             ZCS_GAME_OPTION_STATUS     =  40,
                                             ZCS_START_GAME_NOTIFY      =  41,
                                             ZCS_GAME_GROUP_NOTIFY      =  42,

   GAME_LEADER_TO_CZ_SETUP_BASE     =  64,   LZS_GAME_MODULE_REQUEST    =  65,
                                             LZS_GAME_TYPE_ACTION       =  66,
                                             LZS_PLAYER_ASSIGN_ACTION   =  67,
                                             LZS_GAME_OPTION_ACTION     =  68,
                                             LZS_START_GAME_ACTION      =  69,

   CLIENT_ANY_CHAT_BASE             =  96,   CCH_TEXT                   =  97,
                                             CCH_AUDIO                  =  98,
                                             ANY_SMACKDOWN              =  99,
                                             ANY_PING                   = 100,

   PLAYER_IN_GAME_BASE              = 128,   PGA_START_GAME_NOTIFY      = 129,
                                             PGA_INITIAL_FOREMAN        = 130,
                                             PGA_GAME_ACTION            = 131,
                                             PGA_GAME_UNDO              = 132,
                                             PGA_QUIT_GAME              = 133,

   GAME_LEADER_IN_GAME_BASE         = 160,   LGA_PLAYER_REASSIGN        = 161,
                                             LGA_SETUP_GAME_DATA        = 162,
                                             LGA_START_GAME_ACTION      = 163,

   CZ_TO_GAME_LEADER_IN_GAME_BASE   = 192,   ZLG_GAME_STATUS_REQUEST    = 193,

   GAME_LEADER_TO_CZ_IN_GAME_BASE   = 224,   LZG_GAME_STATUS_REPLY      = 225
};

/* -------------------------------------------------------------------------- */
enum nm_lengths
{
   AUTHENTICATE_SPARE_BYTES   =  9,
   CLIENT_VERSION_BYTES       = 32,
   USER_HANDLE_BYTES          = 24,
   OPTION_DATA_BYTES          = 64,
   PLAYER_NAME_BYTES          = 24,
   GROUP_NAME_BYTES           = 24,
   GROUP_PASSWORD_BYTES       = 24,
   PASSWORD_BUFFER_BYTES      = 24,
   PASSWORD_BYTES             = 12,
   SAVE_GAME_NAME_BYTES       = 40,
   ACTION_DATA_BYTES          = 10,
};

/* -------------------------------------------------------------------------- */
enum nm_jg_reply_codes
{
   FGSTATUS_OK                = 0,  // no error
   FGSTATUS_BADPASSWORD       = 1,  // specified group mismatched on password
   FGSTATUS_GROUPFULL         = 2,  // specified group is full
   FGSTATUS_BADMODULE         = 3,  // group matched, module not supported
   FGSTATUS_NEEDLEADER        = 4,  // no reverse connect, new group requested
   FGSTATUS_BADGROUPNAME      = 5,  // group name invalid, zero length
   FGSTATUS_MAXGROUPSREACHED  = 6,  // new group requested, but no more slots
};

/* -------------------------------------------------------------------------- */
typedef struct nm_czs_authenticate_request
{
   uint_08  Message_Type;     //   1: CZS_AUTHENTICATE_REQUEST
   uint_08  Spare_Data           [AUTHENTICATE_SPARE_BYTES];
   uint_08  Server_Compatilibity [4];
   char     Client_Version       [CLIENT_VERSION_BYTES];
   uint_08  Inbound_Port         [2];  // msb, lsb
   char     User_Handle          [USER_HANDLE_BYTES];
   char     Default_Player_Name  [PLAYER_NAME_BYTES];
   char     Password             [PASSWORD_BUFFER_BYTES]; // encrypted, max len = 12
}nm_czs_authenticate_request;

/* -------------------------------------------------------------------------- */
typedef struct nm_zcs_authenticate_reply
{
   uint_08  Message_Type;     //  33: ZCS_AUTHENTICATE_REPLY
   uint_08  Client_UID           [2];  // msb, lsb client unqiue ID
   uint_08  Made_Inbound_Connect;
   char     CZ_Message           [1];  // over-lengthed
}nm_zcs_authenticate_reply;

/* -------------------------------------------------------------------------- */
typedef struct nm_zcs_game_group_notify
{
   uint_08  Message_Type;     //  42: ZCS_GAME_GROUP_NOTIFY
   uint_08  Group_UID            [2];  // msb, lsb group unique ID
   uint_08  Number_Of_Members;// 0 means that group no longer exists
   uint_08  Game_Module;      //    0 dc, 1 ID, 2+ TBD
   uint_08  Is_Ranked;        //    0: no, 1: yes
   char     Group_Name           [GROUP_NAME_BYTES];
}nm_zcs_game_group_notify;

/* -------------------------------------------------------------------------- */
typedef struct nm_czs_join_group_request
{
   uint_08  Message_Type;     //   2: CZS_JOIN_GROUP_REQUEST
   uint_08  Game_Module;      //    0 dc, 1 ID, 2+ TBD
   uint_08  Is_Ranked;        //    0: no, 1: yes
   uint_08  pad;
   char     Group_Name           [GROUP_NAME_BYTES];
   char     Group_Password       [GROUP_PASSWORD_BYTES];
}nm_czs_join_group_request;

/* -------------------------------------------------------------------------- */
typedef struct nm_zcs_join_group_reply
{
   uint_08  Message_Type;     //  34: ZCS_JOIN_GROUP_REPLY
   uint_08  Group_UID            [2];
   uint_08  Reply_Code;       //    0 for success, else error code (below)
   uint_08  Assigned_Slot_ID; //    0 on fail, 1 if GL, 2+ on other success
   uint_08  GL_IP_Address        [4];
   uint_08  GL_IP_Port           [2];
   char     Group_Name           [GROUP_NAME_BYTES];
   char     Group_Password       [GROUP_PASSWORD_BYTES];
   uint_08  Is_Ranked;        //    0: no, 1: yes
}nm_zcs_join_group_reply;
   
/* -------------------------------------------------------------------------- */
typedef struct nm_czs_quit_group_request
{
   uint_08  Message_Type;     //   3: CZS_QUIT_GROUP_REQUEST
   uint_08  Current_Group_UID    [2];
}nm_czs_quit_group_request;

/* -------------------------------------------------------------------------- */
typedef struct nm_zcs_quit_group_notify
{
   uint_08  Message_Type;     //  35: ZCS_QUIT_GROUP_NOTIFY
   uint_08  Group_UID            [2];
}nm_zcs_quit_group_notify;
 
/* -------------------------------------------------------------------------- */
typedef struct nm_zcs_group_member_status
{
   uint_08  Message_Type;     //  36: ZCS_GROUP_MEMBER_STATUS
   uint_08  Group_UID            [2];
   uint_08  Client_UID           [2];
   uint_08  Slot_ID;          //    1..9
   uint_08  Is_Slot_Filled;   //    0 for empty, 1 for assigned
   uint_08  Supported_Modules    [2];
   char     User_Handle          [USER_HANDLE_BYTES];
   char     Default_Player_Name  [PLAYER_NAME_BYTES];
   uint_08  Current_Player_Rank  [2];
}nm_zcs_group_member_status;

/* -------------------------------------------------------------------------- */
typedef struct nm_xxs_game_module
{
   uint_08  Message_Type;     //  65: LZS_GAME_MODULE_REQUEST or
                              //  37: ZCS_GAME_MODULE_STATUS
   uint_08  Module_ID;        //    1 for ID, 2+ TBD
}nm_xxs_game_module;

/* -------------------------------------------------------------------------- */
typedef struct nm_xxs_game_type
{
   uint_08  Message_Type;     //  66: LZS_GAME_TYPE_ACTION or
                              //  38: ZCS_GAME_TYPE_STATUS
   uint_08  Game_Slot_ID;     // 0 for new, 1+ for GL's saved game
   char     Save_Game_Name       [SAVE_GAME_NAME_BYTES];   
}nm_xxs_game_type;

/* -------------------------------------------------------------------------- */
typedef struct nm_xxs_player_assign
{
   uint_08  Message_Type;     //  67: LZS_PLAYER_ASSIGN_ACTION or
                              //  39: ZCS_PLAYER_ASSIGN_STATUS or
                              // 161: LGA_PLAYER_REASSIGN
   uint_08  Player_ID;        //    1 .. MAX_PLAYERS
   uint_08  Group_Slot_ID;    //    0 for empty, else 1..MAX_MEMBERS, 128 for AI
   uint_08  Color_ID;
   char     Name                 [PLAYER_NAME_BYTES];
}nm_xxs_player_assign;

/* -------------------------------------------------------------------------- */
typedef struct nm_xxs_game_option
{
   uint_08  Message_Type;     //  68: LZS_GAME_OPTION_ACTION or
                              //  40: ZCS_GAME_OPTION_STATUS
   uint_08  Num_Options;      //    0..64
   uint_08  Option_Setting       [1];  //    ... and so on
}nm_xxs_game_option;

/* -------------------------------------------------------------------------- */
typedef struct nm_lga_setup_game_data
{
   uint_08  Message_Type;     // 162: LGA_SETUP_GAME_DATA
   uint_08  First_Flag;       //    1 for new stream, 0 for continuation
   uint_08  Data[1];          //    over-lengthed byte stream
}nm_lga_setup_game_data;

/* -------------------------------------------------------------------------- */
typedef struct nm_xxx_start_game
{
   uint_08  Message_Type;     //  41: ZCS_START_GAME_NOTIFY or
                              //  69: LZS_START_GAME_ACTION or
                              // 129: PGA_START_GAME_NOTIFY or
                              // 163: LGA_START_GAME_NOTIFY
   uint_08  Extra[7];
}nm_xxx_start_game;

/* -------------------------------------------------------------------------- */
typedef struct nm_cch_text
{
   uint_08  Message_Type;     //  97: CCH_TEXT
   uint_08  Group_UID            [2];
   uint_08  Client_UID           [2];
   char     Text_String[1];   //    over-lengthed: null terminated ascii
}nm_cch_text;

/* -------------------------------------------------------------------------- */
typedef struct nm_cch_audio
{
   uint_08  Message_Type;     //  98: CCH_AUDIO
   uint_08  Client_UID           [2];
   uint_08  Is_Block_End;     //    0 or 1
   uint_08  Block_Bytes          [2];
   uint_08  Block_Data[1];    //    over-lengthed
}nm_cch_audio;

/* -------------------------------------------------------------------------- */
typedef struct nm_any_smackdown
{
   uint_08  Message_Type;     //  99: ANY_SMACKDOWN
   uint_08  Group_UID            [2];
   uint_08  Client_UID           [2];
   char     Text_String[1];   //    over-lengthed: optional null-term ascii
}nm_any_smackdown;

/* -------------------------------------------------------------------------- */
typedef struct nm_any_ping
{
   uint_08  Message_Type;     // 100: ANY_PING
   uint_08  Group_UID            [2];
   uint_08  Client_UID           [2];
   uint_08  Bounce_Count;
}nm_any_ping;

/* -------------------------------------------------------------------------- */
typedef struct nm_pga_initial_foreman
{
   uint_08  Message_Type;     // 130: PGA_INITIAL_FOREMAN
   uint_08  Player_ID;
   uint_08  Choice;           // 0 to discard all, 1.. by fm number otherwise
}nm_pga_initial_foreman;

/* -------------------------------------------------------------------------- */
typedef struct nm_pga_game_action
{
   uint_08  Message_Type;     // 131: PGA_GAME_ACTION
   uint_08  Player_ID;
   uint_08  Action_Data          [ACTION_DATA_BYTES];
}nm_pga_game_action;

/* -------------------------------------------------------------------------- */
typedef struct nm_pga_game_undo
{
   uint_08  Message_Type;     // 132: PGA_GAME_UNDO
   uint_08  Player_ID;
}nm_pga_game_undo;

/* -------------------------------------------------------------------------- */
typedef struct nm_pga_quit_game
{
   uint_08  Message_Type;     // 133: PGA_QUIT_GAME
   uint_08  Net_Computer_ID;
}nm_pga_quit_game;

/* -------------------------------------------------------------------------- */
typedef struct nm_zlg_game_status_request
{
   uint_08  Message_Type;     // 193: ZLG_GAME_STATUS_REQUEST
}nm_zlg_game_status_request;

/* -------------------------------------------------------------------------- */
typedef struct player_status
{
   char     User_Handle          [USER_HANDLE_BYTES];
   uint_08  Client_UID           [2];  // 0, 0 for AI
   uint_08  GP                   [2];  // msb, lsb
   uint_08  GP_Earned            [2];  // msb, lsb
   uint_08  MC_Connected;
   uint_08  Current_Rank;
}player_status;

/* -------------------------------------------------------------------------- */
typedef struct nm_lzg_game_status_reply
{
   uint_08  Message_Type;     // 225: LZG_GAME_STATUS_REPLY
   uint_08  Group_UID            [2];
   uint_08  GL_Client_UID        [2];
   char     GL_Password          [PASSWORD_BUFFER_BYTES];
   uint_08  Number_Of_Players;   // 1..6
   uint_08  Winning_Player_ID;   // 0 if game is still going on
   uint_08  Turn_Number          [2];  // msb, lsb
   player_status  Player_Status  [6];
   uint_08  This_XOR_Key         [4];
   uint_08  Game_XOR_Key         [4];
}nm_lzg_game_status_reply;

/* -------------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------------
   end of file   ( n e t _ m e s s a g e s . h )
   -------------------------------------------------------------------------- */
