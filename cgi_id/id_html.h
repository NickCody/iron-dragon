


#ifndef __ID_HTML_H__
#define __ID_HTML_H__

enum { TAGNAME_BYTES = 20 };

class html_tag
{
public:
   html_tag ( const char* tag_name )
   {
      if ( tag_name
      strcpy ( Tag_Name, tag_name );
   }

protected:
   char Tag_Name[TAGNAME_BYTES];
};


#endif // __ID_HTML_H__