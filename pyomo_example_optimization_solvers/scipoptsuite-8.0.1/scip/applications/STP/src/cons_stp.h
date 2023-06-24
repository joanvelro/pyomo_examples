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

/**@file   cons_stp.h
 * @brief  Constraint handler for Steiner problems
 * @author Gerald Gamrath
 * @author Daniel Rehfeldt
 * @author Michael Winkler
 *
 * This file checks solutions for feasibility and separates violated model constraints. For more details see \ref STP_CONS page.
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_CONS_STP_H__
#define __SCIP_CONS_STP_H__


#include "scip/scip.h"
#include "graph.h"

#ifdef __cplusplus
extern "C" {
#endif


/** creates the handler for element constraints and includes it in SCIP */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeConshdlrStp(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** creates and captures a stp constraint */
SCIP_EXPORT
SCIP_RETCODE SCIPcreateConsStp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS**           cons,               /**< pointer to hold the created constraint */
   const char*           name,               /**< name of constraint */
   GRAPH*                graph               /**< graph data structure */
   );

/** sets graph */
SCIP_EXPORT
void SCIPStpConshdlrSetGraph(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** add cut corresponding to contraction */
SCIP_EXPORT
SCIP_RETCODE SCIPStpAddContractionCut(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             edge,               /**< edge */
   SCIP_VAR*             revedge,            /**< reversed edge */
   SCIP_Bool             localcut            /**< add local cut? */
   );

/** returns implications start array */
SCIP_EXPORT
const int* SCIPStpGetPcImplStarts(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** returns number implications starts */
SCIP_EXPORT
int SCIPStpGetPcImplNstarts(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** returns implications vertices array */
SCIP_EXPORT
const int* SCIPStpGetPcImplVerts(
   SCIP*                 scip                /**< SCIP data structure */
   );


#ifdef __cplusplus
}
#endif

#endif
