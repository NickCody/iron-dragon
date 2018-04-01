// =---------------------------------------------------------------------------
// l i s t e n i n g . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Listening Thread main loop. startup_daemon.h contains most of our useful 
//    declarations
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Aug 10   nic   Created.
//

// =---------------------------------------------------------------------------
// Include our standard headers
//
// =---------------------------------------------------------------------------

#include "PreComp.h"

#include "idpcd_defaults.h"
#include "startup_daemon.h"
#include "connection_pool.h"
#include "idpcd_defaults.h"
#include "unique_id.h"
#include "stack_tag.h"

// =---------------------------------------------------------------------------
// Forward Decls
//
void Init_ListenInfo_IPs ( listen_info& li, const hostent* host );

// =---------------------------------------------------------------------------
// (global) P r o c e s s _ L i s t e n i n g
//
// Accepts connections and updates connection pools
//
// When we accept a new socket connection we spawn a thread to
//    handle that connection.
//
// =---------------------------------------------------------------------------

_result Process_Listening ( listen_info& li )
{
   SOCKET            listen_sock    = (SOCKET)li.Listen_Sock;
   SOCKET            data_sock      = INVALID_SOCKET;
   int               sock_err       = 0;

#if UNIX
   unsigned          addr_len       = sizeof(sockaddr_in);
#else
   int               addr_len       = sizeof(sockaddr_in);
#endif

   sockaddr_in       addr;
   char              ip[IP_STRING_BYTES];

   data_sock = accept ( listen_sock, (sockaddr*)&addr, &addr_len );

   SS_Net_Print_IP_Addr ( (char*)ip, &addr );

   // Handle any errors, or silently exit if EWOULDBLOCK
   //
   if ( data_sock == SOCKET_ERROR || !data_sock )
   {
      #if UNIX
         
      if ( errno == EWOULDBLOCK )
         return RS_OK;

      sock_err = errno;

      #elif WIN32

      sock_err = WSAGetLastError();

      if ( sock_err == WSAEWOULDBLOCK )
         return RS_OK;

      #endif

      return RS_BADSOCKET;
   }
   else
   {
      uint_16 FreeConnIdx = Busy_Pool.Find_First_Free_Connection ( );

      if ( FreeConnIdx == 0xFFFF  )
      {
         SS_Net_Close_Socket ( data_sock );
         return RS_OK;
      }

      // Get and Reserve the connection
      //
      connection& Conn = Busy_Pool.Get_Connection ( FreeConnIdx );
      Conn.Make_Busy_Connection ( data_sock, addr );

      // Immediately make new socket blocking since it inherited nonblocking
      // from listen_sock
      //
      SS_Net_Blocking_Socket ( data_sock, false );

      stacktag Connect ( CHANNEL_ACTIVITYLOG, "action", "name=\"connect\"" );
      Sys.Message ( CHANNEL_ACTIVITYLOG, "<ip>%s</ip>", ip );
      Sys.Message ( CHANNEL_ACTIVITYLOG, "<socket>%d</socket>", (int)data_sock );
      //Sys.Message ( CHANNEL_ACTIVITYLOG, "<error>%s</error>", Socket_Error_String(sock_err) );
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (global) S e t u p _ L i s t e n i n g _ S o c k e t
//
// also note that we will report error messages here so the caller need
// not do so unless they have something important to add to what we say here
// =---------------------------------------------------------------------------
_result Setup_Listening_Socket ( uint_16 port, listen_info& li )
{
   int ret;

   // Create the listening socket
   //
   li.Listen_Port = port;
   li.Listen_Sock = socket ( AF_INET, SOCK_STREAM, 0 );

   if ( li.Listen_Sock == INVALID_SOCKET )
   {
      Sys.Message ( CHANNEL_DEBUG, "Setup_Listening_Socket<Listening> : failed" );
      return RS_BADSOCKET;
   }

   // Mark the listening socket as non-blocking
   //
   ret = SS_Net_Blocking_Socket ( li.Listen_Sock, false );

   if ( ret == SOCKET_ERROR )
   {
      Sys.Message ( CHANNEL_DEBUG, "Setup_Listening_Socket<Listening,FIONBIO> : failed" );
      SS_Net_Close_Socket ( li.Listen_Sock );
      return RS_BADSOCKET;
   }

   // Get the host name
   //
   char hostname[256];

   if ( SOCKET_ERROR == gethostname ( hostname, 256 ) )
   {
      Sys.Message ( CHANNEL_DEBUG, "Setup_Listening_Socket<Listening,gethostbyname> : failed" );
      SS_Net_Close_Socket ( li.Listen_Sock );
      return RS_BADSOCKET;
   }
   
   // Get IP address information now that we have host name
   //
   hostent* host = gethostbyname ( hostname );

   if ( !host || !host->h_addr_list || !host->h_addr_list[0])
   {
      Sys.Message ( CHANNEL_DEBUG, "Setup_Listening_Socket<Listening,gethostbyname> : failed" );
      SS_Net_Close_Socket ( li.Listen_Sock );
      return RS_BADSOCKET;
   }

   Init_ListenInfo_IPs ( li, host );

   // Bind to the [default] TCP port
   //
   Sock_Addr.sin_family = AF_INET;
   Sock_Addr.sin_port   = htons(port);
   Sock_Addr.sin_addr.s_addr = htonl(INADDR_ANY);   
   
   int error = bind ( li.Listen_Sock, (sockaddr*)&Sock_Addr, sizeof(Sock_Addr) );
   
   if ( 0 != error )
   {
      Sys.Message ( CHANNEL_DEBUG, "Setup_Listening_Socket<Listening,bind> : failed" );
      SS_Net_Close_Socket ( li.Listen_Sock );
      return RS_BADSOCKET;
   }
   
   // begin listening on the socket, calls to accept can now succeed
   //
   if ( SOCKET_ERROR == listen ( li.Listen_Sock, 10 ) )
   {
      Sys.Message ( CHANNEL_DEBUG, "Setup_Listening_Socket<Listening,listen> : failed" );
      SS_Net_Close_Socket ( li.Listen_Sock );
      return RS_BADSOCKET;
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (global) I n i t _ L i s t e n I n f o _ I P s
//
void Init_ListenInfo_IPs ( listen_info& li, const hostent* host )
{
   // first, count the number of ips...
   //
   uint_16 count = 0;
   char**  p_addr = host->h_addr_list;
   while ( *p_addr )
   {
      count++;
      p_addr++; 
   }

   if ( !count ) return;

   // Now, store each in our listeninfo
   //
   li.Listen_IP_Count = count;
   li.Listen_IPs = new uint_32[count];
   strcpy ( li.Listen_Host_Name, host->h_name );

   for ( uint_16 i=0; i < count; i++ )
      li.Listen_IPs[i] = *(uint_32*)host->h_addr_list[i];
}
