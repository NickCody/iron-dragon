
#if WIN32
   #pragma warning( push, 1 )

   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   #include <commctrl.h>

   #pragma warning( pop ) 
#endif // WIN32

#include <memory.h>

#if UNIX
   #include <unistd.h>
   #include <stdarg.h>
   #include <stdio.h>
   #include <signal.h>
   #include <netdb.h>
   #include <errno.h>
   //#include <sys/time.h>
   //#include <sys/types.h>
   //#include <sys/ioctl.h>
   //#include <sys/socket.h>
   //#include <sys/stat.h>
   //#include <netinet/in.h>
   //#include <arpa/inet.h>
#endif // UNIX

#define interface struct

// For the headers found below, make sure ../../nic_lib is in path...
//
#include <result.h>
#include "../idpcd2/standard_types.h"
#include "../idpcd2/net_subsystem.h"
#include <common.h>
#include <message.h>
//#include <lockable_object.h>
