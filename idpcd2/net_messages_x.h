// =---------------------------------------------------------------------------
// n e t _ m e s s a g e s _ x . h
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Nick's extensions to net_messages.h. I might never use this stuff, but, hell,
//    its here to look at.
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Nov 30   nic   Created.
//

#ifndef __NET_MESSAGES_X_H__
#define __NET_MESSAGES_X_H__

#include "idpcd_defaults.h"
#include "net_messages.h"

extern uint_32 Report_Raw_Bytes;
class connection_idpcd;

// Network-byte-ordered message header (as per spec)
//
#if UNIX
   #define PACKED __attribute__((packed))
#elif WIN32
   #pragma pack ( push ,1 )
   #define PACKED
#endif

struct nm_header
{
   uint_08  Payload_Length[2]   PACKED;
   uint_08  Scramble_Key        PACKED;
   uint_08  Sender_ID           PACKED;
};

struct message_package
{
   message_package ( uint_08* data )
   {
      memset ( this, 0, sizeof(message_package) );
      p_data = data;
   }

   nm_header         hdr; 

   uint_08*          p_data;     // does *not* include header

   message_package*  p_next;     // Used by the message queue only
   message_package*  p_prev;     // Used by the message queue only
};


#if WIN32
#pragma pack ( pop )
#endif

// =---------------------------------------------------------------------------
// MSB - returns the most-significant byte of the specified hardware-formatted
//       WORD, n
// =---------------------------------------------------------------------------
inline uint_08 MSB ( uint_16 n )
{
   return (n >> 8);
}

// =---------------------------------------------------------------------------
// MSB - returns the least-significant byte of the specified hardware-formatted
//       WORD, n
// =---------------------------------------------------------------------------
inline uint_08 LSB ( uint_16 n )
{
   return (n & 0xFF);
}

inline uint_16 Make_Hardware_Word ( uint_08 msb_lsb[2] )
{
   return msb_lsb[0]*256 + msb_lsb[1];
}

inline uint_16 Make_Network_Word ( uint_08 msb_lsb[2] )
{
   return htons ( Make_Hardware_Word(msb_lsb) );
}

inline uint_16 NM_Payload_Length ( nm_header* p_Hdr )
{
   return Make_Hardware_Word ( p_Hdr->Payload_Length );
}

inline uint_08 NM_Scramble_Key ( nm_header* p_Hdr )
{
   return p_Hdr->Scramble_Key;
}

inline uint_08 NM_Sender_ID ( nm_header* p_Hdr )
{
   return p_Hdr->Sender_ID;
}

inline void NM_Write_Header ( nm_header* p_Hdr, uint_16 Payload_Length, uint_08 Sender_ID = IDPCD_SLOT_ID, uint_08 Scramble_Key = IDPCD_SCRAMBLE_KEY )
{
   p_Hdr->Payload_Length[0]  = MSB ( Payload_Length );
   p_Hdr->Payload_Length[1]  = LSB ( Payload_Length );
   p_Hdr->Scramble_Key       = Scramble_Key;
   p_Hdr->Sender_ID          = Sender_ID;
}

// =---------------------------------------------------------------------------
// Reporting Functions
//

struct message_package;

const uint_08 * const Message_Name ( uint_08 Message_Type, uint_08 Message_Type_Ex = 0 );
const uint_08 * const Message_ID   ( uint_08 Message_Type, uint_08 Message_Type_Ex = 0 );
void Report_Message                ( connection_idpcd& Conn, message_package& pkg, bool Is_Scrambled, bool Is_Send, bool Did_Succeed );
void Report_Message_Specifics      ( message_package& pkg, bool Is_Scrambled );
void Report_Message_Bytes          ( message_package& pkg, bool Is_Scrambled );
_result Scramble_Bytes             ( uint_08 *m, uint_16 m_len, uint_08 key /*= IDPCD_SCRAMBLE_KEY*/ );


#endif // __NET_MESSAGES_X_H__
