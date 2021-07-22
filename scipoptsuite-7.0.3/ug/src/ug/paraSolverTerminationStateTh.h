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

/**@file    paraSolverTerminationStateTh.h
 * @brief   ParaSolverTerminationState extension for threads communication.
 * @author  Yuji Shinano
 *
 *
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/


#ifndef __PARA_SOLVER_TERMINATION_STATE_TH_H__
#define __PARA_SOLVER_TERMINATION_STATE_TH_H__

#include "paraSolverTerminationState.h"

namespace UG
{

/** Calculation state in a ParaSolver */
class ParaSolverTerminationStateTh : public ParaSolverTerminationState
{
   /** create ParaNode datatype */
   ParaSolverTerminationStateTh *createDatatype();
public:
   /** default constructor */
   ParaSolverTerminationStateTh(){}
   /** constructor */
   ParaSolverTerminationStateTh(
         int          inInterrupted,                /**< indicate that this solver is interrupted or not. 0: not interrupted, 1: interrputed */
         int          inRank,                       /**< rank of this solver */
         int          inTotalNSolved,               /**< accumulated number of nodes solved in this ParaSolver */
         int          inMinNSolved,                 /**< minimum number of subtree nodes rooted from ParaNode */
         int          inMaxNSolved,                 /**< maximum number of subtree nodes rooted from ParaNode */
         int          inTotalNSent,                 /**< accumulated number of nodes sent from this ParaSolver */
         int          inTotalNImprovedIncumbent,    /**< accumulated number of improvements of incumbent value in this ParaSolver */
         int          inNParaNodesReceived,         /**< number of ParaNodes received in this ParaSolver */
         int          inNParaNodesSolved,           /**< number of ParaNodes solved ( received ) in this ParaSolver */
         int          inNParaNodesSolvedAtRoot,     /**< number of ParaNodes solved at root node before sending  */
         int          inNParaNodesSolvedAtPreCheck, /**< number of ParaNodes solved at pre-checking of root node solvability */
         int          inNTransferredLocalCutsFromSolver,      /**< number of local cuts transferred from this Solver */
         int          inMinTransferredLocalCutsFromSolver,    /**< minimum number of local cuts transferred from this Solver  */
         int          inMaxTransferredLocalCutsFromSolver,    /**< maximum number of local cuts transferred from this Solver  */
         int          inNTransferredBendersCutsFromSolver,    /**< number of benders cuts transferred from this Solver */
         int          inMinTransferredBendersCutsFromSolver,  /**< minimum number of benders cuts transferred from this Solver  */
         int          inMaxTransferredBendersCutsFromSolver,  /**< maximum number of benders cuts transferred from this Solver  */
         int          inNTotalRestarts,             /**< number of total restarts */
         int          inMinRestarts,                /**< minimum number of restarts */
         int          inMaxRestarts,                /**< maximum number of restarts */
         int          inNTightened,                 /**< number of tightened variable bounds during racing stage */
         int          inNTightenedInt,              /**< number of tightened integral variable bounds during racing stage */
         double       inRunningTime,                /**< this solver running time */
         double       inIdleTimeToFirstParaNode,    /**< idle time to start solving the first ParaNode */
         double       inIdleTimeBetweenParaNodes,   /**< idle time between ParaNodes processing */
         double       inIdleTimeAfterLastParaNode,  /**< idle time after the last ParaNode was solved */
         double       inIdleTimeToWaitNotificationId,   /**< idle time to wait notification Id messages  */
         double       inIdleTimeToWaitAckCompletion,    /**< idle time to wait ack completion message  */
         double       inIdleTimeToWaitToken,        /**< idle time to wait token  */
         double       inTotalRootNodeTime,          /**< total time consumed by root node processes */
         double       inMinRootNodeTime,            /**< minimum time consumed by root node processes */
         double       inMaxRootNodeTime,            /**< maximum time consumed by root node processes */
         double       inDetTime                     /**< deterministic time, -1: should be non-deterministic */
   ) : ParaSolverTerminationState(
               inInterrupted,
               inRank,
               inTotalNSolved,
               inMinNSolved,
               inMaxNSolved,
               inTotalNSent,
               inTotalNImprovedIncumbent,
               inNParaNodesReceived,
               inNParaNodesSolved,
               inNParaNodesSolvedAtRoot,
               inNParaNodesSolvedAtPreCheck,
               inNTransferredLocalCutsFromSolver,
               inMinTransferredLocalCutsFromSolver,
               inMaxTransferredLocalCutsFromSolver,
               inNTransferredBendersCutsFromSolver,
               inMinTransferredBendersCutsFromSolver,
               inMaxTransferredBendersCutsFromSolver,
               inNTotalRestarts,
               inMinRestarts,
               inMaxRestarts,
               inNTightened,
               inNTightenedInt,
               inRunningTime,
               inIdleTimeToFirstParaNode,
               inIdleTimeBetweenParaNodes,
               inIdleTimeAfterLastParaNode,
               inIdleTimeToWaitNotificationId,
               inIdleTimeToWaitAckCompletion,
               inIdleTimeToWaitToken,
               inTotalRootNodeTime,
               inMinRootNodeTime,
               inMaxRootNodeTime,
               inDetTime ){}
   void send(ParaComm *comm, int destination, int tag);
   void receive(ParaComm *comm, int source, int tag);
};

}

#endif // __PARA_SOLVER_TERMINATION_STATE_TH_H__

