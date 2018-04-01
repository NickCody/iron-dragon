#include <sys/mman.h>
#include <fcntl.h>

int mm_test()
{
   int fd = open ( "/home/Nick/id/idpcd/temp/idpcd", O_RDONLY );
   if ( fd == 0 )
   {
      printf ( "fopen failed" );
      exit(0);
   }

   struct stat s;
   fstat ( fd, &s );

   printf ( "idpcd (%lu block size)", s.st_blksize );
   printf ( "idpcd (%lu blocks)", s.st_blocks );
   printf ( "idpcd (%lu bytes)\n", s.st_size );

   int file_size = s.st_blksize * s.st_blocks;

   void* pMap = mmap ( 0, file_size, PROT_READ, MAP_PRIVATE, fd, 0 );
   Error_Message ( errno );

   if ( (int)pMap == -1 )
   {
      printf ( "Map failed\n" );
      exit(0);
   }
   else
      printf ( "Map succeeded\n" );

   char* p1 = (char*)pMap;
   char* p2 = p1 + 1;
   char* p3 = p1 + 2;
   char* p4 = p1 + 3;

   printf ( "The first four bytes are...: (0x%x) %c,%c,%c,%c\n", 
	    (int)pMap, *p1, *p2, *p3, *p4);

   int ret = munmap ( pMap, file_size );
   if ( ret == -1 )
   {
      printf ( "munmap failed!\n" );
      exit(0);
   }
   else
   {
      printf ( "munmap succeeded\n" );
   }

   close ( fd );

   exit(0);

}


void client_test()
{

   client_database db;

   _result res = db.Create_Open ( "data/clients.db" );
   if ( RFAILED(res) )
   {
      Result_Message ( res );
      exit(0);
   }

   Message ( "Open_Create succeeded\n" );

   client_record cr;
   memset ( &cr, 0, sizeof(client_record) );

   strcpy ( cr.fname, "Nick" );
   strcpy ( cr.lname, "Codignotto" );
   strcpy ( cr.username, "Cody" );

   uint_32 i;
   for ( i=0; i < 3; i++ )
      db.Add_Record ( (void*)&cr );

   for ( i=0; i < db.Record_Count(); i++ )
   {
      client_record cr1;
      db.Get_Record ( i, (uint_08*)&cr1 );
      printf ( "Record %4d-------\n", (int)i );
      printf ( "First Name: %s\n", cr1.fname );
      printf ( "Last Name:  %s\n", cr1.lname );
      printf ( "Username:   %s\n", cr1.username );
   }

   exit(0);

}
// =---------------------------------------------------------------------------
// (public) H a n d l e _ C l i e n t
//
// =---------------------------------------------------------------------------
_result connection::Handle_Connection(void)
{
   _result res;

   // The first thing we need to know is whether the client supports binary
   // or ascii requests

   bool Speak_Ascii;
   res = Negotiate_ASCII( Speak_Ascii );
   if ( RSUCCEEDED(res) )
   {
      // the second thing that we need to do is negotiate the client's identity
      res = Negotiate_Identity();

      if ( RSUCCEEDED(res) )
      {
         // now, process all client requests...

         uint_32 req_code;
         
         while ( RSUCCEEDED(Get_Request(&req_code)) )
         {
            res = Dispatch_Request ( req_code );
            
            if ( RFAILED(res) )
               return res;
         }
         
         return RS_OK;
      }
   }

   if ( RFAILED(res) )
      Debug_Message ( "Error %s\n", Result_String ( res ) );

   return res;
}

// =---------------------------------------------------------------------------
// (public) N e g o t i a t e _ A S C I I
//
// PROTOCOL:
//
_result connection::Negotiate_ASCII ( bool& ascii )
{
   uint_08   msg[MAX_REQTYPE_LEN];
   int       ret;

   // We determine this by sending out a string, "ASCII? (yes/no)". If the
   // client returns 'yes', we will perform all communications in text mode.
   // If the client returns no, all communications will be in binary

   strcpy ( (char*)msg, "ASCII? (y/n) " );
   //ret = Send_Buffer ( Data_Sock, msg, strlen((char*)msg)-1 ); // -1 no send null

   if ( ret == -1 )
      return RS_BADSOCKETCOMM;

   //ret = Receive_Buffer ( Data_Sock, msg, 1 );
   if ( ret == -1 )
   {
      // TODO: Look at errno and possibly just reread
      return RS_BADSOCKETCOMM;
   }

   ascii = ( msg[0] == 'y' || msg[0] == 'Y' );

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) N e g o t i a t e _ I d e n t i t y
//
// PROTOCOL:
//
//    SERVER: REQ_IDENTIFYING_INFO (4 bytes)
//    CLIENT: ci_size (4 bytes)
//    CLIENT: client_record contents (ci_size bytes)
//    SERVER: send 4 bytes=1 if user found, otherwise =0 (user retransmit)
// =---------------------------------------------------------------------------
_result connection::Negotiate_Identity ( void )
{
   uint_32 req;
   int     ret;
   _result res;
   uint_32 recnum;
   uint_32 cli_size;
   client_record_network  cli;
   client_record          connecting_client;

   // Ask for identifying info

   req = htonl(REQ_IDENTIFYING_INFO);
   //ret = Send_Buffer( Data_Sock, &req, sizeof(uint_32) );
   if ( ret == -1 )
      return RS_BADSOCKETCOMM;

   // Wait for clients notion of client_record_network byte size

   //ret = Receive_Buffer ( Data_Sock, &cli_size, sizeof(uint_32) );
   if ( ret == -1 )
      return RS_BADSOCKETCOMM;

   cli_size = ntohl ( cli_size );

   // now, get the client_record_network structure (username/password)

   //ret = Receive_Buffer ( Data_Sock, &cli, cli_size );
   if ( ret == -1 )
      return RS_BADSOCKETCOMM;

   cli.As_Host();

   // if username or password is bad, the client is flaky, nix them

   if ( strlen(cli.username) == 0 || strlen(cli.password) == 0 )
      return RS_BADCLIENTCOMM;

   Debug_Message ( "Client sent identity: username(%s), password(%s)\n",
                     cli.username, cli.password );

   // Look up the client, for now we just accept them

   res = Client_DB.Find_Record_By_Username ( cli.username, recnum );
   if ( RFAILED(res) )
   {
      Debug_Message ( "This is a new client, adding..." );

      connecting_client = cli;
      res = Client_DB.Add_Record ( (uint_08*)&connecting_client );
      if ( RSUCCEEDED(res) )
         Debug_Message ( "success.\n" );
      else
      {
         Debug_Message ( "failed!\n" );
         return RS_ERR;
      }
   }
   else
   {
      Debug_Message ( "Client is known\n" );
      res = Client_DB.Get_Record ( recnum, (uint_08*)&connecting_client );
   }

   return RS_OK;
}

// =---------------------------------------------------------------------------
// (public) G e t _ R e q u e s t
//
// =---------------------------------------------------------------------------
_result connection::Get_Request ( uint_32* p_req )
{
   // here we need to virtualize ascii and binary conversations, for now,
   // we just assume binary
   //
   int ret; // = Receive_Buffer ( Data_Sock, p_req, sizeof(uint_32), 0 );

   return ret == sizeof(uint_32) ? RS_OK : RS_ERR;
}

// =---------------------------------------------------------------------------
// (public) D i s p a t c h _ R e q u e s t
//
// =---------------------------------------------------------------------------
_result connection::Dispatch_Request ( const uint_32 req )
{
   
   // TODO: Create proper text-message mappings for these constants
   // to support text mappings
   switch ( req )
   {
      case REQ_CLIENT_LIST:
         Message ( "Received a dump_client_list request\n" );
         return RS_OK;
      case REQ_GROUP_LIST:
         Message ( "Received a dump_group_list request\n" );
         return RS_OK;
      default:
         Message ( "Received an unknown request, killing connection.\n" );
         return RS_ERR;
   }
}


// =---------------------------------------------------------------------------
// Mouse Hook Procedure
//
LRESULT CALLBACK AppMouseProc(
  int nCode,      // hook code
  WPARAM wParam,  // message identifier
  LPARAM lParam   // mouse coordinates
)
{
   if ( nCode < 0 )
   {
      Message ( "nCode < 0\r\n" );
      return CallNextHookEx ( idpcd_app::App_MouseHook, nCode, wParam, lParam );
   }
   else
   {
      if ( nCode == HC_ACTION )
         Message ( "Received an HC_ACTION: " );
      else if ( nCode == HC_NOREMOVE )
         Message ( "Received an HC_NOREMOVE: " );
      
      Message ( "\r\n" );
      
      return CallNextHookEx ( idpcd_app::App_MouseHook, nCode, wParam, lParam );
   }
}

// =---------------------------------------------------------------------------
// WM_COMMAND Message Handler
//
LRESULT idpcd_app::On_Create( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
   App_MouseHook = SetWindowsHookEx ( WH_MOUSE, AppMouseProc, App_Instance, 0 );
   
   if ( !App_MouseHook )
      Message ( "Failed to create the mouse hook!\r\n " );

   unsigned long ret = _beginthread ( Win32_Startup_Thread, 0, 0 );

   return 0;
}

// =---------------------------------------------------------------------------
// WM_CLOSE Message Handler
//
// Return 1 to cancel the close
//
LRESULT idpcd_app::On_Close( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   if ( App_MouseHook )
      UnhookWindowsHookEx ( App_MouseHook );

   return DefWindowProc(hWnd, WM_CLOSE, wParam, lParam);

}

// =----------------------------------------------------
// (public) P r o c e s s _ Z e r o _ B y t e _ T r a n s f e r s
//
_result Process_Zero_Byte_Transfers  ( void );

// =---------------------------------------------------------------------------
// (public) P r o c e s s _ Z e r o _ B y t e _ T r a n s f e r s 
//
_result connection_pool::Process_Zero_Byte_Transfers  ( void )
{
   lock_helper save_ferris ( this );

   std::vector<connection>::iterator idx = Connections.begin();
   while ( idx != Connections.end() )
   {
      // TODO: is this what we want, to clear the connection?
      // Perhaps a better approach would be to push on a state_machine
      // that would resync the client with us?
      if ( idx->Zero_Read || idx->Zero_Write )
         idx->Clear_Connection();

      idx++;
   }

   return RS_OK;
}





   // If we have two clients now, then adding this one completes the 
   // Short_Circuit connection
   //
   if ( Connect_Type == CT_SHORT_CIRCUIT && Connections.size() == 2 )
   {
      if ( RFAILED(Broadcast_Final_Packet()) )
      {
         // What do we do if this fails
         // probably nothing, just punt everyone with a Clear_Group
      }

      // Now that we send our clients the final packets (true above flushes send buffers)
      // Let's clear this group
      //
      //Clear_Group();
   }
