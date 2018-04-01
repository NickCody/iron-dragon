
#ifndef __STACK_TAG_H__
#define __STACK_TAG_H__


const int MAX_TAGLEN = 80;

class stacktag
{
public:
   stacktag ( uint_32 Channel, const char* Tag, const char* Attribs = "" )
   {
      SS_Port_Strcpy_Len ( this->Tag, Tag, MAX_TAGLEN );
      this->Channel = Channel;

      Sys.Message ( Channel, "<%s %s>", Tag ? Tag : "", Attribs ? Attribs : "" );
      Sys.Message_Indent ( 2 );

      Is_Closed = false;
   }

   ~ stacktag()
   {
      Close();
   }

   void Close ( void )
   {
      if ( !Is_Closed )
      {
         Is_Closed = true;
         Sys.Message_Indent ( -2 );
         Sys.Message ( Channel, "</%s>", Tag ? Tag : "" );

         if ( Sys.Get_Active_MsgChannels() & Channel )
            Sys.Flush ( );
      }
   }

protected:
   bool    Is_Closed;
   char    Tag[MAX_TAGLEN];
   uint_32 Channel;
};


#endif // __STACK_TAG_H__
