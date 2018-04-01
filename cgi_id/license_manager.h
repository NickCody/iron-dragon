/* --------------------------------------------------------------------------
   l i c e n s e _ m a n a g e r . h

   (c) 2000 by Martin R. Szinger and Nicholas Codignotto
   --------------------------------------------------------------------------

   Date        Who   Description
   ----------  ---   --------------------------------------------------------
   2000-03-29  mrs   Created (Eden's PC version of Mayfair's Iron Dragon).
   -------------------------------------------------------------------------- */

#ifndef  INC_LICENSE_MANAGER_H
#define  INC_LICENSE_MANAGER_H

/* -------------------------------------------------------------------------- */
typedef enum game_module
{
   MODULE_NULL          = 0,
   MODULE_IRON_DRAGON   = 1,
   NUM_GAME_MODULES     = 2,
} game_module;

/* -------------------------------------------------------------------------- */
typedef enum permit_level
{
   PERMIT_NONE = 0,        // no access (non-ID modules)
   PERMIT_CORRUPTED,       // corrupted license
   PERMIT_ANONYMOUS_EVAL,  // turn limits, no online play
   PERMIT_EXPIRED_EVAL,    // after 30 days...
   PERMIT_REGISTERED_EVAL, // full game/net access for 30 days
   PERMIT_REGISTERED_FULL, // full game/net access forever
   NUM_PERMIT_LEVELS,
} permit_level;

/* --------------------------------------------------------------------------
   Encoded data is comprised of 60 blocks of 7 characters each.  The blocks
   are arranged in 10 rows, 6 columns.  Each character holds 6 bits of data,
   for a total of 60 * 7 * 6 = 2520 bits.  The data is triple-redundant, so
   only 840 unique bits; these are groups into 7-character output characters,
   so ultimately a grand total of 120 7-bit bytes of data are represented.
   -------------------------------------------------------------------------- */
typedef struct ld_encoded
{
   char     Code[10][6][8]; // 8 = 7 real characters plus one null.
} ld_encoded;

/* -------------------------------------------------------------------------- */
typedef struct ld_decoded
{                          // (byte)
   char     F_Name   [28]; //   0.. 27
   char     L_Name   [28]; //  28.. 55
   char     Handle   [24]; //  56.. 79    Unique ID for online database
   char     Password [24]; //  80..103
   char     Permit   [12]; // 104..115    "Evaluation" or "Registered"
   uint_08  Module;        // 116         value of 1 for ID;
   uint_08  Reg_Year;      // 117         value of X for 2000 + X AD
   uint_08  Reg_Month;     // 118         1..12
   uint_08  Reg_Day;       // 119         1..31
} ld_decoded;


/* -------------------------------------------------------------------------- */

void           Look_For_Licenses    (int l_year, int l_month, int l_date);
permit_level   Get_Permit_For_Module(int x);
const char    *Module_Name_Of       (int x);
const char    *Permit_Name_Of       (int x);

char const *Encrypt ( ld_decoded* p_DD,
                      ld_encoded* p_ED, 
                      int         make_file,
                      int         write_buf,
                      uint_08*    p_buf      );

/* -------------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------------
   end of file   ( l i c e n s e _ m a n a g e r . h )
   -------------------------------------------------------------------------- */
