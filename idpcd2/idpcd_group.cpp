// =---------------------------------------------------------------------------
// g r o u p . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//  Implements the group class for maintaining groups of client
//  connections
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE         WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jul 01   nic   Created.
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
#include "rank_calc.h"
#include "stack_tag.h"

// =---------------------------------------------------------------------------
// (public, ctor) idpcd_group
//
// =---------------------------------------------------------------------------
idpcd_group::idpcd_group ( bool Lobby, bool Dummy )
{
    rgConnection_Indices = 0;
    rg_Ranked_Clients = 0;
    Clear_Group ( Lobby, Dummy );
}

// =---------------------------------------------------------------------------
// (public, dtor) ~ group
//
// =---------------------------------------------------------------------------
idpcd_group::~ idpcd_group( )
{
    if ( rgConnection_Indices )
    {
        delete [] rgConnection_Indices;
        rgConnection_Indices = 0;
    }

    if ( rg_Ranked_Clients )
    {
        delete [] rg_Ranked_Clients;
        rg_Ranked_Clients = 0;
    }
}

// =---------------------------------------------------------------------------
// (public) C l e a r _ G r o u p
// 
// Clears the group and all connections. Call Disband() if you want to cleanly
// boot players. GS_PRIMAL is state afterwards. Ready to receive players...
//
// =---------------------------------------------------------------------------
void idpcd_group::Clear_Group ( bool Lobby, bool Dummy )
{
    int i = 0;

    if ( rgConnection_Indices )
    {
        delete [] rgConnection_Indices;
        rgConnection_Indices = 0;
    }

    if ( rg_Ranked_Clients )
    {
        delete [] rg_Ranked_Clients;
        rg_Ranked_Clients = 0;
    }

    // Initialize Player Assignments (only valid in real non-lobby groups
    //
    for ( i=0; i < MAX_PLAYERS_PER_GAME; i++ )
    {
        rgColor_ID[i] = 0;
        rgComputerSlot[i] = 0;
        rgName[i][0] = '\0';

    }

    // Lobby starts off open (no game leader needed to initiate this)
    //
    Status      = Lobby ? GS_OPEN : GS_PRIMAL;  
    Num_Slots   = 0;
    Ref_Count   = 0;
    Group_UID   = Lobby ? 1 : 0;
    Ranked      = false;
    this->Dummy = Dummy;
    
    memset ( Group_Name,     0, GROUP_NAME_BYTES );
    memset ( Group_Password, 0, GROUP_PASSWORD_BYTES );

    // LZS_GAME_MODULE_REQUEST
    //
    Module_ID = 0xFF;
    Zero_Module_Options();
    
    // LZS_GAME_TYPE_ACTION options
    //
    Game_Slot_ID = 0;
    memset ( Saved_Game_Name, 0, SAVE_GAME_NAME_BYTES );

    // LZS_GAME_STATUS_REPLY
    //
    memset ( &Ranked_Status, 0, sizeof(nm_lzg_game_status_reply) );
    Cum_Ranked_XOR_Key = 0;

    if ( Is_Lobby() )
        Initialize_Group ( "Lobby", "", 1 /*iron dragon*/, 0 );
}

// =---------------------------------------------------------------------------
// (public) I n i t i a l i z e _ G r o u p
//
// Initializes the group for use by real players, GS_READY
// =---------------------------------------------------------------------------
void idpcd_group::Initialize_Group ( const char * const Name, 
                                     const char * const Password, 
                                     uint_08 game_module, 
                                     bool ranked )
{
    int i = 0;

    if ( Is_Lobby() )   Num_Slots = MAX_PLAYERS_IN_LOBBY;
    else if ( ranked )  Num_Slots = MAX_PLAYERS_PER_GAME;
    else                Num_Slots = MAX_PLAYERS_PER_GROUP;

    // Initialize Connection Pointers
    //
    rgConnection_Indices = new uint_16[Num_Slots];
    rg_Ranked_Clients = new ranked_client[Num_Slots];

    for ( i=0; i < Num_Slots; i++ )
    {
        rgConnection_Indices[i] = 0xFFFF;

        // These are 1:1 to connections. In ranked games, they will be valid even 
        // after the connection is gone. Though Game leader connection better be valid!
        //
        rg_Ranked_Clients[i].p_clirec = 0;
        rg_Ranked_Clients[i].uid = 0;
    }

    // Initialize Player Assignments 
    //
    // Note we loop through to MAX_PLAYERS_PER_GAME and not Num_Slots because
    // in-game we will never have MAX_PLAYERS_PER_GROUP or MAX_PLAYERS_IN_LOBBY
    //
    for ( i=0; i < MAX_PLAYERS_PER_GAME; i++ )
    {
        rgColor_ID[i] = 0;
        rgComputerSlot[i] = 0;
        rgName[i][0] = '\0';
    }

    Ref_Count = 0;
    Ranked = ranked;

    // Only generate a new group id if we're not the lobby
    //
    if ( !Is_Lobby() )
        Group_UID = Generate_Unique_ID ( UID_GROUP );

    strcpy ( Group_Name, Name );
    strcpy ( Group_Password, Password );

    // LZS_GAME_MODULE_REQUEST
    //
    Module_ID = game_module;
    Zero_Module_Options();
    
    Status = GS_READY;

    // LZS_GAME_TYPE_ACTION options
    //
    Game_Slot_ID = 0;
    memset ( Saved_Game_Name, 0, SAVE_GAME_NAME_BYTES );
}


// =---------------------------------------------------------------------------
// (public, dtor) A d d _ C o n n e c t i o n
//
// The connection object is responsible for setting its own p_Group pointer
//
// If Successful, Idx will contain the slot id for the new connection within the
// group
// =---------------------------------------------------------------------------
_result idpcd_group::Add_Connection ( uint_16 PoolIdx, uint_08& Slot_ID )
{
    int i;
    connection_idpcd& NewConn = Busy_Pool.Get_Connection(PoolIdx);

    // If player found, simply ignore and send no other notification
    //
    for ( i=0; i < Num_Slots; i++ )
    {
        if ( rgConnection_Indices[i] == PoolIdx )
            return RS_OK;
    }

    // Find first free slot and add the connection
    //
    Slot_ID = 0;
    
    for ( i=0; i < Num_Slots; i++ )
    {
        if ( rgConnection_Indices[i] != 0xFFFF )
            continue;

        rgConnection_Indices[i] = PoolIdx;
        Slot_ID = i + 1;                     // Slot ID is 1-based
        Ref_Count++;

        // If we're filling in slot 0, group leader, we now consider the group
        // to be 'open' to further joins (lobby is already open!)
        //
        if ( !Is_Lobby() && i == 0 )
        {
            Status = GS_OPEN;
        }

        rg_Ranked_Clients[i].p_clirec = NewConn.Get_Client_Rec_Ptr();
        rg_Ranked_Clients[i].uid        = NewConn.Get_UID();

        break;
    }
    
    if ( !Slot_ID )
        return RS_ERR;

    _result res = RS_OK;

    if ( Is_Lobby() )
    {
        res = Group_Pool.Send_Game_Groups_Summary ( NewConn );
        res = Broadcast_Group_Member_Status ( Slot_ID, 1, NewConn );
    }
    else
    {
        // Notify connections in lobby of the new status for this group
        //
        nm_zcs_game_group_notify ggn;
        _result res = Initialize_Group_Notify ( &ggn );
        RRETURNONFAILURE(res);

        res = Group_Pool.Get_Lobby().Broadcast_Group_Notify ( &ggn );
    }

    return res;
}

// =---------------------------------------------------------------------------
// (public) G e t _ G r o u p _ C o n n e c t i o n
//
connection_idpcd& idpcd_group::Get_Group_Connection ( uint_16 GroupIdx )
{
    uint_16 PoolIdx = Group_To_Busy_Pool_Index (GroupIdx);
    return Busy_Pool.Get_Connection ( PoolIdx );
}

// =---------------------------------------------------------------------------
// (public) R e m o v e _ C o n n e c t i o n
//
// We handle other-group-member notifications and self deletion when Ref_Count=0
//
// Caller is responsible for...
//
// * if GL, Disband the group
// * clear connection's p_Group pointer
//
// =---------------------------------------------------------------------------
_result idpcd_group::Remove_Connection ( uint_16 PoolIdx, bool Quit_Requested )
{
    uint_08 Found_Slot = 0;
    
    connection_idpcd& Conn = Busy_Pool.Get_Connection ( PoolIdx );

    // If the conn is band leader, disband (which will kill this group)
    //
    if ( Is_Game_Leader(Conn) )
        return Disband();

    // Find the player and clear the connection pointer
    //
    for ( int i=0; i < Num_Slots; i++ )
    {
        if ( rgConnection_Indices[i] == PoolIdx )
        {
            Ref_Count--;
            rgConnection_Indices[i] = 0xFFFF;
            Found_Slot = i+1;

            // This code is here to demonstrate what we do *not* want to do
            // in ranking, the client disconnects but we need to maintain
            // this. If Remove_Connection is called durinng game setup, having
            // this here is harmless because a new player will simply overwrite it.

            // Code that should remain commented out...
            //
            //rg_Ranked_Clients[i].p_clirec = 0;
            //rg_Ranked_Clients[i].uid = 0;
        }
    }

    if ( Found_Slot )
    {
        // Only notify of leaving if not a started ranked game
        //
        if ( !Is_Ranked() || Get_Status() != GS_CLOSED )
        {
            // Notify the rest of the players that this connection has left
            //
            Broadcast_Group_Member_Status ( Found_Slot, 0 /*not_filled*/, Conn );

            if ( !Is_Lobby() )
            {
                nm_zcs_game_group_notify ggn;
            
                _result res = Initialize_Group_Notify ( &ggn );
                RRETURNONFAILURE(res);

                Group_Pool.Get_Lobby().Broadcast_Group_Notify ( &ggn );
            }
        }

        return RS_OK;
    }

    // Could not find the connection in the group
    //
    return RS_ERR;
}

// =---------------------------------------------------------------------------
// (public) O b l i t e r a t e G r o u p
//
// =---------------------------------------------------------------------------
_result idpcd_group::ObliterateGroup()
{
    for ( int i=0; i < Num_Slots && Ref_Count; i++ )
    {
        connection_idpcd& Conn = Get_Group_Connection ( i );
        Conn.Clear_Connection();
    }

    Clear_Group ( Is_Lobby() );

    return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) D i s b a n d
//
// =---------------------------------------------------------------------------
_result idpcd_group::Disband ( )
{
    _result res = RS_OK;

    // Create quit group notify message
    //
    nm_zcs_quit_group_notify gd;
    gd.Message_Type = ZCS_QUIT_GROUP_NOTIFY;
    gd.Group_UID[0]  = MSB ( Group_UID );
    gd.Group_UID[1]  = LSB ( Group_UID );

    message_package pkg ( (uint_08*)&gd );
    NM_Write_Header ( &pkg.hdr, sizeof(nm_zcs_quit_group_notify) );

    // Scramble the eggs
    //
    res = Scramble_Bytes ( pkg.p_data, sizeof(nm_zcs_quit_group_notify), IDPCD_SCRAMBLE_KEY );

    if ( RSUCCEEDED(res) )
    {
        for ( int i=0; i < Num_Slots && Ref_Count; i++ )
        {
            connection_idpcd& Conn = Get_Group_Connection ( i );

            if ( Conn.Is_Dummy() )
                continue;

            // NickC 3/14/00 : Don't send quit message once we're closed (game start has been sent)
            //
            // Added this back since it is probably a good thing to notify connections
            //if ( Status != GS_CLOSED )
                res = Conn.Send_Message ( pkg );

            uint_16 This_Index = Group_Pool.Get_Group_Index ( *this );
            Conn.Clear_Group_Reference ( This_Index );


            rgConnection_Indices[i] = 0xFFFF;
            
            // In Disband() case, its safe to nix this stuff
            rg_Ranked_Clients[i].p_clirec = 0;
            rg_Ranked_Clients[i].uid = 0;

            Ref_Count--;
        }
    }

    // Since Ref Count is zero now, this notification will let everyone know
    // that the group no longer exists
    //
    nm_zcs_game_group_notify ggn;
    
    res = Initialize_Group_Notify ( &ggn );
    RRETURNONFAILURE(res);

    res = Group_Pool.Get_Lobby().Broadcast_Group_Notify ( &ggn );

    Clear_Group ( Is_Lobby() );


    return res;
}

// =---------------------------------------------------------------------------
// (public) B r o a d c a s t _ T o _ T a r g e t s
//
// Assumes message is already scrambled.
//
// =---------------------------------------------------------------------------
_result idpcd_group::Broadcast_To_Targets ( message_package& pkg, bool Include_GL )
{
    // Reset sender ID to the CZ
    //
    pkg.hdr.Sender_ID = 0;

    int Start_Idx = Include_GL ? 0 : 1;

    // Send out the message to each client
    //
    for ( int i=Start_Idx; i < Num_Slots; i++ )
    {
        connection_idpcd& Conn = Get_Group_Connection ( i );
        if ( Conn.Is_Dummy() ) 
            continue;

        if ( Conn.Get_Lock_Status() == CS_BUSY )
            Conn.Send_Message ( pkg );
    }

    return RS_OK;
}


// =---------------------------------------------------------------------------
// (public) I s _ G a m e _ L e a d e r
//
// By definition, rgConnections[0] is the game leader. If the address of the 
//   passed-in connection matches rgConnections[0], we return true
//   otherwise, you guessed it, false
// =---------------------------------------------------------------------------
bool idpcd_group::Is_Game_Leader ( connection_idpcd& conn_leader )
{
    if ( Is_Lobby() )
        return false;

    connection_idpcd& Conn = Get_Group_Connection ( 0 );
    return &conn_leader == &Conn;
}

// =---------------------------------------------------------------------------
// (public) I s _ F u l l
//
// By definition, rgConnections[0] is the game leader. If the address of the 
//   passed-in connection matches rgConnections[0], we return true
//
// =---------------------------------------------------------------------------
bool idpcd_group::Is_Full ( void )
{ 
    return Ref_Count >= Num_Slots; 
}

// =---------------------------------------------------------------------------
// (public) I s _ J o i n a b l e
//
bool idpcd_group::Is_Joinable ( void )
{
    return !Is_Full() && Get_Status() == GS_OPEN;
}

// =---------------------------------------------------------------------------
// (public) G e t _ M o d u l e _ O p t i o n
//
// =---------------------------------------------------------------------------
bool idpcd_group::Get_Module_Option( uint_08 option_id )
{
    if ( option_id < OPTION_DATA_BYTES )
        return Module_Options[option_id] != 0;

    return false;
}

// =---------------------------------------------------------------------------
// (public) Z e r o _ M o d u l e _ O p t i o n s
//
// =---------------------------------------------------------------------------
void idpcd_group::Zero_Module_Options ( void ) 
{ 
    Num_Options = 0; 
    memset ( Module_Options, 0, OPTION_DATA_BYTES ); 
}

// =---------------------------------------------------------------------------
// (public) R e p o r t _ G r o u p
//
// =---------------------------------------------------------------------------
void idpcd_group::Report_Group ( void )
{
    if ( Status == GS_PRIMAL )
        return;

    Sys.Message ( CHANNEL_NORMAL, "Group<%s>, Password<%s>, Clients<%d>", 
        Get_Group_Name(), Get_Group_Password(), Get_Ref_Count() );

    Sys.Message_Indent ( 4 );

    for ( int i=0; i < Num_Slots; i++ )
    {
        connection_idpcd& Conn = Get_Group_Connection(i);

        if ( !Conn.Is_Dummy() &&  Conn.Get_Lock_Status() == CS_BUSY )
            Conn.Report_Connection();
    }

    Sys.Message_Indent ( -4 );
}

// =---------------------------------------------------------------------------
// Resolve_Endgame_Ratings 
//
void idpcd_group::Resolve_Endgame_Ratings ( nm_lzg_game_status_reply* p_msg, bool Write_Back )
{
    stacktag Resolution ( CHANNEL_ACTIVITYLOG, "resolve_endgame_ratings" );

    // Update ranks...
    //
    for ( int i=0; i < p_msg->Number_Of_Players; i++ )
    {
        // Calculate new rating for the client
        //

        // bug change! 1/18/00 : I wonder if this is the right thing to do?
        //bool Won_Game = ( i == (p_msg->Winning_Player_ID-1) );
        bool Won_Game = p_msg->Player_Status[i].Current_Rank == 1;

        // Lookup the client record
        //
        uint_16 client_uid = Make_Hardware_Word ( p_msg->Player_Status[i].Client_UID    );

        if ( client_uid == 0 )
        {
            stacktag skip ( CHANNEL_ACTIVITYLOG, "skip" );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<slot>%d</slot>", i );
            continue;
        }

        client_record* p_clirec = Client_RecPtr_From_UID ( client_uid );
        if (!p_clirec)
        {
            stacktag Player ( CHANNEL_ACTIVITYLOG, "skip" );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<slot>%d</slot>", i );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<error>null client</error>", i );
            continue;
        }

        // No check for p_clirec since Validate_Game_Status_Reply should guarantee it

        sint_16 rating      = (sint_16)ntohs(p_clirec->rating);
        uint_16 wins        = (uint_16)ntohs(p_clirec->wins);
        uint_16 losses      = (uint_16)ntohs(p_clirec->losses);
        uint_16 disconnects = (uint_16)ntohs(p_clirec->disconnects);

        if ( rating == 0 )
            rating = RATING_JUMPSTART;

        {
            stacktag Player ( CHANNEL_ACTIVITYLOG, "player" );

            Sys.Message ( CHANNEL_ACTIVITYLOG, "<slot>%d</slot>", i );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<handle>%s</handle>", p_msg->Player_Status[i].User_Handle );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<place>%d</place>", (int)p_msg->Player_Status[i].Current_Rank );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<rating>%d</rating>", (int)rating );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<winner>%s</winner>", Won_Game ? "won" : "lost" );

            // Calculate new rating based on win probability against other players
            //
            int rg_ratings[MAX_PLAYERS_PER_GAME];
            Rating_Clear_Array  ( rg_ratings );

            Initialize_Ratings ( rg_ratings, p_msg->Player_Status, i, p_msg->Number_Of_Players );
            float Prob = Rating_Resolve_Probability ( rg_ratings );
            rating = Rating_Adjust ( rating, Prob, Won_Game );

            if ( Won_Game ) 
                wins++;
            else
                losses++;

            // 1/18/00
            disconnects--;

            if ( Write_Back )
            {
                // Write rank
                //
                p_clirec->rating      = htons ( rating      );
                p_clirec->wins        = htons ( wins        );
                p_clirec->losses      = htons ( losses      );
                
                // 1/19/00 disable this for now
                p_clirec->disconnects = htons ( disconnects );
            }

            Sys.Message ( CHANNEL_ACTIVITYLOG, "<newrating>%d</newrating>", (int)rating );
        }
    }

    try
    {
        SS_DB_Flush ( &Client_MS );
    }
    catch ( ... )
    {
        Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Trying to SS_DB_Flush>" );
    }

    ObliterateGroup();
}

// =---------------------------------------------------------------------------
// (public) I n i t i a l i z e _ R a t i n g s
//
// Returns the count of ratings spunked into the array.
// Ratings are extracted from the client database and out into the array.
// The focus player is put into the firsy array position, regardless of
// order in rg_statuses
//
// =---------------------------------------------------------------------------
uint_08 idpcd_group::Initialize_Ratings ( int*              rg_ratings, 
                                                        player_status* rg_statuses, 
                                                        uint_08         Focus_Player, 
                                                        uint_08         Num_Players )
{
    uint_08         Index = 0;
    uint_16         Client_UID = Make_Hardware_Word (rg_statuses[Focus_Player].Client_UID);
    client_record* p_clirec = Client_RecPtr_From_UID ( Client_UID );

    if ( !p_clirec ) return 0;

    rg_ratings[Index++] = (sint_16)ntohs ( p_clirec->rating );

    for ( int i=0; i < Num_Players; i++ )
    {
        if ( i == Focus_Player ) continue;

        Client_UID = Make_Hardware_Word (rg_statuses[i].Client_UID);
        
        if ( Client_UID == 0 )
            continue;

        p_clirec = Client_RecPtr_From_UID ( Client_UID );
        
        if ( !p_clirec ) return 0;

        rg_ratings[Index++] = (sint_16)ntohs ( p_clirec->rating );
    }

    return Index;

}

// =---------------------------------------------------------------------------
// Validate_Game_Status_Reply
//
_result idpcd_group::Validate_Game_Status_Reply ( nm_lzg_game_status_reply* p_msg )
{
    uint_16         client_uid = 0;

    //stacktag Validate ( CHANNEL_ACTIVITYLOG, "validate_game_status_reply" );

    // loop through and verify rank and client uid are all valid
    //
    _result res = RS_OK;

    for ( int i=0; i < p_msg->Number_Of_Players; i++ )
    {
        // Lookup the client record
        //
        client_uid = Make_Hardware_Word ( p_msg->Player_Status[i].Client_UID  );

        // Skip AI players and/or blank spots
        //
        if ( client_uid == 0 )
            continue;

        {
            stacktag Player ( CHANNEL_ACTIVITYLOG, "player" );

            Sys.Message ( CHANNEL_ACTIVITYLOG, "<slot>%d</slot>", i );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<handle>%s</handle>", p_msg->Player_Status[i].User_Handle );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<place>%d</place>", (int)p_msg->Player_Status[i].Current_Rank );
            Sys.Message ( CHANNEL_ACTIVITYLOG, "<uid>%d</uid>", (int)client_uid );

            if ( !Client_RecPtr_From_UID ( client_uid ) )
            {
                Sys.Message ( CHANNEL_ACTIVITYLOG, "<error>Player #%d[12%s] Client_UID[%d] could not be found. Ranking aborted!</error>",
                    i, p_msg->Player_Status[i].User_Handle, (int)client_uid );

                res = RS_ERR;
                break;
            }
        }


        if ( RFAILED(res) )
            break;
    }

    return res;
}

// =---------------------------------------------------------------------------
// Client_RecPtr_From_UID
//
// UID is hardware byte-ordered
//
client_record* idpcd_group::Client_RecPtr_From_UID ( uint_16 uid )
{
    for ( int i=0; i < Num_Slots; i++ )
    {
        if ( rg_Ranked_Clients[i].uid == uid )
            return rg_Ranked_Clients[i].p_clirec;
    }

    return 0;
}

