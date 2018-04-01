void Debug_Print(char *format, ...);
/* --------------------------------------------------------------------------
   l i c e n s e _ m a n a g e r . c p p

   Checks permissions for game access and provides to any requesters.
   File access code is written Win32-dependent.

   (c) 2000 by Martin R. Szinger and Nicholas Codignotto
   --------------------------------------------------------------------------

   Date        Who   Description
   ----------  ---   --------------------------------------------------------
   2000-03-29  mrs   Created (Eden's PC version of Mayfair's Iron Dragon).
   -------------------------------------------------------------------------- */

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include <stdio.h>
#include "cgi_id_subsystems.h"
#include "standard_types.h"
#include "license_manager.h"

char const *mn[NUM_GAME_MODULES] =
{
   "Null module",
   "IRON DRAGON",
};

char const *pn[NUM_PERMIT_LEVELS] =
{
   "No permit",
   "Corrupted permit file",
   "Anonymous evaluation",
   "Expired evaluation",
   "Registered evaluation",
   "Fully registered",
};

/* --------------------------------------------------------------------------
   Key values: First the three modulators for moving the bits around within
   the encoded data blocks. Next, the lookup table for converting a 6 bit
   value (0..31) to a set of readable ASCII characters.
   -------------------------------------------------------------------------- */
static int           FS_Mod[ 3]  = {37, 19, 29}; // primes are needed!
static char const   *K_64    =   {"G*sOk4Wc" "?tPl5XdH"
                                  "m6YeAIQu" "JfRBZ7vn"  
                                  "gSC08woK" "L19DxpTh"
                                  "2aEyqUiM" "bVzrNjF3"};


/* --------------------------------------------------------------------------
   (static)   E n c r y p t

   Not used by the game client; included as a reference only.

   The caller passes in a completely filled-out decoded data block, in
   accordance with the commented structure defined earlier in this unit.

   The caller gets back the character blocks of decoded data.

   Additionally, if make_file is set true, this function generates a valid
   output license file, and returns the name of the file created.  The naming
   convention is "license.{X}.{Y}.dat", where X is the module number (1 for
   Iron Dragon), and Y is the permit level (3 for registered).

   Otherwise, the return value is 0, or a negative value for a severe error.
   -------------------------------------------------------------------------- */
char const *Encrypt ( ld_decoded* p_DD,
                      ld_encoded* p_ED, 
                      int         make_file,
                      int         write_buf,
                      uint_08*    p_buf      )
{
   static char fname [32];
   char        s     [120];
   int         mod, pmt, i, j, k, r, bc = 1;
   int         hf = -1;
   char *cap;
   uint_08     c;
   uint_08*    p_bufloc = p_buf;

   if ( write_buf && !p_buf )
      return 0;
   else
      p_buf[0] = 0;

   // first, validate the module id
   //
   if ((mod = p_DD->Module) >= NUM_GAME_MODULES) return (char const *)(-1);

   // second, validate the permit
   //
   if       (strcmp(p_DD->Permit, "Evaluation") == 0) pmt = PERMIT_REGISTERED_EVAL;
   else if  (strcmp(p_DD->Permit, "Registered") == 0) pmt = PERMIT_REGISTERED_FULL;
   else     return (char const *)(-2);

   // now we know enough to make the file handle
   //
   sprintf ( fname, "license.%d.%d.dat", mod, pmt );

   if ( make_file )
   {
      hf = SS_File_IO_Create ( fname );

      if (hf == -1) return (char const *)(-3);
   }

   // nothing can stop us now... let's do it!
   //
   memset(p_ED->Code,0,             10 * 6 * 8);
   memcpy(s,      p_DD->F_Name,     28);
   memcpy(s + 28, p_DD->L_Name,     28);
   memcpy(s + 56, p_DD->Handle,     24);
   memcpy(s + 80, p_DD->Password,   24);
   memcpy(s +104, p_DD->Permit,     12);
   s[116] =       p_DD->Module;
   s[117] =       p_DD->Reg_Year;
   s[118] =       p_DD->Reg_Month;
   s[119] =       p_DD->Reg_Day;

   for (i = 0; i < 120; i++)
   {
      c = s[i];
      for (j = 0; j < 7; j++)
      {
         if ((c & (1 << j)) > 0)
         {
            for (r = 0; r < 3; r++)   // triple redundant encoding
            {
               int ofs = ((bc * FS_Mod[r]) % (10 * 2 * 7 * 6));
               int blk =   ofs / 42;            // [20]: 0..13
               int row =   blk %  10;           // [10]: 0..9
               int col = ((blk /  10) * 3) + r; // [ 6]: 0..5
               int byt =  (ofs % 42) / 6;       // [ 7]: 0..6
               int bit =  (ofs % 42) % 6;       // [ 6]: 0..5

               p_ED->Code[row][col][byt] |= (1 << bit);
         }  }
         bc++;
   }  }

   // post-process on the output, and output to file
   //
   cap = "RAIL EMPIRES - License Data\n\n";
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)cap, strlen(cap) );
   if ( write_buf ) { memcpy ( p_bufloc, cap, strlen(cap) ); p_bufloc+=strlen(cap); }
   
   sprintf(s, "Module   : %s\n", Module_Name_Of(mod));
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)s, strlen(s) );
   if ( write_buf ) { memcpy ( p_bufloc, s, strlen(s) ); p_bufloc+=strlen(s); }

   sprintf(s, "User Name: %s %s\n", p_DD->F_Name, p_DD->L_Name );
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)s, strlen(s) );
   if ( write_buf ) { memcpy ( p_bufloc, s, strlen(s) ); p_bufloc+=strlen(s); }

   sprintf(s, "Handle:    %s\n", p_DD->Handle);
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)s, strlen(s) );
   if ( write_buf ) { memcpy ( p_bufloc, s, strlen(s) ); p_bufloc+=strlen(s); }

   sprintf(s, "Permit   : %s\n", Permit_Name_Of(pmt));
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)s, strlen(s) );
   if ( write_buf ) { memcpy ( p_bufloc, s, strlen(s) ); p_bufloc+=strlen(s); }

   sprintf(s, "Date     : 20%02d-%02d-%02d\n", p_DD->Reg_Year, p_DD->Reg_Month, p_DD->Reg_Day);
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)s, strlen(s) );
   if ( write_buf ) { memcpy ( p_bufloc, s, strlen(s) ); p_bufloc+=strlen(s); }

   cap = "\n<<";
   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)cap, strlen(cap) );
   if ( write_buf ) { memcpy ( p_bufloc, cap, strlen(cap) ); p_bufloc+=strlen(cap); }

   for (i = 0; i < 10; i++)
   {
      cap = "\n   ";
      if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)cap, strlen(cap) );
      if ( write_buf ) { memcpy ( p_bufloc, cap, strlen(cap) ); p_bufloc+=strlen(cap); }

      for (j = 0; j < 6; j++)
      {
         for (k = 0; k < 7; k++)
         {
            p_ED->Code[i][j][k] = K_64[p_ED->Code[i][j][k]];
         }

         if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)p_ED->Code[i][j], 7 );
         
         if ( write_buf ) 
         {
            memcpy ( p_bufloc, p_ED->Code[i][j], 7 ); p_bufloc += 7;
         }

         if (j != 5 && make_file ) 
            SS_File_IO_Write ( hf, (uint_08*)" ", 1 );
         if (j != 5 && write_buf ) 
         { 
            *p_bufloc = ' '; 
            p_bufloc++; 
         }
   }  }

   if (pmt == PERMIT_REGISTERED_EVAL)
      cap = "\n>>\n\n"
            "This full evaluation license is good for 30 days.\n\n"
            "Do not change the contents of this file!  -the Management\n";
   else
      cap = "\n>>\n\n"
            "Do not change the contents of this file!  -the Management\n";

   if ( make_file ) SS_File_IO_Write ( hf, (uint_08*)cap, strlen(cap) );
   if ( write_buf ) { memcpy ( p_bufloc, cap, strlen(cap) ); p_bufloc+=strlen(cap); }
   if ( write_buf ) { *p_bufloc = 0; }

   if (make_file)
   {
      SS_File_IO_Close ( hf );
   }

   return fname;
}


/* --------------------------------------------------------------------------
   (static)   M a k e _ D e c r y p t _ B l o c k _ F r o m _ F i l e

   Uses the << and >> delimeters to parse a .dat file.
   -------------------------------------------------------------------------- */
int  Make_Decrypt_Block_From_File( int hf, ld_encoded* p_ED )
{
   int on = 0, done = 0;
   int i, j, k, q;
   uint_32 NOB, *nob = &NOB;

   memset(p_ED->Code, 0, 10 * 6 * 8);

   for ( i = 0; i < 10; i++)
   {
      for ( j = 0; j < 6; j++)
      {
         for ( k = 0; k < 7; k++)
         {
            done = 0;
            while (!done)
            {
               char c;
               //if (!ReadFile(hf, &c, 1, nob, 0) || (NOB != 1))
               if ( !SS_File_IO_Read ( hf, (uint_08*)&c, 1, nob ) || (NOB != 1))
                  return 0;

               if (!on)
               {
                  if (c == '<') on = 1;
               }
               else
               {
                  for (q = 0; q < 64; q++)
                  {
                     if (c == K_64[q])
                     {
                        p_ED->Code[i][j][k] = c;
                        done = 1;
                        break;
   }  }  }  }  }  }  }
   return 1;
}


/* --------------------------------------------------------------------------
   (static)   D e c r y p t

   Reverse operation of the encryption.
   -------------------------------------------------------------------------- */
int Decrypt ( ld_encoded* p_ED, ld_decoded* p_DD)
{
   ld_encoded  ED2 = *p_ED;
   int         i, j, k, q, r, bc = 1;
   int ofs, blk, row, col, byt, bit;
   uint_08     s[3][120];   memset(s, 0, 3 * 120);

   // reverse map the chars
   for (i = 0; i < 10; i++)
   {
      for ( j = 0; j < 6; j++)
      {
         for ( k = 0; k < 7; k++)
         {
            char c = ED2.Code[i][j][k];
            for ( q = 0; q < 64; q++)
            {
               if (K_64[q] == c)
               {
                  ED2.Code[i][j][k] = (char)q;
                  break;
   }  }  }  }  }

   // reverse sample the bits
   for ( i = 0; i < 120; i++)
   {
      for ( j = 0; j < 7; j++)
      {
         for ( r = 0; r < 3; r++)
         {
            ofs = ((bc * FS_Mod[r]) % (10 * 2 * 7 * 6));
            blk =   ofs / 42;            // [20]: 0..19
            row =   blk %  10;           // [10]: 0..9
            col = ((blk /  10) * 3) + r; // [ 6]: 0..5
            byt =  (ofs % 42) / 6;       // [ 7]: 0..6
            bit =  (ofs % 42) % 6;       // [ 6]: 0..5

            if ((ED2.Code[row][col][byt] & (1 << bit)) > 0)
            {
               s[r][i] |= (1 << j);
         }  }
         bc++;
   }  }

   // compare the three s-lines...
   if (memcmp(s[0], s[1], 120) != 0) return 0;
   if (memcmp(s[0], s[2], 120) != 0) return 0;

   memcpy(p_DD->F_Name,    s[0],      28);
   memcpy(p_DD->L_Name,    s[0] + 28, 28);
   memcpy(p_DD->Handle,    s[0] + 56, 24);
   memcpy(p_DD->Password,  s[0] + 80, 24);
   memcpy(p_DD->Permit,    s[0] +104, 12);
   p_DD->Module         =  s[0][116];
   p_DD->Reg_Year       =  s[0][117];
   p_DD->Reg_Month      =  s[0][118];
   p_DD->Reg_Day        =  s[0][119];

   return 1;
}



/* --------------------------------------------------------------------------
   Our local storage for license data
   -------------------------------------------------------------------------- */
static   ld_decoded  LD_Decoded  [NUM_GAME_MODULES];
static   int         L_Year, L_Month, L_Date;

/* --------------------------------------------------------------------------
   (static)   G e t _ P e r m t _ F o r _ D D _ B l o c k
   -------------------------------------------------------------------------- */
permit_level Get_Permit_For_DD_Block( ld_decoded* p_DD)
{
   if (strcmp(p_DD->Permit, "Registered") == 0)
      return PERMIT_REGISTERED_FULL;

   if (strcmp(p_DD->Permit, "Evaluation") == 0)
   {
      // check the date!
      int   my_yr = 2000 + p_DD->Reg_Year,
            my_mo =        p_DD->Reg_Month,
            my_da =        p_DD->Reg_Day;

      // simply add a month to the reg date.
      if (++my_mo == 13)   {my_mo = 1; my_yr++;}

      if ((L_Year > my_yr)                            ||
          ((L_Year == my_yr) && (L_Month >  my_mo))   ||
          ((L_Year == my_yr) && (L_Month == my_mo) && (L_Date > my_da)))
      {
         return PERMIT_EXPIRED_EVAL;
      }
      return PERMIT_REGISTERED_EVAL;
   }

   if (strcmp(p_DD->Permit, "Corrupted") == 0)
      return PERMIT_CORRUPTED;

   return PERMIT_NONE;
}


/* --------------------------------------------------------------------------
   (global)   G e t _ P e r m t _ F o r _ M o d u l e
   -------------------------------------------------------------------------- */
permit_level Get_Permit_For_Module( int x )
{
   ld_decoded* p_DD = &LD_Decoded[x];
   permit_level pl;

   if (x >= NUM_GAME_MODULES) return PERMIT_NONE;

   pl = Get_Permit_For_DD_Block(p_DD);
   if (pl != PERMIT_NONE) return pl;

   // finally, no explicit file needed for anonymous ID eval, but it is
   // for other modules
   return ((x == 1) ? PERMIT_ANONYMOUS_EVAL : PERMIT_NONE);
}


/* --------------------------------------------------------------------------
   (global)   M o d u l e _ N a m e _ O f
   -------------------------------------------------------------------------- */
const char *Module_Name_Of(int x)
{
   if (x >= NUM_GAME_MODULES) return "Illegal module";

   return mn[x];
}


/* --------------------------------------------------------------------------
   (global)   P e r m i t _ N a m e _ O f
   -------------------------------------------------------------------------- */
const char *Permit_Name_Of(int x)
{
   if (x >= NUM_PERMIT_LEVELS) return "Illegal permit";

   return pn[x];
}

#if 0
/* --------------------------------------------------------------------------
   (global)   L o o k _ F o r _ L i c e n s e s

   When called, looks for license files in the runtime directory, and then
   filters the results into the best-case license.  Any .dat file in the
   runtime directory is attempted as a potential license.
   -------------------------------------------------------------------------- */
void Look_For_Licenses(int l_yr, int l_mo, int l_da)
{
   L_Year   = l_yr;
   L_Month  = l_mo;
   L_Date   = l_da;

#if(0)
   // local test usage of the encrypt function
   ld_decoded  t_DD;
   ld_encoded  t_ED;

   memset(&t_DD, 0, sizeof(t_DD));
   strcpy(t_DD.User_Name, "Martin R. Szinger");
   strcpy(t_DD.Password,  "whetkkilo123");
   strcpy(t_DD.Permit,    "Evaluation"); //"Registered");
   t_DD.Reg_Year     =  0;
   t_DD.Reg_Month    =  4;
   t_DD.Reg_Day      = 29;
   t_DD.Module       = MODULE_IRON_DRAGON;   // 1

   char const *fn    = Encrypt(t_DD, t_ED, true);

   if (fn > 0) Debug_Print("made the license file named %s\n", fn);
   // (end of local test) ------------------------------------- //
#endif


   // normality begis here:
   WIN32_FIND_DATA   w32_fd;
   HANDLE            file_find   = FindFirstFile("*.dat", &w32_fd);
   bool              ok          = (file_find != INVALID_HANDLE_VALUE);

   // here's where we initiale our unit static storage
   memset(LD_Decoded, 0, sizeof(ld_decoded) * NUM_GAME_MODULES);

   // parse each .dat file in the run directory
   while (ok)
   {
      HANDLE         file        = CreateFile(w32_fd.cFileName, GENERIC_READ, 
                                    FILE_SHARE_READ, 0, OPEN_EXISTING, 
                                    FILE_FLAG_SEQUENTIAL_SCAN, 0);

      if (file != INVALID_HANDLE_VALUE)
      {
         // parse and categorize; keep the best one of each module
         ld_encoded ED;
         ld_decoded DD;
         if (Make_Decrypt_Block_From_File(file, ED))
         {
            if (Decrypt(ED, DD))
            {
               permit_level pl_new = Get_Permit_For_DD_Block(DD);
               permit_level pl_old = Get_Permit_For_Module  (DD.Module);

               if (pl_new > pl_old)
               {
                  LD_Decoded[DD.Module] = DD;
            }  }
            else
            {
               // tag up to the corrupted level if untouched so far
               for (int m = 1; m < NUM_GAME_MODULES; m++)
               {
                  if (Get_Permit_For_Module(m) == PERMIT_NONE)
                     strcpy(LD_Decoded[m].Permit, "Corrupted");
         }  }  }
         CloseHandle(file);
      }
      ok = (FindNextFile(file_find, &w32_fd) ? true : false);
   }
   FindClose(file_find);
}
#endif

/* --------------------------------------------------------------------------
   end of file   ( l i c e n s e _ m a n a g e r . c p p )
   -------------------------------------------------------------------------- */
