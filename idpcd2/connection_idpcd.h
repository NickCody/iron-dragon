#ifndef __CONNECTION_IDPCD_H__
#define __CONNECTION_IDPCD_H__

#include "db_client.h"
#include "connection.h"
#include "net_messages.h"
#include "net_messages_admin.h"
#include "heartbeat.h"

class idpcd_group;

// =---------------------------------------------------------------------------
// client_version - structure to manage contents of the Client_Version field
//   of the nm_czs_authenticate_request message
//
struct client_version
{
   char Major_Version;
   char Minor_Version[2];
   char Tester_Version;
   char Year[4];
   char Month[2];
   char Day[2];
};

// =---------------------------------------------------------------------------
// connection_idpcd
//

class connection_idpcd : public connection
{
public:
   connection_idpcd ( bool Dummy = false );
   virtual ~ connection_idpcd() {}

   // [connection]
   //
   virtual void    Clear_Connection  ( bool just_init = false, bool Dummy = false );
   virtual void    Report_Connection ( void );
   virtual _result Process_Aux       ( void );
   virtual _result On_Message        ( message_package& pkg );
   virtual _result Send_Message      ( message_package& pkg );

   client_record& Get_Client_Rec     ( void );
   client_record* Get_Client_Rec_Ptr ( void );
   _result        Commit_Client_Rec  ( void );

   uint_16 Get_UID                       ( void        ) const { return Unique_ID; }
   bool    Does_Support_Module           ( uint_08 mod ) const;
   bool    Get_Pending_Reverse_Connect   ( void        ) const { return Pending_Reverse_Connect_Attempt != 0; }
   bool    Get_Reverse_Connect_Succeeded ( void        ) const { return Reverse_Connect_Succeeded; }

   const char* Get_Auth_User_Handle          ( void ) { return (const char*)Auth_Info.User_Handle; }
   const char* Get_Auth_Default_Player_Name  ( void ) { return (const char*)Auth_Info.Default_Player_Name; }
   const char* Get_Auth_Password             ( void ) { return (const char*)Auth_Info.Password; }
   uint_16     Get_Auth_Inbound_Port         ( void ) { return Make_Network_Word ( Auth_Info.Inbound_Port ); }
   sint_16     Get_Current_Rank              ( void ) { return (sint_16)ntohs(Client_Rec.rating); }
   uint_32     Get_Duplicate_Attempt_Count   ( void ) { return Duplicate_Attempt_Count; }

   
   uint_32      Increment_Duplicate_Attempt_Count ( void ) { return ++Duplicate_Attempt_Count; }
   bool         Is_CGI_ID                         ( void ) { return CGI_ID_Client; }
   void         Clear_Group_Reference             ( uint_16 Group_ID );
   idpcd_group& Get_Group                         ( void );

   // Heartbeat Methods
   //
   HEARTBEAT_STATE Get_Heartbeat_State ( void ) { return Heartbeat_State; }
   timecode& Get_Heartbeat_Time        ( void ) { return Heartbeat_Time; }
   _result Start_Heartbeat             ( timecode& Time );
   _result Reset_Heartbeat             ( void );
   
protected:
   
   // Incoming Message Handlers
   //
   _result Handle_Any_Ping             ( message_package& pkg );
   _result Handle_Authenticate_Request ( message_package& pkg );
   _result Handle_Join_Group_Request   ( message_package& pkg );
   _result Handle_Quit_Group_Request   ( message_package& pkg );
   _result Handle_Text_Chat            ( message_package& pkg );
   _result Handle_Admin_Request        ( message_package& pkg );
   _result Handle_Unknown_Message      ( message_package& pkg );
   _result Handle_Smackdown            ( message_package& pkg );

   // Outgoing Message Handlers
   //
   _result Send_Join_Group_Reply ( uint_08 rc, uint_16 Group_Index, uint_08 sid );
   _result Send_Authenticate_Reply ( const char* msg, uint_08 auth, uint_08 rev_connect, uint_16 cid );

   _result Perform_Reverse_Connect ( void );
   bool    Is_Back_End_Operator    ( ) { return Client_Is_Back_End_Operator; }
   _result Verify_Client_Version   ( nm_czs_authenticate_request* p_msg );

   // Misc
   //
   //void Client_To_User_Record ( nm_user_record& ur, client_record& cr );
   //void User_To_Client_Record ( client_record& cr, nm_user_record& ur );

   // Authenticate & Client Information
   //
   client_record               Client_Rec;
   uint_32                     Client_Recnum;
   uint_16                     Group_Index;
   uint_16                     Unique_ID; // hardware byte-ordering
   HEARTBEAT_STATE             Heartbeat_State;
   timecode                    Heartbeat_Time;
   uint_32                     Duplicate_Attempt_Count;

   nm_czs_authenticate_request Auth_Info;

   // Connection Information
   //
   uint_08       Pending_Reverse_Connect_Attempt;
   bool          Client_Authenticated;
   bool          Reverse_Connect_Succeeded;
   bool          Client_Is_Back_End_Operator;
   bool          CGI_ID_Client;
};


#endif // __CONNECTION_IDPCD_H__

