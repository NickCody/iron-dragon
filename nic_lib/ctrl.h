// =---------------------------------------------------------------------------
// c t r l . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Declares the various control classs that out UI consists of
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Mar 03   nic   Created.
//

#ifndef __CTRL_H__
#define __CTRL_H__

#include "common.h"
#include "message.h"

#include <queue>
using std::queue;

// =---------------------------------------------------------------------------
// windows_control
//
// base class that holds window pointer 
// for derived control classes
//

class windows_control
{
public:

   windows_control() { Child_Id=0; Wnd=0; }
   virtual ~ windows_control() { Wnd=0; Child_Id=0; }

   HWND Get_Wnd() { return Wnd; }
   int Get_Id() { return Child_Id; }

   virtual LRESULT On_Notify( LPNMHDR pHdr ) = 0;

protected:
   HWND  Wnd;
   int   Child_Id;
};


// =---------------------------------------------------------------------------
// t o o l b a r
//
class toolbar : public windows_control
{
public:

   toolbar();
   virtual ~toolbar();

   bool Create( HINSTANCE hInst, HWND parent, const char* title, int id );

   virtual LRESULT On_Notify( LPNMHDR ) { return 0; }
};

// =---------------------------------------------------------------------------
// s t a t u s b a r
//
class statusbar : public windows_control
{
public:

   statusbar();
   virtual ~statusbar();

   bool Create( HINSTANCE hInst, HWND parent, const char* title, int id );
   bool CreateEx( HINSTANCE hInst, HWND parent, const char* title, 
                  int id, uint_32 num_parts, sint_32 rg_part_widths[] );

   virtual LRESULT On_Notify( LPNMHDR ) { return 0; }

   void Message_To_Part( int part, const char* message, ... );
   void Ready();

   enum { SBPART_MAIN=0, SBPART_NUMRESOURCES=1, SBPART_SIZERESOURCES=2 };

protected:
   int* rg_Part_Rights;
   int  Status_Parts;
   char buffer[512];

};

// =---------------------------------------------------------------------------
// l o g _ v i e w
//

#define LOGVIEW_QUEUETIMER_MILLISECONDS 1000

class log_view : public windows_control, public MessageSink
{
public:
   log_view();
   virtual ~log_view();

   bool Create( HINSTANCE hInst, HWND parent, const char* title, LPRECT pRect, int id );
   virtual LRESULT On_Notify( LPNMHDR ) { return 0; }

   // [MessageSink]
   virtual void Output ( const char* buf )
   {
      Append_Buffer_To_Window ( buf );
   }

   // =--------------------------------
   // printf outputs a message to the log window
   //
   void    Message                  ( MSGCHANNELS Channel, const char* msg, ... );
   void    Message_okfailed         ( MSGCHANNELS Channel, bool ok, bool crlf = true );
   void    Append_Buffer_To_Window  ( const char* buf );
   void    Set_Font                 ( const char* font_name, int point_size );
   void    Flush                    ( void );
   void    Clear                    ( void );
   uint_32 Set_Active_MsgChannels   ( uint_32 new_channel_mask ) { Active_MsgChannel_Mask = new_channel_mask; }
   uint_32 Get_Active_MsgChannels   ( void ) const { return Active_MsgChannel_Mask; }


protected:
   HFONT          Edit_Font;
   char           Log_Buffer[4096];       // All individual messages must be < 4096 bytes
   uint_32        Create_Thread;
   uint_32        Active_MsgChannel_Mask;
   queue<char*>   Message_Queue;
};

// =---------------------------------------------------------------------------
// e d i t _ c t r l
//

class edit_ctrl : public log_view
{
public:
   edit_ctrl();
   virtual ~edit_ctrl();

   bool Create( HINSTANCE hInst, HWND parent, LPRECT pRect, int id );
   virtual LRESULT On_Notify( LPNMHDR ) { return 0; }

   // =--------------------------------
   // Clears the entire log edit window
   //
   void Clear();

   static LRESULT CALLBACK Wnd_Proc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

protected:
   static WNDPROC Old_WndProc;
};

// =---------------------------------------------------------------------------
// b u t t o n _ c t r l 
//

class button_ctrl : public windows_control
{
public:
   button_ctrl();
   virtual ~button_ctrl();

   bool Create( HINSTANCE hInst, HWND parent, const char* Name, LPRECT pRect, int id );
   virtual LRESULT On_Notify( LPNMHDR ) { return 0; }

   static LRESULT CALLBACK Wnd_Proc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

protected:
   static WNDPROC Old_WndProc;
};

#endif // __CTRL_H__
