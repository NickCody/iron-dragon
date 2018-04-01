// =---------------------------------------------------------------------------
// c o n n e c t i o n _i d p c d . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//   idpcd=specific protocol handling
//
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Oct 18   nic   Replaces the concept of an oracle...

#include "PreComp.h"

#include "idpcd_defaults.h"
#include "connection_idpcd.h"
#include "idpcd_group.h"
#include "startup_daemon.h"
#include "unique_id.h"
#include "group_pool.h"
#include "connection_pool.h"
#include "net_messages_admin.h"
#include "users.h"
#include "rank_calc.h"
#include "port_subsystem.h"
#include "stack_tag.h"
#include "command_line.h"

// =---------------------------------------------------------------------------
// qsort function for rank things...
//
// Descending order for rank! (greater rank means higher on list)
// Descending order for wins
// Ascending order for disconnects
//
int Rating_QSort ( const void* elem1, const void* elem2 )
{
   user_rating_element* p_elem1 = (user_rating_element*)elem1;
   user_rating_element* p_elem2 = (user_rating_element*)elem2;

   // First criteria, Rating (DESCENDING)
   //
   if ( p_elem1->Rating < p_elem2->Rating )
      return 1;
   else if ( p_elem1->Rating > p_elem2->Rating )
      return -1;

   // Handle Rating ties
   //
   if ( p_elem1->Rating == p_elem2->Rating )
   {
      // Second criteria, Win count (DESCENDING)
      //
      if ( p_elem1->Wins < p_elem2->Wins )
         return 1;
      else if ( p_elem1->Wins > p_elem2->Wins )
         return -1;

      // Handle Win  Ties
      //
      if ( p_elem1->Wins == p_elem2->Wins )
      {

         // Third Criteria, Disconnects (ASCENDING)
         //
         if ( p_elem1->Disconnects > p_elem2->Disconnects )
            return 1;
         else if ( p_elem1->Disconnects < p_elem2->Disconnects )
            return -1;
      }

   }

   return 0;
}

// =---------------------------------------------------------------------------
// (public, ctor) 
//
// =---------------------------------------------------------------------------
connection_idpcd::connection_idpcd ( bool Dummy )
{
   connection_idpcd::Clear_Connection ( true /*just_init*/, Dummy );
}

// =---------------------------------------------------------------------------
// (public) S e n d _ M e s s a g e
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Send_Message ( message_package& pkg )
{
   _result res = connection::Send_Message ( pkg );

   // Report Message Details if that option is enabled
   //
   Report_Message ( *this, pkg, true /*is_scrambled*/, true /*is_send*/, RSUCCEEDED(res) );

   return res;
}

// =---------------------------------------------------------------------------
// (public) O n _ M e s s a g e
//
// Returning an error from here is generally fatal and connection should die
//
// Note that the message is *scrambled*
// =---------------------------------------------------------------------------
_result connection_idpcd::On_Message ( message_package& pkg )
{
   if ( !pkg.p_data ) return RS_ERR;

   _result res = RS_OK;

   // Store Message Type and unscramble local variable
   //
   //uint_16 Message_Length  = NM_Payload_Length( &pkg.hdr );
   uint_08 Message_Type    = pkg.p_data[0];

   Scramble_Bytes ( &Message_Type, 1, pkg.hdr.Scramble_Key );

   // Report Start of Message Processing
   //
   Report_Message ( *this, pkg, true /*is_scrambled*/, false /*is_send*/, true /*did_succeed*/ );

   Sys.Message_Indent ( 2 ); // extra indenting so child messages are clear

   // Make sure client is authenticated before processing anything
   // but an authentication request
   //
   if ( !Client_Authenticated && (Message_Type != CZS_AUTHENTICATE_REQUEST) )
   {
      Sys.Message ( CHANNEL_DEBUG, "<error>Non Authenticate request from a non-authenticated client</error>" );
      res = RS_ERR;
   }

   // Any message causes the heartbeat flag to reset
   //
   Heartbeat_State = HB_RECEIVEDANY;

   // Process The Message
   //
   if ( RSUCCEEDED(res) )
   {
      switch ( Message_Type )
      {
         // Handle Client Messages
         //

         case ANY_PING:
         {
            Handle_Any_Ping ( pkg );
            break;
         }

         case CZS_AUTHENTICATE_REQUEST : 
            res = Handle_Authenticate_Request ( pkg );
            break;

         case CZS_JOIN_GROUP_REQUEST :
            res = Handle_Join_Group_Request ( pkg );
            break;

         case CZS_QUIT_GROUP_REQUEST :
            res = Handle_Quit_Group_Request ( pkg );
            break;

         case CCH_TEXT:
            res = Handle_Text_Chat ( pkg );
            break;

         // Handle Leader Messages
         //
         case LZS_GAME_MODULE_REQUEST:
         case LZS_GAME_TYPE_ACTION:
         case LZS_PLAYER_ASSIGN_ACTION:
         case LZS_GAME_OPTION_ACTION:
         case LZS_START_GAME_ACTION:
         case LZG_GAME_STATUS_REPLY:         // 5/6/00
         {
            idpcd_group& Group = Get_Group();

            if ( !Group.Is_Dummy() )
               res = Group.Handle_Leader_Message ( *this, pkg );
            break;
         }
         case ANY_SMACKDOWN:
            res = Handle_Smackdown ( pkg );

            break;
         case 0xFF:
            res = Handle_Admin_Request ( pkg );
            break;

         default:
            res = Handle_Unknown_Message ( pkg );
      }
   } 

   Sys.Message_Indent ( -2 ); // reverse extra indenting 

   return res;
}

// =---------------------------------------------------------------------------
// (public, virtual) P r o c e s s _ A u x
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Process_Aux ( void )
{
   return Perform_Reverse_Connect();
}

// =---------------------------------------------------------------------------
// (public, virtual) C l e a r _ C o n n e c t i o] n
//
// =---------------------------------------------------------------------------
void connection_idpcd::Clear_Connection ( bool just_init, bool Dummy )
{
   connection::Clear_Connection ( true /*just_init*/, Dummy );

   // This is the default case. Get the connection's group and first remove
   // the connection from the group before clearing the connection
   //
   if ( !just_init )
   {
      idpcd_group& Group = Get_Group();

      if ( !Group.Is_Dummy() ) 
      {
         uint_16 Idx = Busy_Pool.Get_Connection_Index ( *this );

         if ( Idx != 0xFFFF )
            Group.Remove_Connection ( Idx, false /*quit requested*/ );
      }
   }

   // We store a copy of the connection's complete auth request
   memset ( &Auth_Info, 0, sizeof(nm_czs_authenticate_request) );

   Client_Authenticated             = false;
   Unique_ID                        = 0;
   Pending_Reverse_Connect_Attempt  = 0; // 0 means no attempt should be made
   Reverse_Connect_Succeeded        = false;
   Client_Is_Back_End_Operator      = false;
   CGI_ID_Client                    = false;
   Heartbeat_State                  = HB_PRIMAL;
   Duplicate_Attempt_Count          = 0;

   connection::Clear_Connection ( just_init );

   Group_Index = 0xFFFF;
}

// =---------------------------------------------------------------------------
// (public) C l e a r _ G r o u p _ R e f e r e n c e
//
// Clears the group reference only if it equals the parameter
//
// =---------------------------------------------------------------------------
void connection_idpcd::Clear_Group_Reference ( uint_16 Group_Index )
{
   if ( this->Group_Index == Group_Index )
      Group_Index = 0xFFFF;
}

// =---------------------------------------------------------------------------
// (public) G e t _ G r o u p
//
// Clears the group reference only if it equals the parameter
//
// =---------------------------------------------------------------------------
idpcd_group& connection_idpcd::Get_Group ( void )
{
   return Group_Pool.Get_Group ( Group_Index );
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ A n y _ P i n g
//
// Message is assumed to be scrambled
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Any_Ping( message_package& pkg )
{
   nm_any_ping* p_msg = (nm_any_ping*)pkg.p_data;

   // Unscramble the message
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   if ( Get_Heartbeat_State() == HB_WAITING )
      Heartbeat_State = HB_RECEIVED;

   if ( p_msg->Bounce_Count > 0 )
   {
      p_msg->Bounce_Count--;
      p_msg->Client_UID[0] = 0; // this is what the client expects
      p_msg->Client_UID[1] = 1; // this is what the client expects

      if ( !Get_Group().Is_Dummy() )
      {
         p_msg->Group_UID[0] = 0;
         p_msg->Group_UID[1] = 1;
      }

      _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );

      if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ A u t h e n t i c a t e _ R e q u e s t
//
// Message is assumed to be scrambled
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Authenticate_Request ( message_package& pkg )
{
   nm_czs_authenticate_request* p_msg = (nm_czs_authenticate_request*)pkg.p_data;

   // Unscramble the message
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   // Check that the user's version is valid
   res = Verify_Client_Version ( p_msg );

   stacktag AuthTag ( CHANNEL_CONNECTIONLOG, "action", "name=\"authenticate_request\"" );

   if ( RFAILED(res) )
   {
      // before we print it, good idea to make sure its terminated properly
      p_msg->Client_Version[CLIENT_VERSION_BYTES-1] = 0;
      Sys.Message ( CHANNEL_CONNECTIONLOG, "<error>Outdated Game Detected [%s]</error>", p_msg->Client_Version );
      Client_Authenticated = 0;
      Pending_Reverse_Connect_Attempt = 0;
      Send_Authenticate_Reply ( "Outdated Game Detected. Please download latest.", 0, 0, 0 );
      
      return RS_ERR; // returning an error here should disconnect the client
   }

   // Lookup user in the database
   //
   uint_32 recnum;
 
   Sys.Message ( CHANNEL_CONNECTIONLOG, "<handle>%s</handle>", p_msg->User_Handle );
   Sys.Message ( CHANNEL_CONNECTIONLOG, "<player>%s</player>", p_msg->Default_Player_Name );

   res = Find_Client_By_Username ( (const char*)p_msg->User_Handle, recnum );

   // Process failure
   //
   if ( RFAILED(res) )
   {
      Sys.Message ( CHANNEL_CONNECTIONLOG, "<error>failed to find user</error>" );

      Client_Authenticated = 0;
      Pending_Reverse_Connect_Attempt = 0;
      Send_Authenticate_Reply ( "The Username could not be found.", 0, 0, 0 );
      
      return RS_ERR; // returning an error here should disconnect the client
   }

   // Now, check password
   //
   SS_DB_Get_Record ( &Client_MS, recnum, &Client_Rec );
   Client_Recnum = recnum;

   // Check user's password
   //
   if ( strcmp ( Client_Rec.password, (const char*)p_msg->Password ) == 0 )
   {
      // Disconnect any other connection using this ID
      // (this method fails if there is no diplicate user)
      //
      if ( !Is_CGI_ID() ) // only detect this if we're a game client
      {
         ///*
         if ( RS_OK == Busy_Pool.Detect_Duplicate_User ( (const char*)p_msg->User_Handle ) )
         {
            Sys.Message ( CHANNEL_CONNECTIONLOG, "<error>duplicate user</error>" );

            Client_Authenticated = 0;
            Pending_Reverse_Connect_Attempt = 0;
            Send_Authenticate_Reply ( "You are already logged in elsewhere, aborting.", 0, 0, 0 );
            return RS_ERR;
         }
         //*/
      }

   }
   else
   {
      Sys.Message ( CHANNEL_CONNECTIONLOG, "<error>bad password</error>" );

      Client_Authenticated = 0;
      Pending_Reverse_Connect_Attempt = 0;
      Send_Authenticate_Reply ( "Username ok, but password was bad.", 0, 0, 0 );

      // returning an error here should disconnect the client
      return RS_ERR; 
   }

   // Store the user's auth info, which contains reverse connect port, license info, etc.
   //
   memcpy ( &Auth_Info, p_msg, sizeof(nm_czs_authenticate_request) );

   Client_Authenticated = true;
   Pending_Reverse_Connect_Attempt = 1;      // 1 means, first attempt should be made
   Unique_ID = Generate_Unique_ID ( UID_CONNECTION );         // Generate a unique id for this client, hardware byte-order

   // Check for ad administration connection if so do not attempt a reverse connect
   //
   if ( User_Is_Back_End_Operator ( p_msg->User_Handle ) && Is_CGI_ID())
   {
      Client_Is_Back_End_Operator = true;
      Pending_Reverse_Connect_Attempt = 3; // from 3 12/15/00;
      SS_Net_Blocking_Socket(Get_Socket(), true);
   }

   // Update Counter for number of Authenticated users
   //
   Busy_Pool.Inc_Authenticated ( Is_Back_End_Operator() );
   
   // Update the user's Connect count
   //
   client_record cr;
   SS_DB_Get_Record ( &Client_MS, recnum, &cr );
   cr.connects = htonl ( ntohl ( Get_Client_Rec().connects ) + 1 );
   SS_DB_Set_Record ( &Client_MS, recnum, &cr );
   
   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ J o i n _ G r o u p _ R e q u e s t
//
// Message is assumed to be scrambled.
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Join_Group_Request ( message_package& pkg )
{
   nm_czs_join_group_request* p_msg = (nm_czs_join_group_request*)pkg.p_data;

   // Unscramble the message
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   // Find a group, based on the preferences specified in the join_group_req msg
   //
   uint_16 Find_Group_Index;
   uint_08 Reply_Code = Group_Pool.Find_Group ( p_msg->Game_Module,
                                                p_msg->Group_Name, 
                                                p_msg->Group_Password, 
                                                Reverse_Connect_Succeeded /*leader ok*/,
                                                p_msg->Is_Ranked != 0,
                                                *this,
                                                Find_Group_Index );
   
   if ( Reply_Code == FGSTATUS_OK )
   {

      uint_16 PoolIdx = Busy_Pool.Get_Connection_Index ( *this );

      // Remove player from the group they're already in
      //
      idpcd_group& Group = Get_Group();

      if ( !Group.Is_Dummy() )
      {
         Group.Remove_Connection( PoolIdx, true /*quit requested*/ );
         Group_Index = 0xFFFF;
      }

      idpcd_group& Find_Group = Group_Pool.Get_Group ( Find_Group_Index );

      // Try and add the connection
      //
      uint_08 Slot_ID = 0;

      if ( RSUCCEEDED(Find_Group.Add_Connection ( PoolIdx, Slot_ID )) )
         Group_Index = Find_Group_Index;
      else
         Reply_Code = FGSTATUS_GROUPFULL;

      res = Send_Join_Group_Reply ( Reply_Code, Find_Group_Index, Slot_ID );

      res = Find_Group.Send_All_Summaries ( *this, Slot_ID );
   }
   else
   {
      res = Send_Join_Group_Reply ( Reply_Code, 0xFFFF, 0 );
   }

   return res;
}


// =---------------------------------------------------------------------------
// (public) H a n d l e _ Q u i t _ G r o u p _ R e q u e s t
//
// Message is assumed to be scrambled.
//
// We notify client if succeeded and Remove_Connection
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Quit_Group_Request ( message_package& pkg )
{
   nm_czs_quit_group_request* p_msg = (nm_czs_quit_group_request*)pkg.p_data;

   // Unscramble Message
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   idpcd_group& Group = Get_Group();

   if ( !Group.Is_Dummy() ) 
   {
      uint_16 Group_UID         = Group.Get_Group_UID();
      uint_16 Current_Group_UID = Make_Hardware_Word(p_msg->Current_Group_UID);

      if ( Group_UID != Current_Group_UID )
      {
         Sys.Message ( CHANNEL_DEBUG, "Warning: Actual Group_UID<%d> != p_msg->Current_Group_UID<%d> in Handle_Quit_Group_Request",
           (int)Group_UID, (int)Current_Group_UID );
      }

      uint_16 PoolIdx = Busy_Pool.Get_Connection_Index(*this);

      Group.Remove_Connection ( PoolIdx, true /*quit requested*/ );

      // If we're the leader, p_Group is now toast
      //
      Group_Index = 0xFFFF;

      // Package and send out our reply
      //
      nm_zcs_quit_group_notify gsn;
      gsn.Message_Type  = ZCS_QUIT_GROUP_NOTIFY;

      if ( Group_UID >= 2 )
         { gsn.Group_UID[0]  = MSB(1); gsn.Group_UID[1]  = LSB(1); }
      else if ( Group_UID == 1 /*lobby*/ )
         { gsn.Group_UID[0]  = 0; gsn.Group_UID[1]  = 0; }

      message_package pkg_out ( (uint_08*)&gsn );
      NM_Write_Header ( &pkg_out.hdr, sizeof(nm_zcs_quit_group_notify) );
      
      res = Scramble_Bytes ( pkg_out.p_data, sizeof(nm_zcs_quit_group_notify), IDPCD_SCRAMBLE_KEY );
      
      if ( RSUCCEEDED(res) ) res = Send_Message ( pkg_out );

      // If they were in a non-lobby group Add them to the lobby now
      //
      if ( Group_UID >= 2 )
      {
         uint_08 Slot;
         res = Group_Pool.Get_Lobby().Add_Connection ( PoolIdx, Slot );
         
         if ( RSUCCEEDED(res) )
         {
            res = Group_Pool.Get_Lobby().Broadcast_Group_Member_Status ( Slot, 1, *this );
            res = Group_Pool.Get_Lobby().Send_Group_Member_Status_Summary ( *this );
            Group_Index = Group_Pool.Get_Lobby_Group_Index();
            return RS_OK;
         }
      }

      // Kill the connection if they are quitting from the door or the lobby or they could not
      // be added to the lobby (Add_Connection returned an error)
      //
      Clear_Connection ( );
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ T e x t _ C h a t
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Text_Chat ( message_package& pkg ) 
{
   idpcd_group& Group = Get_Group();

   if ( !Group.Is_Dummy() ) 
   {
      Group.Broadcast_To_Targets ( pkg );
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ A d m i n _ R e q u e s t
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Admin_Request ( message_package& pkg )
{
   if ( !Is_Back_End_Operator() )
      return RS_ERR;

   int len = Make_Hardware_Word ( pkg.hdr.Payload_Length );

   _result res = Scramble_Bytes ( pkg.p_data, len , pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   switch ( pkg.p_data[1] )
   {
      case ACZ_INFO:
      {
         uint_32 busyc, freec;
         Busy_Pool.Num_Connections ( busyc, freec );
         
         uint_32 normg, freeg;
         Group_Pool.Num_Groups ( normg, freeg );

         // Initialize Info message types
         //
         nm_cza_info info;
         info.Message_Type    = 0xFF;
         info.Message_Type_Ex = CZA_INFO;

         // Initialize Info srtings
         //
         SS_Port_Strcpy_Len ( (char*)info.Server_Name, Listen_Info_IDPCD.Listen_Host_Name, SERVER_NAME_BYTES );
         SS_Port_Strcpy_Len ( (char*)info.Start_Time, Daemon_Start_DateTime(), DATETIME_BYTES );

         // Initialize Counters
         //
         *(uint_16*)info.Active_Connections     = htons ( (uint_16)busyc );
         *(uint_16*)info.Active_Groups          = htons ( (uint_16)normg );

         *(uint_32*)info.Total_Connections      = htonl ( Busy_Pool.Get_Total_Clients_Connected() );
         *(uint_32*)info.Total_Connections_Authenticated = htonl ( Busy_Pool.Get_Total_Clients_Authenticated() );
         *(uint_32*)info.Total_Private_Groups   = 0; //htonl ( Group_Pool.Get_Private_Groups_Formed() );
         *(uint_32*)info.Total_Open_Groups      = 0; //htonl ( Group_Pool.Get_Open_Groups_Formed() );
         *(uint_32*)info.Total_Ranked_Groups    = 0; //htonl ( Group_Pool.Get_Ranked_Groups_Formed() );

         // Package and send out our message
         //
         message_package pkg ( (uint_08*)&info );
         NM_Write_Header ( &pkg.hdr, sizeof(nm_cza_info) );

         res = Scramble_Bytes ( (uint_08*)&info, sizeof(nm_cza_info), IDPCD_SCRAMBLE_KEY );

         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );
         break;
      }

      case ACZ_GETLOG:
      {
         // Initialize Info message types
         //
         nm_cza_getlog getlog;
         getlog.Message_Type    = 0xFF;
         getlog.Message_Type_Ex = CZA_GETLOG;
         SS_Port_Strcpy_Len ( (char*)getlog.Log_Filename, Log_Filename, 255 /*not 256, just to be safe*/ );

         // Package and send out our message
         //
         message_package pkg ( (uint_08*)&getlog );
         NM_Write_Header ( &pkg.hdr, sizeof(nm_cza_getlog) );

         res = Scramble_Bytes ( (uint_08*)&getlog, sizeof(nm_cza_getlog), IDPCD_SCRAMBLE_KEY );

         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );
         break;
      }

      case BIDIR_NUMUSERS:
      {
         nm_bidir_numusers numusr;
         numusr.Message_Type = 0xFF;
         numusr.Message_Type_Ex = BIDIR_NUMUSERS;
         *(uint_32*)numusr.Num_Users = htonl ( SS_DB_Get_Record_Count(&Client_MS) );

         // Package and send out our message
         //
         message_package pkg ( (uint_08*)&numusr );
         NM_Write_Header ( &pkg.hdr, sizeof(nm_bidir_numusers) );

         res = Scramble_Bytes ( (uint_08*)&numusr, sizeof(nm_bidir_numusers), IDPCD_SCRAMBLE_KEY );

         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );
         break;
      }

      case BIDIR_GETUSERS:
      {
         nm_bidir_getusers* p_msg = (nm_bidir_getusers*)pkg.p_data;

         uint_32 Rec_Count        = SS_DB_Get_Record_Count ( &Client_MS );
         uint_32 start            = ntohl ( *(uint_32*)p_msg->Start_User );
         uint_32 end              = ntohl ( *(uint_32*)p_msg->End_User );
         uint_32 i;

         user_rating_element* rg_elements = 0;
         client_record        cr;

         if ( !Rec_Count )
            break;

         // Rescramble the same bytes
         //
         res = Scramble_Bytes ( (uint_08*)pkg.p_data, sizeof(nm_bidir_getusers), pkg.hdr.Scramble_Key );
         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );
         res = Scramble_Bytes ( (uint_08*)pkg.p_data, sizeof(nm_bidir_getusers), pkg.hdr.Scramble_Key );

         // Sort the returned clients on rating if the client requested it
         //
         if ( p_msg->Rank_Sorted )
         {
            // first initialize the array
            //
            rg_elements = new user_rating_element [ Rec_Count ];
            for ( i = 0; i < Rec_Count; i++ )
            {
               SS_DB_Get_Record ( &Client_MS, i, &cr );
               rg_elements[i].Index       = i;
               rg_elements[i].Rating      =          ntohs(cr.rating);
               rg_elements[i].Wins        = (uint_16)ntohs(cr.wins);
               rg_elements[i].Losses      = (uint_16)ntohs(cr.losses);
               rg_elements[i].Disconnects = (uint_16)ntohs(cr.disconnects);
            }

            qsort ( rg_elements, Rec_Count, sizeof(user_rating_element), Rating_QSort );
         }

         // Loop through and send out each client.
         //
         for ( i=start; i <= end; i++ )
         {
            if ( p_msg->Rank_Sorted )
               SS_DB_Get_Record ( &Client_MS, rg_elements[i].Index, &cr );
            else
               SS_DB_Get_Record ( &Client_MS, i, &cr );

            nm_user_record ur;
            memcpy ( &ur.record, &cr, sizeof(client_record) );
            *(uint_32*)ur.index = p_msg->Rank_Sorted ? htonl(rg_elements[i].Index) : htonl(i);

            // TODO: Scramble?
            int sent = SS_Net_Send_Buffer ( Conn_Sock , &ur, sizeof(ur), 0 );
            if (sent != sizeof(ur)) {
               Sys.Message ( CHANNEL_ERRORLOG, "<error>Sending user to admin, ur size was %d instead of %d. Loop iteration %d in %d-%d.",
                sent, (int)sizeof(ur), (int)i, (int)start, (int)end);
                
               break;
            }

         }

         delete [] rg_elements;

         break;
      }
      
      case BIDIR_ADDUSER:
      {
         nm_bidir_user*   p_msg  = (nm_bidir_user*)pkg.p_data;
         nm_user_record*  p_user = (nm_user_record*)(pkg.p_data + sizeof(nm_bidir_user));
         uint_32 recnum          = 0xFFFFFFFF;

         // Check for duplicate, we must be unique on username, failure code is 0xFE
         //
         if ( RSUCCEEDED(Find_Client_By_Username ( p_user->record.username, recnum ) ) )
         {
            p_msg->Success_Code = 0xFE;
            *(uint_32*)p_user->index = htonl(recnum);
         }
         else
         {
            // Ignore these settings from the client
            //
            p_user->record.connects       = htonl(0);
            p_user->record.lastgroup_uid  = 0xFFFF;
            p_user->record.status         = DBCLIENTSTATUS_NORMAL;
            p_user->record.registration   = DBREGISTRATION_NONE;
            p_user->record.rating         = htons(0);

            uint_32 recnum;
            res = (_result)SS_DB_Add_Record ( &Client_MS, &p_user->record, &recnum );

            if ( RSUCCEEDED(res) ) 
            {
               p_msg->Success_Code =  0;
               Find_Client_By_Username ( p_user->record.username, recnum );
               *(uint_32*)p_user->index = htonl(recnum);
            }
            else
            {
               p_msg->Success_Code = 0xFF;
               *(uint_32*)p_user->index = htonl(0xFFFFFFFF);
            }
         }

         // Rescramble the same bytes
         //
         res = Scramble_Bytes ( (uint_08*)pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );

         break;
      }
      
      case BIDIR_SETUSER:
      {
         nm_bidir_user*  p_msg  = (nm_bidir_user*)pkg.p_data;
         nm_user_record* p_user = (nm_user_record*)(pkg.p_data + sizeof(nm_bidir_user));
         uint_32         recnum = ntohl(*(uint_32*)p_msg->Recnum);

         //res = Client_DB.Find_Record_By_Username ( p_user->record.username, recnum );
         //RRETURNONFAILURE(res);

         // Lookup original record
         //
         client_record cr;
         res = (_result)SS_DB_Get_Record ( &Client_MS, recnum, &cr );
         RRETURNONFAILURE(res);

         // Make changes (restore password if left empty since user is saying they don't want to change it.
         //
         uint_08 Password[PASSWORD_BYTES];
         memcpy ( Password, cr.password, PASSWORD_BYTES );
         
         memcpy ( &cr, &p_user->record, sizeof(client_record) );

         if ( p_user->record.password[0] == '\0' )
            memcpy ( cr.password, Password, PASSWORD_BYTES );

         res = (_result)SS_DB_Set_Record ( &Client_MS, recnum, &cr );

         *(uint_32*)p_user->index = htonl( RFAILED(res) ? 0xFFFFFFFF : recnum );

         // Rescramble the same bytes
         //
         res = Scramble_Bytes ( (uint_08*)pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );

         break;
      }

      case BIDIR_LOOKUPUSER:
      {
         nm_bidir_lookupuser* p_msg  = (nm_bidir_lookupuser*)pkg.p_data;
         client_record        cr;
         nm_user_record*      p_usr_rec;
         uint_32              recnum;
         _result              res;
         uint_16              len;
         uint_08*             p_outmsg;

         res = Find_Client_By_Username ( p_msg->Username, recnum );
         RRETURNONFAILURE(res);

         res = (_result)SS_DB_Get_Record ( &Client_MS, recnum, &cr );
         RRETURNONFAILURE(res);

         // construct a concatenated message
         //
         len      = sizeof(nm_bidir_lookupuser)+sizeof(nm_user_record);
         p_outmsg = new uint_08[len];

         if ( !p_outmsg ) return RS_NOMEMORY;

         p_msg = (nm_bidir_lookupuser*)p_outmsg;
         memcpy ( p_msg, (nm_bidir_lookupuser*)pkg.p_data, sizeof(nm_bidir_lookupuser) );

         p_usr_rec = (nm_user_record*) ( p_outmsg + sizeof(nm_bidir_lookupuser) );
         //Client_To_User_Record ( *p_usr_rec, cr );
         memcpy ( &p_usr_rec->record, &cr, sizeof(client_record) );

         message_package pkg ( p_outmsg );
         NM_Write_Header ( &pkg.hdr, len );

         res = Scramble_Bytes ( p_outmsg, len, IDPCD_SCRAMBLE_KEY );

         if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );

         delete [] p_outmsg;

         break;
      }
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ U n k n o w n _ M e s s a g e
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Unknown_Message ( message_package& pkg )
{
   Sys.Message ( CHANNEL_MESSAGE_DETAIL, "Receiving Unknown Message<%d>", (int)pkg.p_data[0] );

   Sys.Message_Indent ( 2 );
   Report_Message_Bytes ( pkg, true );
   Sys.Message_Indent ( -2 );

   Sys.Message ( CHANNEL_MESSAGE_DETAIL, "End Receiving Unknown Message<%d>", (int)pkg.p_data[0] );

   //return RS_BADSOCKETCOMM;
   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) H a n d l e _ S m a c k d o w n
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Handle_Smackdown ( message_package& pkg )
{
   //nm_any_smackdown* p_as = (nm_any_smackdown*)pkg.p_data;
   
   // Unscramble Message
   //
   _result res = Scramble_Bytes ( pkg.p_data, NM_Payload_Length(&pkg.hdr), pkg.hdr.Scramble_Key );
   RRETURNONFAILURE(res);

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) P e r f o r m _ R e v e r s e _ C o n n e c t 
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Perform_Reverse_Connect  ( void )
{
   if ( !Pending_Reverse_Connect_Attempt )
      return RS_OK;

   stacktag RevConnTag ( CHANNEL_ACTIVITYLOG, "action", "name=\"reverse_connect\"" );

   char datestr[32];
   SS_Port_Get_DateTime_String ( datestr );
   Sys.Message ( CHANNEL_ACTIVITYLOG, "<datetime>%s</datetime>", datestr );

   // If zero port, simply clear pending flag
   //
   if ( !Make_Hardware_Word(Auth_Info.Inbound_Port) )
   {
      Pending_Reverse_Connect_Attempt = 0;
   }
   // otherwise, attempt the reverse connect
   else 
   {
      // create an unnamed socket for the reverse connect
      //
      SOCKET RC_Sock = socket ( AF_INET, SOCK_STREAM, 0 );

      if ( RC_Sock == SOCKET_ERROR ) return RS_BADSOCKET;

      SS_Net_Blocking_Socket ( RC_Sock, true );

      // name the socket (copy what we already know)
      //
      sockaddr_in Client_Addr;
      int namelen = sizeof(sockaddr_in);
      getpeername ( Conn_Sock, (sockaddr*)&Client_Addr, &namelen );

      // Auth_Info's port is already in network byte order
      //
      Client_Addr.sin_port = *(uint_16*)Auth_Info.Inbound_Port;
      uint_16 port = ntohs(Client_Addr.sin_port);

      Sys.Message ( CHANNEL_ACTIVITYLOG, "<socket>%d</socket>", (int)Conn_Sock );
      Sys.Message ( CHANNEL_ACTIVITYLOG, "<ip>%s</ip>", Get_IP_String() );
      Sys.Message ( CHANNEL_ACTIVITYLOG, "<port>%d,0x%x</port>", (int)port, (int)port );

      // Try and connect
      //
      int ret = connect ( RC_Sock, (sockaddr*)&Client_Addr, sizeof(Client_Addr) );
   
      if ( ret == SOCKET_ERROR )
      {
         // Handle passthrough case
         //
         #if UNIX
         if ( errno == EWOULDBLOCK )
         #elif WIN32
         if ( WSAGetLastError() == WSAEWOULDBLOCK )
         #endif
         {
            SS_Net_Close_Socket ( RC_Sock );
            return RS_OK;
         }

         // Not passthrough, real error
         //
         Sys.Message ( CHANNEL_ACTIVITYLOG, "<error>Failed to connect on attempt %d</error>",
            (int)Pending_Reverse_Connect_Attempt );

         // On third attempt, set pending flag to stop future attempts
         //
         if ( Pending_Reverse_Connect_Attempt == 3 ) Pending_Reverse_Connect_Attempt = 0;
         else Pending_Reverse_Connect_Attempt++;

         Reverse_Connect_Succeeded = false;
      }
      else
      {
         Pending_Reverse_Connect_Attempt = 0;         // no need to try again, we succeeded
         Reverse_Connect_Succeeded = true;
      }

      SS_Net_Close_Socket ( RC_Sock );
   }

   _result res = RS_OK;

   // Now, send authenmtication reply only if there are no more pending 
   // reverse connects
   //
   if ( !Pending_Reverse_Connect_Attempt )
   {
      // Now, send the user their Authentication reply!
      //
      if ( Client_Authenticated )
      {
         char szTemp[256];
         Write_Server_Message ( szTemp );
         res = Send_Authenticate_Reply ( szTemp, Client_Authenticated, Reverse_Connect_Succeeded, Unique_ID );
      }
      else
      {
         res = Send_Authenticate_Reply ( "Server failed to authenticate you", Client_Authenticated, Reverse_Connect_Succeeded, 0 );
         Clear_Connection();
      }

   }

   return res;
}

// =---------------------------------------------------------------------------
// (protected) S e n d _ A u t h e n t i c a t e _ R e p l y
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Send_Authenticate_Reply ( const char* msg, uint_08 auth, uint_08 rev_connect, uint_16 cid )
{
   uint_16 msg_len = strlen(msg);
   uint_16 auth_buf_len = sizeof(nm_zcs_authenticate_reply) + msg_len + 1;

   _result res = RS_OK;

   // allocate our dynamically sized authenticate reply
   //
   uint_08* p_buf = new uint_08[auth_buf_len];

   if ( !p_buf ) return RS_NOMEMORY;

   nm_zcs_authenticate_reply* p_reply = (nm_zcs_authenticate_reply*)p_buf;

   // Assemble the message
   //
   p_reply->Message_Type         = ZCS_AUTHENTICATE_REPLY;
   p_reply->Made_Inbound_Connect = rev_connect;
   p_reply->Client_UID[0]        = MSB(cid);
   p_reply->Client_UID[1]        = LSB(cid);

   SS_Port_Strcpy_Len( (char*)p_reply->CZ_Message, (const char*)msg, msg_len+1 );

   // Package and send out our message
   //
   message_package pkg ( p_buf );
   NM_Write_Header ( &pkg.hdr, auth_buf_len );

   res = Scramble_Bytes ( p_buf, auth_buf_len, IDPCD_SCRAMBLE_KEY );

   if ( RSUCCEEDED(res) ) res = Send_Message ( pkg );

   delete [] p_buf;

   // Very important! cgi_id should not be put in lobby
   //
   if ( !Is_CGI_ID() )
   {
      // Now smack the client into the lobby
      //
      uint_08 Slot_ID;
      idpcd_group& Lobby = Group_Pool.Get_Lobby();

      uint_16 PoolIdx = Busy_Pool.Get_Connection_Index(*this);
      res = Lobby.Add_Connection ( PoolIdx, Slot_ID );

      if ( RSUCCEEDED(res) )
      {
         Group_Index = Group_Pool.Get_Lobby_Group_Index();
         // NickC 11/1/00 : This is done in Add_Connection
         //res = Lobby.Broadcast_Group_Member_Status ( Slot_ID, 1, *this );
         res = Lobby.Send_Group_Member_Status_Summary ( *this );
      }
   }

   return res;
}


// =---------------------------------------------------------------------------
// (public, virtual) R e p o r t _ C o n n e c t i o 
//
// Derived classes should call this base implementation first
//
// =---------------------------------------------------------------------------
void connection_idpcd::Report_Connection ( void )
{
   idpcd_group& Group = Get_Group();

   stacktag ConnTag ( CHANNEL_NORMAL, "connection_idpcd" );

   // Report idpcd-specific details
   //
   Sys.Message ( CHANNEL_NORMAL, "<authenticated>%s</authenticated>", Client_Authenticated ? "true" : "false" );

   Sys.Message ( CHANNEL_NORMAL, "<pending_reverse_connect_attempt>%s</pending_reverse_connect_attempt>", Pending_Reverse_Connect_Attempt ? "true" : "false" );

   Sys.Message ( CHANNEL_NORMAL, "<game_leader_ok>%s</game_leader_ok>", Reverse_Connect_Succeeded ? "true" : "false" );

   Sys.Message ( CHANNEL_NORMAL, "<name>%s</name>", Client_Rec.username );

   Sys.Message ( CHANNEL_NORMAL, "<rating>%s</rating>", (int)ntohs(Client_Rec.rating) );

   Sys.Message ( CHANNEL_NORMAL, "<in_group>%s</in_group>", Group.Is_Dummy() ? "no" : "yes" );

   if ( !Group.Is_Dummy()  )
   {
      Sys.Message ( CHANNEL_NORMAL, "<group_uid>%d</group_uid>",  Group.Get_Group_UID() );
      Sys.Message ( CHANNEL_NORMAL, "<group_name>%s</group_name>", Group.Get_Group_Name() );
   }

   // Call base implementation
   //
   connection::Report_Connection();

}

// =---------------------------------------------------------------------------
// (public) D o e s _ S u p p o r t _ M o d u l e
//
// Returns true/false based on whether the user supports the specified game module
// based on their license data
// =---------------------------------------------------------------------------
bool connection_idpcd::Does_Support_Module ( uint_08 mod ) const
{
   return mod == 1;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ J o i n _ G r o u p _ R e p l y
//
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Send_Join_Group_Reply ( uint_08 rc, uint_16 Group_Index, uint_08 sid )
{
   idpcd_group& Join_Group = Group_Pool.Get_Group ( Group_Index );

   if ( Join_Group.Is_Dummy() )
      return RS_ERR;

   _result res = RS_OK;

   nm_zcs_join_group_reply gsn;
   memset ( &gsn, 0, sizeof(nm_zcs_join_group_reply) );

   gsn.Message_Type = ZCS_JOIN_GROUP_REPLY;
   gsn.Reply_Code = rc;

   if ( rc == FGSTATUS_OK )
   {
      // Success
      //
      gsn.Assigned_Slot_ID = sid;
      gsn.Group_UID[0] = MSB ( Join_Group.Get_Group_UID() );
      gsn.Group_UID[1] = LSB ( Join_Group.Get_Group_UID() );
      strcpy ( gsn.Group_Name, Join_Group.Get_Group_Name() );
      strcpy ( gsn.Group_Password, Join_Group.Get_Group_Password() );
   
      *(uint_16*)gsn.GL_IP_Port    = Join_Group.Get_Game_Leader().Get_Auth_Inbound_Port();
      *(uint_32*)gsn.GL_IP_Address = Join_Group.Get_Game_Leader().Get_IP_Address();
      gsn.Is_Ranked = Join_Group.Is_Ranked();
   }
   else
   {
      // Failure
      //
      gsn.Assigned_Slot_ID = 0;
      gsn.Group_UID[0] = 0;
      gsn.Group_UID[1] = 0;
   }

   // Package and send out our reply
   //
   message_package pkg_out ( (uint_08*)&gsn );

   NM_Write_Header ( &pkg_out.hdr, sizeof(nm_zcs_join_group_reply) );

   // Scramble Outgoing Message
   //
   res = Scramble_Bytes ( pkg_out.p_data, sizeof(nm_zcs_join_group_reply), IDPCD_SCRAMBLE_KEY );

   if ( RSUCCEEDED(res) )
      res = Send_Message ( pkg_out );

   return res;
}

// =---------------------------------------------------------------------------
// (public) Verify_Client_Version
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Verify_Client_Version ( nm_czs_authenticate_request* p_msg )
{
   client_version cliver;
   memset ( &cliver, 0, sizeof(client_version) );

   if ( p_msg->Client_Version[0] == 'c' &&
        p_msg->Client_Version[1] == 'g' &&
        p_msg->Client_Version[2] == 'i' &&
        p_msg->Client_Version[3] == '_' &&
        p_msg->Client_Version[4] == 'i' &&
        p_msg->Client_Version[5] == 'd' )
   {
      CGI_ID_Client = true;
      return RS_OK;
   }
   else
   {
      CGI_ID_Client = false;

      cliver.Major_Version    = p_msg->Client_Version[0];
      cliver.Minor_Version[0] = p_msg->Client_Version[2];
      cliver.Minor_Version[1] = p_msg->Client_Version[3];
      cliver.Tester_Version   = p_msg->Client_Version[4];
      cliver.Year[0]          = p_msg->Client_Version[6];
      cliver.Year[1]          = p_msg->Client_Version[7];
      cliver.Year[2]          = p_msg->Client_Version[8];
      cliver.Year[3]          = p_msg->Client_Version[9];
      cliver.Month[0]         = p_msg->Client_Version[11];
      cliver.Month[1]         = p_msg->Client_Version[12];
      cliver.Day[0]           = p_msg->Client_Version[13];
      cliver.Day[1]           = p_msg->Client_Version[14];

      // Put additional constraints here
      //
      if ( cliver.Major_Version == '1' )
         return RS_OK;
   }

   return RS_ERR;
}

// =---------------------------------------------------------------------------
// (public) G e t _ C l i e n t _ R e c
//
// =---------------------------------------------------------------------------
client_record& connection_idpcd::Get_Client_Rec ( void )
{
   SS_DB_Get_Record ( &Client_MS, Client_Recnum, &Client_Rec );
   return Client_Rec;
}

// =---------------------------------------------------------------------------
// (public) G e t _ C l i e n t _ R e c _ P t r
//
// =---------------------------------------------------------------------------
client_record* connection_idpcd::Get_Client_Rec_Ptr ( void )
{
   client_record* p_rec = 0;

   if ( 0 == SS_DB_Get_Record_Ref ( &Client_MS, Client_Recnum, (void**)&p_rec ) )
      return p_rec;
   else
      return 0;
}

// =---------------------------------------------------------------------------
// (public) C o m m i t _ C l i e n t _ R e c
//
// Dangerous, callers should definitely make sure local client rec is up to date
// before committing!
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Commit_Client_Rec ( void )
{
   return (_result)SS_DB_Set_Record ( &Client_MS, Client_Recnum, &Client_Rec );
}

// =---------------------------------------------------------------------------
// (public) S t a r t _ H e a r t b e a t
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Start_Heartbeat ( timecode& Time )
{
   Heartbeat_State = HB_WAITING;
   Heartbeat_Time  = Time;

   // Schedule expire time
   //
   Heartbeat_Time.Advance_Time ( HEARTBEAT_EXPIRATION_SECONDS );

   nm_any_ping ping;
   SS_Port_ZeroMemory ( &ping, sizeof(nm_any_ping) );

   // Package and send out our message
   //
   message_package pkg ( (uint_08*)&ping );
   NM_Write_Header ( &pkg.hdr, sizeof(nm_any_ping) );

   ping.Message_Type = ANY_PING;
   ping.Bounce_Count = 1;
   ping.Client_UID[0] = 0; // this is what the client expects
   ping.Client_UID[1] = 1; // this is what the client expects

   if ( !Get_Group().Is_Dummy() )
   {
      ping.Group_UID[0] = MSB ( Get_Group().Get_Group_UID() );
      ping.Group_UID[1] = LSB ( Get_Group().Get_Group_UID() );
   }

   _result res = Scramble_Bytes ( (uint_08*)&ping, sizeof(nm_any_ping), IDPCD_SCRAMBLE_KEY );

   if ( RSUCCEEDED(res) )
      res = Send_Message ( pkg );

   return res;
}

// =---------------------------------------------------------------------------
// (public) R e s e t _ H e a r t b e a t
//
// =---------------------------------------------------------------------------
_result connection_idpcd::Reset_Heartbeat ( void )
{
   Heartbeat_State = HB_PRIMAL;

   return RS_OK;
}

