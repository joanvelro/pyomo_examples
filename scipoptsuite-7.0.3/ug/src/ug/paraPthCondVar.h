/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*             This file is part of the program and software framework       */
/*                  UG --- Ubquity Generator Framework                       */
/*                                                                           */
/*    Copyright (C) 2002-2020 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  UG is distributed under the terms of the ZIB Academic Licence.           */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with UG; see the file COPYING. If not email to scip@zib.de.        */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file    paraPthCondVar.h
 * @brief   Pthread condition variable for UG Framework
 * @author  Yuji Shinano
 *
 * Many thanks to Daniel Junglas.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __PARA_PTH_COND_VAR_H__
#define __PARA_PTH_COND_VAR_H__

#include <cassert>
#include "paraPthLock.h"

namespace UG
{

/** Condition variable.
 * This class wraps around pthread_cond_t. A condition variable is always
 * associated with a mutex (which must be held when the condition controlled
 * by the condition variable is modified). There are two ways to associate
 * a mutex (an instance of Lock) with a condition variable:
 * 1. Create the Lock instance and provide a pointer to it to the
 *    ConditionVariable constructor.
 * 2. Provide a NULL pointer to the ConditionVariable constructor. In that
 *    case the constructor will allocate a new Lock instance (and the
 *    destructor will destroy that lock).
 */
class ConditionVariable {
   /** The lock associated with this instance. */
   Lock *lck;
   /** This is true iff the constructor did allocate lck. */
   bool ownMutex;
   /** The low-level condition variable. */
   pthread_cond_t cond;

   // No copying or assignment of condition variables.
   ConditionVariable(ConditionVariable const &);
   ConditionVariable &operator=(ConditionVariable const &);
public:
   /** Create a new condition variable.
    * If l is NULL then the constructor will create a new Lock instance and
    * will associate this with the newly created condition variable. The Lock
    * instance will be destroyed in the destructor.
    * If l is not NULL then the constructor will associate the newly created
    * condition variable with l (the destructor will _not_ delete l).
    */
   ConditionVariable(Lock *l = 0) : lck(l), ownMutex(l == 0) {
      if ( ownMutex )
         lck = new Lock(); // May throw LockException or std::bad_alloc
      int const error = pthread_cond_init(&cond, 0);
      if ( error ) {
         if ( ownMutex )
            delete lck;
         throw LockException(error);
      }
   }

   /** Destroy this condition variable.
    * If the constructor allocated a new Lock for this condition variable
    * then this Lock is also destroyed.
    */
   ~ConditionVariable() {
      pthread_cond_destroy(&cond);
      if ( ownMutex )
         delete lck;
   }

   /* set Lock from outside */
   void setLock(Lock *l)
   {
      if( ownMutex ) delete lck;
      lck = l;
      ownMutex = false;
   }

   /** Acquire the lock associated with this condition variable. */
   void lock() { lck->lock(); }
   /** Acquire the lock associated with this condition variable. */
   void lock(char const *file, int line) { lck->lock(file, line); }
   /** Release the lock associated with this condition variable. */
   void unlock() { lck->unlock(); }

   /** Wait for this condition variable.
    * The calling thread MUST hold the lock associated with this condition
    * variable when calling this function.
    */
   void wait() {
      int const error = pthread_cond_wait(&cond, &lck->mtx);
      if ( error )
         throw LockException(error);
   }
   /** Signal this condition variable.
    * Signalling a condition variable will wake up ONE thread that is
    * currently waiting on the variable.
    */
   void signal() {
      int const error = pthread_cond_signal(&cond);
      if ( error )
         throw LockException(error);
   }

   /** Signal on this condition variable.
    * Broadcasting on a condition variable will wake up ALL threads that are
    * currently waiting on the variable.
    */
   void broadcast() {
      int const error = pthread_cond_broadcast(&cond);
      if ( error )
         throw LockException(error);
   }
   /** Get the lock associated with this condition variable. */
   Lock *getLock() const { return lck; }
};

/** Perform exception-safe waiting on a condition variable.
 * This macro waits on VAR until COND is true.
 */
#define CONDITIONVARIABLE_WAIT(var,cond) do {                   \
      LockRAII raiiwait_((var)->getLock(), __FILE__, __LINE__); \
      while (!(cond))                                           \
         (var)->wait();                                         \
   } while (0)

}
#endif // __PARA_PTH_COND_VAR_H__
