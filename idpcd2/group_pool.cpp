// =---------------------------------------------------------------------------
// g r o u p _ p o o l . c p p
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the group pool, which is very similar to the connection_pool.
//    In fact, the size of the group pool should be identical to the size of
//    the connection pool, since in a worst-case scenario, every connection
//    might be in their own group.
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
// 
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jul 28   nic   Created.
//

#include "PreComp.h"

#include "idpcd_defaults.h"
#include "idpcd_group.h"
#include "group_pool.h"

// From startup_daemon.cpp, certain operations can't be done if the server
// is running (i.e.: Continue_Processing = true )
//
extern bool Continue_Processing;

idpcd_group Dummy_Group ( false /*lobby*/, true /*dummy*/ );

// =---------------------------------------------------------------------------
// ctor - group_pool
//
// =---------------------------------------------------------------------------
group_pool::group_pool ( )
{
   Group_Count = 0;
   rgGroups = 0;
   
   Resize ( DEF_MAX_CONNECTIONS );

   // Used for server statictics (total groups formed)
   //
   Private_Groups_Formed  =
   Open_Groups_Formed     =
   Ranked_Groups_Formed   = 0;
}

// =---------------------------------------------------------------------------
// dtor - ~ group_pool
//
// =---------------------------------------------------------------------------
group_pool::~group_pool ( )
{
   Resize ( 0 );
}

// =---------------------------------------------------------------------------
// (public) D i s b a n d _ A l l _ G r o u p s
//
// =---------------------------------------------------------------------------
_result group_pool::Disband_All_Groups ( void )
{
   if ( rgGroups )
   {
      for ( uint_16 i=0; i < Group_Count; i++ )
      {
         if ( rgGroups[i] )
            rgGroups[i]->Disband( );
      }
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) F i n d _ G r o u p
//
// FGSTATUS_OK                = 0,  // no error
// FGSTATUS_BADPASSWORD       = 1,  // specified group mismatched on password
// FGSTATUS_GROUPFULL         = 2,  // specified group is full
// FGSTATUS_BADMODULE         = 3,  // group matched, module not supported
// FGSTATUS_NEEDLEADER        = 4,  // no reverse connect, new group requested
// FGSTATUS_BADGROUPNAME      = 5,  // group name invalid, zero length
// FGSTATUS_MAXGROUPSREACHED  = 6,  // new group requested, but no more slots
// =---------------------------------------------------------------------------

uint_08 group_pool::Find_Group ( uint_08            Game_Module,
                                 const char * const Group_Name,
                                 const char * const Group_Password,
                                 bool               Leader_Ok,
                                 bool               Need_Ranked,
                                 connection_idpcd&  Conn,
                                 uint_16&           Found_Group_Index )
{
   uint_16 i = 0;

   Found_Group_Index = 0xFFFF;

   if ( !Group_Name || !strlen(Group_Name) )
      return FGSTATUS_BADGROUPNAME;

   for ( i = 0; i < Group_Count; i++ )
   {
      idpcd_group& Current_Group = Get_Group ( i );

      if ( Current_Group.Is_Dummy() || Current_Group.Is_Lobby() )
         continue;

      if ( strcmp ( Group_Name, (const char *)Current_Group.Get_Group_Name() ) == 0 )
      {
         // first ensure both passwords are non-null
         //
         bool Both_Null     = !Group_Password && !Current_Group.Get_Group_Password();
         bool Null_Mismatch = (!Both_Null && !Group_Password) || (!Both_Null && !Current_Group.Get_Group_Password());

         // If one is null, but not the other, we fail (null and empoty string do not match)
         //
         if ( Null_Mismatch )
            return FGSTATUS_BADPASSWORD;

         // Now check if they match
         //
         if ( Both_Null || 
              (strcmp ( Group_Password, (const char *)Current_Group.Get_Group_Password()) == 0) )
         {
            if ( Current_Group.Is_Joinable() )
            {
               if ( Conn.Does_Support_Module ( Current_Group.Get_Module_ID()) )
                  Found_Group_Index = i;
               else
                  return FGSTATUS_BADMODULE;
            }
            else
               return FGSTATUS_GROUPFULL;
         }
         else
            return FGSTATUS_BADPASSWORD;
      }
   }

   // Check if group found, if so exit now
   //
   if ( Found_Group_Index != 0xFFFF )
      return FGSTATUS_OK;

   // We have to create a group now with these characteristics, bail if Leader_OK is false
   //
   if ( !Leader_Ok )
      return FGSTATUS_NEEDLEADER;

   // Find empty group index and create the group
   //
   for ( i=0; i < Group_Count; i++ )
   {
      idpcd_group& Group = Get_Group ( i );

      if ( Group.Is_Dummy() || Group.Is_Lobby() )
         continue;

      // Only consider primal groups
      //
      if ( Group.Get_Status() == GS_PRIMAL ) 
      {

         // Create and verify a new Group (puts it in GS_READY state)
         //
         Group.Initialize_Group ( Group_Name, Group_Password, Game_Module, Need_Ranked );

         Found_Group_Index = i;

         // TODO: Increment Counters, as appropriate
         //

         return FGSTATUS_OK;
      }

   }
   
   return FGSTATUS_MAXGROUPSREACHED;
}

// =------------------------------------------------------------------------
// (public) G e t _ L o b b y
//
idpcd_group& group_pool::Get_Lobby ( void )
{
   return Get_Group ( 0 );
}

// =------------------------------------------------------------------------
// (public) G e t _ L o b b y _ G r o u p _ I n d e x
//
uint_16 group_pool::Get_Lobby_Group_Index ( void )
{
   return 0;
}

// =---------------------------------------------------------------------------
// Summarize_Game_Groups
//
_result group_pool::Send_Game_Groups_Summary ( connection_idpcd& conn )
{
   for ( uint_16 i = 0; i < Group_Count; i++ )
   {
      idpcd_group& Group = Get_Group ( i );
      
      if ( !Group.Is_Dummy() && 
           !Group.Is_Lobby() &&
           Group.Get_Status() == GS_OPEN )
      {
         Group.Send_Group_Notify ( conn );
      }
   }

   return RS_OK;
}


// =---------------------------------------------------------------------------
// (public) R e s i z e
//
// resizes the number of connection connections that can be managed at once
//
// =---------------------------------------------------------------------------
bool group_pool::Resize ( uint_16 new_size )
{
   // We can't resize the group pool if the server is still running!
   //
   if ( Continue_Processing     ) return false;

   if ( new_size == Group_Count ) return true;

   _result res = Disband_All_Groups ();
   
   if ( RFAILED(res) )
      return false;

   if ( rgGroups && Group_Count )
   {
      for ( uint_16 i=0; i < Group_Count; i++ )
         delete rgGroups[i];

      delete [] rgGroups;
      rgGroups = 0;
   }


   if ( new_size )
   {
      rgGroups = new idpcd_group*[new_size];

      for ( uint_16 i=0; i < new_size; i++ )
         rgGroups[i] = new idpcd_group ( i == 0 /*lobby is first only*/ );
      
      Group_Count = new_size;
   }
   

   return true;
}

// =---------------------------------------------------------------------------
// (public) N u m _ G r o u p s
//
// =---------------------------------------------------------------------------
void group_pool::Num_Groups ( uint_32& n_normal, uint_32& n_free )
{
   n_normal = n_free = 0;
   
   for ( uint_16 idx = 0; idx < Group_Count; idx++ )
   {
      idpcd_group* p_grp = rgGroups[idx];

      if ( p_grp->Get_Status() == GS_PRIMAL ) n_free++;
      else n_normal++;
   }
}

// =---------------------------------------------------------------------------
// (public) R e p o r t _ G r o u p s
//
// =---------------------------------------------------------------------------
void group_pool::Report_Groups ( void )
{
   uint_32 num_displayed = 0;

   for ( uint_16 i = 0; i < Group_Count; i++ )
   {
      idpcd_group& Group = Get_Group ( i );

      if ( !Group.Is_Dummy() )
      {
         Group.Report_Group();
         num_displayed++;
      }
   }

   if ( !num_displayed ) 
      Sys.Message ( CHANNEL_NORMAL, "    There are no active groups." );
}

// =---------------------------------------------------------------------------
// (public) G e t _ G r o u p _ C o n n e c t i o n _ C o u n t s
//
// Returns the connections in each category
//
// =---------------------------------------------------------------------------
void group_pool::Get_Group_Connection_Counts ( uint_32& lobby, uint_32& game_setup, uint_32& ranked_in_play )
{
   lobby = game_setup = ranked_in_play = 0;

   for ( uint_16 i = 0; i < Group_Count; i++ )
   {
      idpcd_group& Group = Get_Group ( i );

      if ( !Group.Is_Dummy() )
      {

         if ( Group.Is_Lobby() )
         {
            lobby += Group.Get_Ref_Count();
         }
         else if ( Group.Get_Status() == GS_OPEN )
         {
            game_setup += Group.Get_Ref_Count();
         }
         else if ( Group.Get_Status() == GS_CLOSED )
         {
            if ( Group.Is_Ranked() )
               ranked_in_play++; // only game leader is connected
         }
      }
   }
}

// =---------------------------------------------------------------------------
// (public) G e t _ G r o u p
// (public) G e t _ G r o u p _ I n d e x
//
idpcd_group& group_pool::Get_Group ( uint_16 Group_Index )
{
   if ( Group_Index < Group_Count )
   {
      if ( rgGroups[Group_Index] )
         return *rgGroups[Group_Index];
   }

   return Dummy_Group;
}

uint_16 group_pool::Get_Group_Index ( idpcd_group& Group )
{
   for ( uint_16 i=0; i < Group_Count; i++ )
   {
      idpcd_group& LocalGroup = Get_Group ( i );

      if ( LocalGroup.Is_Dummy() )
         continue;

      if ( &LocalGroup == &Group )
         return i;
   }

   return 0xFFFF;
}

