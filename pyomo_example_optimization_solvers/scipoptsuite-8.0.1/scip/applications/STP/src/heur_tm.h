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

/**@file   heur_tm.h
 * @brief  shortest paths based primal heuristics for Steiner problems
 * @author Gerald Gamrath
 * @author Thorsten Koch
 * @author Daniel Rehfeldt
 * @author Michael Winkler
 *
 * This file implements several shortest paths based primal heuristics for Steiner problems, see
 * "SCIP-Jack - A solver for STP and variants with parallelization extensions" by
 * Gamrath, Koch, Maher, Rehfeldt and Shinano
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_HEUR_TM_H__
#define __SCIP_HEUR_TM_H__

#include "scip/scip.h"
#include "graph.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_HOPFACTOR 0.33


/** TM mode for PC/MW
 *  NOTE: bias_simple is only used for PC/RPC */
enum PCMW_TmMode { pcmode_simple = 0, pcmode_bias = 1, pcmode_biasfull = 2,
                   pcmode_fulltree = 3, pcmode_all = 4, pcmode_biasAndFulltree = 5, pcmode_fromheurdata = 6 };


/** compute starting points among marked (w.r.t. g->mark) vertices for constructive heuristics */
SCIP_EXPORT
void SCIPStpHeurTMCompStarts(
   GRAPH*                graph,              /**< graph data structure */
   int*                  starts,             /**< starting points array */
   int*                  runs                /**< pointer to number of runs */
   );

/** creates the TM primal heuristic and includes it in SCIP */
SCIP_EXPORT
SCIP_RETCODE SCIPStpIncludeHeurTM(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** execute shortest paths heuristic to obtain a Steiner tree */
SCIP_EXPORT
SCIP_RETCODE SCIPStpHeurTMRun(
   SCIP*                 scip,               /**< SCIP data structure */
   enum PCMW_TmMode      pcmw_tmmode,        /**< mode for PC/MW */
   GRAPH*                graph,              /**< graph data structure */
   int*                  starts,             /**< array containing start vertices (NULL to not provide any) */
   const SCIP_Real*      prize,              /**< prizes (for PCMW) or NULL */
   int*                  best_result,        /**< array indicating whether an arc is part of the solution (CONNECTED/UNKNOWN) */
   int                   runs,               /**< number of runs */
   int                   bestincstart,       /**< best incumbent start vertex */
   SCIP_Real*            cost,               /**< arc costs */
   SCIP_Real*            costrev,            /**< reversed arc costs */
   SCIP_Real*            hopfactor,          /**< edge cost multiplicator for HC problems */
   SCIP_Real*            nodepriority,       /**< vertex priorities for vertices to be starting points (NULL for no priorities) */
   SCIP_Bool*            success             /**< pointer to store whether a solution could be found */
   );

/** run shortest path heuristic, but bias edge costs towards best current LP solution */
SCIP_EXPORT
SCIP_RETCODE SCIPStpHeurTMRunLP(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   SCIP_HEUR*            heur,               /**< heuristic or NULL */
   int*                  result,             /**< array indicating whether an arc is part of the solution (CONNECTED/UNKNOWN) */
   int                   runs,               /**< number of runs */
   SCIP_Bool*            success             /**< pointer to store whether a solution could be found */
   );

/** build (rooted) prize collecting Steiner tree in such a way that all leaves are positive-weight vertices */
SCIP_EXPORT
SCIP_RETCODE SCIPStpHeurTMBuildTreePcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph structure */
   SCIP_Bool             useRootSym,         /**< use? */
   PATH*                 mst,                /**< path data structure array */
   const SCIP_Real*      cost,               /**< edge costs */
   SCIP_Real*            objresult,          /**< pointer to store objective value of result */
   int*                  connected           /**< CONNECT/UNKNOWN */
   );

/** build Steiner tree in such a way that all leaves are terminals */
SCIP_EXPORT
void SCIPStpHeurTMBuildTree(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph structure */
   PATH*                 mst,                /**< path data structure array */
   const SCIP_Real*      cost,               /**< edge costs */
   SCIP_Real*            objresult,          /**< pointer to store objective value of result */
   int*                  connected           /**< CONNECT/UNKNOWN */
   );

/** prune a degree constrained Steiner tree in such a way that all leaves are terminals */
SCIP_EXPORT
SCIP_RETCODE SCIPStpHeurTMBuildTreeDc(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph structure */
   int*                  result,             /**< ST edges */
   STP_Bool*             connected           /**< ST nodes */
   );

#ifdef __cplusplus
}
#endif

#endif
