/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2022 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not visit scipopt.org.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file   branch_stp.h
 * @brief  Steiner vertex branching rule
 * @author Daniel Rehfeldt
 *
 * The Steiner branching rule implemented in this file is described in
 * "A Generic Approach to Solving the Steiner Tree Problem and Variants" by Daniel Rehfeldt.
 * It removes includes and exludes Steiner vertices during branching.
 *
*/
/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_BRANCH_STP_H__
#define __SCIP_BRANCH_STP_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BRANCH_STP_VERTEX_UNSET       -2
#define BRANCH_STP_VERTEX_KILLED      -1
#define BRANCH_STP_VERTEX_NONTERM      0
#define BRANCH_STP_VERTEX_TERM         1


/** parse constraint name and apply changes to graph or array */
SCIP_EXPORT
SCIP_RETCODE STPStpBranchruleParseConsname(
   int*                  vertexchgs,         /**< array to store changes */
   const char*           consname,           /**< constraint name */
   SCIP_Bool*            conflictFound       /**< conflict with existing vertex changes found? */
   );

/** applies vertex changes caused by this branching rule, either on a graph or on an array */
SCIP_EXPORT
SCIP_RETCODE SCIPStpBranchruleGetVertexChgs(
   SCIP*                 scip,               /**< SCIP data structure */
   int*                  vertexchgs,         /**< array to store changes */
   SCIP_Bool*            conflictFound       /**< conflict with existing vertex changes found? */
   );


/** get last change */
SCIP_EXPORT
SCIP_RETCODE SCIPStpBranchruleGetVertexChgLast(
   SCIP*                 scip,               /**< SCIP data structure */
   int*                  vertex,             /**< changed vertex */
   SCIP_Bool*            isDeleted           /**< deleted? (otherwise terminal) */
   );


/** is the branching rule active? */
SCIP_EXPORT
SCIP_Bool SCIPStpBranchruleIsActive(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** applies vertex changes caused by this branching rule, either on a graph or on an array */
SCIP_EXPORT
void SCIPStpBranchruleInitNodeState(
   const GRAPH*          g,                  /**< graph data structure */
   int*                  nodestate           /**< node state array */
   );

/** creates the stp branching rule and includes it to SCIP */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeBranchruleStp(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** returns whether branching-rule is compatible with given graph problem type */
SCIP_EXPORT
SCIP_Bool SCIPStpBranchruleProbIsCompatible(
   const GRAPH*          graph               /**< graph */
);

#ifdef __cplusplus
}
#endif

#endif
