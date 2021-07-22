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

/**@file    scipParaInitialStatTh.h
 * @brief   ScipParaInitialStat extension for threads communication.
 * @author  Yuji Shinano
 *
 *
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/


#ifndef __SCIP_PARA_INITIAL_STAT_TH_H__
#define __SCIP_PARA_INITIAL_STAT_TH_H__

#include <iostream>
#include "ug/paraTagDefTh.h"
#include "ug/paraComm.h"
#include "scipParaInitialStat.h"
#include "scip/scip.h"

namespace ParaSCIP
{

/** The initial statistic collecting data class: this is base class */
class ScipParaInitialStatTh : public ScipParaInitialStat
{
   UG::ParaInitialStat *createDatatype(UG::ParaComm *comm);
public:
   /** default constructor */
   ScipParaInitialStatTh(
         )
   {
   }

   /** constructor for clone */
   ScipParaInitialStatTh(
            int inMaxDepth,
            int inMaxTotalDepth,
            int inNVarBranchStatsDown,
            int inNVarBranchStatsUp,
            int *inIdxLBranchStatsVarsDown,
            int *inNVarBranchingDown,
            int *inIdxLBranchStatsVarsUp,
            int *inNVarBranchingUp,
            SCIP_Real *inDownpscost,
            SCIP_Real *inDownvsids,
            SCIP_Real *inDownconflen,
            SCIP_Real *inDowninfer,
            SCIP_Real *inDowncutoff,
            SCIP_Real *inUppscost,
            SCIP_Real *inUpvsids,
            SCIP_Real *inUpconflen,
            SCIP_Real *inUpinfer,
            SCIP_Real *inUpcutoff
            )
   {
      maxDepth = inMaxDepth;
      maxTotalDepth = inMaxTotalDepth;
      nVarBranchStatsDown = inNVarBranchStatsDown;
      nVarBranchStatsUp = inNVarBranchStatsUp;
      idxLBranchStatsVarsDown = inIdxLBranchStatsVarsDown;
      nVarBranchingDown = inNVarBranchingDown;
      idxLBranchStatsVarsUp = inIdxLBranchStatsVarsUp;
      nVarBranchingUp = inNVarBranchingUp;
      downpscost = inDownpscost;
      downvsids = inDownvsids;
      downconflen = inDownconflen;
      downinfer = inDowninfer;
      downcutoff = inDowncutoff;
      uppscost = inUppscost;
      upvsids = inUpvsids;
      upconflen = inUpconflen;
      upinfer = inUpinfer;
      upcutoff = inUpcutoff;
   }

   /** constructor to create this object */
   ScipParaInitialStatTh(
      SCIP *scip
      ) : ScipParaInitialStat(scip) 
   {
   }

   /** destractor */
   virtual ~ScipParaInitialStatTh()
   {
   }

   /** user should implement send method */
   void send(UG::ParaComm *comm, int dest);

   /** user should implement receive method */
   void receive(UG::ParaComm *comm, int source);
};

}

#endif    // __SCIP_PARA_INITIAL_STAT_TH_H__
