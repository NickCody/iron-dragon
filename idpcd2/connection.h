// =---------------------------------------------------------------------------
// c o n n e c t i o n . h
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the client class for maintaining client
//    socket and query state information
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
//

#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "net_messages_x.h"

enum
{
   CS_FREE = 1,      // indicates connection is ready to receive a client
   CS_BUSY = 2       // indicates connection is in use
};

// =---------------------------------------------------------------------------
// (global) c o n n e c t i o n
//
// =---------------------------------------------------------------------------

class connection
{
   friend class connection_pool;

public:
   connection  ( bool Dummy = false );
   virtual ~ connection( );

   // State Query functions
   //
   virtual bool    Ready_Read       ( void ) const; // always true
   virtual bool    Ready_Write      ( void ) const; // If Outbound_Queue is not empty
   virtual _result Process_Aux      ( void ) { return RS_OK; }

   // Basic (non-override) Data Transfer
   //
   _result Read_Data                ( void );
   _result Write_Data               ( void );

   _result Shutdown_Socket          ( void );
   _result Close_Socket             ( void );
   _result Flush_Messages           ( void );
   void    Clear_Messages           ( void );

   virtual _result Send_Message     ( message_package& pkg );
   virtual void Clear_Connection    ( bool just_init = false, bool Dummy = false );
   virtual void Report_Connection   ( void );

   // Initialization
   //
   _result Make_Busy_Connection  ( SOCKET sock, sockaddr_in& addr );
   
   SOCKET   Get_Socket              ( void ) const { return Conn_Sock; }
   uint_32  Get_Lock_Status         ( void ) const { return Lock_Status; }
   uint_32  Get_IP_Address          ( void ) const;
   uint_16  Get_IP_Port             ( void ) const;
   const char * const Get_IP_String ( void );

   bool     Is_Dummy                ( void ) { return Dummy; }

protected:
   
   // derivatives must override this and handle messages
   // Note that the message is *scrambled*
   //
   virtual _result On_Message ( message_package& pkg ) = 0;

   // List manipulation functions
   //
   void              Pop_Front   ( void );
   void              Push_Back   ( message_package* p_msg );

   message_package*  p_Outbound_Queue_Head;
   message_package*  p_Outbound_Queue_Tail;

   SOCKET             Conn_Sock;
   struct sockaddr_in Conn_Addr;
   uint_32            Lock_Status;            // threading protection (sort of obsolete)
   char               ip[IP_STRING_BYTES];
   bool               Dummy;
};

#endif // __CONNECTION_H__
