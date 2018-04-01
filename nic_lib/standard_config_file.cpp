/* --------------------------------------------------------------------------
   s t a n d a r d _ c o n f i g _ f i l e . c p p

   Parser for sectional text-based, line-based config file
   --------------------------------------------------------------------------

   Date        Who   Notes
   ----------  ---   --------------------------------------------------------
   1999-11-09  MRS   Created.
   -------------------------------------------------------------------------- */

#include "PreComp.h"
#include "standard_config_file.h"

#include <sys/stat.h>

/* -------------------------------------------------------------------------- */
struct entry
{
   char const *Label;
   char const *Value;
   bool        Own_Value_String;
   entry      *Next_Entry;

   entry(char const *l = 0, char const *v = 0)
      :
      Label(l ? l : "No Entry Label"),
      Value(v ? v : "No Entry Value"),
      Own_Value_String(false),
      Next_Entry(0)
   {  }

  ~entry()
   {
      if (Own_Value_String) {delete[] (char *)Value; Value = 0;}
   }
};

/* -------------------------------------------------------------------------- */
struct section
{
   char const *Name;
   entry      *First_Entry;
   section    *Next_Section;

   section(char const *n = 0)
      :
      Name(n ? n : "No Section Name"),
      First_Entry(0),
      Next_Section(0)
   {  }
};

/* --------------------------------------------------------------------------
   (public, constructor)   s t d _ f i l e _ c o n f i g
   -------------------------------------------------------------------------- */
standard_config_file::standard_config_file( char const *f_name, char delim_section,
                                            char delim_value,   char delim_comment)
   :
   First_Section  (0),
   Data           (0)
{
   //HANDLE fh = CreateFile(f_name, GENERIC_READ, FILE_SHARE_READ, NULL, 
   //   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

   int fh = SS_File_IO_Open ( f_name, 1 /*readonly*/ );

   if ( fh == -1 )
      return;

   // bring the whole file into memory, it's likely to be small
   struct stat f_stats;
   SS_File_IO_Stat ( fh, &f_stats );
   int f_len = f_stats.st_size;

   Data = new char[f_len + 1];
   uint_32 nobr;
   SS_File_IO_Read ( fh, (uint_08*)Data, f_len, &nobr );
   Data[f_len] = 0;
   SS_File_IO_Close ( fh );
   fh = 0;

   // create our section/entry pointer structure
   section *s        = 0;
   entry   *e        = 0;
   char    *beg      = 0;
   int      as_how   = 1;  // 0 = sec, 1 = label, 2 = value
   bool     comment  = false;

   for (int x = 0; x <= f_len; x++)
   {
      if ((Data[x] == delim_section) || (Data[x] == delim_value))
      {
         if (!comment)
         {
            if    (Data[x] == delim_section) as_how = 0;
            else                             as_how = 2;
            Data[x] = 0;
            beg = 0;
         }
      }
      else if  (Data[x] == delim_comment) {Data[x] = 0; comment = true;}
      else if  (Data[x] < ' ')
      {
         beg      = 0;
         as_how   = 1;
         comment  = false;
         Data[x]  = 0;
      }
      else if  (Data[x] >  ' ')
      {
         if (!beg && !comment)
         {
            beg = Data + x;

            if (as_how == 0)       {section *ns = new section(beg);
                                    if (!s)  First_Section     = ns;
                                    else     s->Next_Section   = ns;
                                    s = ns; e = 0;}

            else if (as_how == 1)  {if (!s) First_Section = (s = new section);
                                    entry *ne = new entry(beg);
                                    if (!e)  s->First_Entry    = ne;
                                    else     e->Next_Entry     = ne;
                                    e = ne;}

            else                   {if (!s) {First_Section = (s = new section);
                                             e = 0;}
                                    if (!e) s->First_Entry= (e = new entry);
                                    e->Value = beg;}

            // backfill previous whitespace
            int y = x;
            while (--y >= 0)  {if (Data[y] > ' ') break; else Data[y] = 0;}
         }
      }
   }
}


/* --------------------------------------------------------------------------
   (public, destructor)   ~ s t d _ f i l e _ c o n f i g
   -------------------------------------------------------------------------- */
standard_config_file::~standard_config_file()
{
   delete [] Data; Data = 0;

   while (First_Section)
   {
      section *s = (section *) First_Section;
      while (s->First_Entry)
      {
         entry *ne = s->First_Entry->Next_Entry;
         delete s->First_Entry;
         s->First_Entry = ne;
      }
      section *ns = s->Next_Section;
      delete s;
      First_Section = ns;
   }
}


/* --------------------------------------------------------------------------
   (public)   G e t _ N u m _ S e c t i o n s
   -------------------------------------------------------------------------- */
int standard_config_file::Get_Num_Sections() const
{
   int rv = 0;
   section const *s = (section const *)First_Section;
   while (s) {rv++; s = s->Next_Section;}
   return rv;
}


/* --------------------------------------------------------------------------
   (public)   G e t _ S e c t i o n _ N a m e
   -------------------------------------------------------------------------- */
char const *standard_config_file::Get_Section_Name(int sn) const
{
   if (sn < 0) return "Bad Section Number";

   section const *s = (section const *)First_Section;
   while (s)  {if (sn-- == 0) return s->Name;
               s = s->Next_Section;}
   return "Bad Section Number";
}


/* --------------------------------------------------------------------------
   (public)   G e t _ S e c t i o n _ N u m b e r
   -------------------------------------------------------------------------- */
int standard_config_file::Get_Section_Number(char const *ss) const
{
   if (!ss) return -1;
   int rv = 0;
   section const *s = (section const *)First_Section;
   while (s)  {if (strcmp(ss, s->Name) == 0) return rv;
               rv++; s = s->Next_Section;}
   return -1;
}


/* --------------------------------------------------------------------------
   (public)   G e t _ N u m _ S e c t i o n _ E n t r i e s
   -------------------------------------------------------------------------- */
int standard_config_file::Get_Num_Section_Entries(int sn) const
{
   if (sn < 0) return -1;

   section const *s = (section const *)First_Section;
   while (s)  {if (sn-- == 0) break; s = s->Next_Section;}
   if (!s) return -1;

   int rv = 0;
   entry const *e = s->First_Entry;
   while (e)  {rv++; e = e->Next_Entry;}

   return rv;
}


/* --------------------------------------------------------------------------
   (public)   G e t _ E n t r y _ L a b e l _ N a m e
   -------------------------------------------------------------------------- */
char const *standard_config_file::Get_Entry_Label_Name(int sn, int en) const
{
   if (sn < 0) return "Bad Section Number";
   if (en < 0) return "Bad Entry Number";

   section const *s = (section const *)First_Section;
   while (s)  {if (sn-- == 0) break; s = s->Next_Section;}
   if (!s) return "Bad Section Number";

   entry const *e = s->First_Entry;
   while (e)  {if (en-- == 0) break; e = e->Next_Entry;}
   if (!e) return "Bad Entry Number";

   return e->Label;
}


/* --------------------------------------------------------------------------
   (public)   G e t _ E n t r y _ L a b e l _ N u m b e r
   -------------------------------------------------------------------------- */
int standard_config_file::Get_Entry_Label_Number(int sn, char const *ls) const
{
   if (sn < 0) return -1;
   if (!ls)    return -1;

   section const *s = (section const *)First_Section;
   while (s)  {if (sn-- == 0) break; s = s->Next_Section;}
   if (!s) return -1;

   int rv = 0;
   entry const *e = s->First_Entry;
   while (e)  {if (strcmp(ls, e->Label) == 0) return rv;
               rv++; e = e->Next_Entry;}
   return -1;
}


/* --------------------------------------------------------------------------
   (public)   G e t _ E n t r y _ V a l u e
   -------------------------------------------------------------------------- */
char const *standard_config_file::Get_Entry_Value(int sn, int en) const
{
   if (sn < 0) return "Bad Section Number";
   if (en < 0) return "Bad Entry Number";

   section const *s = (section const *)First_Section;
   while (s)  {if (sn-- == 0) break; s = s->Next_Section;}
   if (!s) return "Bad Section Number";

   entry const *e = s->First_Entry;
   while (e)  {if (en-- == 0) break; e = e->Next_Entry;}
   if (!e) return "Bad Entry Number";

   return e->Value;
}


/* --------------------------------------------------------------------------
   (public)   S e t _ E n t r y _ V a l u e
   -------------------------------------------------------------------------- */
bool standard_config_file::Set_Entry_Value(int sn, int en, char const *vs)
{
   if (sn < 0) return false;
   if (en < 0) return false;
   if (!vs)    return false;

   section const *s = (section const *)First_Section;
   while (s)  {if (sn-- == 0) break; s = s->Next_Section;}
   if (!s) return false;

   entry *e = s->First_Entry;
   while (e)  {if (en-- == 0) break; e = e->Next_Entry;}
   if (!e) return false;

   int len = strlen(vs) + 1;
   if (e->Own_Value_String)   delete[] (char *)e->Value;
   char *new_s = new char[len];
   memcpy(new_s, vs, len);
   e->Value             = new_s;
   e->Own_Value_String  = true;

   return true;
}


/* --------------------------------------------------------------------------
   end of file   ( s t a n d a r d _ c o n f i g _ f i l e . c p p )
   -------------------------------------------------------------------------- */
