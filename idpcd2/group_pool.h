// =---------------------------------------------------------------------------
// g r o u p _ p o o l . h
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

#ifndef __GROUP_POOL_H__
#define __GROUP_POOL_H__

#include "connection_idpcd.h"

/*
#define FGSTATUS_OK                 0
#define FGSTATUS_BADPASSWORD        1
#define FGSTATUS_GROUPFULL          2
#define FGSTATUS_BADMODULE          3
#define FGSTATUS_NEEDLEADER         4
#define FGSTATUS_BADGROUPNAME       5
#define FGSTATUS_MAXGROUPSREACHED   6
*/


// =---------------------------------------------------------------------------
// (global) g r o u p _ p o o l 
//
// =---------------------------------------------------------------------------

class group_pool //: public lockable_object
{
public:

   // =----------------------------------------------------
   // (public) ctor
   group_pool ( );
   ~group_pool ( );

   // =------------------------------------------------------------------------
   // (public) D i s b a n d _ A l l _ G r o u p s
   //
   // Doesn't free connections, just boots all connections out
   // of all their groups
   //
   // =------------------------------------------------------------------------
   _result Disband_All_Groups ( void );

   // =------------------------------------------------------------------------
   // (public) F i n d _ F i r s t _ F r e e _ G r o u p
   //
   // Finds the first group object in the pool that's free for reassignment
   // =------------------------------------------------------------------------
   uint_08 Find_Group ( uint_08            Game_Module,
                        const char * const Group_Name,
                        const char * const Group_Password,
                        bool               Leader_OK,
                        bool               Need_Ranked,
                        connection_idpcd&  Conn,       // Who is asking?
                        uint_16&           Found_Group_Index );

   // =------------------------------------------------------------------------
   // (public) N u m _ G r o u p s
   //
   // =------------------------------------------------------------------------
   void Num_Groups ( uint_32& n_normal, uint_32& n_free );

   // =------------------------------------------------------------------------
   // (public) R e s i z e - 
   //
   // resizes the number of groups that can be managed at once, should
   // be synchronized (or at least larger) than the number of connections
   // in the connection_pool
   // =------------------------------------------------------------------------
   bool Resize ( uint_16 new_size );

   // =------------------------------------------------------------------------
   // (public) R e p o r t _ G r o u p s
   //
   // =------------------------------------------------------------------------
   void Report_Groups ( void );

   // =------------------------------------------------------------------------
   // (public) C l e a r _ C o u n t e r s
   //
   // =------------------------------------------------------------------------
   void Clear_Counters ( void ) { Private_Groups_Formed = 0;
                                  Open_Groups_Formed = 0;
                                  Ranked_Groups_Formed = 0; }

   void Get_Group_Connection_Counts ( uint_32& lobby, uint_32& game_setup, uint_32& ranked );

   // =------------------------------------------------------------------------
   // G e t _ G r o u p
   // G e t _ G r o u p _ I n d e x
   //
   idpcd_group& Get_Group       ( uint_16 Group_Index );
   uint_16      Get_Group_Index ( idpcd_group& Group );

   // =------------------------------------------------------------------------
   // (public) G e t _ L o b b y
   //
   idpcd_group& Get_Lobby ( void );
   uint_16      Get_Lobby_Group_Index ( void );
   _result      Send_Game_Groups_Summary ( connection_idpcd& conn_about );

   // =------------------------------------------------------------------------
   // (public) G e t _ XXX _ G r o u p s _ F o r m e d
   //
   // =------------------------------------------------------------------------
   uint_32 Get_Private_Groups_Formed() { return Private_Groups_Formed; }
   uint_32 Get_Open_Groups_Formed()    { return Open_Groups_Formed; }
   uint_32 Get_Ranked_Groups_Formed()  { return Ranked_Groups_Formed; }

protected:
   idpcd_group** rgGroups;
   uint_16       Group_Count;
   uint_32       Private_Groups_Formed;
   uint_32       Open_Groups_Formed;
   uint_32       Ranked_Groups_Formed;
};

#endif //  __GROUP_POOL_H__

