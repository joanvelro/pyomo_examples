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
/*  along with SCIP; see the file COPYING. If not visit scip.zib.de.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   mst.h
 *
 *
 * @brief  minimum spanning tree based algorithms for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file encompasses various minimum spanning tree based algorithms.
 * Note: This file is supposed to (partly) replace graph_path.c in the long run, as it includes the faster implementations.
 *
 */
#ifndef APPLICATIONS_STP_SRC_MST_H_
#define APPLICATIONS_STP_SRC_MST_H_

#include "graph.h"
#include "completegraph.h"


#ifdef __cplusplus
extern "C" {
#endif



/** information for (sparse) MST computations */
typedef
struct minimum_spanning_tree
{
   const CSR*            csr;                /**< CSR */
   DHEAP*                dheap;              /**< Dijkstra heap */
   SCIP_Real* RESTRICT   nodes_dist;         /**< distance array (on vertices) */
   int* RESTRICT         nodes_predEdge;     /**< predecessor edge array (on vertices);
                                                NOTE: with respect to original graph edge IDs
                                                NOTE: might contain uninitialized values in opt mode! */
} MST;


extern SCIP_RETCODE mst_init(SCIP*, const GRAPH*, MST**);
extern void mst_free(SCIP*, MST**);
extern void mst_computeOnMarked(const GRAPH*, const STP_Bool*, int, MST*);
extern SCIP_Real mst_getObj(const GRAPH*, const MST*);
extern void mst_getSoledges(const GRAPH*, const MST*, int* RESTRICT);


#ifdef __cplusplus
}
#endif



#endif /* APPLICATIONS_STP_SRC_MST_H_ */
