/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1996-2020 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file  spxautopr.h
 * @brief Auto pricer.
 */
#ifndef _SPXAUTOPR_H_
#define _SPXAUTOPR_H_

#include <assert.h>

#include "soplex/spxpricer.h"
#include "soplex/spxdevexpr.h"
#include "soplex/spxsteeppr.h"
#include "soplex/spxsteepexpr.h"


namespace soplex
{

/**@brief   Auto pricer.
   @ingroup Algo

   This pricer switches between Devex and Steepest edge pricer based on the difficulty of the problem
   which is determined by the number of iterations.

   See SPxPricer for a class documentation.
*/
template <class R>
class SPxAutoPR : public SPxPricer<R>
{
private:

   int            switchIters;   ///< number of iterations before switching pricers
   SPxPricer<R>*     activepricer;  ///< pointer to currently selected pricer
   SPxDevexPR<R>     devex;         ///< internal Devex pricer
   SPxSteepExPR<R>     steep;         ///< internal Steepest edge pricer

   bool setActivePricer(typename SPxSolverBase<R>::Type
                        type);          ///< switches active pricing method

public:

   //-------------------------------------
   /**@name Constructors / destructors */
   ///@{
   /// default constructor
   SPxAutoPR()
      : SPxPricer<R>("Auto")
      , switchIters(10000)
      , activepricer(&devex)
      , devex()
      , steep()
   {}
   /// copy constructor
   SPxAutoPR(const SPxAutoPR& old)
      : SPxPricer<R>(old)
      , switchIters(old.switchIters)
      , devex(old.devex)
      , steep(old.steep)
   {
      assert(old.activepricer == &old.devex || old.activepricer == &old.steep);

      if(old.activepricer == &old.devex)
         activepricer = &devex;
      else
         activepricer = &steep;
   }
   /// assignment operator
   SPxAutoPR& operator=(const SPxAutoPR& rhs)
   {
      if(this != &rhs)
      {
         SPxPricer<R>::operator=(rhs);
         switchIters = rhs.switchIters;
         devex = rhs.devex;
         steep = rhs.steep;

         assert(rhs.activepricer == &rhs.devex || rhs.activepricer == &rhs.steep);

         if(rhs.activepricer == &rhs.devex)
            activepricer = &devex;
         else
            activepricer = &steep;
      }

      return *this;
   }
   /// destructor
   virtual ~SPxAutoPR()
   {}
   /// clone function for polymorphism
   inline virtual SPxPricer<R>* clone() const
   {
      return new SPxAutoPR(*this);
   }
   ///@}

   //-------------------------------------
   /**@name Access / modification */
   ///@{
   /// set max number of iterations before switching pricers
   void setSwitchIters(int iters);
   /// clear the data
   void clear();
   /// set epsilon of internal pricers
   void setEpsilon(R eps);
   /// set the solver
   virtual void load(SPxSolverBase<R>* base);
   /// set entering/leaving algorithm
   virtual void setType(typename SPxSolverBase<R>::Type);
   /// set row/column representation
   virtual void setRep(typename SPxSolverBase<R>::Representation);
   ///
   virtual int selectLeave();
   ///
   virtual SPxId selectEnter();
   ///
   virtual void left4(int n, SPxId id);
   ///
   virtual void entered4(SPxId id, int n);
   ///@}
};
} // namespace soplex

// For general templated functions
#include "spxautopr.hpp"

#endif // _SPXAUTOPR_H_
