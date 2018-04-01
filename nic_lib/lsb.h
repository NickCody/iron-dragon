/* --------------------------------------------------------------------------
   l s b . h

   Handles LSB to MSB type conversions
   --------------------------------------------------------------------------

   Date        Who   Description
   ----------- ---   --------------------------------------------------------
   1999-May-12 mrs   Created.

   -------------------------------------------------------------------------- */

#ifndef  INC_LSB_H
#define  INC_LSB_H

/* -------------------------------------------------------------------------- */
#include "standard_types.h"

/* -------------------------------------------------------------------------- */
class lsb
{
public:
   static bool const Is()
   {
      static uint_16 const x = 0xbbee;
      return (*((uint_08 *)&x) == 0xee);
   }

   static uint_08 Do(uint_08 &x) {return x;}
   static sint_08 Do(sint_08 &x) {return x;}

   static uint_16 Do(uint_16 &x)
   {
      if (lsb::Is()) return x;
      uint_16 y;
      uint_08 *xp = (uint_08 *)&x, *yp = (uint_08 *)(&y) + 1;
      *yp-- = *xp++; *yp-- = *xp++;
      x = y;
      return x;
   }

   static sint_16 Do(sint_16 &x)
   {
      if (lsb::Is()) return x;
      sint_16 y;
      uint_08 *xp = (uint_08 *)&x, *yp = (uint_08 *)(&y) + 1;
      *yp-- = *xp++; *yp-- = *xp++;
      x = y;
      return x;
   }

   static uint_32 Do(uint_32 &x)
   {
      if (lsb::Is()) return x;
      uint_32 y;
      uint_08 *xp = (uint_08 *)&x, *yp = (uint_08 *)(&y) + 3;
      *yp-- = *xp++; *yp-- = *xp++; *yp-- = *xp++; *yp-- = *xp++;
      x = y;
      return x;
   }

   static sint_32 Do(sint_32 &x)
   {
      if (lsb::Is()) return x;
      sint_32 y;
      uint_08 *xp = (uint_08 *)&x, *yp = (uint_08 *)(&y) + 3;
      *yp-- = *xp++; *yp-- = *xp++; *yp-- = *xp++; *yp-- = *xp++;
      x = y;
      return x;
   }
};

/* -------------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------------
   end of file   ( l s b . h ) .
   -------------------------------------------------------------------------- */
