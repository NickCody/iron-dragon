// =---------------------------------------------------------------------------
// i d p c d _ d e f a u l t s . h
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//   Defines default port numbers, client connections, etc.
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

#ifndef __IDPCD_DEFAULTS_H__
#define __IDPCD_DEFAULTS_H__

#define DEF_MAX_REQTYPE_LEN                256
#define DEF_MAX_CLIENT_POOLSIZE           1024
#define DEF_MAX_CONNECTIONS                128
#define DEF_IDPCD_PORT                   44099
//#define DEF_INBOUND_CONNECT_PORT         44100
//#define DEF_START_PLAYER_PORT            44090
#define BUSY_LISTEN_LOOP_TIMEOUT_MS         30
#define MAIN_LISTEN_LOOP_TIMEOUT_MS         30
#define IDLE_LISTEN_LOOP_TIMEOUT_MS       1000
#define HEARTBEAT_EXPIRATION_SECONDS       120
#define HEARTBEAT_CHECK_EXPIRATION_SECONDS  60
#define MAX_RECEIVE_ATTEMPTS              1024
#define RECEIVE_ATTEMPT_DELAY_MILLISECOND  100
#define MAX_PLAYERS_IN_LOBBY               254 //(no higher since a byte is used to index...)
#define MAX_PLAYERS_PER_GROUP                9
#define MAX_PLAYERS_PER_GAME                 6
#define IDPCD_SLOT_ID                        0
#define IDPCD_SCRAMBLE_KEY                0x0D
#define MAX_MSG_LEN                        512

#define HOSTNAME_BUFFER_BYTES              256
#define SERVERNAME_BUFFER_BYTES            256
#define IP_STRING_BYTES                     21

#define INVALID_GROUP_INDEX             0xFFFF
#define INVALID_CONNECTION_INDEX        0xFFFF

// For our Message_* API defined in common.h, we define these CHANNELS 
// to map to the basic types
// Used to Persist a DWORD to the registry or config file

#define CHANNEL_CONNECTIONLOG    CHANNEL_DEBUG1
#define CHANNEL_ACTIVITYLOG      CHANNEL_DEBUG2
#define CHANNEL_ERRORLOG         CHANNEL_DEBUG3
#define CHANNEL_MESSAGE_DETAIL   CHANNEL_DEBUG4
#define CHANNEL_EXCEPTIONLOG     CHANNEL_DEBUG5

#endif // __IDPCD_DEFAULTS_H__



