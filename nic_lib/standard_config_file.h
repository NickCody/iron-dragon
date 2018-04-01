/* --------------------------------------------------------------------------
   s t a n d a r d _ c o n f i g _ f i l e . h

   Parser for sectional text-based, line-based config file

   A config file can have N sections, indexed from 0 to N - 1.
   Each section can have M entries, indexed from 0 to M - 1.
   Each entry has a Label string and a Value string.
   --------------------------------------------------------------------------

   Date        Who   Notes
   ----------  ---   --------------------------------------------------------
   1999-11-09  MRS   Created.
   -------------------------------------------------------------------------- */

#ifndef  INC_STANDARD_CONFIG_FILE_H
#define  INC_STANDARD_CONFIG_FILE_H

/* -------------------------------------------------------------------------- */
class standard_config_file
{
public:
   standard_config_file(char const   *f_name,
                        char          delim_section  = '$',
                        char          delim_value    = '=',
                        char          delim_comment  = ';');

  ~standard_config_file();

   int         Get_Num_Sections        ()                         const;
   char const *Get_Section_Name        (int sn)                   const;
   int         Get_Section_Number      (char const *ss)           const;

   int         Get_Num_Section_Entries (int sn)                   const;
   char const *Get_Entry_Label_Name    (int sn, int en)           const;
   int         Get_Entry_Label_Number  (int sn, char const *ls)   const;
   char const *Get_Entry_Value         (int sn, int en)           const;

   bool        Set_Entry_Value         (int sn, int en, char const *vs);

private:
   void *First_Section;
   char *Data;
};


/* -------------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------------
   end of file   ( s t a n d a r d _ c o n f i g _ f i l e . h )
   -------------------------------------------------------------------------- */
