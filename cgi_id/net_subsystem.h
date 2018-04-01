
#ifndef __NET_SUBSYSTEM_H__
#define __NET_SUBSYSTEM_H__

#if UNIX
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#elif WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#endif


#if UNIX
   typedef void* HANDLE;
   #define SOCKET           int
   #define INVALID_SOCKET   (SOCKET)(~0)
   #define SOCKET_ERROR             (-1)
   #define MAX_PATH                 255
   //#define MAP_FAILED              ((caddr_t)-1)
#elif WIN32
   #define MAP_FAILED              (0)
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

int      SS_Net_Create_Socket     ( );
int      SS_Net_Connect           ( int sock, uint_32 ip, uint_16 port );
int      SS_Net_Receive_Buffer    ( int sock, void* buf, int len, int flags );
int      SS_Net_Receive_Buffer_Ex ( int sock, void* buf, int len, int flags );
int      SS_Net_Send_Buffer       ( int sock, void* buf, int len, int flags );
int      SS_Net_Send_Buffer_Ex    ( int sock, void* buf, int len, int flags );
int      SS_Net_Close_Socket      ( int sock );
int      SS_Net_Blocking_Socket   ( int sock, int blocking );
int      SS_Net_Print_IP          ( char* p_string, uint_32 ip );
int      SS_Net_Print_IP_Addr     ( char* p_string, struct sockaddr_in* p_addr );
uint_32  SS_Net_Server_Name_To_IP ( const char* p_servername );
void     SS_Net_Scramble_Bytes    ( uint_08 *m, uint_16 m_len, uint_08 key );
int      SS_Net_Send_Mail         ( const char* SMTP_Server,
                                    const char* From,
                                    const char* To,
                                    const char* Subject,
                                    const char* Message );

#ifdef __cplusplus
}
#endif

#endif // __NET_SUBSYSTEM_H__

