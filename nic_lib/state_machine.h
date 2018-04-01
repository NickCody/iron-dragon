// =---------------------------------------------------------------------------
// s t a t e _ m a c h i n e . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Aug 02   nic   Created.
//

#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

class state_machine;

// =--------------------------------------------------------------------------
// INTERFACES

interface IStateNotify
{
   // no clear purpose for On_Begin_Machine, yet
   virtual _result On_Begin_Machine  ( state_machine& sm ) = 0;

   virtual _result On_Traverse_State ( state_machine& sm,
                                       uint_32 old_state, 
                                       uint_32& new_state ) = 0;

   // When state machine is done
   virtual _result On_End_Machine    ( state_machine& sm ) = 0;

};

// =--------------------------------------------------------------------------
// ENUMERATIONS

enum xfer_mode
{
   XM_ERROR    = 0,
   XM_READ     = 1,
   XM_WRITE    = 2,
   XM_SHUTDOWN = 3
};

#define STATE_BEGIN (-1)
#define STATE_END   (-2)

// =--------------------------------------------------------------------------
// state_entry
//
struct state_entry
{
   uint_32   Current_Byte_Count;
   uint_32   Byte_Count;
   xfer_mode Xfer_Mode;
   uint_08*  p_Buffer;
};

// =--------------------------------------------------------------------------
// state_machine
//

// p_sn is the client who should be notified when states are traversed
// mid is the machine_id so that when the client is notified, 
//    the client an determine which state machine did the notifying
//
class state_machine
{
public:
   // =------------------------------------------------------------------------
   // (public) ctor
   //
   state_machine ( IStateNotify* p_sn, uint_32 mid = 0 )
   {
      p_Notify      = p_sn;
      Current_State = STATE_BEGIN;
      Machine_Id    = mid;
   }

   // =------------------------------------------------------------------------
   // (public) copy ctor
   //
   state_machine ( const state_machine& rhs )
   {
      *this = rhs;
   }

   // =------------------------------------------------------------------------
   // (public) dtor
   //
   virtual ~ state_machine ( ) 
   {
      Clear();
   }

   // =------------------------------------------------------------------------
   // (public) o p e r a t o r   =
   //
   state_machine& operator = ( const state_machine& rhs )
   {
      Clear();

      p_Notify      = rhs.p_Notify;
      Current_State = rhs.Current_State;
      Machine_Id    = rhs.Machine_Id;

      // Copy the RHS state table, by adding each entry from it using 
      // Add_State_Entry
      
      // TODO: How can I avoid the const cast const_cast<>
      state_machine* p_nc_rhs = const_cast<state_machine*>(&rhs);

      std::vector<state_entry>::iterator si = p_nc_rhs->State_Table.begin();
      while ( si != p_nc_rhs->State_Table.end() )
      {

         Add_State_Entry ( si->Byte_Count, si->Xfer_Mode, si->p_Buffer );
         si++;
      }

      return *this;
   }

   // =------------------------------------------------------------------------
   // (public) G e t _ C u r r e n t _ X f e r _ M o d e
   //
   xfer_mode Get_Current_Xfer_Mode ( void ) const
   {
      if ( Current_State == STATE_BEGIN ||
           Current_State == STATE_END )
         return XM_ERROR;
      else
         return State_Table[Current_State].Xfer_Mode;
   }

   // =------------------------------------------------------------------------
   // (public) G e t _ C u r r e n t _ S t a t e _ E n t r y 
   //
   _result Get_Current_State_Entry ( state_entry& se ) const
   {
      return Get_State_Entry ( Current_State, se );
   }

   // =------------------------------------------------------------------------
   // (public) G e t _ S t a t e _ E n t r y
   //
   _result Get_State_Entry ( uint_32 idx, state_entry& se ) const
   {
      if ( idx < State_Table.size() )
      {
         se = State_Table[idx];

         return RS_OK;
      }
      else
      {
         return RS_ERR;
      }
   }

   // =------------------------------------------------------------------------
   // (public) S e t _ S t a t e _ E n t r y
   //
   _result Set_State_Entry ( uint_32 idx, const state_entry& se )
   {
      if ( idx < State_Table.size() )
      {
         if ( State_Table[idx].Byte_Count != se.Byte_Count )
            return RS_ERR;

         State_Table[idx].Byte_Count         = se.Byte_Count;
         State_Table[idx].Current_Byte_Count = se.Current_Byte_Count;
         State_Table[idx].Xfer_Mode          = se.Xfer_Mode;
         memcpy ( State_Table[idx].p_Buffer, se.p_Buffer, se.Byte_Count );

         return RS_OK;
      }
      else
      {
         return RS_ERR;
      }
   }

   // =------------------------------------------------------------------------
   // (public) G e t _ M a c h i n e _ I d
   //
   uint_32 Get_Machine_Id ( ) const
   {
      return Machine_Id;
   }

   // =------------------------------------------------------------------------
   // (public) S e t _ M a c h i n e _ I d
   //
   uint_32 Set_Machine_Id ( uint_32 mid )
   {
      uint_32 temp_mid = Machine_Id;
      Machine_Id = mid;
      return temp_mid;
   }

   // =------------------------------------------------------------------------
   // (public) G e t _ C u r r e n t _ S t a t e
   //
   sint_32 Get_Current_State ( void ) const
   {
      return Current_State;
   }

   // =------------------------------------------------------------------------
   // (public) S e t _ C u r r e n t _ S t a t e
   //
   _result Set_Current_State ( sint_32 state )
   {
      if ( state < 0 || state >= (sint_32)State_Table.size() )
         return RS_ERR;

      Current_State = state;

      return RS_OK;
   }

   // =------------------------------------------------------------------------
   // (public) A d d _ S t a t e _ E n t r y
   //
   _result Add_State_Entry ( uint_32 byte_count, xfer_mode xm, void* p_initdata = 0 )
   {
      state_entry se;
      se.Byte_Count = byte_count;
      se.Xfer_Mode = xm;
      se.Current_Byte_Count = 0;

      if ( byte_count )
      {
         se.p_Buffer = new uint_08[byte_count];

         if ( p_initdata )
            memcpy ( se.p_Buffer, p_initdata, byte_count );
         else
            memset ( se.p_Buffer, 0, byte_count );
      }
      else
      {
         se.p_Buffer = 0;
      }

      State_Table.push_back ( se );
      return RS_OK;
   }

   // =------------------------------------------------------------------------
   // (public) A d d _ L o n g _ S t a t e _ E n t r y
   //
   // Handles to Network Byte order conversions
   //
   _result Add_Long_State_Entry ( xfer_mode xm, uint_32* p_initdata = 0 )
   {
      if ( xm == XM_WRITE && p_initdata )
      {
         uint_32 msb_long = htonl ( *p_initdata );
         Add_State_Entry ( 4, xm, &msb_long );
      }
      else
      {
         Add_State_Entry ( 4, xm );
      }
   }

   // =------------------------------------------------------------------------
   // (public) A d d _ S h o r t _ S t a t e _ E n t r y
   //
   // Handles to Network Byte order conversions
   //
   _result Add_Short_State_Entry ( xfer_mode xm, uint_16* p_initdata = 0 )
   {
      if ( xm == XM_WRITE && p_initdata )
      {
         uint_16 msb_short = htons ( *p_initdata );
         Add_State_Entry ( 4, xm, &msb_short );
      }
      else
      {
         Add_State_Entry ( 4, xm );
      }

      return RS_OK;
   }

   // =------------------------------------------------------------------------
   // (public) C l e a r
   //
   _result Clear ( void )
   {
      // we need to clear any buffer bytes that we allocated for each
      // state entry
      std::vector<state_entry>::iterator si = State_Table.begin();
      while ( si != State_Table.end() )
      {
         delete [] si->p_Buffer;
         si++;
      }

      State_Table.clear();
      Current_State = STATE_BEGIN;
      return RS_OK;
   }

   // =------------------------------------------------------------------------
   // (public) A d v a n c e _ N e x t _ S t a t e
   // 
   // Advance to the next state machine state.
   //
   // CAVEAT: Cannot be called by an instance of state_machine because this
   // function gives permission to IStateNotify instance to destroy us...

   _result Advance_Next_State ( void )
   {
      // It is an error to call Next_State when already on the last state
      //
      if ( Current_State == STATE_END )
         return RS_ERR;

      // When we go to the next state, we default to the current state + 1
      //
      uint_32 next_state = Current_State+1 < (sint_32)State_Table.size() ?
                           Current_State+1 : STATE_END;

      // notify client in case they want to revert back or modify the next
      // state based on anything that's happened so far
      if ( p_Notify )
      {
         _result res;

         if ( next_state == STATE_END )
            res = p_Notify->On_End_Machine ( *this );
         else
            res = p_Notify->On_Traverse_State ( *this, Current_State, next_state );

         if ( RFAILED(res) )
            return res;
      }

      // advance our notion of current state
      //
      Current_State = next_state;

      return RS_OK;
   }

protected:
   IStateNotify*            p_Notify;
   std::vector<state_entry> State_Table;
   sint_32                  Current_State;
   uint_32                  Machine_Id;
};

#endif //__STATE_MACHINE_H
