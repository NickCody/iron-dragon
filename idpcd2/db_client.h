
#ifndef __DB_CLIENT_H__
#define __DB_CLIENT_H__

// =---------------------------------------------------------------------------
// struct client_record 
//
// The client_record is stored on disk and is 256-bytes big. The
// 
// For the client password, we define two constants: buffer size and user size
// * the buffer size, MAX_CLIENT_PASSWORD, defines the raw size of the pwd buf
// * the user size, MAX_CLIENT_PASSWORD_LEN, defines the maximum characters
// that the user can enter. Rational: encrypted password may be larger than raw
// password
//
// The size of the client_record must be 256 bytes or less, thus SLUSH_SIZE
// needs to be adjusted for every added member
//
// =---------------------------------------------------------------------------

#define MAX_CLIENT_FNAME         20
#define MAX_CLIENT_LNAME         20
#define MAX_CLIENT_USERNAME      24 /* Same as PLAYER_NAME_BYTES */
#define MAX_CLIENT_PASSWORD      24 /* PASSWORD_BUFFER_BYTES     */
#define MAX_CLIENT_EMAIL         32
#define CONNECTS_FIELD_LEN        4
#define LASTGROUP_FIELD_LEN       2
#define STATUS_FLAG_FIELD_LEN     1
                                    /* subtotal 127 : 12/1/99 */

#define REGISTRATION_LEN          1 /* 1 byte code, 0=not paid, 1=paid, etc. */
                                    /* subtotal 1 : 4/7/00 */

#define ADDRESS1_LEN             30
#define ADDRESS2_LEN             30
#define CITY_LEN                 12
#define STATEPROVINCE_LEN        14
#define COUNTRYCODE_LEN           4
#define AGE_LEN                   1 /* 1 byte                        */
#define GENDER_LEN                1 /* 1 byte, 'M' or 'F'            */
#define ZIPCODE_LEN              10 /* 10 bytes for xxxxx-xxxx */
                                    /* subtotal: 101 : 4/7/00         */
#define RATING_LEN                2
#define WIN_COUNT                 2
#define LOSS_COUNT                2
#define DISCONNECT_COUNT          2
#define CLIENT_SLUSH_SIZE        18
//                      -----------
//                      (total) 256

#define MAX_CLIENT_PASSWORD_LEN  12

// some constants:

typedef enum DBCLIENTSTATUS
{
   DBCLIENTSTATUS_START    = 0,
   DBCLIENTSTATUS_NORMAL   = 0,
   DBCLIENTSTATUS_DELETED  = 1,
   DBCLIENTSTATUS_END      = 1
} DBCLIENTSTATUS;

typedef enum DBREGISTRATION
{
   DBREGISTRATION_START       = 0,
   DBREGISTRATION_NONE        = 0,
   DBREGISTRATION_EVALREG     = 1,
   DBREGISTRATION_REGISTERED  = 2,
   DBREGISTRATION_END         = 2
} DBREGISTRATION;

#if UNIX
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#if WIN32
#pragma pack ( push ,1 )
#endif

// all multi-byte numerics are in network-byte order
//
typedef struct client_record
{
   char    username[MAX_CLIENT_USERNAME]     PACKED;
   char    password[MAX_CLIENT_PASSWORD]     PACKED;
   char    fname[MAX_CLIENT_FNAME]           PACKED;
   char    lname[MAX_CLIENT_LNAME]           PACKED;
   char    email[MAX_CLIENT_EMAIL]           PACKED;
   uint_32 connects                          PACKED;  // network byte order
   uint_16 lastgroup_uid                     PACKED;  // network byte order
   uint_08 status                            PACKED;
   uint_08 registration                      PACKED;
   char    address1[ADDRESS1_LEN]            PACKED;
   char    address2[ADDRESS2_LEN]            PACKED;
   char    city[CITY_LEN]                    PACKED;
   char    state_province[STATEPROVINCE_LEN] PACKED;
   char    country_code[COUNTRYCODE_LEN]     PACKED;
   uint_08 age                               PACKED;
   uint_08 gender                            PACKED; // M or F
   char    zip_code[ZIPCODE_LEN]             PACKED; // xxxxx-xxxx
   sint_16 rating                            PACKED; // actually signed, so cast away
   uint_16 wins                              PACKED;
   uint_16 losses                            PACKED;
   uint_16 disconnects                       PACKED;
   char    _slush[CLIENT_SLUSH_SIZE]         PACKED;

} client_record;

#if WIN32
#pragma pack ( pop )
#endif

#endif // __DB_CLIENT_H__
