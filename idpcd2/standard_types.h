/* --------------------------------------------------------------------------
   s t a n d a r d _ t y p e s . h
   -------------------------------------------------------------------------- */

#ifndef  INC_STANDARD_TYPES_H
#define  INC_STANDARD_TYPES_H

/* -------------------------------------------------------------------------- */
/* WIN16 and WIN32 -- assumed to be X86 CPU. */
#if (defined(WIN16) || defined(WIN32) || defined(UNIX))

typedef    signed       char     sint_08;
typedef  unsigned       char     uint_08;
typedef    signed short int      sint_16;
typedef  unsigned short int      uint_16;
typedef    signed long  int      sint_32;
typedef  unsigned long  int      uint_32;
                                 
typedef                 float    real_32;
typedef                 double   real_64;

#ifndef  MS_WINDOWS
#define  MS_WINDOWS
#endif
#endif   /* WIN16, WIN32 */


/* -------------------------------------------------------------------------- */
/* Alpha. */
#if (defined(ALPHA))

typedef    signed       char     sint_08;
typedef  unsigned       char     uint_08;
typedef    signed short int      sint_16;
typedef  unsigned short int      uint_16;
typedef    signed       int      sint_32;
typedef  unsigned       int      uint_32;

typedef                 float    real_32;
typedef                 double   real_64;

#ifndef  UNIX
#define  UNIX
#endif
#endif   /* ALPHA */


/* -------------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------------
   end of file   ( s t a n d a r d _ t y p e s . h ) .
   -------------------------------------------------------------------------- */
