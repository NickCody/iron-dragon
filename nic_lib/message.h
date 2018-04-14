// =---------------------------------------------------------------------------
// message.h
//
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Definition of helper classes and interfaces for log/trace functionality
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Jan 15   nic   Created.
//

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "../idpcd2/file_io_subsystem.h"
#include "../idpcd2/port_subsystem.h"

enum MSGCHANNELS
{
   CHANNEL_DEBUG     =  0x0001,        // the default debug channel
   CHANNEL_DEBUG1    =  0x0002,
   CHANNEL_DEBUG2    =  0x0004,
   CHANNEL_DEBUG3    =  0x0008,
   CHANNEL_DEBUG4    =  0x0010,
   CHANNEL_DEBUG5    =  0x0020,
   CHANNEL_DEBUG6    =  0x0040,
   CHANNEL_DEBUG7    =  0x0080,
   CHANNEL_DEBUG8    =  0x0100,
   CHANNEL_DEBUG9    =  0x0200,
   CHANNEL_NORMAL    =  0x0800,

   CHANNEL_DATETIME  =  0x8000,        // Adds a date to each output message
   CHANNEL_ALL       =  0xFFFF
};

// =---------------------------------------------------------------------------
// (pure virtual) interface MessageSink
//
//
// =---------------------------------------------------------------------------
class MessageSink
{
public:
   virtual void Output ( const char* buf ) = 0;
   virtual void Flush ( void ) {}
};

// =---------------------------------------------------------------------------
// class MessageHelper
//
//
// =---------------------------------------------------------------------------

#define MAX_SIMPLE_INDENT   (64)
#define SIMPLE_INDENT_BYTES (MAX_SIMPLE_INDENT + 1)
#define NEWLINE_BYTES       (4)
#define OUTBUFFER_BYTES     (4096)

class MessageHelper
{
public:

   MessageHelper ( MessageSink& s ) : Sink(s)
   {
      Active_Channels = CHANNEL_ALL;
      Current_Indent  = 0;
      Pending_Newline = false;

      #if UNIX
      strcpy ( Newline_String, "\n" );
      #elif WIN32
      strcpy ( Newline_String, "\r\n" );
      #endif

      Out_Buffer_Bytes = OUTBUFFER_BYTES;
      Out_Buffer = new char[OUTBUFFER_BYTES];
   }

   ~ MessageHelper ( )
   {
      delete Out_Buffer;
      Out_Buffer = 0;
   }

   // [MessageSink]

   // =------------------------------------------------------------------------
   // S e t / G e t _ A c t i v e _ M s g C h a n n e l
   //
   // =------------------------------------------------------------------------
   uint_32 Set_Active_MsgChannels ( uint_32 nc_mask )
   {
      uint_32 temp = Active_Channels;
      Active_Channels = nc_mask;
      return temp;
   }

   uint_32 Get_Active_MsgChannels ( void ) { return Active_Channels; }

   // =------------------------------------------------------------------------
   // S e t / G e t _ D e f a u l t _ M s g C h a n n e l
   //
   // =------------------------------------------------------------------------
   void Set_Default_MsgChannel ( uint_32 dc ) { Default_Channel = dc; }
   uint_32 Get_Default_MsgChannel ( void ) { return Default_Channel; }

   // =------------------------------------------------------------------------
   // M e s s a g e
   //
   // =------------------------------------------------------------------------
   void Message ( uint_32 channel, const char* format, ... )
   {
      if ( channel & Active_Channels )
      {
         if ( Active_Channels & CHANNEL_DATETIME &&
              !Pending_Newline )
         {
            char datestr[32];
            SS_Port_Get_DateTime_String(datestr);
            Sink.Output ( "[ " );
            Sink.Output ( datestr );
            Sink.Output ( " ]" );
         }

         if ( Current_Indent > 0 && !Pending_Newline )
            Sink.Output ( Indent_String );

         va_list v1;
         va_start ( v1, format );
         vsprintf ( Out_Buffer, (char*)format, v1 );
         va_end ( v1 );

         strcat ( Out_Buffer, Newline_String );

         Sink.Output ( Out_Buffer );

         Pending_Newline = false;
      }
   }

   // =------------------------------------------------------------------------
   // M e s s a g e _ N o N e w l i n e
   //
   // =------------------------------------------------------------------------
   void Message_NoNewline ( uint_32 channel, const char* format, ... )
   {
      if ( channel & Active_Channels )
      {
         if ( Active_Channels & CHANNEL_DATETIME &&
              !Pending_Newline )
         {
            char datestr[32];
            SS_Port_Get_DateTime_String(datestr);
            Sink.Output ( "[ " );
            Sink.Output ( datestr );
            Sink.Output ( " ] " );
         }

         va_list v1;
         va_start ( v1, format );
         vsprintf ( Out_Buffer, (char*)format, v1 );
         va_end ( v1 );

         if ( Current_Indent > 0 && !Pending_Newline )
            Sink.Output ( Indent_String );

         Sink.Output ( Out_Buffer );

         Pending_Newline = true;
      }
   }

   void Message_Indent ( sint_32 Delta )
   {
      if ( (Current_Indent + Delta) > MAX_SIMPLE_INDENT )
         return;

      Current_Indent += Delta;

      for ( int i=0; i < Current_Indent; i++ )
      {
         Indent_String[i] = ' ';
      }

      if ( Current_Indent >= 0 )
         Indent_String[Current_Indent] = '\0';

      return;
   }
   // =------------------------------------------------------------------------
   // S e t _ N e w l i n e _ S t r i n g
   //
   // =------------------------------------------------------------------------
   void Set_Newline_String ( const char* ns )
   {
      strncpy ( Newline_String, ns, NEWLINE_BYTES );
      Newline_String[NEWLINE_BYTES-1] = '\0';
   }

   // =------------------------------------------------------------------------
   // S e t _ B u f f e r _ B y t e s
   //
   // =------------------------------------------------------------------------
   void Set_Buffer_Bytes ( uint_32 size )
   {
      if ( size && size != Out_Buffer_Bytes )
      {
         Out_Buffer_Bytes = size;
         delete [] Out_Buffer;
         Out_Buffer = new char[Out_Buffer_Bytes];
      }
   }

   void Flush ( )
   {
      Sink.Flush();
   }

protected:

   MessageSink& Sink;

   uint_32  Active_Channels;
   uint_32  Default_Channel;

   bool     Pending_Newline;
   char     Newline_String    [NEWLINE_BYTES];
   sint_32  Current_Indent;
   char     Indent_String     [SIMPLE_INDENT_BYTES];

   char*    Out_Buffer;
   uint_32  Out_Buffer_Bytes;

};

extern MessageHelper Sys;

// =---------------------------------------------------------------------------
// StackReporter
// define STACKREP in the cpp to get this to work
// =---------------------------------------------------------------------------
class StackReporter
{
public:
   StackReporter ( uint_32 c, const char* msg )
   {
#ifdef STACKREP
      p_msg    = msg;
      channels = c;

      Sys.Message_NoNewline ( channels, "BEGIN " );
      Sys.Message ( channels, p_msg );

      if ( Sys.Get_Active_MsgChannels() & channels )
         Sys.Message_Indent ( 2 );
#endif
   }

   ~ StackReporter ( )
   {
#ifdef STACKREP
      if ( Sys.Get_Active_MsgChannels() & channels )
         Sys.Message_Indent ( -2 );

      Sys.Message_NoNewline ( channels, "END " );
      Sys.Message ( channels, p_msg );
#endif
   }

protected:
#ifdef STACKREP
   const char* p_msg;
   uint_32     channels;
#endif
};

// =---------------------------------------------------------------------------
// Stdout_Sink - Simple stdout
//
// =---------------------------------------------------------------------------
class Stdout_Sink : public MessageSink
{
public:
   Stdout_Sink( ) {}

   virtual void Output ( const char* buf )
   {
      printf ( "%s", buf );
   }
};

// =---------------------------------------------------------------------------
// File_Sink - Simple stdout
//
// =---------------------------------------------------------------------------
class File_Sink : public MessageSink
{
public:
   File_Sink( )
   {
      Outfile = 0;
   }

   virtual ~ File_Sink ( )
   {
      Close();
   }

   virtual void Output ( const char* buf )
   {
      if ( Outfile > 0 )
      {
         SS_File_IO_Write ( Outfile, (uint_08*)buf, strlen(buf) );
         SS_File_IO_Flush ( Outfile );
      }
   }

   virtual void Flush ( void )
   {
      if ( Outfile )
         SS_File_IO_Flush ( Outfile );
   }

   _result Open  ( const char* filename )
   {
      if ( filename && strlen(filename) )
      {
         Outfile = SS_File_IO_Open_Append ( filename );

         if ( Outfile <= 0 )
            return RS_ERR;
      }

      return RS_OK;
   }

   _result CreateNew  ( const char* filename )
   {
      if ( filename && strlen(filename) )
      {
         Outfile = SS_File_IO_Open ( filename, 0 /*readonly*/ );

         // Fail if the file already exists...
         //
         if ( Outfile > 0 )
         {
            SS_File_IO_Close ( Outfile );
            Outfile = 0;
            return RS_ERR;
         }

         Outfile = SS_File_IO_Create ( filename );

         if ( Outfile > 0 )
            return RS_OK;
      }

      return RS_ERR;
   }

   void Close ( void )
   {
      if ( Outfile > 0 )
      {
         SS_File_IO_Close ( Outfile );
         Outfile = 0;
      }
   }

protected:

   int Outfile;

};

class Dual_Sink : public MessageSink
{
public:
   Dual_Sink ( MessageSink& sink1, MessageSink& sink2 )
      : Sink1 ( sink1 ), Sink2 ( sink2 )
   {
   }

   virtual void Output ( const char* buf )
   {
      Sink1.Output ( buf );
      Sink2.Output ( buf );
   }

protected:
   MessageSink& Sink1;
   MessageSink& Sink2;
};

#endif // __MESSAGE_H__
