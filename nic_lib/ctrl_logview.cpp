// =---------------------------------------------------------------------------
// c t r l _ l o g v i e w . c p p
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
//   99 Mar 03   nic   Created.
//


#include "PreComp.h"
#include "ctrl.h"

// =---------------------------------------------------------------------------
// ctor

log_view::log_view()
{
   Edit_Font = 0;

   Create_Thread = GetCurrentThreadId ( );

   #ifdef DEBUG
      Active_MsgChannel_Mask = CHANNEL_ALL;
   #else
      Active_MsgChannel_Mask = CHANNEL_ALL & ~(CHANNEL_DEBUG);
   #endif

}

// =---------------------------------------------------------------------------
// dtor

log_view::~log_view()
{
   if ( Edit_Font )
   {
      Set_Font ( 0, 0 );

      DeleteObject ( Edit_Font );
      Edit_Font = 0;
   }
}

// =---------------------------------------------------------------------------
// Create

bool log_view::Create( HINSTANCE hInst, HWND parent, const char* title, LPRECT pRect, int id )
{
   // create the tree_view windowdon'
   //
   Wnd = CreateWindowEx(  WS_EX_CLIENTEDGE, "EDIT" , title, 
                           WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_VSCROLL | WS_HSCROLL |
                           ES_MULTILINE | ES_READONLY,
                           pRect->left , pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top, 
                           parent, (HMENU)id, hInst, 0 );

   // if successful, set the Child_Id and return
   //
   if ( Wnd )
   {
      SendMessage ( Wnd, EM_SETLIMITTEXT, 1024*1024 /*(1MB)*/, 0 );

      Set_Font ( "Lucida Console", 12 ); 

      Child_Id = id;
      return true;
   }

   return false;
}

// =---------------------------------------------------------------------------
// F l u s h
//
void log_view::Flush ( void )
{
   // an empty call to Message is enough to flush us
   //
   Message ( CHANNEL_NORMAL, "" );
}

// =---------------------------------------------------------------------------
// Message
//
// This function got to be a lot more of a mess that I wanted. Basically,
// calling SendMessage from another thread turns out to be a bad idea. So,
// when we call this from threads other than the creation thread, we queue
// the message. The owner (main app) should call Message ( "" ) in a timer
// to flush out all queued messages. Cest la vie.
//
void log_view::Message ( MSGCHANNELS channel, const char* msg, ... )
{
//   lock_helper save_ferris ( *this );

   if ( !Wnd )
      return;

   char* Queued_Buffer = 0;

   // create string based on arguments
   //
   va_list v1; 
   va_start( v1, msg );
   wvsprintf( Log_Buffer, msg, v1 );
   va_end( v1 ); 

   // On secondary threads, just queue the message
   //
   if ( Create_Thread != GetCurrentThreadId() )
   {
      Queued_Buffer = strdup ( Log_Buffer );
      Message_Queue.push ( Queued_Buffer );
   }
   else
   {
      while ( Message_Queue.size() )
      {
         Queued_Buffer = Message_Queue.front();
         Append_Buffer_To_Window ( Queued_Buffer );
         Message_Queue.pop();
         free ( Queued_Buffer );
         Queued_Buffer = 0;
      }

      if ( strlen(Log_Buffer) )
         Append_Buffer_To_Window ( Log_Buffer );
   }
}

// =---------------------------------------------------------------------------
// Message_okfailed
//
// No need to synchronize since it calls Message(), which is synchronized
//
void log_view::Message_okfailed  ( MSGCHANNELS Channel, bool ok, bool crlf )
{
   if ( ok )
      Message( Channel, "OK%s", crlf ? "\r\n" : "" );
   else
      Message( Channel, "Failed%s", crlf ? "\r\n" : "" );
}

// =---------------------------------------------------------------------------
// C l e a r 
//

void log_view::Clear( void )
{
   // Secondary threads can't clear because I am too lazy to write the code
   // Shoot me.
   if ( Create_Thread != GetCurrentThreadId() )
      return;

   SetWindowText( Wnd, "" );
}

// =---------------------------------------------------------------------------
// A p p e n d _ B u f f e r _ T o _ W i n d o w
//
// Not synchronized, caller should take care of that.
//
void log_view::Append_Buffer_To_Window (  const char* buf )
{
   if ( Wnd && IsWindow(Wnd) )
   {
      int ndx = GetWindowTextLength (Wnd);
      SendMessage ( Wnd, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx  );
      SendMessage ( Wnd, EM_REPLACESEL, 0, (LPARAM)buf );
   }
}

// =---------------------------------------------------------------------------
// S e t _ F o n t  
//
// Not synchronized, caller should take care of that.
//
void log_view::Set_Font ( const char* font_name, int point_size )
{
   if ( !IsWindow(Wnd) )
      return;

   if ( !font_name || font_name[0] == 0 )
   {
      SendMessage ( Wnd, WM_SETFONT, (WPARAM)0 /*default system font*/, 
         MAKELPARAM(true, 0) );
   }
   else
   {
      LOGFONT lf;
      ZeroMemory( &lf, sizeof(LOGFONT) );
      lf.lfHeight = -point_size;
      lf.lfWeight = FW_NORMAL;
      strcpy( lf.lfFaceName, font_name );
      HFONT hFont = CreateFontIndirect( &lf );

      if ( hFont )
      {
         SendMessage ( Wnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true, 0) );

         if ( Edit_Font )
            DeleteObject ( Edit_Font );

         Edit_Font = hFont;
      }
   }
}
