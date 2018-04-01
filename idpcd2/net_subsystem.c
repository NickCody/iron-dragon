// =---------------------------------------------------------------------------
// n e t _ s u b s y s t e m . h
// 
//   (C) 1999-2001 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE         WHO   REVISION
//   -----------  ---   --------------------------------------------------------
//   2000 Jan 20  nic   Created.
//

#ifdef UNIX
#include <unistd.h>
#include <string.h>
#endif

#include <stdio.h>
#include <time.h>
#include "standard_types.h"
#include "port_subsystem.h"
#include "net_subsystem.h"

#ifdef WIN32

int Winsock_Initialized = 0;

// =---------------------------------------------------------------------------
// (global) I n i t i a l i z e _ W i n s o c k
//
int Initialize_Winsock ( )
{
   if ( !Winsock_Initialized )
   {
      // Perform Win32-specific Initialization
      //
      WORD wVersionRequested = MAKEWORD( 2, 2 ); 
      WSADATA wsaData;

      Winsock_Initialized = (0 == WSAStartup( wVersionRequested, &wsaData ));
   }

   return Winsock_Initialized;
}
#endif

// =---------------------------------------------------------------------------
// (global) S S _ N e t _ C r e a t e _ S o c k e t
//
// (Win32) If no error occurs, socket returns a descriptor referencing the new 
//         socket. Otherwise, a value of INVALID_SOCKET is returned, and a 
//         specific error code can be retrieved by calling WSAGetLastError.
//
//

int SS_Net_Create_Socket ( )
{

#ifdef WIN32
   if ( !Initialize_Winsock ( ) )
      return -1;
#endif

   return socket ( AF_INET, SOCK_STREAM, 0 );
}

// =---------------------------------------------------------------------------
// (global) S S _ N e t _ C o n n e c t
//
// ip andport are assumed to be in network byte order
// Returns 0 on success, -1 on error
//
int SS_Net_Connect ( int sock, uint_32 ip, uint_16 port )
{
   struct sockaddr_in addr;
   int         addr_len;
   int         ret;
   
   // Setup address
   //
   addr_len                  = sizeof(struct sockaddr_in);
   addr.sin_port             = port;
   addr.sin_family           = AF_INET;

#if WIN32
   addr.sin_addr.S_un.S_addr = ip;
#elif UNIX
   addr.sin_addr.s_addr = ip;
#endif

   ret = connect ( sock, (const struct sockaddr*)&addr, addr_len );

   return ret;
}

// =---------------------------------------------------------------------------
// (global) S S _ N e t _ R e c e i v e _ B u f f e r
//
// This version doesn't loop until all data specified has been received
// See SS_Port_Receive_Buffer_Ex for that
//
int SS_Net_Receive_Buffer ( int sock, void* buf, int len, int flags )
{
   int ret = recv ( sock, (char*)buf, len, flags );

   if ( ret == SOCKET_ERROR )
   {
#if UNIX
      // TODO: need to handle zero reads / reset connections like we do in Win32
      return 0;
#elif WIN32
      int last_error = WSAGetLastError ( );
      switch ( last_error )
      {
         case WSAEWOULDBLOCK:

         case WSAESHUTDOWN:
         case WSAECONNRESET:     // server disconnected us?
            return 0;
      }
#endif
   }

   return ret;
}

// =---------------------------------------------------------------------------
// (global) S S _ P o r t _Net_R e c e i v e _ B u f f e r _ E x
//
int SS_Net_Receive_Buffer_Ex ( int sock, void* buf, int len, int flags )
{
   int ret_bytes = 0;
   while ( ret_bytes < len )
   {
      int ret = recv ( sock, ((char*)buf) + ret_bytes, len-ret_bytes, flags );

      if ( ret == SOCKET_ERROR )
      {
#if UNIX
         // TODO: need to handle zero reads / reset connections like we do in Win32
         return 0;
#elif WIN32
         int last_error = WSAGetLastError ( );
         switch ( last_error )
         {
            case WSAESHUTDOWN:
            case WSAECONNRESET:     // server disconnected us?
               return 0;
         }
#endif
         return -1;
      }

      if ( ret == 0 )
         break;

      ret_bytes += ret;
   }

   return ret_bytes;
}

// =---------------------------------------------------------------------------
// (global) S S _ N e t _ S e n d _ B u f f e r
//
// This version does not attempt to keep looping until send() sent all 
// requested bytes. For that see SS_Port_Send_Buffer_Ex

int SS_Net_Send_Buffer ( int sock, void* buf, int len, int flags )
{
   int ret = send ( sock, (char*)buf, len, flags );

   if ( ret == SOCKET_ERROR )
   {
#if UNIX
      // TODO: #error We need to implement UNIX error code for send()
      return 0;
#elif WIN32
      int last_error = WSAGetLastError ( );
      switch ( last_error )
      {
         case WSAESHUTDOWN:
         case WSAECONNRESET:     // server disconnected us?
            return 0;
      }
#endif
   }

   return ret;
}

// =---------------------------------------------------------------------------
// (global) S S _ N e t _ S e n d _ B u f f e r _ E x
//
int SS_Net_Send_Buffer_Ex ( int sock, void* buf, int len, int flags )
{
   int ret_bytes = 0;
   int ret = 0;

   while ( ret_bytes < len )
   {
      ret = send ( sock, ((char*)buf) + ret_bytes, len-ret_bytes, flags );

      if ( ret == SOCKET_ERROR )
      {

#ifdef WIN32
         int last_error = WSAGetLastError ( );

         switch ( last_error )
         {
            case WSAESHUTDOWN:
            case WSAECONNRESET:     // server disconnected us?
               return 0;
         }
#endif

         return -1;
      }

      if ( ret == 0 )
         break;

      ret_bytes += ret;
   }

   return ret_bytes;
}

// =---------------------------------------------------------------------------
// S S _ N e t _ C l o s e _ S o c k e t
//
int SS_Net_Close_Socket ( int sock )
{
   int ret;

   shutdown ( sock, 2 /*SD_BOTH*/);

#if UNIX
   ret = close( sock );
#elif WIN32
   ret = closesocket ( sock );
#endif

   return ret;
}

// =---------------------------------------------------------------------------
// P o r t _ N e t _ B l o c k i n g _ S o c k e t
//
int SS_Net_Blocking_Socket ( int sock, int blocking )
{
   uint_32 uval = blocking ? 0 : 1;

#if UNIX
   return ioctl ( sock, FIONBIO, &uval );
#elif WIN32
   return ioctlsocket ( sock, FIONBIO, &uval );
#endif
}

// =---------------------------------------------------------------------------
// P o r t _ N e t _ P r i n t _ I P
//
// ip address is assumed to be in network-byte order
// =---------------------------------------------------------------------------
int SS_Net_Print_IP ( char* p_string, uint_32 ip )
{
   return sprintf ( p_string, "%lu.%lu.%lu.%lu",
      //(ip >> 24), 
      //(ip & 0x00FFFFFF) >> 16,
      //(ip & 0x0000FFFF) >> 8,
      //(ip & 0x000000FF) );

      (ip & 0x000000FF),
      (ip & 0x0000FFFF) >> 8,
      (ip & 0x00FFFFFF) >> 16,
      (ip >> 24) );
}

int SS_Net_Print_IP_Addr ( char* p_string, struct sockaddr_in* p_addr )
{
   #if UNIX
      SS_Net_Print_IP ( p_string, p_addr->sin_addr.s_addr );
   #elif WIN32
      SS_Net_Print_IP ( p_string, p_addr->sin_addr.S_un.S_addr );
   #endif

   return 0;
}

// =---------------------------------------------------------------------------
// S S _ N e t _ S e r v e r _ N a m e _ T o _ I P
//
// Returns INADDR_NONE on error, valid network ordered long on success
//
uint_32 SS_Net_Server_Name_To_IP ( const char* p_servername )
{
   // First try and resolve server name as a dotted address (10.0.0.1)
   //
   uint_32 ip_server = inet_addr ( p_servername );
   
   // If that didn't work, Lookup the ip, assuming Server_Name is a domain name
   //
   if ( ip_server == INADDR_NONE )
   {
      struct hostent* he = gethostbyname( p_servername );
      
      if ( !he )
         return INADDR_NONE;

      ip_server = *(uint_32*)he->h_addr_list[0];
   }

   return ip_server;
}


// =---------------------------------------------------------------------------
// S S _ N e t _ S c r a m b l e _ B y t e  s
// =---------------------------------------------------------------------------
void SS_Net_Scramble_Bytes( uint_08 *m, uint_16 m_len, uint_08 key )
{
   uint_32  sval;
   uint_08  bshf;

   if (!key) return;

   //sval = 0x31415918;
   sval = 0xEF5DBAF7;  // NickC 4/20/00 : Changed to protect security (accidental source code post)
   bshf = 0;

   key &= 0x1f;   // modulate the key to the range 0..31

   while (m_len--)
   {
      if (key) {sval = (sval << key) | (sval >> (32 - key));
                key = 0;}

      *m++ ^= (uint_08)((sval >> bshf) & 0xff);

      if (bshf != 24) bshf += 8; else {bshf = 0; key = 1;}
   }
}

// =---------------------------------------------------------------------------
// S S _ N e t _ S e n d _ M  a i l 
// =---------------------------------------------------------------------------
int SS_Net_Send_Mail ( const char* SMTP_Server,
                       const char* From,
                       const char* To,
                       const char* Subject,
                       const char* Message )
{

   int      ret  = 0;
   int      Sock = 0;
   uint_32  ip   = 0;
   char     Command[256];
   char     Response[256];

   #ifdef WIN32
      if ( !Initialize_Winsock ( ) )
         return -1;
   #endif

   ip = SS_Net_Server_Name_To_IP ( SMTP_Server );
   if ( ip == INADDR_NONE ) 
   { 
      // TODO: Fix...
      //SS_HTML_WriteP ( "Failed to resolve domain [%s].", SMTP_Server ); 
      return -1; 
   }

   Sock = SS_Net_Create_Socket ( );
   if ( Sock == 01 ) 
   { 
      // TODO: Fix
      //SS_HTML_WriteP ( "Failed to create socket." ); 
      return -1; 
   }

   ret = SS_Net_Connect ( Sock, ip, htons(25) /*SMTP Port*/ );
   if ( ret == -1 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      //SS_HTML_WriteP ( "Failed to connect to server [%s].", SMTP_Server ); 
      return -1;
   }

   SS_Port_ZeroMemory ( Command, 256  );
   SS_Port_ZeroMemory ( Response, 256 );

	// Read the "HELO" response from the server
   //
   ret = SS_Net_Receive_Buffer ( Sock, Response, 256, 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      //SS_HTML_WriteP ( "Server did not reply." ); 
      return -1; 
   }
   //SS_HTML_WriteP ( "Server sent: %s", Response );

	// and check to see if we're talking to an SMTP server
	// 
	if ( strncmp(Response, "220", 3) != 0 )
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      //SS_HTML_WriteP ( "Did not get 220 (HELO) response from server." ); 
      return -1; 
   }

	// Send the "FROM" line
	// 
   strcpy ( Command, "MAIL FROM:<" );
	strcat ( Command, From );
   strcat ( Command, ">\r\n" );

   ret = SS_Net_Send_Buffer ( Sock, Command, strlen(Command), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      //SS_HTML_WriteP ( "Could not send FROM line." ); 
      return -1; 
   }

	// and check the response
	// 
   ret = SS_Net_Receive_Buffer ( Sock, Response, 256, 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      //SS_HTML_WriteP ( "Did not get a reply after sending FROM line." ); 
      return -1; 
   }
   //SS_HTML_WriteP ( "Server sent: %s", Response );

	if ( strncmp(Response, "250", 3) != 0 )
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      //SS_HTML_WriteP ( "Server rejectsed FROM line." ); 
      return -1; 
   }

	// Send the "RCPT" line
	// 
   strcpy ( Command, "RCPT TO:<" );
	strcat ( Command, To );
   strcat ( Command, ">\r\n" );

   ret = SS_Net_Send_Buffer ( Sock, Command, strlen(Command), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      // SS_HTML_WriteP ( "Could not send RCPT TO line." ); 
      return -1; 
   }

	// and check the response
	// 
   ret = SS_Net_Receive_Buffer ( Sock, Response, 256, 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Did not get a reply after sending RCPT line." ); 
      return -1; 
   }
   //SS_HTML_WriteP ( "Server sent: %s", Response );

	if ( strncmp(Response, "250", 3) != 0 )
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Server rejected Recipient line." ); 
      return -1; 
   }

	// Send the "DATA" line
	// 
   strcpy ( Command, "DATA\r\n" );
   ret = SS_Net_Send_Buffer ( Sock, Command, strlen(Command), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Could not send DATA line." ); 
      return -1; 
   }

	// and check the response
	// 
   ret = SS_Net_Receive_Buffer ( Sock, Response, 256, 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Did not get a reply after sending RCPT line." ); 
      return -1; 
   }
   //SS_HTML_WriteP ( "PServer sent: %s", Response );

	if ( strncmp(Response, "354", 3) != 0 )
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Server rejected DATA line." ); 
      return -1; 
   }

	// Send the "Subject" line
	// 
   strcpy ( Command, "Subject: " );
	strcat ( Command, Subject );
   strcat ( Command, "\r\n" );

   ret = SS_Net_Send_Buffer ( Sock, Command, strlen(Command), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      //SS_HTML_WriteP ( "Could not send Subject line." ); 
      return -1; 
   }

	// No response from server expectd

	// Send the message data
	// This code assumes the message data contains CRLF pairs 
	// where appropriate.
   //
   ret = SS_Net_Send_Buffer ( Sock, (void*)Message, strlen(Message), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Could not send Message part." ); 
      return -1; 
   }

	// No response from server expectd

	// Send the termination line
   //
   strcpy ( Command, "\r\n.\r\n" );

   ret = SS_Net_Send_Buffer ( Sock, Command, strlen(Command), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: FIx
      // SS_HTML_WriteP ( "Could not send Termination line." ); 
      return -1; 
   }

	// and check the response
   //
   ret = SS_Net_Receive_Buffer ( Sock, Response, 256, 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      // SS_HTML_WriteP ( "Did not get a reply after sending RCPT line." ); 
      return -1; 
   }
   //SS_HTML_WriteP ( "Server sent: %s", Response );

	if ( strncmp(Response, "250", 3) != 0 )
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      // SS_HTML_WriteP ( "Server rejected message body." ); 
      return -1; 
   }

	// Send the "QUIT" line
   //
   strcpy ( Command, "QUIT\r\n" );

   ret = SS_Net_Send_Buffer ( Sock, Command, strlen(Command), 0 );
   if ( ret <= 0 ) 
   { 
      SS_Net_Close_Socket(Sock); 
      // TODO: Fix
      // SS_HTML_WriteP ( "Could not send QUIT line." ); 
      return -1; 
   }

   SS_Net_Close_Socket ( Sock );

	return 0;
}
