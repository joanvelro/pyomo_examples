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

/**@file    scipParaCommMpi.h
 * @brief   SCIP ParaComm extension for MPI communication.
 * @author  Yuji Shinano
 *
 *
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/


#ifndef __SCIP_PARA_COMM_MPI_H__
#define __SCIP_PARA_COMM_MPI_H__
#include "ug/paraCommMpi.h"
#include "scipDiffParamSet.h"
#include "scipParaInstanceMpi.h"
#include "scipParaSolutionMpi.h"
#include "scipParaDiffSubproblemMpi.h"
#include "scipParaInitialStatMpi.h"
#include "scipParaRacingRampUpParamSetMpi.h"
#include "scip/scip.h"
#ifdef UG_WITH_UGS
#include "ugs/ugsParaCommMpi.h"
#endif

namespace ParaSCIP
{

class ScipParaSolver;

class ScipParaCommMpi : public UG::ParaCommMpi
{
public:
#ifdef UG_WITH_UGS
   ScipParaCommMpi(){}
   ScipParaCommMpi( MPI_Comm inComm) : ParaCommMpi(inComm)
   {
      MPI_CALL(
         MPI_Comm_rank(myComm, &myRank)
      );
      MPI_CALL(
         MPI_Comm_size(myComm, &myCommSize)
      );
   }
   void setUgsComm( UGS::UgsParaCommMpi *inCommUgs )
   {
      commUgs = inCommUgs;
   }
#else
   ScipParaCommMpi(){}
#endif
   ~ScipParaCommMpi(){}
   void init(int argc, char **argv)
   {
      UG::ParaCommMpi::init(argc,argv);
#ifdef UG_WITH_UGS
      if( !commUgs )
      {
         myComm = MPI_COMM_WORLD;
         MPI_CALL(
            MPI_Comm_rank(myComm, &myRank)
         );
         MPI_CALL(
            MPI_Comm_size(myComm, &myCommSize)
         );
      }
#else
      myComm = MPI_COMM_WORLD;
      MPI_CALL(
         MPI_Comm_rank(myComm, &myRank)
      );
      MPI_CALL(
         MPI_Comm_size(myComm, &myCommSize)
      );
#endif
   }
   /*******************************************************************************
   * transfer object factory
   *******************************************************************************/
   UG::ParaDiffSubproblem *createParaDiffSubproblem();
   UG::ParaInitialStat* createParaInitialStat();
   UG::ParaRacingRampUpParamSet* createParaRacingRampUpParamSet();
   UG::ParaInstance *createParaInstance();
   UG::ParaSolution *createParaSolution();

   ScipParaInstance *createScipParaInstance(SCIP *scip, int method);
   ScipParaSolution *createScipParaSolution(ScipParaSolver *solver, SCIP_Real objval, int inNvars, SCIP_VAR ** vars, SCIP_Real *vals);
   ScipParaSolution *createScipParaSolution(SCIP_Real objval, int inNvars, int *inIndicesAmongSolvers, SCIP_Real *vals);
   ScipParaDiffSubproblem *createScipParaDiffSubproblem(         
            SCIP *scip,
            ScipParaSolver *scipParaSolver,
            int nNewBranchVars,
            SCIP_VAR **newBranchVars,
            SCIP_Real *newBranchBounds,
            SCIP_BOUNDTYPE *newBoundTypes,
            int nAddedConss,
            SCIP_CONS **addedConss
         );
   ScipParaInitialStat *createScipParaInitialStat(SCIP *scip);
   ScipParaInitialStat *createScipParaInitialStat(
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
         );
   ScipParaRacingRampUpParamSet *createScipParaRacingRampUpParamSet(
         int inTerminationCriteria,
         int inNNodesLeft,
         double inTimeLimit,
         int inScipRacingParamSeed,
         int inPermuteProbSeed,
         int inGenerateBranchOrderSeed,
         ScipDiffParamSet *inScipDiffParamSet
         );
   ScipDiffParamSet *createScipDiffParamSet();
   ScipDiffParamSet *createScipDiffParamSet( SCIP *scip );
};

#define DEF_SCIP_PARA_COMM( scip_para_comm, comm ) ScipParaCommMpi *scip_para_comm = dynamic_cast< ScipParaCommMpi* >(comm)

}
#endif // __SCIP_PARA_COMM_MPI_H__
