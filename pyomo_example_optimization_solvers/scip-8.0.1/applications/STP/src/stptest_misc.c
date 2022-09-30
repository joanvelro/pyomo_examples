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

/**@file   stptest_misc.c
 * @brief  tests for Steiner tree methods
 * @author Daniel Rehfeldt
 *
 * This file implements tests for Steiner problem heuristics.
 *
 * A list of all interface methods can be found in stptest.h.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
//#define SCIP_DEBUG
#include <stdio.h>
#include <assert.h>
#include "stptest.h"
#include "graph.h"
#include "completegraph.h"
#include "heur_local.h"
#include "heur_tm.h"


static
SCIP_RETCODE graphBuildV5E5(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH**               g,                  /**< the graph */
   SCIP_Bool             pc                  /**< create pc graph? */
)
{
   GRAPH* graph;
   const int nnodes = 5;
   const int nedges = 5;

   SCIP_CALL(graph_init(scip, g, nnodes, 2 * nedges, 1));
   graph = *g;

   for( int i = 1; i < nnodes; i++ )
      graph_knot_add(graph, -1);

   graph_knot_add(graph, 0);

   graph->source = 0;

   graph_edge_add(scip, graph, 0, 1, 1.0, 1.0);
   graph_edge_add(scip, graph, 1, 2, 1.0, 1.0);
   graph_edge_add(scip, graph, 1, 4, 1.0, 1.0);
   graph_edge_add(scip, graph, 2, 3, 1.0, 1.0);
   graph_edge_add(scip, graph, 3, 4, 1.0, 1.0);

   if( pc )
   {
      SCIP_CALL( graph_pc_initPrizes(scip, graph, nnodes) );

      for( int i = 0; i < nnodes; i++ )
         graph->prize[i] = 0.0;

      graph->prize[0] = 1.0;

      SCIP_CALL( graph_transPc(scip, graph) );
   }

   SCIP_CALL(graph_initHistory(scip, graph));
   SCIP_CALL(graph_path_init(scip, graph));

   return SCIP_OKAY;
}

static
SCIP_RETCODE pseudoDelSingle(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   const int nnodes = 4;
   const int nedges = 3;
   SCIP_Real cutoff[] = {FARAWAY, FARAWAY, FARAWAY};
   SCIP_Bool success;

   SCIP_CALL(graph_init(scip, &graph, nnodes, 2 * nedges, 1));

   for( int i = 1; i < nnodes; i++ )
      graph_knot_add(graph, -1);

   graph_knot_add(graph, 0);

   graph->source = 0;

   graph_edge_add(scip, graph, 0, 1, 1.0, 1.0);
   graph_edge_add(scip, graph, 0, 2, 1.0, 1.0);
   graph_edge_add(scip, graph, 0, 3, 1.0, 1.0);

   SCIP_CALL(graph_initHistory(scip, graph));
   SCIP_CALL(graph_path_init(scip, graph));


   for( int e = 0; e < nedges; e += 2 )
      assert(graph_edge_nPseudoAncestors(graph, e) == 0);

   SCIP_CALL( graph_knot_delPseudo(scip, graph, cutoff, NULL, NULL, 0, NULL, &success) );

   assert(success);

   for( int e = 0; e < nedges; e += 2 )
   {
      assert(graph_edge_nPseudoAncestors(graph, e) == 1);
      assert(graph_edge_getPseudoAncestors(graph, e)[0] == 0);
   }


   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}


static
SCIP_RETCODE pseudoDelDouble(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   const int nnodes = 5;
   const int nedges = 4;
   SCIP_Real cutoff[] = {FARAWAY, FARAWAY, FARAWAY};
   SCIP_Bool success;

   SCIP_CALL(graph_init(scip, &graph, nnodes, 2 * nedges, 1));

   for( int i = 1; i < nnodes; i++ )
      graph_knot_add(graph, -1);

   graph_knot_add(graph, 0);

   graph->source = 0;

   graph_edge_add(scip, graph, 0, 1, 1.0, 1.0);
   graph_edge_add(scip, graph, 0, 2, 1.0, 1.0);
   graph_edge_add(scip, graph, 0, 3, 1.0, 1.0);
   graph_edge_add(scip, graph, 3, 4, 1.0, 1.0);

   SCIP_CALL(graph_initHistory(scip, graph));
   SCIP_CALL(graph_path_init(scip, graph));

   for( int e = 0; e < nedges; e += 2 )
      assert(graph_edge_nPseudoAncestors(graph, e) == 0);

   SCIP_CALL( graph_knot_delPseudo(scip, graph, cutoff, NULL, NULL, 0, NULL, &success) );
   assert(success);

   for( int e = graph->outbeg[1]; e != EAT_LAST; e = graph->oeat[e] )
   {
      if( graph->head[e] == 2 )
      {
         graph_edge_del(scip, graph, e, TRUE);

         break;
      }
   }

   SCIP_CALL( graph_knot_delPseudo(scip, graph, cutoff, NULL, NULL, 3, NULL, &success) );
   assert(success);

   assert(graph->grad[1] == 1);
   assert(graph->grad[2] == 1);
   assert(graph->grad[4] == 2);


   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}

static
SCIP_RETCODE pseudoAncestorsCreation(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   int pseudoancestor;

   assert(scip);

   SCIP_CALL( graphBuildV5E5(scip, &graph, FALSE) );
   assert(graph->knots == 5 && graph->edges == 10);

   graph_addPseudoAncestor(graph, &pseudoancestor);
   STPTEST_ASSERT(pseudoancestor == 0);
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 0, graph) );
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 0) == 1);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 1) == 1);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 2) == 0);

   graph_addPseudoAncestor(graph, &pseudoancestor);
   STPTEST_ASSERT(pseudoancestor == 1);
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 1, graph) );
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 0) == 2);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 1) == 2);

   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   STPTEST_ASSERT(pseudoancestor == 4);
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 1, 4, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 1, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 1, 2, graph) );
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 0) == 5);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 1) == 5);

   graph_edge_delPseudoAncestors(scip, 1, graph);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 0) == 0);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 1) == 0);

   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 2, graph) );
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 0) == 1);
   STPTEST_ASSERT(graph_edge_nPseudoAncestors(graph, 1) == 1);

   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}


static
SCIP_RETCODE pseudoAncestorsMerge(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   SCIP_Bool conflict;
   int pseudoancestor;

   assert(scip);

   SCIP_CALL( graphBuildV5E5(scip, &graph, FALSE) );
   assert(graph->knots == 5 && graph->edges == 10);

   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 2, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 4, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 4, 4, graph) );
   SCIP_CALL( graph_pseudoAncestors_appendMoveEdge(scip, 0, 4, FALSE, graph, &conflict)  );
   STPTEST_ASSERT(conflict);
   STPTEST_ASSERT( graph_edge_nPseudoAncestors(graph, 0) == 3 );
   STPTEST_ASSERT( graph_edge_nPseudoAncestors(graph, 4) == 0);

   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 6, 1, graph) );
   SCIP_CALL( graph_pseudoAncestors_appendMoveEdge(scip, 0, 6, FALSE, graph, &conflict)  );
   STPTEST_ASSERT(!conflict);
   STPTEST_ASSERT( graph_edge_nPseudoAncestors(graph, 0) == 4 );
   STPTEST_ASSERT( graph_edge_nPseudoAncestors(graph, 6) == 0);

   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}


static
SCIP_RETCODE pseudoAncestorsMergePc(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   SCIP_Bool conflict;

   assert(scip);

   SCIP_CALL( graphBuildV5E5(scip, &graph, TRUE) );
   assert(graph->knots > 5 && graph->edges > 10);

   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 0, 2, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 0, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 4, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 4, 4, graph) );
   SCIP_CALL( graph_pseudoAncestors_appendMoveNode(scip, 0, 4, FALSE, graph, &conflict)  );
   assert(conflict);
   assert( graph_knot_nPseudoAncestors(graph, 0) == 3 );
   assert( graph_knot_nPseudoAncestors(graph, 4) == 0);


   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 2, 1, graph) );
   SCIP_CALL( graph_pseudoAncestors_appendMoveNode(scip, 0, 2, FALSE, graph, &conflict)  );
   assert(!conflict);
   assert( graph_knot_nPseudoAncestors(graph, 0) == 4 );
   assert( graph_knot_nPseudoAncestors(graph, 2) == 0);


   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}


static
SCIP_RETCODE pseudoAncestorsHash(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   SCIP_Bool conflict;
   int* hasharr;
   int pseudoancestor;

   assert(scip);

   SCIP_CALL( graphBuildV5E5(scip, &graph, FALSE) );
   assert(graph->knots == 5 && graph->edges == 10);

   SCIP_CALL( SCIPallocCleanBufferArray(scip, &hasharr, graph_pseudoAncestorsGetHashArraySize(graph->pseudoancestors)) );

   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   graph_addPseudoAncestor(graph, &pseudoancestor);
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 2, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 0, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 4, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 4, 4, graph) );
   graph_pseudoAncestors_hashEdge(graph->pseudoancestors, 0, hasharr);
   graph_pseudoAncestors_hashEdgeDirty(graph->pseudoancestors, 4, TRUE, &conflict, hasharr);
   STPTEST_ASSERT(conflict);

   SCIP_CALL( graph_pseudoAncestors_addToEdge(scip, 6, 1, graph) );
   graph_pseudoAncestors_hashEdgeDirty(graph->pseudoancestors, 6, TRUE, &conflict, hasharr);
   STPTEST_ASSERT(!conflict);

   graph_pseudoAncestors_unhashEdge(graph->pseudoancestors, 6, hasharr);
   graph_pseudoAncestors_unhashEdge(graph->pseudoancestors, 0, hasharr);

   for( int k = 0; k < graph->knots; k++ )
      STPTEST_ASSERT(hasharr[k] == 0);

   SCIPfreeCleanBufferArray(scip, &hasharr);

   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}

static
SCIP_RETCODE pseudoAncestorsHashPc(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   GRAPH* graph;
   SCIP_Bool conflict;
   int* hasharr;

   assert(scip);

   SCIP_CALL( graphBuildV5E5(scip, &graph, TRUE) );
   assert(graph->knots > 5 && graph->edges > 10);

   SCIP_CALL( SCIPallocCleanBufferArray(scip, &hasharr, graph_pseudoAncestorsGetHashArraySize(graph->pseudoancestors)) );

   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 0, 2, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 0, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 4, 3, graph) );
   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 4, 4, graph) );
   graph_pseudoAncestors_hashNode(graph->pseudoancestors, 0, hasharr);
   graph_pseudoAncestors_hashNodeDirty(graph->pseudoancestors, 4, TRUE, &conflict, hasharr);
   assert(conflict);


   SCIP_CALL( graph_pseudoAncestors_addToNode(scip, 2, 1, graph) );
   graph_pseudoAncestors_hashNodeDirty(graph->pseudoancestors, 2, TRUE, &conflict, hasharr);
   assert(!conflict);

   graph_pseudoAncestors_unhashNode(graph->pseudoancestors, 2, hasharr);
   graph_pseudoAncestors_unhashNode(graph->pseudoancestors, 0, hasharr);

   for( int k = 0; k < graph->knots; k++ )
      assert(hasharr[k] == 0);

   SCIPfreeCleanBufferArray(scip, &hasharr);

   graph_path_exit(scip, graph);
   graph_free(scip, &graph, TRUE);

   return SCIP_OKAY;
}




/** extension test */
static
SCIP_RETCODE completegraph(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   CGRAPH* cgraph;
   const int maxnnodes = 11;

   SCIP_CALL( cgraph_init(scip, &cgraph, maxnnodes) );

   assert(0 == cgraph->nnodes_curr);
   assert(maxnnodes == cgraph->nnodes_max);

   cgraph_node_append(cgraph, 1);

   if( !cgraph_valid(cgraph) )
   {
      SCIPdebugMessage("cgraph not valid! \n");
      return SCIP_ERROR;
   }

   cgraph_node_deleteTop(cgraph);

   if( !cgraph_valid(cgraph) )
   {
      SCIPdebugMessage("cgraph not valid! \n");
      return SCIP_ERROR;
   }

   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 3);

   cgraph_node_deleteTop(cgraph);

   if( cgraph->nnodes_curr != 1 )
   {
      SCIPdebugMessage("wrong node count cgraph not valid! \n");
      return SCIP_ERROR;
   }

   if( !cgraph_valid(cgraph) )
   {
      SCIPdebugMessage("cgraph not valid! \n");
      return SCIP_ERROR;
   }

   cgraph_node_deleteTop(cgraph);

   if( cgraph->nnodes_curr != 0 )
   {
      SCIPdebugMessage("wrong node count cgraph not valid! \n");
      return SCIP_ERROR;
   }

   if( !cgraph_valid(cgraph) )
   {
      SCIPdebugMessage("cgraph not valid! \n");
      return SCIP_ERROR;
   }


   cgraph_free(scip, &cgraph);

   return SCIP_OKAY;
}


/** mst test */
static
SCIP_RETCODE completemst1(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   CGRAPH* cgraph;
   CMST* cmst;
   const int maxnnodes = 7;
   SCIP_Real adjedges1[] = { FARAWAY, 2.0, 3.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges2[] = { 2.0, FARAWAY, 3.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges3[] = { 1.0,  0.1, FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };

   SCIP_CALL( cgraph_init(scip, &cgraph, maxnnodes) );
   SCIP_CALL( cmst_init(scip, &cmst, maxnnodes) );

   cgraph_node_append(cgraph, 1);
   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 3);

   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges1, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 0, 1);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges2, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 1, 2);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges3, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 2, 3);

   cgraph_node_repositionTop(cgraph, 2);

   cmst_computeMst(cgraph, 0, cmst);

   if( !SCIPisEQ(scip, cmst->mstobj, 1.0) )
   {
      printf("completemst1: wrong obj: %f  \n", cmst->mstobj);
      return SCIP_ERROR;
   }

   if( cmst->predecessors[0] != -1 || cmst->predecessors[1] != 0 )
   {
      printf("completemst1: wrong ancestor \n");
      return SCIP_ERROR;
   }

   cmst_free(scip, &cmst);
   cgraph_free(scip, &cgraph);

   return SCIP_OKAY;
}


/** mst test */
static
SCIP_RETCODE completemst2(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   CGRAPH* cgraph;
   CMST* cmst;
   const int maxnnodes = 3;
   SCIP_Real adjedges1[] = { FARAWAY, 1.0, 2.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges2[] = { 1.0, FARAWAY, 2.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges3[] = { 2.0, 1.0 , FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };

   SCIP_CALL( cgraph_init(scip, &cgraph, maxnnodes) );
   SCIP_CALL( cmst_init(scip, &cmst, maxnnodes) );

   cgraph_node_append(cgraph, 1);
   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 77);


   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges1, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 0, 1);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges2, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 1, 2);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges3, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 2, 77);

   cmst_computeMst(cgraph, 0, cmst);

   if( !SCIPisEQ(scip, cmst->mstobj, 2.0) )
   {
      printf("completemst2: wrong obj: %f \n", cmst->mstobj);
      return SCIP_ERROR;
   }

   if( cmst->predecessors[0] != -1 || cmst->predecessors[1] != 0 || cmst->predecessors[2] != 1 )
   {
      printf("completemst2: wrong ancestor \n");
      return SCIP_ERROR;
   }

   cmst_free(scip, &cmst);
   cgraph_free(scip, &cgraph);

   return SCIP_OKAY;
}


/** mst test */
static
SCIP_RETCODE completemst3(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   CGRAPH* cgraph;
   CMST* cmst;
   const int maxnnodes = 3;
   SCIP_Real adjedges1[] = { FARAWAY, 1.0, 2.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges2[] = { 1.0, FARAWAY, 1.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges3[] = { 2.0, 2.0 , FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };

   SCIP_CALL( cgraph_init(scip, &cgraph, maxnnodes) );
   SCIP_CALL( cmst_init(scip, &cmst, maxnnodes) );

   cgraph_node_append(cgraph, 1);
   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 77);

   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges1, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 0, 1);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges2, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 1, 2);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges3, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 2, 77);

   cmst_computeMst(cgraph, 0, cmst);

   if( !SCIPisEQ(scip, cmst->mstobj, 2.0) )
   {
      printf("completemst2: wrong obj: %f \n", cmst->mstobj);
      return SCIP_ERROR;
   }

   if( cmst->predecessors[0] != -1 || cmst->predecessors[1] != 0 || cmst->predecessors[2] != 1 )
   {
      printf("completemst2: wrong ancestor \n");
      return SCIP_ERROR;
   }

   cmst_free(scip, &cmst);
   cgraph_free(scip, &cgraph);

   return SCIP_OKAY;
}


/** mst test */
static
SCIP_RETCODE completemst4(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   CGRAPH* cgraph;
   CMST* cmst;
   const int maxnnodes = 3;
   SCIP_Real adjedges1[] = { FARAWAY, 1.0, 2.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges2[] = { 1.0, FARAWAY, 1.0, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges3[] = { 2.0, 2.0 , FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };

   SCIP_CALL( cgraph_init(scip, &cgraph, maxnnodes) );
   SCIP_CALL( cmst_init(scip, &cmst, maxnnodes) );

   cgraph_node_append(cgraph, 1);
   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 77);

   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges1, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 0, 1);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges2, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 1, 2);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges3, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 2, 77);

   cmst_computeMst(cgraph, 0, cmst);

   cgraph_node_deleteTop(cgraph);
   cgraph_node_deleteTop(cgraph);

   cgraph_node_append(cgraph, 7);
   cgraph_node_append(cgraph, 88);

   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges2, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 1, 7);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges3, 4);
   cgraph_node_applyMinAdjCosts(cgraph, 2, 88);

   cmst_computeMst(cgraph, 0, cmst);

   if( !SCIPisEQ(scip, cmst->mstobj, 2.0) )
   {
      printf("completemst2: wrong obj: %f \n", cmst->mstobj);
      return SCIP_ERROR;
   }

   if( cmst->predecessors[0] != -1 || cmst->predecessors[1] != 0 || cmst->predecessors[2] != 1 )
   {
      printf("completemst2: wrong ancestor \n");
      return SCIP_ERROR;
   }

   cmst_free(scip, &cmst);
   cgraph_free(scip, &cgraph);

   return SCIP_OKAY;
}



/** mst test */
static
SCIP_RETCODE completemst5(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   CGRAPH* cgraph;
   CMST* cmst;
   const int maxnnodes = 7;
   SCIP_Real adjedges1[] = { FARAWAY, FARAWAY, FARAWAY, FARAWAY, FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges2[] = { 2.0, FARAWAY, FARAWAY, FARAWAY, FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges3[] = { 4.0, 3.0, FARAWAY, FARAWAY, FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges4[] = { 4.0, 4.0, 1.0, FARAWAY, FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };
   SCIP_Real adjedges5[] = { 1.0, 1.0, 2.0, 3.0, FARAWAY, CGRAPH_EDGECOST_UNDEFINED_VALUE };

   SCIP_CALL( cgraph_init(scip, &cgraph, maxnnodes) );
   SCIP_CALL( cmst_init(scip, &cmst, maxnnodes) );

   cgraph_node_append(cgraph, 0);
   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 3);
   cgraph_node_append(cgraph, 4);

   cgraph_node_deleteTop(cgraph);
   cgraph_node_deleteTop(cgraph);
   cgraph_node_deleteTop(cgraph);
   cgraph_node_deleteTop(cgraph);

   cgraph_node_append(cgraph, 1);
   cgraph_node_append(cgraph, 2);
   cgraph_node_append(cgraph, 3);
   cgraph_node_append(cgraph, 7);
   cgraph_node_append(cgraph, 66);

   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges1, 6);
   cgraph_node_applyMinAdjCosts(cgraph, 0, 1);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges2, 6);
   cgraph_node_applyMinAdjCosts(cgraph, 1, 2);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges3, 6);
   cgraph_node_applyMinAdjCosts(cgraph, 2, 3);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges4, 6);
   cgraph_node_applyMinAdjCosts(cgraph, 3, 7);
   BMScopyMemoryArray(cgraph->adjedgecosts, adjedges5, 6);
   cgraph_node_applyMinAdjCosts(cgraph, 4, 66);

   cmst_computeMst(cgraph, 3, cmst);

   if( !SCIPisEQ(scip, cmst->mstobj, 5.0) )
   {
      SCIPdebugMessage("wrong obj: %f  \n", cmst->mstobj);
      return SCIP_ERROR;
   }

   if( cmst->predecessors[0] != 4 || cmst->predecessors[1] != 4  || cmst->predecessors[4] != 2 || cmst->predecessors[2] != 3 )
   {
      SCIPdebugMessage("wrong ancestor \n");
      return SCIP_ERROR;
   }

   cmst_free(scip, &cmst);
   cgraph_free(scip, &cgraph);

   return SCIP_OKAY;
}



/** test pseudo ancestors */
SCIP_RETCODE stptest_pseudoAncestors(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   assert(scip);

   SCIP_CALL( pseudoAncestorsCreation(scip) );
   SCIP_CALL( pseudoAncestorsMerge(scip) );
   SCIP_CALL( pseudoAncestorsHash(scip) );
   SCIP_CALL( pseudoAncestorsMergePc(scip) );
   SCIP_CALL( pseudoAncestorsHashPc(scip) );

   printf("pseudoAncestors_test passed \n");

   return SCIP_OKAY;
}

/** test pseudo deletion */
SCIP_RETCODE stptest_pseudoDel(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   assert(scip);

   SCIP_CALL( pseudoDelSingle(scip) );
   SCIP_CALL( pseudoDelDouble(scip) );

   printf("pseudoDeletion test: all ok \n");

   return SCIP_OKAY;
}



SCIP_RETCODE stptest_dheap(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   DHEAP* heap = NULL;

   int min = -1;
   graph_heap_create(scip, 13, NULL, NULL, &heap);
   assert(heap != NULL);

   graph_heap_correct(1, 2.0, heap);
   graph_heap_correct(2, 1.0, heap);
   graph_heap_correct(0, 1.5, heap);

   assert(heap->size == 3);

   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 2);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 0);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 1);

   assert(heap->size == 0);
   graph_heap_clean(TRUE, heap);


   graph_heap_correct(1, 2.0, heap);
   graph_heap_correct(2, 2.7, heap);
   graph_heap_correct(0, 1.5, heap);
   graph_heap_correct(2, 1.9, heap);
   graph_heap_correct(4, 0.5, heap);

   assert(heap->size == 4);

   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 4);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 0);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 2);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 1);

   assert(heap->size == 0);
   graph_heap_clean(TRUE, heap);

   graph_heap_correct(1, 2.0, heap);
   graph_heap_correct(2, 3.0, heap);
   graph_heap_correct(0, 1.5, heap);
   graph_heap_correct(2, 1.6, heap);
   graph_heap_correct(12, 22.5, heap);
   graph_heap_correct(12, 7.7, heap);
   graph_heap_correct(4, 8.5, heap);


   assert(heap->size == 5);

   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 0);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 2);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 1);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 12);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 4);


   assert(heap->size == 0);


   graph_heap_free(scip, TRUE, TRUE, &heap);
   assert(heap == NULL);

   graph_heap_create(scip, 3, NULL, NULL, &heap);

   graph_heap_correct(1, 2.0, heap);
   graph_heap_correct(2, 3.0, heap);
   graph_heap_correct(0, 1.5, heap);
   graph_heap_correct(2, 2.5, heap);

   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 0);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 1);
   graph_heap_deleteMinGetNode(&min, heap);
   assert(min == 2);

   assert(heap->size == 0);

   graph_heap_free(scip, TRUE, TRUE, &heap);
   assert(heap == NULL);

   printf("stptest_dheap: all ok \n");

   return SCIP_OKAY;
}



SCIP_RETCODE stptest_completegraph(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   SCIP_CALL( completegraph(scip) );
   SCIP_CALL( completemst1(scip) );
   SCIP_CALL( completemst2(scip) );
   SCIP_CALL( completemst3(scip) );
   SCIP_CALL( completemst4(scip) );
   SCIP_CALL( completemst5(scip) );

   printf("stptest_completegraph: all ok \n");

   return SCIP_OKAY;
}
