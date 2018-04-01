
#if WIN32
   #pragma warning( push, 1 )

   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   #include <commctrl.h>
   #include <winsock.h>
   #pragma warning( pop ) 
#endif // WIN32

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#if UNIX
   #include <unistd.h>
   #include <stdarg.h>
   #include <netdb.h>
   #include <errno.h>
#endif // UNIX

#define interface struct

// For the headers found below, make sure ../../nic_lib is in path...
//
#include <result.h>
#include "standard_types.h"
#include <common.h>
#include "net_subsystem.h"
#include "port_subsystem.h"
#include <message.h>
#include "idpcd_defaults.h"

