// =---------------------------------------------------------------------------
// u n i q u e _ i d . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//   Generate_Unique_User_ID generates a unique user/grup id
//   Yeh, plus it is thread safe.
// 
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Aug 31   nic   Created.
//

#include "PreComp.h"
#include "unique_id.h"

// =---------------------------------------------------------------------------
// 
//
uint_16 Last_Unique_User  = 2; // 0 and 1 are reserved, 2 should be the first id given 
uint_16 Last_Unique_Group = 2;

// =---------------------------------------------------------------------------
// (public, static) G e n e r a t e _ U n i q u e _ I D
//
// =---------------------------------------------------------------------------
uint_16 Generate_Unique_ID ( uid_type type )
{
   if ( type == UID_GROUP )
      return Last_Unique_Group++;
   else
      return Last_Unique_User++;
}
