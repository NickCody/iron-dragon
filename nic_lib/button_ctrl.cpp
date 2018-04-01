// =---------------------------------------------------------------------------
// c t r l _ b u t t o n . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    
//
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Sep 02   nic   Created.
//

#include "PreComp.h"
#include "ctrl.h"

WNDPROC button_ctrl::Old_WndProc = 0;

// =---------------------------------------------------------------------------
// ctor

button_ctrl::button_ctrl()
{
}

// =---------------------------------------------------------------------------
// dtor

button_ctrl::~button_ctrl()
{
}

// =---------------------------------------------------------------------------
// Create

bool button_ctrl::Create( HINSTANCE hInst, HWND parent, const char* Name, LPRECT pRect, int id )
{

   // create the tree_view windowdon'
   //
   Wnd = CreateWindowEx(  WS_EX_CLIENTEDGE, "BUTTON" , Name, 
                           WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_OVERLAPPED,
                           pRect->left , pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top, 
                           parent, (HMENU)id, hInst, 0 );

   // if successful, set the Child_Id and return
   //
   if ( Wnd )
   {
      if ( !Old_WndProc )
      {
         Old_WndProc = (WNDPROC)GetWindowLong ( Wnd, GWL_WNDPROC );
         SetWindowLong ( Wnd, GWL_WNDPROC, (LONG)button_ctrl::Wnd_Proc );
      }

      Child_Id = id;
      return true;
   }

   return false;
}

// =---------------------------------------------------------------------------
//  FUNCTION: Wnd_Proc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND - process the application menu
//  WM_PAINT   - Paint the main window
//  WM_DESTROY - post a quit message and return
//
//
LRESULT CALLBACK button_ctrl::Wnd_Proc( 
   HWND hWnd, 
   UINT message, 
   WPARAM wParam, 
   LPARAM lParam )
{
   switch (message) 
   {
      case WM_CHAR:
      {
         if ( wParam == VK_TAB )
         {
            HWND hWndNext = GetNextDlgTabItem ( GetParent(hWnd), hWnd, false );
            if ( hWndNext )
               SetFocus ( hWndNext );
            return 0;
         }
         break;
      }
      case WM_GETDLGCODE :
      {
         return DLGC_DEFPUSHBUTTON | DLGC_BUTTON;
      }
      case WM_CREATE :
         break;
      case WM_COMMAND:
         break;
      case WM_PAINT:
         break;
      case WM_SIZE:
         break;
      case WM_NOTIFY:
         break;
      case WM_CTLCOLORSTATIC: // sent since our edit is read-only
         break;
      case WM_CLOSE:
         break;
      case WM_DESTROY:
         break;
   }

   return CallWindowProcA ( Old_WndProc, hWnd, message, wParam, lParam );
}
