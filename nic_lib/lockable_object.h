// =---------------------------------------------------------------------------
// l o c k a b l e _ o b j e c t . h
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Implements base functionality for all lockable (synchronized) objects
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//    We use UNIX mutexes and condition variables
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jun 29   nic   Created.
//

#ifndef __LOCKABLE_OBJECT_H__
#define __LOCKABLE_OBJECT_H__

class lockable_object
{
public:

   lockable_object();
   ~ lockable_object();

   // =---------------------------------------------
   // (public) synchronization functions
   // =---------------------------------------------
   int Lock       (void);
   int Unlock     (void);
   int Wait       (void);
   int Cond_Signal(void);

protected:

#ifdef UNIX_MULTITHREADED
   pthread_mutex_t lock_mutex;
   pthread_cond_t  lock_cond;
#elif WIN32
   HANDLE          lock_mutex;
#endif

};

// =---------------------------------------------------------------------------
// lock_helper
//
// This neat little class will lock an object (argument) in its ctor and unlock
// that same object in its dtor. Most useful to lock an object within a
// specific calling scope, as in:
//
// int foo()
// {
//    lock_helper save_ferris ( g_lock_obj );
//
//    if ( sys_fun() == -1 )
//    {
//       // no need to unlock object, when lock_helper goes out
//       // of scope, it will automatically unlock g_lock_obj
//       //
//       return -1;
//    }
//
//    ...
//

class lock_helper
{
public:
   lock_helper ( lockable_object& l ) : lockee(l)
   {
      lockee.Lock();
   }

   ~ lock_helper ( )
   {
      lockee.Unlock();
   }

protected:
   lockable_object& lockee;
};

#endif // __LOCKABLE_OBJECT_H__










