/* --------------------------------------------------------------------------
   r e s o u r c e s . h

   Enumerations and structures related to the NMRF resource file format.

   --------------------------------------------------------------------------

   Date        Who   Description
   ----------- ---   --------------------------------------------------------
   1999-May-12 mrs   Created.

   -------------------------------------------------------------------------- */

#ifndef  INC_RESOURCES_H
#define  INC_RESOURCES_H

/* -------------------------------------------------------------------------- */
#include "lsb.h"

/* --------------------------------------------------------------------------
   Requisite header for any valid resource file

   It is always 64 bytes long, starting with the 'NMRF' key, and followed
   by a uint_32 format code.  The remaining 56 bytes are format-specific.

   Currently, only format 1 is defined.  As we identify flags needed to
   support this format, we'll carve them out of the slack area.
   -------------------------------------------------------------------------- */
struct nmrf_header
{
	char	   NMRF[4];    // identifier ‘NMRF’; also the default file extension.
	uint_32	Format;	   // = 1, would change for any future new formats.
	char	   Slack[56];  // slack bytes to ensure header is 64-bytes long.

   bool As_LSB()
   {
      lsb::Do(Format);
      return true;
   }
};


/* --------------------------------------------------------------------------
   Requisite header that precedes each resource in an NMRF file.
   -------------------------------------------------------------------------- */
enum {RF_NAMELEN = 32};

struct resource_header
{
   uint_32  Header_Size;
   char     Name[RF_NAMELEN]; // null terminated
   uint_16  Type;             // must be on the valid list of type codes.
   uint_08  Compression;      // must be on the valid list of compressions.
   uint_08  Encryption;       // must be on the valid list of encryptions.
   uint_32  Version;          // resolves name collisions.
   uint_32  File_Size;        // # bytes of file data following the header.
   uint_32  Real_Size;        // # bytes of expanded/decoded data.

   bool As_LSB()
   {
      lsb::Do(Header_Size);
      lsb::Do(Type);
      lsb::Do(Compression);
      lsb::Do(Encryption);
      lsb::Do(Version);
      lsb::Do(File_Size);
      lsb::Do(Real_Size);

      return true;
   }
};


/* --------------------------------------------------------------------------
   Known and supported compression types
   -------------------------------------------------------------------------- */
enum resource_compression
{
   RF_COMPRESS_NONE  = 0,
   RF_COMPRESS_GZIP  = 1,
};


/* --------------------------------------------------------------------------
   Known and supported encryption types
   -------------------------------------------------------------------------- */
enum resource_encryption
{
   RF_ENCRYPT_NONE           = 0,
   RF_ENCRYPT_WINDOWED_XOR   = 1,
};


/* --------------------------------------------------------------------------
   The list of known and supported resource types
   -------------------------------------------------------------------------- */
enum resource_type
{
   RF_TYPE_NONE                                       = 0,
   RF_TYPE_UNCOMPRESSED_256_COLOR_BITMAP              = 1,
   RF_TYPE_UNCOMPRESSED_GRAYSCALE_BITMAPPED_FONT      = 2,
   RF_TYPE_ASCII_BASED_STRING_COLLECTION              = 3,
// RF_TYPE_UNCOMPRESSED_TRUE_COLOR_PLUS_ALPHA_BITMAP  = 4,
   RF_TYPE_WAVE_AUDIO                                 = 21,
   RF_TYPE_MISC                                       = 999
};


/* --------------------------------------------------------------------------
   CONTENTS of TYPE 1 (RF_TYPE_UNCOMPRESSED_256_COLOR_BITMAP)
   --------------------------------------------------------------------------
   uint_32  : bpw, bitmap width in pixels
   uint_32  : bph, bitmap height in pixels
   uint_32  : npe, number of palette entries
   uint_32[]: array of size (npe), colors in 0x00rrggbb (?) format
   uint_08[]: array of size (bpw x bph), pixels in range (0..npe - 1)
              data is arranged "top-down".
   -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
   CONTENTS of TYPE 2 (RF_TYPE_UNCOMPRESSED_GRAYSCALE_BITMAPPED_FONT)
   --------------------------------------------------------------------------
   uint_32	: fw, font bitmap width in pixels.
   uint_32	: fh, font bitmap height in pixels.
   uint_16	: cl, ID of first (low) legal character in range
   uint_16	: ch, ID of last (high) legal character in range
   uint_16	: Bitmap width for first character (CL)
   uint_16	: Kerning base index for first character (CL), 0 for no kerning
   ...	     4-byte Width/Kerning pairs for remaining characters
   uint_32	: nk, number of kerning table entries
   uint_16	: ID of preceding character; 0 to end this kerning section
   sint_16	: Number of pixels to add/remove from inter-character spacing
   ...	      4-byte ID/Spacing entry pairs for kerning entries 2..NK
   uint_08[]: array of size (fw x fh), font bitmap pixels, 
               0 for fully transparent, to 255 for full opacity.
               Pixels are row-ordered, top down, left to right.
               Characters are "vertically stacked" in order within the bitmap.
   -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
   CONTENTS of TYPE 3 (RF_TYPE_ASCII_BASED_STRING_COLLECTION)
   --------------------------------------------------------------------------
   uint_32  : ns, number of strings in this collection
   uint_32  : internal ID of string #1
   uint_32  : offset to string #1 from beginning of string data block
   ...      : 8-byte ID/Offset pairs for remaining strings
   uint_08[]: string data block, series of ns null-terminated strings
   -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
   CONTENTS of TYPE 21 (RF_TYPE_WAVE_AUDIO)
   --------------------------------------------------------------------------
   uint_08	: src, sample rate code:  1 = 44 kHz, 2 = 22 kHz, 3 = 11 kHz
   uint_08	: nc, number of channels: 1 = mono, 2 = stereo
   uint_08  : bps, bytes per sample:  1 = 8-bit unsigned, 2 = 16-bit signed
   uint_08	: 0
   uint_32	: dl, data length: number of bytes of audio data
   uint_08[]: raw audio data, dl bytes
   -------------------------------------------------------------------------- */

#endif
/* --------------------------------------------------------------------------
   end of file   ( r e s o u r c e s . h ) .
   -------------------------------------------------------------------------- */
