// =---------------------------------------------------------------------------
// mail_encode.c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    MIME Encoding and packaging 
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Apr 17   nic   Created.
//

#include "cgi_id_subsystems.h"
#include "license_manager.h"
#include "databases.h"

// =---------------------------------------------------------------------------
// Forward declarations
//
// =---------------------------------------------------------------------------
int mime_main( int nargs, char *cargs[], uint_08 inbuf[], int insize, char outbuf[] );

// =---------------------------------------------------------------------------
// S e n d _ L i c e n s e _F i l e
//
// =---------------------------------------------------------------------------
int Send_License_File ( const char* email, 
                        const char* FileName, 
                        uint_08*    Lic_Data )
{
   char License_Sender[SYSSET_VALUE_BYTES];

   char  outbuf[10000];
   int   ret;
   char* args[4];
   char  SMTP_Server   [SYSSET_VALUE_BYTES];

   if ( !outbuf ) return -1;

   SysSet_Get_Value ( "SMTP_Server", SMTP_Server );

   args[0] = "mime_main";
   args[1] = (char*)FileName;
   args[2] = (char*)FileName;
   args[3] = "-e";

   ret = mime_main( 4, args, Lic_Data, strlen(Lic_Data), outbuf );

   if ( ret == 0 )
   {
      /*
      SS_HTML_Write ( "<TEXTAREA cols=80 rows=20>" );
      SS_HTML_Write ( "Raw mime data:" );
      SS_HTML_Write ( "%s", outbuf );
      SS_HTML_Write ( "</TEXTAREA>" );
      */
      SysSet_Get_Value ( "License_Sender", License_Sender );
      ret = SS_Net_Send_Mail ( SMTP_Server, License_Sender, email, "Iron Dragon License File", outbuf );
   }
   else
   {
      SS_HTML_WriteP ( "Failed to encode the license file. Please contact Eden Support." );
   }

   return ret;
}