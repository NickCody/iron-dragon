// =---------------------------------------------------------------------------
// c o n n e c t i o n _ p o o l . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    The connection pool maintains a pool of connection objects. This implementation uses
//    an array. AT all times, we can guarantee that all connections in the pool are valid
//    as the server will not start if this is not so. The connection pool cannot be
//    resized while the server is running.
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
// 
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jul 28   nic   Created.
//

#include "PreComp.h"

#include "connection_pool.h"
#include "idpcd_defaults.h"
#include "idpcd_group.h"

// =---------------------------------------------------------------------------
// Continue_Processing
//
// From startup_daemon.cpp, certain operations can't be done if the server
// is running (i.e.: Continue_Processing = true )
//
extern bool Continue_Processing;

// =---------------------------------------------------------------------------
// Dummy Reference
//
// This dummy reference to a connection_idpcd object makes the pointerless
// implementation possible. This reference is used when invalid indices are
// specified
//
connection_idpcd DummyConnection ( true /*dummy*/ );

// =---------------------------------------------------------------------------
// ctor - connection_pool
//
// =---------------------------------------------------------------------------
connection_pool::connection_pool ( )
{
   Connection_Count = 0;
   rgConnections = 0;

   Resize ( DEF_MAX_CONNECTIONS );

   // Used for server statistics
   //
   Total_Clients_Authenticated = 0;
   Total_BE_Ops_Authenticated = 0;
   Total_Clients_Connected = 0;
}

// =---------------------------------------------------------------------------
// dtor - ~ connection_pool
//
// =---------------------------------------------------------------------------
connection_pool::~connection_pool ( )
{
   Resize ( 0 );
}

// =---------------------------------------------------------------------------
// (public) Initialize_Multiplex_Descriptors
//
// =---------------------------------------------------------------------------
_result connection_pool::Initialize_Multiplex_Descriptors ( 
   fd_set& fd_reads, fd_set& fd_writes, int& max_desc )
{
   FD_ZERO ( &fd_reads );
   FD_ZERO ( &fd_writes );

   max_desc = 0;

   for ( int i=0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection ( i );

      if ( Conn.Get_Lock_Status() == CS_BUSY )
      {
         // check if this connection is ready to read data
         //
         if ( Conn.Ready_Read() )
         {
            FD_SET ( Conn.Get_Socket(), &fd_reads );
            if ( Conn.Get_Socket() > (SOCKET)max_desc )
               max_desc = Conn.Get_Socket();
         }

         // check if this connection is ready to write data
         //
         if ( Conn.Ready_Write() )
         {
            FD_SET ( Conn.Get_Socket(), &fd_writes );
            if ( Conn.Get_Socket() > (SOCKET)max_desc )
               max_desc = Conn.Get_Socket();
         }
      }
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) P r o c e s s _ P e n d i n g _ T r a n s f e r s 
//
// =---------------------------------------------------------------------------
_result connection_pool::Process_Pending_Transfers ( 
   fd_set& fd_reads, fd_set& fd_writes )
{
   _result res;

   for ( int i = 0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection ( i );

      if ( Conn.Get_Lock_Status() == CS_BUSY )
      {
         res = RS_OK;

         // Process Reads
         //
         if ( Conn.Ready_Read() && FD_ISSET( Conn.Get_Socket(), &fd_reads ) )
            res = Conn.Read_Data();

         if ( RFAILED(res) )
         {
            char* reason = 0;

            if ( res == RS_NOTOPEN )
               reason = "disconnected on read 0";
            else
               reason = "error on read";

            Sys.Message ( CHANNEL_CONNECTIONLOG, "<action name=\"free_connection\">" );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <handle>%s</handle>", Conn.Get_Auth_User_Handle() );
            //Sys.Message ( CHANNEL_CONNECTIONLOG, "  <ip>%s</ip>", Conn.Get_IP_String() );
            //Sys.Message ( CHANNEL_CONNECTIONLOG, "  <socket>%d</socket>", (int)Conn.Get_Socket() );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <reason>%s</reason>", reason );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <error>%s</error>", Result_String(res) );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "</action>" );

            Conn.Clear_Connection();
            continue;
         }

         // Process Writes
         //
         if ( Conn.Ready_Write() && FD_ISSET( Conn.Get_Socket(), &fd_writes ) )
            res = Conn.Write_Data();

         if ( RFAILED(res) )
         {
            char* reason = "error on write";
         
            Sys.Message ( CHANNEL_CONNECTIONLOG, "<action name=\"free_connection\">" );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <handle>%s</handle>", Conn.Get_Auth_User_Handle() );
            //Sys.Message ( CHANNEL_CONNECTIONLOG, "  <ip>%s</ip>", Conn.Get_IP_String() );
            //Sys.Message ( CHANNEL_CONNECTIONLOG, "  <socket>%d</socket>", (int)Conn.Get_Socket() );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <reason>%s</reason>", reason );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <error>%s</error>", Result_String(res) );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "</action>" );

            Conn.Clear_Connection();
            continue;
         }

         // Process Auxilliary Tasks
         //
         res = Conn.Process_Aux();

         if ( RFAILED(res) )
         {
            char* reason = "error on aux";
         
            Sys.Message ( CHANNEL_CONNECTIONLOG, "<action name=\"free_connection\">" );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <handle>%s</handle>", Conn.Get_Auth_User_Handle() );
            //Sys.Message ( CHANNEL_CONNECTIONLOG, "  <ip>%s</ip>", Conn.Get_IP_String() );
            //Sys.Message ( CHANNEL_CONNECTIONLOG, "  <socket>%d</socket>", (int)Conn.Get_Socket() );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <reason>%s</reason>", reason );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "  <error>%s</error>", Result_String(res) );
            Sys.Message ( CHANNEL_CONNECTIONLOG, "</action>" );

            Conn.Clear_Connection();

            return RS_ERR;
         }

      }
   }

   return RS_OK;
   
}

// =------------------------------------------------------------------------
// (public) Process_Heartbeat
//
_result connection_pool::Process_Heartbeat ( )
{
   for ( int i = 0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection ( i );

      if ( Conn.Get_Lock_Status() == CS_FREE )
         continue;

      if ( Conn.Get_Pending_Reverse_Connect() )
         continue;

      timecode Time;

      switch ( Conn.Get_Heartbeat_State() )
         
      {
         case HB_PRIMAL:
         {
            // If we fail to even set this up, blow away the connection
            //
            if ( RFAILED(Conn.Start_Heartbeat ( Time )) )
               Conn.Clear_Connection();

            break;
         }

         case HB_WAITING:
         {
            if ( Time.Is_Expired ( Conn.Get_Heartbeat_Time() ) )
            {
               // NickC 10/22/00 : ALways clear connection if client failed to respond...
               //
               //if ( Conn.Get_Duplicate_Attempt_Count ( ) > 0 )
                  Conn.Clear_Connection();
               //else
               //   Conn.Reset_Heartbeat(); // ok, fine, we'll wait and try again
            }


            break;
         }

         case HB_RECEIVED:
         case HB_RECEIVEDANY:
         {
            Conn.Reset_Heartbeat();
            break;
         }

      } // switch

   } // for

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) N u m _ C o n n e c t i o n s
//
// =---------------------------------------------------------------------------
void connection_pool::Num_Connections ( uint_32& n_busy, uint_32& n_free )
{
   n_busy = n_free = 0;
   
   for ( int i = 0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection ( i );
      
      if ( Conn.Is_Dummy() )
         continue;

      switch ( Conn.Get_Lock_Status() )
      {
         case CS_FREE:
            n_free++;
            break;
         case CS_BUSY:
            n_busy++; 
            break;
      }
   }
}

// =---------------------------------------------------------------------------
// (public) F r e e _ A l l _ C o n n e c t i o n s 
//
// =---------------------------------------------------------------------------
_result connection_pool::Free_All_Connections ( void )
{
   for ( int i=0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection (i);

      if ( Conn.Is_Dummy() )
         continue;

      Conn.Clear_Connection ( );
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) F i n d _ F i r s t _ F r e e _ C o n n e c t i o n
//
// =---------------------------------------------------------------------------
uint_16 connection_pool::Find_First_Free_Connection ( )
{
   for ( int i=0; i < Connection_Count; i++ )
   {
      connection& Conn = Get_Connection ( i );
      
      if ( Conn.Is_Dummy() )
         continue;

      if ( Conn.Get_Lock_Status() != CS_BUSY )
         return i;
   }
   
   return INVALID_CONNECTION_INDEX;
}

// =----------------------------------------------------
// (public) G e t _ C o n n e c t i o n
//
// =----------------------------------------------------
connection_idpcd& connection_pool::Get_Connection ( uint_16 ConnIdx )
{
   if ( ConnIdx < Connection_Count )
   {
      if ( rgConnections[ConnIdx] )
         return *rgConnections[ConnIdx];
   }

   return DummyConnection;
}

// =---------------------------------------------------------------------------
// Get_Connection_Index
//
// =---------------------------------------------------------------------------
uint_16 connection_pool::Get_Connection_Index ( connection_idpcd& Conn )
{
   for ( int i=0; i < Connection_Count; i++ )
   {
      connection& LocalConn = Get_Connection ( i );

      if ( LocalConn.Is_Dummy() )
         continue;

      if ( &LocalConn == &Conn )
         return i;
   }

   return INVALID_CONNECTION_INDEX;
}

// =---------------------------------------------------------------------------
// Resize - resizes the number of connection connections that can be managed at once
//
// =---------------------------------------------------------------------------
bool connection_pool::Resize ( uint_16 new_size )
{
   // We can't resize the connection pool while the server is running!
   //
   if ( Continue_Processing ) return false;

   if ( new_size == Connection_Count ) return true;

   if ( rgConnections )
   {
      for ( int i=0; i < Connection_Count; i++ )
      {
         rgConnections[i]->Clear_Connection ( );
         delete rgConnections[i];
      }

      delete [] rgConnections;
      rgConnections = 0;
   }

   if ( new_size )
   {
      rgConnections = new connection_idpcd*[new_size];
      
      for ( int i=0; i < new_size; i++ )
      {
         rgConnections[i] = new connection_idpcd; 
      }

      Connection_Count = new_size;
   }
   
   return true;
}

// =---------------------------------------------------------------------------
// R e p o r t _ C o n n e c t i o n s
//
// =---------------------------------------------------------------------------
void connection_pool::Report_Connections ( void )
{
   uint_16 num_displayed = 0;

   for ( int i = 0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection (i);

      if ( Conn.Get_Lock_Status() == CS_BUSY )
      {
         Conn.Report_Connection();
         num_displayed++;
      }
   }

   if ( !num_displayed ) Sys.Message ( CHANNEL_NORMAL, "    There are no active clients." );
}

// =------------------------------------------------------------------------
// (public) D e t e c t _ D u p l i c a t e _ U s e r
//
// Returns RS_OK if a duplicate user was found, RS_ERR otherwise
//
// =------------------------------------------------------------------------
_result connection_pool::Detect_Duplicate_User ( const char* Username )
{
   _result res = RS_ERR;

   for ( int i = 0; i < Connection_Count; i++ )
   {
      connection_idpcd& Conn = Get_Connection (i);

      if ( strcmp ( Conn.Get_Auth_User_Handle(), Username ) == 0 )
      {
         if ( !Conn.Is_CGI_ID() )
         {
            Conn.Increment_Duplicate_Attempt_Count();
            res = RS_OK;
         }
      }
   }

   return res;
}

