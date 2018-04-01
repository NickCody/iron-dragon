// =---------------------------------------------------------------------------
// r a n k _ c a l c . h 
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the group pool, which is very similar to the connection_pool.
//    
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

#ifndef __RATING_CALC_H__
#define __RATING_CALC_H__

#include "standard_types.h"

enum 
{
   RATING_SENTRY                 = -1,
   RATING_MAXPLAYERS             = 6,
   RATING_JUMPSTART              = 1000,
   RATING_INTERMEDIATETHRESHOLD  = 1000,
   RATING_ADVANCEDTHRESHOLD      = 2000

};

#ifdef __cplusplus
   extern "C"
   {
#endif

      float Rating_Calc_AB             ( int rank_a, int rank_b );
      float Rating_Resolve_Probability ( int rg_ranks[RATING_MAXPLAYERS] );
      int   Rating_Adjust              ( int Start_Rating, float Win_Probability, int Win );
      void  Rating_Clear_Array         ( int rg_array[] );
      int   Rating_QSort               ( const void* elem1, const void* elem2 );


#ifdef __cplusplus
   }
#endif

typedef struct user_rating_element
{
   uint_32 Index;
   int     Rating;
   int     Wins;
   int     Losses;
   int     Disconnects;
} user_rating_element;

#endif // __RATING_CALC_H__
