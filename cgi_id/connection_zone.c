
#include <stdio.h>
#include "cgi_id_subsystems.h"
#include "net_messages.h"
#include "net_messages_admin.h"
//#include "License.h"
#include "connection_zone.h"

// =---------------------------------------------------------------------------
// C Z _ C o n n e c t _ A n d _ A u t h e n t i c a t e
//
// p_auth_reply needs to be allocated to the message's maximum of 1024 bytes.
//
// =---------------------------------------------------------------------------

int CZ_Connect_And_Authenticate ( const char* Username, const char* Password, nm_zcs_authenticate_reply* p_auth_reply )
{
   int      sock;
   int      ret;
   uint_32  ip;

   nm_czs_authenticate_request auth_request;

   sock = SS_Net_Create_Socket ( );

   if ( sock == -1 )
   {
      strcpy ( p_auth_reply->CZ_Message, "Failed to authenticate because of local catastrophic failure." );
      return -1;
   }

   ip = SS_Net_Server_Name_To_IP ( Page_Info.Server_Name  );

   if ( ip == -1 )
   {
      sprintf ( p_auth_reply->CZ_Message, "Could not resolve domain name [%s].", Page_Info.Server_Name );
      return -1;
   }

   ret = SS_Net_Connect ( sock, ip, Page_Info.Port );
   if ( ret != 0 )
   {
      sprintf ( p_auth_reply->CZ_Message, "<b>%s</b> is not responding.", Page_Info.Server_Name );
      return -1;
   }

   // Authentication Request
   //
   auth_request.Message_Type = CZS_AUTHENTICATE_REQUEST;
   SS_Port_Strcpy_Len   ( auth_request.User_Handle,         Username,      USER_HANDLE_BYTES   );
   SS_Port_Strcpy_Len   ( auth_request.Default_Player_Name, "(undefined)", PLAYER_NAME_BYTES     );
   SS_Port_Strcpy_Len   ( auth_request.Password,            Password,      PASSWORD_BUFFER_BYTES );
   SS_Port_Strcpy_Len   ( auth_request.Client_Version,      "cgi_id",      7 );

   //Create_Admin_License ( auth_request.License_Data );
   *(uint_16*)auth_request.Inbound_Port = 0;

   ret = CZ_Send_Message ( sock, (uint_08*)&auth_request, sizeof(auth_request) );
   if ( ret != 0 )
   {
      sprintf ( p_auth_reply->CZ_Message, "Failed to send authenticate message." );

      SS_Net_Close_Socket ( sock );
      return -1;
   }

   // Authentication Reply
   //
   ret = CZ_Receive_Message ( sock, (uint_08*)p_auth_reply, 1024 );
   if ( ret != 0 )
   {
      sprintf ( p_auth_reply->CZ_Message, "Failed to receive server's authenticate reply." );
      SS_Net_Close_Socket ( sock );
      return -1;
   }

   if ( ntohs(*(uint_16*)p_auth_reply->Client_UID) == 0 )
   {
      sprintf ( p_auth_reply->CZ_Message, "Failed to authenticate : final authentication failure. Bad username/password?" );
      SS_Net_Close_Socket ( sock );
      return -1;
   }

   return sock;
}

// =---------------------------------------------------------------------------
// C Z _ S e n d _ M e s s a g e
//
// len is assumed to be hardware format
// Returns 0 on success, -1 on failure
// =---------------------------------------------------------------------------
int CZ_Send_Message ( int sock, uint_08* p_msg, uint_16 len )
{
   int ret;
   nm_header_admin header;

   *(uint_16*)header.Payload_Length = htons(len);
   header.Scramble_key              = CGI_ID_SCRAMBLE_KEY;
   header.Sender_ID                 = 0;

   ret = SS_Net_Send_Buffer ( sock, &header, sizeof(header), 0 );
   if ( ret != sizeof(header) )
      return -1;

   // Scramble the message before we send it
   //
   SS_Net_Scramble_Bytes ( p_msg, len, CGI_ID_SCRAMBLE_KEY );

   ret = SS_Net_Send_Buffer ( sock, p_msg, len, 0 );
   if ( ret != len )
      return -1;

   // Unscramble the message so caller won't be pissed
   //
   SS_Net_Scramble_Bytes ( p_msg, len, CGI_ID_SCRAMBLE_KEY );

   return 0;
}

// =---------------------------------------------------------------------------
// C Z _ R e c e i v e _ M e s s a g e
//
// len is assumed to be hardware format, specifies maximum length of buffer
//   in order to handle dynamically sized messages
// Returns 0 on success, -1 on failure
// =---------------------------------------------------------------------------
int CZ_Receive_Message ( int sock, uint_08* p_msg, uint_16 len )
{
   int ret;
   nm_header_admin header;

   SS_Port_ZeroMemory ( &header, sizeof(nm_header_admin) );

   ret = SS_Net_Receive_Buffer ( sock, &header, sizeof(header), 0 );
   if ( ret != sizeof(header) )
      return -1;

   //len = (uint_16) min ( len, ntohs(*(uint_16*)header.Payload_Length) );

   if ( ntohs(*(uint_16*)header.Payload_Length) < len )
      len = ntohs(*(uint_16*)header.Payload_Length);


   ret = SS_Net_Receive_Buffer ( sock, p_msg, len, 0 );

   if ( ret != len ) return -1;

   // Unscramble the message before we send it
   //
   SS_Net_Scramble_Bytes ( p_msg, len, header.Scramble_key );

   return 0;
}
