// =---------------------------------------------------------------------------
// c t r l _ t o o l b a r . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the Windows Common Control Toolbar
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
//

toolbar::toolbar() : windows_control()
{
}

// =---------------------------------------------------------------------------
// dtor
//

toolbar::~toolbar()
{
}

// =---------------------------------------------------------------------------
// create - creates the toolbar window
//

bool toolbar::Create( HINSTANCE hInst, HWND parent, const char* title, int id )
{
   // create the toolbar
   //
   Wnd = CreateWindowEx( 0, TOOLBARCLASSNAME , title, WS_CHILD | WS_VISIBLE, 
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                        parent, (HMENU)id, hInst, 0 );

   if ( Wnd )
   {
      Child_Id = id;
   }

   return ( Wnd != 0 );
}

