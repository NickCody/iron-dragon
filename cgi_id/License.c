
#include "standard_types.h"
#include "License.h"
#include "net_messages.h"

#include <memory.h>

uint_08* Static_License = 
   // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234
     (uint_08*)"DrWeQaO9Mf^8N73fl)*$bh&^5$%hl95bhk*(64gjO0nNjJGH^%3dDB>>?d{+08^$#FVMll,o((&%32fB<//esdsadweeddf";

// =---------------------------------------------------------------------------
// C h e c k _ F o r _ A d m i n i s t r a t o r
//
// This function is obsolete
//
// Returns 1 if License_Data verifies and we are from an authenticated
//   domain
//
//int Check_Admin_License( uint_08* License_Data )
//{
   //uint_08 Lic[LICENSE_DATA_BYTES];
   
   //Create_Admin_License ( Lic );

   //if ( memcmp ( Lic, License_Data, LICENSE_DATA_BYTES ) != 0 )
   //   return 0;

   // TODO: Check for primordia.com, irondragon.org or Kronyth(167.206.195.103)


// return 0;
//}

// =---------------------------------------------------------------------------
// C r e a t e _ A d m i n _ L i c e n s e
//
// =---------------------------------------------------------------------------
void Create_Admin_License ( uint_08* Lic )
{
   memcpy ( Lic, Static_License, LICENSE_DATA_BYTES );
}

// =---------------------------------------------------------------------------
// A u t o_ G e n e r a t e _ P a s s w o r d 
// =---------------------------------------------------------------------------
const char* Auto_Generate_Password ( )
{
   return "autopwd";
}
