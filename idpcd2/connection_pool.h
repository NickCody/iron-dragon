// =-----------------------------------------------------------------------------------------------
// c o n n e c t i o n _ p o o l . h
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
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
//   99 Jun 15   nic   Created.
//

#ifndef __CONNECTION_POOL_H__
#define __CONNECTION_POOL_H__

#include "connection_idpcd.h"

// =---------------------------------------------------------------------------
// (global) c o n n e c t i o n _ p o o l 
//
// =---------------------------------------------------------------------------

class connection_pool //: public lockable_object
{
public:

   // =------------------------------------------------------------------------
   // ctor
   connection_pool ( );
   ~connection_pool ( );

   // =------------------------------------------------------------------------
   // Initialize_Multiplex_Descriptors
   //
   _result Initialize_Multiplex_Descriptors ( 
      fd_set& fd_reads, fd_set& fd_writes, int& max_desc );

   // =------------------------------------------------------------------------
   // Process_Pending_Transfers
   //
   _result Process_Pending_Transfers ( fd_set& fd_reads, fd_set& fd_writes );

   // =------------------------------------------------------------------------
   // Process_Heartbeat
   //
   _result Process_Heartbeat ( );

   // =------------------------------------------------------------------------
   // Num_Connections
   //
   void Num_Connections ( uint_32& n_busy, uint_32& n_free );

   // =------------------------------------------------------------------------
   // Free_All_Connections
   //
   // Boots everybody
   //
   _result Free_All_Connections ( void );

   // =------------------------------------------------------------------------
   // Find_First_Free_Connection
   //
   // Finds the first free connection and returns the connection's index in the pool
   //
   uint_16 Find_First_Free_Connection ( );

   // =------------------------------------------------------------------------
   // Get_Connection
   //
   connection_idpcd& Get_Connection ( uint_16 ConnIdx );

   // =------------------------------------------------------------------------
   // Get_Connection_Index
   //
   uint_16 Get_Connection_Index ( connection_idpcd& Conn );

   // =------------------------------------------------------------------------
   // Resize
   //
   // Resizes the number of connection connections that can be managed at once.
   //
   bool Resize ( uint_16 new_size );

   // =------------------------------------------------------------------------
   // Report_Connections
   // 
   void Report_Connections ( void );

   // =------------------------------------------------------------------------
   // Clear_Counters
   //
   void Clear_Counters ( void ) 
   { 
      Total_Clients_Authenticated = 0;
      Total_Clients_Connected = 0; 
   }
   
   // =------------------------------------------------------------------------
   // Inc_Authenticated
   // Inc_Connected
   // 
   void Inc_Authenticated( bool fBack_End_Operator ) 
   { 
      if ( fBack_End_Operator )
         Total_BE_Ops_Authenticated++;
      else
         Total_Clients_Authenticated++;
   }

   void Inc_Connected() 
   { 
      Total_Clients_Connected++; 
   }

   // =------------------------------------------------------------------------
   // Get_Total_Clients_Authenticated
   // Get_Total_Clients_Connected
   // 
   uint_32 Get_Total_Clients_Authenticated() 
   { 
      return Total_Clients_Authenticated; 
   }

   uint_32 Get_Total_Clients_Connected()
   { 
      return Total_Clients_Connected; 
   }

   // =------------------------------------------------------------------------
   // Detect_Duplicate_User
   // 
   _result Detect_Duplicate_User ( const char* Username );

protected:
   connection_idpcd** rgConnections;
   uint_16            Connection_Count;
   uint_32            Total_Clients_Authenticated;
   uint_32            Total_BE_Ops_Authenticated;
   uint_32            Total_Clients_Connected;
};

#endif //  __CONNECTION_POOL_H__
