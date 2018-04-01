// =---------------------------------------------------------------------------
// l o c k a b l e _ o b j e c t . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements the guts of an object that needs sychronization (locking)
//    support using a mutex and a condition variable (UNIX)
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jun 23   nic   Created.
//

#include "PreComp.h"

#include "lockable_object.h"

// =---------------------------------------------------------------------------
// (public ctor) lockable_object
// =---------------------------------------------------------------------------
lockable_object::lockable_object()
{
#ifdef UNIX_MULTITHREADED
   pthread_mutex_init ( &lock_mutex, 0 );
   pthread_cond_init ( &lock_cond, 0 );
   
   // for some reason, these initializers gave a strange compile error
   // although they should be correct
   //
   //lock_mutex = PTHREAD_MUTEX_INITIALIZER;
   //lock_cond  = PTHREAD_COND_INITIALIZER;
#elif WIN32
   lock_mutex = CreateMutex ( 0, true, 0 /*unnamed*/ );
   ReleaseMutex ( lock_mutex );
#endif
}

// =---------------------------------------------------------------------------
// (public dtor) ~ lockable_object
// =---------------------------------------------------------------------------
lockable_object::~ lockable_object() 
{
#ifdef UNIX_MULTITHREADED
   pthread_mutex_destroy ( &lock_mutex );
   pthread_cond_destroy ( &lock_cond );
#elif WIN32
   CloseHandle ( lock_mutex );
#endif
}

// =---------------------------------------------------------------------------
// (public, unix)
// 
// =---------------------------------------------------------------------------
int lockable_object::Lock(void)
{
#ifdef UNIX_MULTITHREADED
   return pthread_mutex_lock(&lock_mutex); 
#elif WIN32
   // TODO: Change INFINITE to something else and perform error checking if 
   // we return via timeout
   return WaitForSingleObject ( lock_mutex, INFINITE );
#else
   return 0;
#endif
}
   
// =---------------------------------------------------------------------------
// (public, unix)
// 
// =---------------------------------------------------------------------------
int lockable_object::Unlock(void) 
{
#ifdef UNIX_MULTITHREADED
   return pthread_mutex_unlock(&lock_mutex); 
#elif WIN32
   // TODO: Change INFINITE to something else and perform error checking if 
   // we return via timeout
   return ReleaseMutex ( lock_mutex );
#else
   return 0;
#endif
}
   
// =---------------------------------------------------------------------------
// (public, unix)
// 
// =---------------------------------------------------------------------------
int lockable_object::Wait(void)   
{
#ifdef UNIX_MULTITHREADED
   return pthread_cond_wait(&lock_cond, &lock_mutex); 
#elif WIN32
   // TODO: Do we need the condition variable in Win32?
   return 0;
#else
   return 0;
#endif
}
   
// =---------------------------------------------------------------------------
// (public, unix)
// 
// =---------------------------------------------------------------------------
int lockable_object::Cond_Signal(void) 
{ 
#ifdef UNIX_MULTITHREADED
   return pthread_cond_signal ( &lock_cond ); 
#elif WIN32
   // TODO: Do we need the condition variable in Win32?
   return 0;
#else
   return 0;
#endif
}





