
#ifndef __CONNECTION_ZONE_H__
#define __CONNECTION_ZONE_H__

#include "net_messages.h"
#include "net_messages_admin.h"

int CZ_Connect_And_Authenticate ( const char* Username, const char* Password, nm_zcs_authenticate_reply* p_auth_reply );
int CZ_Send_Message             ( int sock, uint_08* p_msg, uint_16 len );
int CZ_Receive_Message          ( int sock, uint_08* p_msg, uint_16 len );

#endif // __CONNECTION_ZONE_H__