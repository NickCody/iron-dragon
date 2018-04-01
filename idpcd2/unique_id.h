// =---------------------------------------------------------------------------
// u n i q u e _ i d . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//   Generate_Unique_User_ID generates a unique user id... really?
//   Yeh, plus it is thread safe.
// 
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Aug 31   nic   Created.
//

#ifndef __UNIQUE_ID_H__
#define __UNIQUE_ID_H__

extern class lockable_object  Lock_UID_Generate;
extern uint_16                Last_Unique_User;

typedef enum uid_type
{
   UID_GROUP      = 0,
   UID_CONNECTION = 1
} uit_type;

uint_16 Generate_Unique_ID ( uid_type type );

#endif // __UNIQUE_ID_H__
