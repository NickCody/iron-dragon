// =---------------------------------------------------------------------------
// c t r l _ e d i t . c p p
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

WNDPROC edit_ctrl::Old_WndProc = 0;

// =---------------------------------------------------------------------------
// ctor

edit_ctrl::edit_ctrl()
{
}

// =---------------------------------------------------------------------------
// dtor

edit_ctrl::~edit_ctrl()
{
}

// =---------------------------------------------------------------------------
// Create

bool edit_ctrl::Create( HINSTANCE hInst, HWND parent, LPRECT pRect, int id )
{
   // create the tree_view windowdon'
   //
   Wnd = CreateWindowEx(  WS_EX_CLIENTEDGE, "EDIT" , 0, 
                           WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_OVERLAPPED |
                           ES_WANTRETURN | ES_AUTOHSCROLL,
                           pRect->left , pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top, 
                           parent, (HMENU)id, hInst, 0 );

   // if successful, set the Child_Id and return
   //
   if ( Wnd )
   {
      if ( !Old_WndProc )
      {
         Old_WndProc = (WNDPROC)GetWindowLong ( Wnd, GWL_WNDPROC );
         SetWindowLong ( Wnd, GWL_WNDPROC, (LONG)edit_ctrl::Wnd_Proc );
      }

      SendMessage ( Wnd, EM_SETLIMITTEXT, 512, 0 );

      Set_Font ( "Arial", 12 ); 

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
LRESULT CALLBACK edit_ctrl::Wnd_Proc( 
   HWND hWnd, 
   UINT message, 
   WPARAM wParam, 
   LPARAM lParam )
{
   switch (message) 
   {
      case WM_GETDLGCODE :
      {
         return DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;
      }
      case WM_CHAR:
      {
         switch ( wParam )
         {
            case VK_TAB:
            {
               HWND hWndNext = GetNextDlgTabItem ( GetParent(hWnd), hWnd, false );
               if ( hWndNext )
                  SetFocus ( hWndNext );
               return 0;
            }
            case VK_RETURN:
            {
               HWND hWndNext = GetNextDlgTabItem ( GetParent(hWnd), hWnd, false );
               if ( hWndNext )
               {
                  SendMessage ( hWndNext, WM_KEYDOWN, (WPARAM)VK_SPACE, lParam );
                  //SendMessage ( hWndNext, WM_KEYUP, (WPARAM)VK_SPACE, lParam );
                  SetFocus ( hWnd );
               }
               return 0;
            }
         }
         break;
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
