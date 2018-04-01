// =---------------------------------------------------------------------------
// r a n k _ c a l c . c
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Iron Dragon Server Daemon Startup Routines
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Jul 11   nic   Created.
//

#include <math.h>
#include <memory.h>
#include "standard_types.h"
#include "rank_calc.h"


int   Rating_Strip_Player        ( int rg_dest [], int rg_ranks[], int Remove );
void  Rating_Copy_Array          ( int rg_dest [], int rg_src[] );
int   Rating_Count_Array         ( int rg_array[] );

// =---------------------------------------------------------------------------
// R a t i n g  _ C a l c _ A B
//
// Returns the probability that A will beat B
//
float Rating_Calc_AB ( int rating_a, int rating_b )
{
   float D = (float)rating_a - (float)rating_b;
   return 1.0f / ( 1.0f + (float)pow(10.0f,(-D/400.0f)) );
}

// =---------------------------------------------------------------------------
// R a t i n g _ A d j u s t
//
// Returns the probability that rg_ratings[0] will beat remaining players.
// Currently, we find the player from 1..count with the best rating and use that
//
int Rating_Adjust ( int Start_Rating, float Win_Probability, int Win )
{
   float New_Rating = (float)Start_Rating;
   float Factor = 20.0f;

   if ( Start_Rating < RATING_INTERMEDIATETHRESHOLD )
      Factor = 50;
   else if ( Start_Rating < RATING_ADVANCEDTHRESHOLD )
      Factor = 30;

   if ( Win )
      New_Rating += Factor * ( 1.0f - Win_Probability);
   else
      New_Rating -= Factor * Win_Probability;

   // Make sure that New_Rating never dips below 0
   //
   return (int)New_Rating >= 0 ? (int)New_Rating : 0;
}

// =------------------------------------------------------------------------
// Resolve_Probability - Resolves the probability that rg_ranks[0] will
//                       beat all other players in a round-robin tournament.
//
//
//
float Rating_Resolve_Probability ( int rg_ranks[RATING_MAXPLAYERS] )
{
   int   rg_temp[RATING_MAXPLAYERS];

   float Total = 0.0f;
   int   Count = Rating_Count_Array ( rg_ranks );
   int   i;

   if ( Count == 0 )
      return 0.0; // we should never get here

   else if ( Count == 1 )
      return 1.0; // 100% chance of winning a solo game!

   else if ( Count == 2 )
      return Rating_Calc_AB ( rg_ranks[0], rg_ranks[1] );

   else if ( Count == 3 )
   {
      float ab = Rating_Calc_AB ( rg_ranks[0], rg_ranks[1] );
      float ac = Rating_Calc_AB ( rg_ranks[0], rg_ranks[2] );
      float cb = Rating_Calc_AB ( rg_ranks[2], rg_ranks[1] );
      float bc = Rating_Calc_AB ( rg_ranks[1], rg_ranks[2] );

      Total =   ab   * (ac)
              + ac   * (ab)
              + 1.0f * (ac*cb + ab*bc);

      return Total /= 3.0f;
   }
   else
   {
      for ( i=1; i < Count; i++ )
      {
         float ab = Rating_Calc_AB ( rg_ranks[0], rg_ranks[i] );

         // creates a new array without i
         //
         if ( Rating_Strip_Player ( rg_temp, rg_ranks, i ) )
         {
            ab *= Rating_Resolve_Probability ( rg_temp );

            Total += ab;
         }
      }
   }

   return Total / (Count-1);
}

// =------------------------------------------------------------------------
// R a t in g _ S t r i p _ P l a y e r
//
//
//
int Rating_Strip_Player ( int rg_dest[], int rg_ranks[], int Remove )
{
   int i = 0, Dest_Index = 0;

   Rating_Clear_Array ( rg_dest );

   for ( i=0; i < RATING_MAXPLAYERS; i++ )
   {
      if ( i != Remove )
         rg_dest[Dest_Index++] = rg_ranks[i];
   }

   return Dest_Index;
}

// =------------------------------------------------------------------------
// R a t i n g _ C o p y _ A r r a y
//
//
//
void Rating_Copy_Array ( int rg_dest[], int rg_src[] )
{
   memcpy ( rg_dest, rg_src, sizeof(int)*RATING_MAXPLAYERS);
}

// =------------------------------------------------------------------------
// R a t i n g _ C l e a r _ A r r a y
//
// 
void Rating_Clear_Array ( int rg_array[] )
{
   int i = 0;

   for ( i=0; i < RATING_MAXPLAYERS; i++ )
      rg_array[i] = RATING_SENTRY;
}

// =------------------------------------------------------------------------
// R a t i n g _ C o u n t _ A r r a y
//
// 
int Rating_Count_Array ( int rg_array[] )
{
   int i = 0, count = 0;

   for ( i=0; i < RATING_MAXPLAYERS; i++ )
      if ( rg_array[i] != RATING_SENTRY )
         count++;

   return count;
}
