// =---------------------------------------------------------------------------
// c t r l _ s t a t u s b a r . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the Windows Common Control Status Bar
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Mar 03   nic   Created.
//

#include "PreComp.h"
#include "ctrl.h"

// =---------------------------------------------------------------------------
// ctor

statusbar::statusbar() : windows_control()
{
   rg_Part_Rights = 0;
   Status_Parts = 0;
}

// =---------------------------------------------------------------------------
// dtor

statusbar::~statusbar()
{
   Status_Parts = 0;
   delete [] rg_Part_Rights;
   rg_Part_Rights = 0;
}

// =---------------------------------------------------------------------------
// create - creates the window, sets the child id (for detection in WM_COMMAND
// and WM_NOTIFY
//

bool statusbar::Create( HINSTANCE hInst, 
                        HWND parent, 
                        const char* title, 
                        int id )
{

   sint_32 rg_part_widths[] = { 300, 400, -1 };

   return CreateEx ( hInst, parent, title, id, 3, rg_part_widths );
}

bool statusbar::CreateEx( HINSTANCE hInst, 
                          HWND parent, 
                          const char* title, 
                          int id, 
                          uint_32 num_parts,
                          sint_32 rg_part_widths[] )
{
   // create the toolbar
   //
   Wnd = CreateWindowEx( 0, STATUSCLASSNAME , title, WS_CHILD | WS_VISIBLE, 
                        0, 0, 200, 200, parent, (HMENU)id, hInst, 0 );


   if ( Wnd )
   {
      // create additional panes
      Status_Parts = num_parts;
      rg_Part_Rights = new int[Status_Parts];
      for ( int i=0; i < Status_Parts; i++ )
         rg_Part_Rights[i] = rg_part_widths[i];

      SendMessage ( Wnd, SB_SETPARTS, (WPARAM)Status_Parts, (LPARAM) (LPINT) rg_Part_Rights );
      
      Child_Id = id;
   }

   return ( Wnd != 0 );
}

// =---------------------------------------------------------------------------
// printf - sets the text of the status bar (instantaneous update from 
//    wherever) on the specified "part"
//

void statusbar::Message_To_Part( int part, const char* msg, ... )
{
   // create string based on arguments
   //
   va_list v1; 
   va_start( v1, msg );
   wvsprintf( buffer, msg, v1 );
   va_end( v1 ); 

   // SB_SETTEXT 
   // wParam = (WPARAM) iPart | uType; 
   // lParam = (LPARAM) (LPSTR) szText; 
   //
   SendMessage ( Wnd, SB_SETTEXT, (WPARAM)part, (LPARAM)buffer );
}

// =---------------------------------------------------------------------------
// ready - quick helper that simply displays  'Ready' in the status bar
//

void statusbar::Ready()
{
   Message_To_Part ( SBPART_MAIN, "Ready" );
}

