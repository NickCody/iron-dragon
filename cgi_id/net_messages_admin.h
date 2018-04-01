// ----------------------------------------------------------------------------
//   n e t _ m e s s a g e s _ a d m i n . h
//
//   Message definitions for all admin messages. See net_messages.h for
//   more information on the game client messages.
//
//   We use the exact same header, as follows:
//
//      uint_08  Payload data length MSB
//      uint_08  Payload data length LSB
//      uint_08  Scramble key
//      uint_08  Sender ID
//
//    (again, see net_messages.h for more information)
//
//
//   The administration messages start with the client-like Message_Type byte,
//   but this byte is always 0xFF. When we encounter a 0xFF, we look at the next
//   byte for the messages real type
//
//
//   (c) 1999-2000 by Martin R. Szinger and Nicholas Codignotto
//
//   --------------------------------------------------------------------------
//
//   Date        Who   Description
//   ----------  ---   --------------------------------------------------------
//   2000-01-20  NIC   Created (Eden's PC version of Mayfair's Iron Dragon).
//   --------------------------------------------------------------------------

#ifndef  INC_NET_MESSAGES_ADMIN_H
#define  INC_NET_MESSAGES_ADMIN_H

// ----------------------------------------------------------------------------
#include "standard_types.h"
#include "net_messages.h"
#include "db_client.h"        // c++ features are excluded by __cplusplus symbol check

// ----------------------------------------------------------------------------
enum nm_type_admin
{
   ADMIN_TO_CZ_BASE  = 0,        ACZ_INFO          =  ADMIN_TO_CZ_BASE +   1,
                                 ACZ_NUMUSERS      =  ADMIN_TO_CZ_BASE +   2,
                                 ACZ_GETUSERS      =  ADMIN_TO_CZ_BASE +   3,
                                 ACZ_GETLOG        =  ADMIN_TO_CZ_BASE +   4,

   CZ_TO_ADMIN_BASE  = 32,       CZA_INFO          =  CZ_TO_ADMIN_BASE +   1,
                                 CZA_GETLOG        =  CZ_TO_ADMIN_BASE +   2,

   BIDIR_ADMIN_BASE  = 64,       BIDIR_NUMUSERS    =  BIDIR_ADMIN_BASE +   1,
                                 BIDIR_GETUSERS    =  BIDIR_ADMIN_BASE +   2,
                                 BIDIR_ADDUSER     =  BIDIR_ADMIN_BASE +   3,
                                 BIDIR_SETUSER     =  BIDIR_ADMIN_BASE +   4,
                                 BIDIR_LOOKUPUSER  =  BIDIR_ADMIN_BASE +   5,
   LAST = 0xFF
};

// ----------------------------------------------------------------------------
enum nm_admin_lengths
{
   SERVER_NAME_BYTES = 36,
   DATETIME_BYTES    = 32
};

// ----------------------------------------------------------------------------
typedef struct nm_header_admin
{
      uint_08  Payload_Length[2];
      uint_08  Scramble_key     ;
      uint_08  Sender_ID        ;                  // Always 0
} nm_header_admin;

// --------------------------------------------------------------------------
typedef struct nm_acz_info
{
   uint_08  Message_Type        ;                  // always 0xFF
   uint_08  Message_Type_Ex     ;                  // ACZ_IINFO
} nm_acz_info;

// ----------------------------------------------------------------------------
// The total size of this structure should be 128 bytes */
// The server replies with this when asked for server stats (ACZ_IINFO)
//
// ----------------------------------------------------------------------------

typedef struct nm_cza_info
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // CZA_INFO
   uint_08  Server_Name[SERVER_NAME_BYTES];
   uint_08  Start_Time[DATETIME_BYTES];
   uint_08  Active_Connections[2];           // MSB, LSB
   uint_08  Active_Groups[2];                // MSB, LSB
   uint_08  Total_Connections[4];            // DWORD (MSB-first), cumulative since start
   uint_08  Total_Connections_Authenticated[4]; // DWORD (MSB-first), cumulative since start
   uint_08  Total_Private_Groups[4];         // DWORD (MSB-first), cumulative since start
   uint_08  Total_Open_Groups[4];            // DWORD (MSB-first), cumulative since start
   uint_08  Total_Ranked_Groups[4];          // DWORD (MSB-first), cumulative since start
   uint_08  Extra[38];
} nm_cza_info;

// Get log filename protocol
//
typedef struct nm_acz_getlog
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // ACZ_GETLOG
} nm_acz_getlog;

typedef struct nm_cza_getlog
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // CZA_GETLOG
   uint_08  Log_Filename[256];
} nm_cza_getlog;

// ----------------------------------------------------------------------------
// Use this message to ask the server how many userss are in its database
// Server will reply with BIDIR_NUMUSERS
// ----------------------------------------------------------------------------
typedef struct nm_bidir_numusers
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // BIDIR_NUMUSERS
   uint_08  Num_Users[4];                    // DWORD MSB first, unused in ACZ direction
} nm_bidir_numusers;

// ----------------------------------------------------------------------------
// Use this message to get for a specific range of users
// Server will reply with BIDIR_GETUSERS, memory after this will be that
// many user records
// ----------------------------------------------------------------------------
typedef struct nm_bidir_getusers
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // BIDIR_GETUSERS
   uint_08  Start_User[4];                   // DWORD (MSB-first) Index of first user to return
   uint_08  End_User[4];                     // DWORD (MSB-first) Index of last user to return
   uint_08  Rank_Sorted;                     // if 0, in record order, if 1, in ranked order
} nm_bidir_getusers;

// ----------------------------------------------------------------------------
// One or more of these immediately follow nm_bidir_getusers in stream
// from server.
//
// ----------------------------------------------------------------------------
typedef struct nm_user_record
{
   uint_08  index    [4];                       // DWORD, MSB-first, index in db
   client_record     record;
} nm_user_record;

// ----------------------------------------------------------------------------
// 
// BIDIR_ADDUSER : Use this message to add a new user
//   Server will reply with BIDIR_ADDUSER, user record index = 0 on successful
//   add of new user, -1 on failure. User-Index is ignored on send.
//
// BIDIR_SETUSER : Use this message to modify an existing user
//   Server will reply with BIDIR_SETUSER, user record index = 0 on successful 
//   modification of that record, -1 on failure. 
//
// For all uses, on send to server, this message is immediately followed by 
//   a nm_user_record that *is* included in Package_Length
//
// ----------------------------------------------------------------------------
typedef struct nm_bidir_user
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // BIDIR_ADDUSER
   uint_08  Success_Code;                    // server sends 0 on success, -1 on error
   uint_08  Recnum[4];                       // network-byte order (MSB), used for BIDIR_SETUSER
} nm_bidir_user;

// ----------------------------------------------------------------------------
// BIDIR_LOOKUPUSER : Use this message to lookup a user via a username
//   Server will reply with BIDIR_LOOKUPUSER, where this message is immediately
//   followed by a nm_user_record
//
// ----------------------------------------------------------------------------
typedef struct nm_bidir_lookupuser
{
   uint_08  Message_Type;                    // always 0xFF
   uint_08  Message_Type_Ex;                 // BIDIR_ADDUSER
   char     Username[PLAYER_NAME_BYTES];
} nm_bidir_lookupuser;

#endif // INC_NET_MESSAGES_ADMIN_H


