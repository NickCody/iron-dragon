// =---------------------------------------------------------------------------
// r a n k i n g s . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Declares public rankings functions.
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Apr 24   nic   Created.
//

#ifndef __RANKINGS_H__
#define __RANKINGS_H__

typedef enum RANKINGS_STATE
{
   RANKINGS_STATE_NORMAL   = 0,
   RANKINGS_STATE_INACTIVE = 1,
   RANKINGS_STATE_DELETED  = 2
} RANKINGS_STATE;


#if UNIX
   #define PACKED __attribute__((packed))
#elif WIN32
   #pragma pack ( push ,1 )
   #define PACKED
#endif

typedef struct db_rankings
{
   sint_32 Rank                   PACKED;
   char    Player_Username [ 24 ] PACKED; // USERNAME_BYTES
   char    Last_Play [ 32 ]       PACKED;
   uint_08 State                  PACKED;
   uint_08 Slush [ 67 ]           PACKED; // to make entire record 128 bytes

} db_rankings;

#if UNIX
   #undef PACKED
#elif WIN32
   #pragma pack ( pop )
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int Write_Rankings_Table ( int Num_Rankings );

#ifdef __cplusplus
}
#endif

#endif // __RANKINGS_H__
