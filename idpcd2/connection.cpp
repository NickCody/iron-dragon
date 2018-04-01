// =---------------------------------------------------------------------------
// c o n n e c t i o n . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//   Low-level connection logic. Handles the mundane tasks of piecing together 
//   larger, higher-level, messages
//
//   This class is general purpose and can be used by any kind of client 
//   application that wants to 
//
//   The connection is in one of three states, defines buy Lock_Status:
//       CS_FREE....... the connection is not being used and is free
//       CS_BUSY....... the connection is active and a socket is assigned
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jun 23   nic   Created.
//   99 Aug 15   nic   Added the idea of the oracle
//   99 Oct 13   nic   Yanked the idea of an oracle (Tom's Birthday)
//   99 Nov 28   nic   Yanked out dependencies on protocols.h for Ver 2.0
//

#include "PreComp.h"

#include <ctype.h>               // for isalpha()
#include "connection.h"
#include "common.h"
#include "idpcd_defaults.h"
#include "stack_tag.h"

#include "connection_pool.h"
extern connection_pool Busy_Pool;

// =---------------------------------------------------------------------------
// default constructor
//
// =---------------------------------------------------------------------------
connection::connection ( bool Dummy )
{
   Clear_Connection( true /*just init, do not free*/, Dummy );
}

// =---------------------------------------------------------------------------
// destructor
//
// =---------------------------------------------------------------------------
connection:: ~ connection()
{
   Clear_Connection ( false );
}

// =---------------------------------------------------------------------------
// (public, virtual) C l e a r _ C o n n e c t i o] n
//
// If just_init is false, we try and deallocate any allocated resources
// =---------------------------------------------------------------------------
void connection::Clear_Connection ( bool just_init, bool Dummy )
{
   // just_init usually called from the ctor only
   //
   if ( !just_init )
   {
      Clear_Messages();

      if ( Conn_Sock && Conn_Sock != -1 )
         SS_Net_Close_Socket ( Conn_Sock );
   }

   this->Dummy           = Dummy;
   p_Outbound_Queue_Head = 0;
   Lock_Status           = CS_FREE;
   Conn_Sock             = 0;

   memset ( &Conn_Addr, 0, sizeof(sockaddr_in) );
}

// =---------------------------------------------------------------------------
// (public) M a k e _ B u s y _ C o n n e c t i o n
//
// Makes a connection busy
//
// =---------------------------------------------------------------------------
_result connection::Make_Busy_Connection ( SOCKET sock, sockaddr_in& addr )
{
   Conn_Addr   = addr;
   Conn_Sock   = sock;
   Lock_Status = CS_BUSY;

   Busy_Pool.Inc_Connected();

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) R e a d y _ R e a d
//
// returns true if the connection is ready to read data, false otherwise
// =---------------------------------------------------------------------------
bool connection::Ready_Read ( void ) const
{
   // Now that we reoriented the server to always be listening to messages
   // we always want them!

   return true;
}

// =---------------------------------------------------------------------------
// (public) R e a d y _ W r i t e
//
// returns true if the connection is ready to read data, false otherwise
// =---------------------------------------------------------------------------
bool connection::Ready_Write ( void ) const
{
   return p_Outbound_Queue_Head != 0;
}

// =---------------------------------------------------------------------------
// (public) R e a d _ D a t a
//
// We assume that we always get complete messages. So we always first ask for 
// message length (16-bits) first, then we ask for that many bytes
//
// If this function returns an error, the connection should be killed.
//
// =---------------------------------------------------------------------------
_result connection::Read_Data ( )
{
   int       ret     = 0;
   _result   res     = RS_OK;
   uint_16   msg_len = 0;
   uint_08*  p_msg   = 0;
   nm_header hdr;

   // First, read the header
   //
   ret = SS_Net_Receive_Buffer ( Conn_Sock, &hdr, sizeof(nm_header), 0 ); 

   if ( ret == SOCKET_ERROR ) return RS_ERR;
   if ( !ret                ) return RS_NOTOPEN;

   msg_len = NM_Payload_Length( &hdr );
   if ( !msg_len ) return RS_BADSOCKETCOMM;

   // If msg_len is larger than our biggest defined message, we're likely
   // screwed up to the max. The connection should be shot immediately
   //
   if ( msg_len > MAX_MSG_LEN ) 
   {
      Sys.Message ( CHANNEL_DEBUG, "Message size (%d) is greater than max of %d", msg_len, (int)MAX_MSG_LEN );
      return RS_BADCLIENTCOMM;
   }

   // Now, read the rest of the message. Keep trying until we get it all
   //
   try
   {
      uint_16 total = 0;
      uint_32 tries = 0;
      p_msg = new uint_08[msg_len];

      while ( tries++ < MAX_RECEIVE_ATTEMPTS && total < msg_len )
      {
         ret = SS_Net_Receive_Buffer ( Conn_Sock, &p_msg[total], msg_len-total, 0 );
         
         if ( ret > 0 )
         {
            total += ret;
            if ( total == msg_len )
            {
               message_package pkg ( p_msg );
               pkg.hdr = hdr;
               res = On_Message ( pkg );
               break;
            }
         }
         else if ( ret < 0 )
         {
            // We don't deal with errors very well
            //
            #if UNIX
            int err = errno;
            #elif WIN32
            int err = WSAGetLastError();
            #endif
            Sys.Message ( CHANNEL_DEBUG, "We got a header, but tripped up when reading the Payload: %s\r", Socket_Error_String ( err ) );
            res = RS_BADCLIENTCOMM;
         }

         SS_Port_Sleep ( RECEIVE_ATTEMPT_DELAY_MILLISECOND );
      }
   }
   catch ( ... )
   {
      Sys.Message ( CHANNEL_EXCEPTIONLOG, "Exception<Read_Data()>" );
      res = RS_EXCEPTION;
   }

   delete [] p_msg;

   return res;
}

// =---------------------------------------------------------------------------
// (public) W r i t e _ D a t a
//
// The package is assumed to be in MSB
//
// Note: You can shut down the connection by queuing a zero length message
//
// =---------------------------------------------------------------------------
_result connection::Write_Data ( )
{
   int     ret = 1;
   _result res = RS_OK;

   while ( ret != 0 && ret != SOCKET_ERROR && p_Outbound_Queue_Head )
   {
      // zero-length message signifies we want to close the connection
      //
      if ( NM_Payload_Length(&p_Outbound_Queue_Head->hdr) == 0 )
         return RS_ERR;

      // Assumes the message is already scrambled
      //
      res = Send_Message ( *p_Outbound_Queue_Head );

      if ( RSUCCEEDED(res) ) Pop_Front();
      else return RS_NOTOPEN;
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) S h u t d o w n _ S o c k e t
//
// =---------------------------------------------------------------------------
_result connection::Shutdown_Socket ( void )
{
   // Flush
   //
   Write_Data ( );

#if UNIX
   if ( SOCKET_ERROR == shutdown ( Conn_Sock, 0x02 /*SHUT_RDWR*/ ) )
#elif WIN32
  if ( SOCKET_ERROR == shutdown ( Conn_Sock, 0x02 /*SD_BOTH*/ ) )
  #endif
      return RS_BADSOCKETCOMM;

   return RS_OK;
}
// =---------------------------------------------------------------------------
// (public) C l o s e _ S o c k e t
//
// =---------------------------------------------------------------------------
_result connection::Close_Socket ( void )
{
   // Flush
   //
   Write_Data ( );

   if ( SOCKET_ERROR == SS_Net_Close_Socket ( Conn_Sock ) ) return RS_BADSOCKETCOMM;

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) S e n d _ M e s s a g e
//
// Sends the message right away, bypassing message queue (note that Write_Data,
//   which uses the Queue, calls Send_Message once per message in the queue!)
//
// Accepts a scrambled message and sends it out over the network.
//
// =---------------------------------------------------------------------------
_result connection::Send_Message ( message_package& pkg )
{
   if ( !Conn_Sock )
      return RS_ERR;

   int     ret;
   _result res = RS_ERR;

   // Write the header
   //
   ret = SS_Net_Send_Buffer ( Conn_Sock, &pkg.hdr, sizeof(nm_header), 0 );
   
   if ( ret == sizeof(nm_header) ) 
   {
      // Write the message
      //
      ret = SS_Net_Send_Buffer ( Conn_Sock, pkg.p_data, NM_Payload_Length(&pkg.hdr), 0 );

      if ( ret == NM_Payload_Length(&pkg.hdr) )
         res = RS_OK;
   }

   return res;
}

// =---------------------------------------------------------------------------
// (public) C l e a r _ M e s s a g e s
//
// Clears message queue, deleting messages, but not sending them
// =---------------------------------------------------------------------------
void connection::Clear_Messages ( void )
{
   while ( p_Outbound_Queue_Head )
      Pop_Front();

}

// =---------------------------------------------------------------------------
// (protected) P o p _ F r o n t
//
// Pops the front element off listand destroys it (assumes message is no 
//   longer needed!)
// =---------------------------------------------------------------------------
void connection::Pop_Front ( void )
{
   message_package* p_head = p_Outbound_Queue_Head;
   message_package* p_next = p_head ? p_Outbound_Queue_Head->p_next : 0;
   
   if ( p_next ) p_next->p_prev = 0;
   else p_Outbound_Queue_Tail = 0;

   p_Outbound_Queue_Head = p_next;

   if ( p_head )
   {
      delete [] p_head->p_data;
      delete p_head;
   }
}

// =---------------------------------------------------------------------------
// (protected) P u s h _ B a c k
//
// We assume that p_msg is not part of the list, so we go and overwrite
//   next and prev pointers without regard to their previous values.
//
// We take control of memory for p_pkg
//
// =---------------------------------------------------------------------------
void connection::Push_Back ( message_package* p_pkg )
{
   if ( !p_Outbound_Queue_Head )
   {
      p_pkg->p_next = p_pkg->p_prev = 0;
      p_Outbound_Queue_Head = p_Outbound_Queue_Tail = p_pkg;
   }
   else
   {
      p_Outbound_Queue_Tail->p_next = p_pkg;
      p_pkg->p_next = 0;
      p_pkg->p_prev = p_Outbound_Queue_Tail;
      p_Outbound_Queue_Tail = p_pkg;
   }
}

// =---------------------------------------------------------------------------
// (public, virtual) R e p o r t _ C o n n e c t i o n
//
// Derived classes should call this base implementation last since it newlines
//
// =---------------------------------------------------------------------------
void connection::Report_Connection ( void )
{
   stacktag ConnTag ( CHANNEL_NORMAL, "connection" );

   Sys.Message ( CHANNEL_NORMAL, "<socket>%d</socket>", (int)Get_Socket() );
   Sys.Message ( CHANNEL_NORMAL, "<ip>%s</ip>", Get_IP_String() );
}

// =---------------------------------------------------------------------------
// (public) G e t _ I P _ A d d r e s s
//
// Returns the IP Address used by the connection in network byte order
//
// =---------------------------------------------------------------------------
uint_32 connection::Get_IP_Address ( void ) const
{
   #if UNIX
      return Conn_Addr.sin_addr.s_addr;
   #elif WIN32
      return Conn_Addr.sin_addr.S_un.S_addr;
   #endif
}

// =---------------------------------------------------------------------------
// (public) G e t _ I P _ P o r t
//
// Returns the IP Port used by the connection in network byte order
//
// =---------------------------------------------------------------------------
uint_16 connection::Get_IP_Port ( void ) const
{
   return Conn_Addr.sin_port;
}

// =---------------------------------------------------------------------------
// (public) G e t _ I P _ S t r i n g
//
// Returns the IP address in non-moifiable string form
//
// =---------------------------------------------------------------------------
const char * const connection::Get_IP_String ( void )
{
   SS_Net_Print_IP ( ip, Get_IP_Address() );
   return ip;
}
