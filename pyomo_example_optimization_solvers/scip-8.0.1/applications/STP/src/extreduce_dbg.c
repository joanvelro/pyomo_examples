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

/**@file   extreduce_dbg.c
 * @brief  extended reductions for Steiner tree problems
 * @author Daniel Rehfeldt
 *
 * This file implements extended reduction debugging routines for several Steiner problems.
 *
 * A list of all interface methods can be found in reduce.h.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
// #define SCIP_DEBUG
// #define STP_DEBUG_EXTPC // use if special sds for PC are deactivated

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "graph.h"
#include "portab.h"
#include <limits.h>
#include "extreduce.h"


/** get SD MST weight
 *  NOTE: might deviate because only getSd is used...maybe only use double in the code? */
static
SCIP_Real sdmstGetWeight(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   nnodes,             /**< number of nodes for MST computation */
   const int             nodes[],            /**< nodes (from graph) for MST computation */
   SCIP_Bool             computeLower,       /**< compute lower bound? (otherwise upper) */
   EXTDATA*              extdata             /**< extension data */
   )
{
   SCIP_Real mstweight;
   CGRAPH* cgraph;
   CMST* cmst;
   SCIP_Real* adjcosts;

   SCIP_CALL_ABORT( cgraph_init(scip, &cgraph, STP_EXTTREE_MAXNLEAVES_GUARD) );
   SCIP_CALL_ABORT( cmst_init(scip, &cmst, STP_EXTTREE_MAXNLEAVES_GUARD) );

   adjcosts = cgraph->adjedgecosts;

   assert(adjcosts);
   assert(nnodes > 0 && nnodes <= STP_EXTTREE_MAXNLEAVES_GUARD);

   //printf(" \n");

   /* build the MST graph */
   for( int i = 0; i < nnodes; i++ )
      cgraph_node_append(cgraph, i);

   for( int i = 0; i < nnodes; i++ )
   {
      const int startnode = nodes[i];

      for( int j = 0; j < nnodes; j++ )
      {
         SCIP_Real specialDist;

         if( i == j )
         {
            specialDist = FARAWAY;
         }
         else
         {
            const int endnode = nodes[j];

            if( computeLower )
            {
               specialDist = extreduce_extGetSdDouble(scip, graph, startnode, endnode, extdata);
            }
            else
            {
               const SCIP_Real sd1 = extreduce_extGetSd(scip, graph, startnode, endnode, extdata);
               const SCIP_Real sd2 = extreduce_extGetSd(scip, graph, endnode, startnode, extdata);

               if( EQ(sd1, -1.0) || EQ(sd2, -1.0) )
               {
                  specialDist = -1.0;
               }
               else
               {
                  specialDist = MAX(sd1, sd2);
               }
            }

            if( specialDist <= 0.0 )
            {
               assert(EQ(specialDist, -1.0));
               specialDist = BLOCKED;
            }

          //  printf("sd %d->%d:  %f \n", startnode, endnode, specialDist);
         }

         adjcosts[j] = specialDist;
      }

      cgraph_node_applyMinAdjCosts(cgraph, i, i);
   }

   /* compute the MST */
   cmst_computeMst(cgraph, 0, cmst);

   mstweight = cmst->mstobj;

   cmst_free(scip, &cmst);
   cgraph_free(scip, &cgraph);

   assert(GE(mstweight, 0.0));

   if( GE(mstweight, BLOCKED) )
   {
      mstweight = FARAWAY;
   }

   return mstweight;
}


/** gets MST weight for SD MST spanning all leaves and extension vertex
 *  NOTE: only for debugging! very slow!
 * */
static
SCIP_Real sdmstGetExtWeight(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   extvert,            /**< extended vertex */
   SCIP_Bool             computeLower,       /**< compute lower bound? (otherwise upper) */
   EXTDATA*              extdata             /**< extension data */
)
{
   SCIP_Real mstweight;
   int* extleaves;
   const int* const leaves = extdata->tree_leaves;
   const int nleaves = extdata->tree_nleaves;

   assert(extvert >= 0 && extvert < graph->knots);

   SCIP_CALL_ABORT( SCIPallocBufferArray(scip, &extleaves, STP_EXTTREE_MAXNLEAVES_GUARD + 1) );

   assert(nleaves <= STP_EXTTREE_MAXNLEAVES_GUARD);

   for( int i = 0; i < nleaves; ++i )
   {
      const int leaf = leaves[i];
      assert(extvert != leaf);

      extleaves[i] = leaf;
   }

   extleaves[nleaves] = extvert;

   mstweight = sdmstGetWeight(scip, graph, nleaves + 1, extleaves, computeLower, extdata);

   SCIPfreeBufferArray(scip, &extleaves);

   return mstweight;
}


/** gets nodes of top levelbase MST */
static
void mstTopLevelBaseGetNodes(
   int                   extnode,            /**< extension nodes */
   int                   nnodes,             /**< number of nodes */
   int                   nodes[],            /**< nodes (from graph) to be filled in */
   const EXTDATA*        extdata             /**< extension data */
)
{
   const int* const leaves = extdata->tree_leaves;
   int j = 0;
   const int nleaves = extdata->tree_nleaves;

   assert(nnodes == nleaves - 1);

   SCIPdebugMessage("mstTopLevel nodes: \n");

   for( int i = 0; i < nleaves; ++i )
   {
      const int leaf = leaves[i];

      if( leaf == extnode )
         continue;

      SCIPdebugMessage("   %d \n", leaf);

      nodes[j] = leaf;

      j++;
   }

   assert(j == nnodes);
}


/** is the weight of the top levelbase MST valid? */
static
SCIP_Bool mstTopLevelBaseValidWeight(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   base_nnodes,        /**< number of nodes */
   int                   base_nodes[],       /**< MST nodes (from graph) */
   const CSR*            mst_base,           /**< the stored MST */
   EXTDATA*              extdata             /**< extension data */
)
{
   const SCIP_Real mstweight_org = reduce_dcmstGetWeight(scip, mst_base);
   const SCIP_Real mstweight_upper = sdmstGetWeight(scip, graph, base_nnodes, base_nodes, FALSE, extdata);
   SCIP_Bool isValid = TRUE;

   assert(base_nnodes == mst_base->nnodes);

   if( GT(mstweight_org, mstweight_upper) )
   {
      SCIPdebugMessage("top baselevel MST weight violates upper bound: %f > %f \n",
          mstweight_org, mstweight_upper);

      isValid = FALSE;
   }
   else
   {
      const SCIP_Real mstweight_lower = sdmstGetWeight(scip, graph, base_nnodes, base_nodes, TRUE, extdata);

      if( LT(mstweight_org, mstweight_lower) )
      {
         SCIPdebugMessage("top baselevel MST weight violates lower bound: %f < %f \n",
             mstweight_org, mstweight_lower);

         isValid = FALSE;
      }
   }

   return isValid;
}



/** returns shortest distance between given vertices */
static
SCIP_Real distGetRestricted(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   int                   vertexBlocked,      /**< forbidden vertex */
   const DISTDATA*       distdata,           /**< distance data */
   int                   vertex1,            /**< first vertex */
   int                   vertex2             /**< second vertex */
   )
{
   SCIP_Real* dist;
   DHEAP* dheap = NULL;
   int* state;
   const SCIP_Real* const pc_costshifts = distdata->dijkdata->node_bias;
   const int nnodes = g->knots;
   const SCIP_Bool isPc = graph_pc_isPc(g);
   SCIP_Real distance = FARAWAY;

   assert(!isPc || pc_costshifts);

   SCIP_CALL_ABORT( graph_heap_create(scip, nnodes, NULL, NULL, &dheap) );
   SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &dist, nnodes) );

   state = dheap->position;

   for( int k = 0; k < nnodes; k++ )
   {
      dist[k] = FARAWAY;
      assert(state[k] == UNKNOWN);
   }

   dist[vertex1] = 0.0;
   graph_heap_correct(vertex1, 0.0, dheap);

   assert(dheap->size == 1);

   /* main loop */
   while( dheap->size > 0 )
   {
      /* get nearest labeled node */
      const int k = graph_heap_deleteMinReturnNode(dheap);
      const SCIP_Real k_dist = dist[k];

      if( isPc && k != vertex1 )
      {
         dist[k] += pc_costshifts[k];
      }

      if( k == vertex2 )
      {
         distance = k_dist;
         break;
      }

      /* correct adjacent nodes */
      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
      {
         const int m = g->head[e];
         assert(g->mark[m]);

         if( m == vertexBlocked )
            continue;

         if( state[m] != CONNECT )
         {
            const SCIP_Real distnew = isPc ?
               k_dist + g->cost[e] - pc_costshifts[m]
             : k_dist + g->cost[e];

            if( distnew < dist[m] )
            {
               dist[m] = distnew;
               graph_heap_correct(m, distnew, dheap);
            }
         }
      }
   }

   SCIPfreeMemoryArray(scip, &dist);
   graph_heap_free(scip, TRUE, TRUE, &dheap);

   return distance;
}


/** Helper.
 *  Gives maximum cost among all close nodes. */
static inline
SCIP_Real distCloseNodesGetMaxCost(
   int                   vertex,             /**< vertex for which to get the maximum cost*/
   const DISTDATA*       distdata            /**< distance data */
   )
{
   const SCIP_Real* const distances = distdata->closenodes_distances;
   const RANGE* const range = distdata->closenodes_range;
   const int range_start = range[vertex].start;
   const int range_end = range[vertex].end;
   SCIP_Real maxcost = -FARAWAY;

   for( int i = range_start; i < range_end; ++i )
   {
      const SCIP_Real dist = distances[i];

      if( dist > maxcost )
         maxcost = dist;
   }

   assert(GE(maxcost, 0.0) || range_start == range_end);

   return maxcost;
}


/** Gets close nodes and corresponding distances.
 *  NOTE: needs to correspond to 'distDataComputeCloseNodes' in 'extreduce_dbg.c' */
static
SCIP_RETCODE distCloseNodesCompute(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   const DISTDATA*       distdata,           /**< distance data */
   int                   startvertex,        /**< start vertex */
   int*                  closenodes_indices, /**< indices of close nodes */
   SCIP_Real*            closenodes_dists,   /**< distances of close nodes */
   int*                  nclosenodes         /**< number of added close nodes */
   )
{
   SCIP_Real* dist;
   DHEAP* dheap = NULL;
   int* state;
   DCSR* const dcsr = g->dcsr_storage;
   const RANGE* const RESTRICT range_csr = dcsr->range;
   const int* const RESTRICT head_csr = dcsr->head;
   const SCIP_Real* const RESTRICT cost_csr = dcsr->cost;
   const SCIP_Real* const pc_costshifts = distdata->dijkdata->node_bias;
   const int nnodes = g->knots;
   const SCIP_Real closenodes_maxcost = distCloseNodesGetMaxCost(startvertex, distdata);
   int clodenode_count;
   const SCIP_Bool isPc = graph_pc_isPc(g);

   assert(dcsr && g && distdata);
   assert(startvertex >= 0 && startvertex < g->knots);
   assert(!isPc || pc_costshifts);

   SCIP_CALL( graph_heap_create(scip, nnodes, NULL, NULL, &dheap) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &dist, nnodes) );

   state = dheap->position;

   for( int k = 0; k < nnodes; k++ )
   {
      dist[k] = FARAWAY;
      assert(state[k] == UNKNOWN);
   }

   clodenode_count = 0;
   dist[startvertex] = 0.0;
   graph_heap_correct(startvertex, 0.0, dheap);

   assert(dheap->size == 1);

   /* main loop */
   while( dheap->size > 0 )
   {
      /* get nearest labeled node */
      const int k = graph_heap_deleteMinReturnNode(dheap);
      const int k_start = range_csr[k].start;
      const int k_end = range_csr[k].end;
      const SCIP_Real k_dist = dist[k];

      if( k != startvertex )
      {
         if( isPc )
            dist[k] += pc_costshifts[k];

         if( GT(k_dist, closenodes_maxcost) )
            break;

         closenodes_indices[clodenode_count] = k;
         closenodes_dists[clodenode_count] = dist[k];

         clodenode_count++;
      }

      /* correct adjacent nodes */
      for( int e = k_start; e < k_end; e++ )
      {
         const int m = head_csr[e];
         assert(g->mark[m]);

         if( state[m] != CONNECT )
         {
            const SCIP_Real distnew = isPc ?
               k_dist + cost_csr[e] - pc_costshifts[m]
             : k_dist + cost_csr[e];

            if( distnew < dist[m] )
            {
               dist[m] = distnew;
               graph_heap_correct(m, distnew, dheap);
            }
         }
      }
   }

   SCIPfreeMemoryArray(scip, &dist);
   graph_heap_free(scip, TRUE, TRUE, &dheap);

   *nclosenodes = clodenode_count;

   return SCIP_OKAY;
}



/* prints info about lost node */
static
void distCloseNodesPrintLostNodeInfo(
   const GRAPH*          g,                  /**< graph data structure */
   const DISTDATA*       distdata,           /**< distance data */
   int                   vertex_base,        /**< vertex for check */
   int                   index_lost,         /**< vertex that is lost */
   const int*            closenodes_indices, /**< indices of newly found close nodes */
   const SCIP_Real*      closenodes_dists,   /**< distances of newly found close nodes */
   int                   nclosenodes         /**< number of newly found close nodes */
)
{
#ifdef SCIP_DEBUG
   RANGE* const org_range = distdata->closenodes_range;
   int* const org_indices = distdata->closenodes_indices;
   SCIP_Real* const org_dists = distdata->closenodes_distances;
   const int org_start = org_range[vertex_base].start;
   const int org_end = org_range[vertex_base].end;
   const int vertex_lost = org_indices[index_lost];

   assert(org_start <= index_lost && index_lost < org_end);

   SCIPdebugMessage("could not find vertex_base %d in new close nodes \n", vertex_lost);
   printf("new nodes: \n");

   for( int k = 0; k < nclosenodes; ++k )
   {
      printf("(idx=%d dist=%f) ", k, closenodes_dists[k]);
      graph_knot_printInfo(g, closenodes_indices[k]);
   }

   printf("original nodes: \n");

   for( int k = org_start; k < org_end; ++k )
   {
      printf("(dist=%f) ", org_dists[k]);
      graph_knot_printInfo(g, org_indices[k]);
   }

#ifndef NDEBUG
   {
      int index = index_lost;
      int vertex = vertex_lost;
      const int* const prededges = distdata->closenodes_prededges;

      assert(prededges);

      printf("path from lost node=%d to base=%d \n", vertex_lost, vertex_base);

      while( vertex != vertex_base )
      {
         const int prededge = prededges[index];

         assert(prededge >= 0);

         graph_edge_printInfo(g, prededge);

         if( distdata->dijkdata->node_bias )
            printf("vertex %d shift=%f \n", vertex, distdata->dijkdata->node_bias[vertex]);

         assert(vertex == g->head[prededge] || vertex == g->tail[prededge]);

         if( vertex == g->head[prededge] )
            vertex = g->tail[prededge];
         else
            vertex = g->head[prededge];

         if( vertex != vertex_base )
         {
            /* get the index of the next vertex */
            for( index = org_start; index != org_end; index++ )
            {
               if( org_indices[index] == vertex )
                  break;
            }

            assert(index != org_end);
         }
      }
   }
#endif
#endif
}

/* Is each original close node to 'vertex' included in the 'closenodes_indices' array?
 * And if so, with correct costs? */
static
SCIP_Bool distCloseNodesIncluded(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   const DISTDATA*       distdata,           /**< distance data */
   int                   vertex,             /**< vertex for check */
   const int*            closenodes_indices, /**< indices of newly found close nodes */
   const SCIP_Real*      closenodes_dists,   /**< distances of newly found close nodes */
   int                   nclosenodes         /**< number of newly found close nodes */
)
{
   RANGE* const org_range = distdata->closenodes_range;
   int* const org_indices = distdata->closenodes_indices;
   SCIP_Real* const org_dists = distdata->closenodes_distances;
   int* newnodeindex;
   const int nnodes = graph_get_nNodes(g);
   const int org_start = org_range[vertex].start;
   const int org_end = org_range[vertex].end;
   const int org_nclosenodes = org_end - org_start;
   SCIP_Bool isIncluded = TRUE;

   if( nclosenodes < org_nclosenodes )
   {
      SCIPdebugMessage("too few new closenodes! %d < %d \n", nclosenodes, org_nclosenodes);
      return FALSE;
   }

   SCIP_CALL_ABORT( SCIPallocCleanBufferArray(scip, &newnodeindex, nnodes) );

   /* mark the indices of new close nodes */
   for( int j = 0; j < nclosenodes; j++ )
   {
      const int cnode = closenodes_indices[j];

      assert(newnodeindex[cnode] == 0);

      newnodeindex[cnode] = j + 1;
   }

   /* the actual checks */
   for( int i = org_start; i < org_end; ++i )
   {
      const SCIP_Real org_dist = org_dists[i];
      const int org_node = org_indices[i];
      const int new_index = newnodeindex[org_node] - 1;
#ifndef NDEBUG
      const int prededge = distdata->closenodes_prededges[i];

      assert(new_index >= -1);
      assert(distdata->closenodes_prededges);
      assert(org_node == g->tail[prededge] || org_node == g->head[prededge]);
#endif

      if( new_index == -1 )
      {
         distCloseNodesPrintLostNodeInfo(g, distdata, vertex, i, closenodes_indices, closenodes_dists, nclosenodes);

         isIncluded = FALSE;
         break;
      }

      assert(org_node == closenodes_indices[new_index]);

      if( !EQ(closenodes_dists[new_index], org_dist) )
      {
         SCIPdebugMessage("wrong distances: %f != %f \n", org_dists[i], closenodes_dists[new_index]);

         isIncluded = FALSE;
         break;
      }
   }

   /* unmark the new close nodes */
   for( int j = 0; j < nclosenodes; j++ )
   {
      const int cnode = closenodes_indices[j];

      assert(newnodeindex[cnode] == j + 1);

      newnodeindex[cnode] = 0;
   }

   SCIPfreeCleanBufferArray(scip, &newnodeindex);

   return isIncluded;
}


/** computes counters for degrees and edges */
static
void treeGetCounters(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata,            /**< extension data */
   int                   edgecount[],        /**< edge count */
   int                   degreecount[],      /**< degree count */
   SCIP_Bool*            treeIsFlawed        /**< is tree flawed?) */
)
{
   const int* const tree_edges = extdata->tree_edges;
   const int* const tree_deg = extdata->tree_deg;
   const int tree_nedges = extdata->tree_nedges;

   for( int i = 0; i < tree_nedges; i++ )
   {
      const int e = tree_edges[i];
      const int head = graph->head[e];
      const int tail = graph->tail[e];

      assert(e >= 0 && e < graph->edges);

      if( edgecount[e] > 0 || edgecount[flipedge(e)] > 0  )
      {
         printf("tree_nedges %d \n", tree_nedges);
         printf("FLAW: double edge \n");
         graph_edge_printInfo(graph, e);
         *treeIsFlawed = TRUE;
      }

      if( tree_deg[tail] <= 0 )
      {
         printf("FLAW: non-positive degree for %d (%d) \n", tail, tree_deg[tail]);
         *treeIsFlawed = TRUE;
      }

      if( tree_deg[head] <= 0 )
      {
         printf("FLAW: non-positive degree for %d (%d) \n", head, tree_deg[head]);
         *treeIsFlawed = TRUE;
      }

      degreecount[tail]++;
      degreecount[head]++;

      edgecount[e]++;
   }
}


/** reset counters */
static
void treeResetCounters(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata,            /**< extension data */
   int                   edgecount[],        /**< edge count */
   int                   degreecount[]       /**< degree count */
)
{
   const int* const tree_edges = extdata->tree_edges;
   const int nnodes = graph_get_nNodes(graph);
   const int nedges = graph_get_nEdges(graph);
   const int tree_nedges = extdata->tree_nedges;

   for( int i = 0; i < tree_nedges; i++ )
   {
      const int e = tree_edges[i];
      const int head = graph->head[e];
      const int tail = graph->tail[e];

      edgecount[e] = 0;
      degreecount[tail] = 0;
      degreecount[head] = 0;
   }

   for( int i = 0; i < nnodes; i++ )
   {
      assert(degreecount[i] == 0);
   }

   for( int i = 0; i < nedges; i++ )
   {
      assert(edgecount[i] == 0);
   }
}


/** are the degrees flawed? */
static
SCIP_Bool treeDegreesAreFlawed(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata,            /**< extension data */
   const int             degreecount[]       /**< degree count */
)
{
   const int* const tree_edges = extdata->tree_edges;
   const int* const tree_deg = extdata->tree_deg;
   const int tree_nedges = extdata->tree_nedges;

   /* degree check */
   for( int i = 0; i < tree_nedges; i++ )
   {
      const int e = tree_edges[i];
      const int head = graph->head[e];

      if( degreecount[head] != tree_deg[head] )
      {
         printf("FLAW: wrong degree  \n");
         return TRUE;
      }
   }

   return FALSE;
}


/** flaw with the tree leaves? */
static
SCIP_Bool treeLeavesAreFlawed(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata,            /**< extension data */
   const int             degreecount[]       /**< degree count */
)
{
   const int* const tree_edges = extdata->tree_edges;
   const int* const tree_leaves = extdata->tree_leaves;
   const int tree_nedges = extdata->tree_nedges;
   const int nleaves = extdata->tree_nleaves;
   int leavescount = 1; /* for tail of initial edge */

   assert(nleaves >= 1);

   for( int i = 0; i < tree_nedges; i++ )
   {
      const int e = tree_edges[i];
      const int head = graph->head[e];

      if( degreecount[head] == 1 )
         leavescount++;
   }

   if( leavescount != nleaves )
   {
      printf("FLAW wrong leaves count %d != %d \n", leavescount, nleaves);
      return TRUE;
   }

   for( int i = 0; i < nleaves; i++ )
   {
      const int leaf = tree_leaves[i];

      if( degreecount[leaf] != 1 )
      {
         printf("FLAW wrong leaf %d degree %d != %d \n", leaf, degreecount[leaf], 1);
         return TRUE;
      }
   }

   return FALSE;
}


/** flaw with the inner nodes? */
static
SCIP_Bool treeInnerNodesAreFlawed(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata,            /**< extension data */
   const int             degreecount[]       /**< degree count */
)
{
   const int* const tree_edges = extdata->tree_edges;
   const int* const tree_innerNodes = extdata->tree_innerNodes;
   const int tree_nedges = extdata->tree_nedges;
   const int ninnerNodes = extdata->tree_ninnerNodes;
   int innercount = 0;

   for( int i = 0; i < tree_nedges; i++ )
   {
      const int e = tree_edges[i];
      const int head = graph->head[e];

      if( degreecount[head] > 1 )
         innercount++;
   }

   if( innercount != ninnerNodes )
   {
      printf("FLAW wrong inner count %d != %d \n", innercount, ninnerNodes);
      return TRUE;
   }

   for( int i = 0; i < ninnerNodes; i++ )
   {
      const int node = tree_innerNodes[i];

      if( degreecount[node] <= 1 )
      {
         printf("FLAW wrong inner node %d degree %d <= %d \n", node, degreecount[node], 1);
         return TRUE;
      }
   }

   return FALSE;
}


/** are distance values of tree flawed? */
static
SCIP_Bool treeDistsAreFlawed(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata             /**< extension data */
)
{
   const int nnodes = graph_get_nNodes(graph);
   const SCIP_Bool isPc = graph_pc_isPcMw(graph);

   for( int i = 0; i < nnodes; i++ )
   {
      if( isPc && extdata->pcdata->pcSdToNode[i] >= -0.5 )
      {
         printf("FLAW wrong pcSdToNode entry[%d]=%f \n", i, extdata->pcdata->pcSdToNode[i]);
         return TRUE;
      }
      if( extdata->tree_bottleneckDistNode[i] >= -0.5 )
      {
         printf("FLAW wrong tree_bottleneckDistNode entry[%d]=%f \n", i, extdata->tree_bottleneckDistNode[i]);
         return TRUE;
      }
   }

   return FALSE;
}


/** cleans extension data extensively for debugging */
void extreduce_extdataCleanArraysDbg(
   const GRAPH*          graph,              /**< graph data structure */
   EXTDATA*              extdata             /**< extension data */
)
{
   const int nnodes = graph_get_nNodes(graph);
   const int maxstacksize = extreduce_getMaxStackSize();
   const int maxncomponents = extreduce_getMaxStackNcomponents(graph);
   int* const extstack_data = extdata->extstack_data;
   int* const extstack_start = extdata->extstack_start;
   int* const extstack_state = extdata->extstack_state;
   int* const tree_edges = extdata->tree_edges;
   int* const tree_leaves = extdata->tree_leaves;
   int* const tree_innerNodes = extdata->tree_innerNodes;
   int* const tree_parentNode = extdata->tree_parentNode;
   SCIP_Real* const tree_parentEdgeCost = extdata->tree_parentEdgeCost;
   SCIP_Real* const redcost_treenodeswaps = extdata->reddata->redcost_treenodeswaps;
   const int redcost_nlevels = extdata->reddata->redcost_nlevels;

   for( int i = 0; i < maxstacksize; ++i )
   {
      extstack_data[i] = INT_MIN;
   }

   for( int i = 0; i < maxncomponents + 1; ++i )
   {
      extstack_start[i] = INT_MIN;
      extstack_state[i] = INT_MIN;
   }

   for( int i = 0; i < nnodes; ++i )
   {
      tree_edges[i] = INT_MIN;
      tree_leaves[i] = INT_MIN;
      tree_innerNodes[i] = INT_MIN;
      tree_parentNode[i] = INT_MIN;
   }

   for( int i = 0; i < nnodes; ++i )
   {
      tree_edges[i] = INT_MIN;
      tree_leaves[i] = INT_MIN;
   }


   for( int i = 0; i < nnodes; ++i )
   {
      tree_parentEdgeCost[i] = -FARAWAY;
   }

   for( int i = 0; i < nnodes * redcost_nlevels; i++ )
   {
      redcost_treenodeswaps[i] = -FARAWAY;
   }
}


/** is current tree flawed? */
SCIP_Bool extreduce_treeIsFlawed(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata             /**< extension data */
)
{
   int* edgecount;
   int* degreecount;
   const int nnodes = graph_get_nNodes(graph);
   const int nedges = graph_get_nEdges(graph);
   SCIP_Bool flawed = FALSE;

   SCIP_CALL_ABORT( SCIPallocCleanBufferArray(scip, &edgecount, nedges) );
   SCIP_CALL_ABORT( SCIPallocCleanBufferArray(scip, &degreecount, nnodes) );

   treeGetCounters(graph, extdata, edgecount, degreecount, &flawed);

   if( flawed )
   {
      flawed = TRUE;
   }
   else if( treeDegreesAreFlawed(graph, extdata, degreecount) )
   {
      flawed = TRUE;
   }
   else if( treeLeavesAreFlawed(graph, extdata, degreecount) )
   {
      flawed = TRUE;
   }
   else if( treeInnerNodesAreFlawed(graph, extdata, degreecount) )
   {
      flawed = TRUE;
   }
   else if( treeDistsAreFlawed(graph, extdata) )
   {
      flawed = TRUE;
   }

   treeResetCounters(graph, extdata, edgecount, degreecount);

   SCIPfreeCleanBufferArray(scip, &degreecount);
   SCIPfreeCleanBufferArray(scip, &edgecount);

   return flawed;
}


/** is current tree completely hashed? */
SCIP_Bool extreduce_treeIsHashed(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata             /**< extension data */
)
{
   const REDDATA* const reddata = extdata->reddata;

   for( int i = 0; i < extdata->tree_nedges; i++ )
   {
      const int edge = extdata->tree_edges[i];
      const int nAncestors = graph_edge_nPseudoAncestors(graph, edge);

      assert(nAncestors >= 0);

      if( nAncestors == 0 )
         continue;

      if( !graph_pseudoAncestors_edgeIsHashed(graph->pseudoancestors, edge, reddata->pseudoancestor_mark) )
         return FALSE;
   }

   return TRUE;
}


/** prints the leaves of the tree */
void extreduce_printLeaves(
   const EXTDATA*        extdata             /**< extension data */
)
{
   const int nleaves = extdata->tree_nleaves;
   const int* const leaves = extdata->tree_leaves;

   printf("tree leaves: \n");

   for( int i = 0; i < nleaves; ++i )
   {
      const int leaf = leaves[i];
      printf("%d: leaf=%d \n", i, leaf);
   }
}


/** prints the current stack */
void extreduce_printStack(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata             /**< extension data */
)
{
   const int* const extstack_data = extdata->extstack_data;
   const int* const extstack_start = extdata->extstack_start;
   const int stackpos = extdata->extstack_ncomponents - 1;

   for( int j = 0; j <= stackpos; j++ )
   {
      if( extdata->extstack_state[j] == EXT_STATE_NONE )
         printf("pos=%d state=NONE \n", j);
      else if( extdata->extstack_state[j] == EXT_STATE_EXPANDED )
         printf("pos=%d state=EXPANDED \n", j);
      else
      {
         assert(extdata->extstack_state[j] == EXT_STATE_MARKED);

         printf("pos=%d state=MARKED \n", j);
      }

      /* check all leaves of current component */
      for( int i = extstack_start[j]; i < extstack_start[j + 1]; i++ )
      {
         const int edge = extstack_data[i];

         if( edge == EXT_EDGE_WRAPPED )
         {
            printf("  EDGE_WRAPPED\n");
            continue;
         }

         assert(edge >= 0 && edge < graph->edges);
         printf("  ");
         graph_edge_printInfo(graph, edge);
      }
   }
}


/** Prints top horizontal level */
void extreduce_printTopLevel(
   const EXTDATA*        extdata             /**< extension data */
)
{
   const REDDATA* const reddata = extdata->reddata;
   const MLDISTS* const sds_horizontal = reddata->sds_horizontal;
   const int levelsize = extreduce_mldistsTopLevelNSlots(sds_horizontal);
   const int* const levelids = extreduce_mldistsTopLevelBases(sds_horizontal);

   printf("current (horizontal) top level: \n");

   for( int i = 0; i < levelsize; i++ )
   {
      printf("%d  ", levelids[i]);
   }

   printf("\n");
}


/** is the node in the current top component of the stack? */
SCIP_Bool extreduce_nodeIsInStackTop(
   const GRAPH*          graph,              /**< graph data structure */
   const EXTDATA*        extdata,            /**< extension data */
   int                   node                /**< the node */
   )
{
   const int stackpos = extStackGetPosition(extdata);
   const int* const stack_start = extdata->extstack_start;
   const int* const stack_data = extdata->extstack_data;

   assert(graph);
   assert(stack_start && stack_data);
   assert(node >= 0 && node < graph->knots);
   assert(extdata->extstack_state[stackpos] == EXT_STATE_EXPANDED || extdata->extstack_state[stackpos] == EXT_STATE_MARKED);

   for( int i = stack_start[stackpos]; i < stack_start[stackpos + 1]; i++ )
   {
      const int topnode = graph->head[stack_data[i]];

      assert(topnode >= 0 && topnode < graph->knots);

      if( topnode == node )
         return TRUE;
   }

   return FALSE;
}


/** Are the close-nodes still valid?
 *  NOTE: expensive method, just designed for debugging! */
SCIP_Bool extreduce_distCloseNodesAreValid(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   const DISTDATA*       distdata            /**< distance data */
)
{
   SCIP_Real* closenodes_dists;
   int* closenodes_indices;
   int nclosenodes = 0;
   const int nnodes = graph_get_nNodes(g);
   SCIP_Bool isValid = TRUE;

   SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &closenodes_indices, nnodes) );
   SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &closenodes_dists, nnodes) );

   assert(scip && distdata);
   assert(distdata->pathroot_isdirty);

   for( int i = 0; i < nnodes; i++ )
   {
      if( distdata->pathroot_isdirty[i] )
         continue;

      SCIP_CALL_ABORT( distCloseNodesCompute(scip, g, distdata, i,
            closenodes_indices, closenodes_dists, &nclosenodes) );

      if( !distCloseNodesIncluded(scip, g, distdata, i, closenodes_indices, closenodes_dists, nclosenodes) )
      {
#ifdef SCIP_DEBUG
         SCIPdebugMessage("corrupted node: ");
         graph_knot_printInfo(g, i);
#endif

         isValid = FALSE;
         break;
      }
   }

   SCIPfreeMemoryArray(scip, &closenodes_dists);
   SCIPfreeMemoryArray(scip, &closenodes_indices);

   return isValid;
}


/** Computes actual distance between two nodes.
 *  NOTE: expensive method, just designed for debugging! */
SCIP_Real extreduce_distComputeRestrictedDist(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   int                   vertexBlocked,      /**< forbidden vertex */
   const DISTDATA*       distdata,           /**< distance data */
   int                   vertex1,            /**< first vertex */
   int                   vertex2             /**< second vertex */
)
{
   SCIP_Real dist;

   assert(scip && g && distdata);
   assert(graph_knot_isInRange(g, vertex1));
   assert(graph_knot_isInRange(g, vertex2));
   assert(vertex1 != vertex2);
   assert(vertexBlocked != vertex1 && vertexBlocked != vertex2);

   dist = distGetRestricted(scip, g, vertexBlocked, distdata, vertex1, vertex2);

   return dist;
}


/** debug initialization */
void extreduce_extendInitDebug(
   int*                  extedgesstart,      /**< array */
   int*                  extedges            /**< array */
)
{
   assert(extedgesstart && extedges);

   for( int i = 0; i < STP_EXT_MAXGRAD; i++ )
      extedgesstart[i] = -1;

   for( int i = 0; i < STP_EXT_MAXGRAD * STP_EXT_MAXGRAD; i++ )
      extedges[i] = -1;
}


/** check whether vertical SDs are up to date for given leaf of component */
SCIP_Bool extreduce_sdsverticalInSync(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   compsize,           /**< size of component */
   int                   nleaves_ancestors,  /**< number of leaves to ancestors */
   int                   topleaf,            /**< component leaf to check for */
   EXTDATA*              extdata             /**< extension data */
   )
{
   const MLDISTS* const sds_vertical = extdata->reddata->sds_vertical;
   const int* const leaves = extdata->tree_leaves;
   const int nleaves = extdata->tree_nleaves;
   const SCIP_Real* const adjedgecosts = extreduce_mldistsTopTargetDists(sds_vertical, topleaf);
   const SCIP_Bool atInitialComp = extIsAtInitialComp(extdata);
   const int nleaves_old = atInitialComp ? 1 : (nleaves - compsize);
   SCIP_Bool isInSync = TRUE;
#ifndef NDEBUG
   const int* const adjids = extreduce_mldistsTopTargetIds(sds_vertical, topleaf);
#endif

   assert(adjedgecosts);
   assert(nleaves_old == nleaves_ancestors);
   assert(nleaves_old > 0 && nleaves_old < nleaves);

#ifndef STP_DEBUG_EXTPC
   if( graph_pc_isPc(graph) )
      return TRUE;
#endif

   /* get the SDs to the ancestor (lower) leafs and compare */
   for( int j = 0; j < nleaves_old; j++ )
   {
      const int leaf = leaves[j];
      const SCIP_Real sd_old = adjedgecosts[j];
      const SCIP_Real specialDist_new = extreduce_extGetSd(scip, graph, topleaf, leaf, extdata);
      const SCIP_Real sd_new = (specialDist_new >= -0.5) ? specialDist_new : FARAWAY;

      assert(!extreduce_nodeIsInStackTop(graph, extdata, leaf));
      assert(extdata->tree_deg[leaf] == 1);
      assert(leaf != topleaf);
      assert(adjids[j] == leaf);

       if( !EQ(sd_old, sd_new) )
       {
          SCIPdebugMessage("vertical SDs are wrong! %f!=%f \n", sd_old, sd_new);

          isInSync = FALSE;
          break;
       }
   }


   return isInSync;
}


/** check whether horizontal SDs are up to date for given leaf of component */
SCIP_Bool extreduce_sdshorizontalInSync(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   topleaf,            /**< component leaf to check for */
   EXTDATA*              extdata             /**< extension data */
   )
{
   const MLDISTS* const sds_horizontal = extdata->reddata->sds_horizontal;
   const int* const extstack_data = extdata->extstack_data;
   const int* const ghead = graph->head;
   const int stackpos = extStackGetPosition(extdata);
   const int stackstart = extStackGetTopOutEdgesStart(extdata, stackpos);
   const int stackend = extStackGetTopOutEdgesEnd(extdata, stackpos);
   SCIP_Bool isInSync = TRUE;
#ifndef NDEBUG
   SCIP_Bool hitTopLeaf = FALSE;
#endif

#ifndef STP_DEBUG_EXTPC
   if( graph_pc_isPc(graph) )
      return TRUE;
#endif

   for( int i = stackstart, j = 0; i < stackend; i++, j++ )
   {
      const int edge2sibling = extstack_data[i];
      const int sibling = ghead[edge2sibling];

      assert(extreduce_nodeIsInStackTop(graph, extdata, sibling));
      assert(extdata->tree_deg[sibling] == 1);

      if( sibling == topleaf )
      {
#ifndef NDEBUG
         hitTopLeaf = TRUE;
#endif
         continue;
      }
      else
      {
         const SCIP_Real sd_old = extreduce_mldistsTopTargetDist(sds_horizontal, topleaf, sibling);
         const SCIP_Real sd_new = extreduce_extGetSdProperDouble(scip, graph, topleaf, sibling, extdata);

         if( !EQ(sd_old, sd_new) )
         {
            SCIPdebugMessage("vertical SDs are wrong! %f!=%f \n", sd_old, sd_new);

            isInSync = FALSE;
            break;
         }
      }
   }

   assert(hitTopLeaf || !isInSync);

   return isInSync;
}


/** are sds from top component leaf corresponding to current tree? */
SCIP_Bool extreduce_sdsTopInSync(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   const SCIP_Real       sds[],              /**< SDs from top leaf */
   int                   topleaf,            /**< component leaf to check for */
   EXTDATA*              extdata             /**< extension data */
   )
{
   const int* const leaves = extdata->tree_leaves;
   const int nleaves = extdata->tree_nleaves;
   SCIP_Bool isInSync = TRUE;

#ifndef STP_DEBUG_EXTPC
   if( graph_pc_isPc(graph) )
      return TRUE;
#endif

   for( int j = 0; j < nleaves; j++ )
   {
      const int leaf = leaves[j];

      if( leaf != topleaf )
      {
         const SCIP_Real sd_double = extreduce_extGetSdProperDouble(scip, graph, topleaf, leaf, extdata);
         const SCIP_Real sd = extreduce_extGetSdProper(scip, graph, topleaf, leaf, extdata);

         if( !EQ(sds[j], sd_double) && !EQ(sds[j], sd) )
         {
            SCIPdebugMessage("SD from %d to %d not correct! \n", topleaf, leaf);
            SCIPdebugMessage("new sds (double, single): %f, %f ... old sd: %f  \n", sd_double, sd, sds[j]);

            isInSync = FALSE;
            break;
         }
      }
      else
      {
         if( !EQ(sds[j], FARAWAY) )
         {
            SCIPdebugMessage("SD to topleaf not FARAWAY! (but %f) \n", sds[j]);

            isInSync = FALSE;
            break;
         }
      }
   }

   return isInSync;
}


/** is the top level base MST objective in sync with the current tree? */
SCIP_Bool extreduce_mstTopLevelBaseObjValid(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   extnode,            /**< node from which the level was extended */
   EXTDATA*              extdata             /**< extension data */
)
{
   SCIP_Bool isValid = TRUE;

   assert(extnode >= 0 && extnode < graph->knots);
   assert(extdata->tree_deg[extnode] == 1);

   if( !graph_pc_isPcMw(graph) )
   {
      CSR mst_base;
      const REDDATA* const reddata = extdata->reddata;
      const CSRDEPO* const msts_levelbase = reddata->msts_levelbase;
      int* nodes;
      const int nleaves = extdata->tree_nleaves;
      const int nnodes = nleaves - 1;

      assert(nnodes >= 1);

      SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &nodes, nnodes) );

      graph_csrdepo_getTopCSR(msts_levelbase, &mst_base);

      assert(nnodes == mst_base.nnodes);
      assert(reduce_dcmstMstIsValid(scip, &mst_base));

      /* get the nodes for the new MST computations */
      mstTopLevelBaseGetNodes(extnode, nnodes, nodes, extdata);

      if( !mstTopLevelBaseValidWeight(scip, graph, nnodes, nodes, &mst_base, extdata) )
      {
         isValid = FALSE;
      }

      SCIPfreeMemoryArray(scip, &nodes);
   }

   return isValid;
}


/** is the objective of the top MST extension valid for the tree? */
SCIP_Bool extreduce_mstTopCompExtObjValid(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   int                   extvert,            /**< extended vertex */
   SCIP_Real             extobj,             /**< objective of extension */
   EXTDATA*              extdata             /**< extension data */
)
{
   SCIP_Bool isValid = TRUE;

   /** does currently not work because of weird SD computation */
   if( !graph_pc_isPcMw(graph) )
   {
      const SCIP_Real mstweight_upper = sdmstGetExtWeight(scip, graph, extvert, FALSE, extdata);

      if( GT(extobj, mstweight_upper) )
      {
         SCIPdebugMessage("top extension MST weight violates upper bound: %f > %f \n",
               extobj, mstweight_upper);

         isValid = FALSE;
      }
      else
      {
         const SCIP_Real mstweight_lower = sdmstGetExtWeight(scip, graph, extvert, TRUE, extdata);

         if( LT(extobj, mstweight_lower) )
         {
            SCIPdebugMessage("top extension MST weight violates lower bound: %f < %f \n",
                  extobj, mstweight_lower);

            isValid = FALSE;
         }
      }
   }

   return isValid;
}


/** is the objective of the top MST sync with the tree? */
SCIP_Bool extreduce_mstTopCompObjValid(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_Real             compobj,            /**< alleged objective of component */
   EXTDATA*              extdata             /**< extension data */
)
{
   SCIP_Bool isValid = TRUE;

   if( !graph_pc_isPcMw(graph) )
   {
      const int nleaves = extdata->tree_nleaves;
      const int* const leaves = extdata->tree_leaves;
      const SCIP_Real mstweight_upper = sdmstGetWeight(scip, graph, nleaves, leaves, FALSE, extdata);
      const SCIP_Real mstweight_lower = sdmstGetWeight(scip, graph, nleaves, leaves, TRUE, extdata);

      if( GT(compobj, mstweight_upper) )
      {
         SCIPdebugMessage("top MST weight violates upper bound: %f > %f \n",
               compobj, mstweight_upper);

         isValid = FALSE;
      }
      else if( LT(compobj, mstweight_lower) )
      {
         SCIPdebugMessage("top MST weight violates lower bound: %f < %f \n",
               compobj, mstweight_lower);

         isValid = FALSE;
      }
   }

   return isValid;
}


/** is the top MST sync with the tree? (does not check objective) */
SCIP_Bool extreduce_mstTopCompInSync(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure */
   EXTDATA*              extdata             /**< extension data */
)
{
   CSR topmst;
   const REDDATA* const reddata = extdata->reddata;
   const CSRDEPO* const msts_comp = reddata->msts_comp;
   const int* const leaves = extdata->tree_leaves;
   const int nleaves = extdata->tree_nleaves;

   graph_csrdepo_getTopCSR(msts_comp, &topmst);

   if( topmst.nnodes != nleaves )
   {
      SCIPdebugMessage("wrong top mst size nodes=%d leaves=%d \n", topmst.nnodes, nleaves);

      return FALSE;
   }

   if( graph_pc_isPcMw(graph) )
   {
      return TRUE;
   }

   /* make sure that the edge costs of the MST correspond to the SDs in the tree */
   for( int i = 0; i < nleaves; i++ )
   {
      const int startleaf = leaves[i];

      for( int j = topmst.start[i]; j != topmst.start[i + 1]; j++ )
      {
         const int head = topmst.head[j];
         const SCIP_Real edgecost = topmst.cost[j];
         int endleaf;
         SCIP_Real sd_lower;
         SCIP_Real sd_upper;

         assert(head >= 0 && head < nleaves);

         endleaf = leaves[head];
         sd_lower = extreduce_extGetSdProperDouble(scip, graph, startleaf, endleaf, extdata);
         sd_upper = MAX(extreduce_extGetSdProper(scip, graph, startleaf, endleaf, extdata),
                        extreduce_extGetSdProper(scip, graph, endleaf, startleaf, extdata));

         if( LT(edgecost, sd_lower) )
         {
            SCIPdebugMessage("MST edge %d->%d violates lower bound: %f < %f \n",
                  startleaf, endleaf , edgecost, sd_lower);

            return FALSE;
         }
         else if( GT(edgecost, sd_upper) )
         {
            SCIPdebugMessage("MST edge %d->%d violates upper bound: %f > %f \n",
                  startleaf, endleaf , edgecost, sd_upper);

            return FALSE;
         }
      }
   }

   return TRUE;
}


/** are the internal data MST structures in sync. with each other? */
SCIP_Bool extreduce_mstInternalsInSync(
   const EXTDATA*        extdata             /**< extension data */
)
{
   const REDDATA* const reddata = extdata->reddata;
   const CSRDEPO* const msts_comp = reddata->msts_comp;
   const CSRDEPO* const msts_levelbase = reddata->msts_levelbase;
   const MLDISTS* const sds_vertical = reddata->sds_vertical;
   const MLDISTS* const sds_horizontal = reddata->sds_horizontal;

   assert(!extreduce_mldistsEmptySlotExists(sds_horizontal));
   assert(!extreduce_mldistsEmptySlotExists(sds_vertical));

   if( extreduce_mldistsNlevels(sds_horizontal) != extreduce_mldistsNlevels(sds_vertical) )
   {
      SCIPdebugMessage("MLDISTS out of sync %d!=%d \n", extreduce_mldistsNlevels(sds_horizontal),
         extreduce_mldistsNlevels(sds_vertical));

      return FALSE;
   }

   if( graph_csrdepo_getNcsrs(msts_comp) != graph_csrdepo_getNcsrs(msts_levelbase) )
   {
      SCIPdebugMessage("CSR depos out of sync %d!=%d \n", graph_csrdepo_getNcsrs(msts_comp),
         graph_csrdepo_getNcsrs(msts_levelbase));

      return FALSE;
   }

   if (extreduce_mldistsTopLevelNSlots(sds_horizontal) != extreduce_mldistsTopLevelNSlots(sds_vertical) )
   {
      SCIPdebugMessage("sds out of sync %d!=%d \n", extreduce_mldistsTopLevelNSlots(sds_horizontal),
         extreduce_mldistsTopLevelNSlots(sds_vertical));

      return FALSE;
   }

   return TRUE;
}


/** is that complete current stack hashed? */
SCIP_Bool extreduce_stackTopIsHashed(
   const GRAPH*          graph,             /**< graph data structure */
   const EXTDATA*        extdata            /**< extension data */
)
{
   const int* const extstack_data = extdata->extstack_data;
   const int* const extstack_start = extdata->extstack_start;
   const int* const hasharr = extdata->reddata->pseudoancestor_mark;
   const int stackpos = extStackGetPosition(extdata);

   for( int i = extstack_start[stackpos]; i < extstack_start[stackpos + 1]; i++ )
   {
      const int edge = extstack_data[i];

      if( graph_edge_nPseudoAncestors(graph, edge) == 0 )
         continue;

      if( !graph_pseudoAncestors_edgeIsHashed(graph->pseudoancestors, edge, hasharr) )
      {
         SCIPdebugMessage("stacktop edge %d is not hashed! \n", edge);

         return FALSE;
      }
   }

   return TRUE;
}


/** returns size of component on the stack */
int extreduce_extStackCompNOutedges(
   const EXTDATA*        extdata,            /**< extension data */
   int                   stackpos            /**< position on the stack */
)
{
   const int* const stack_start = extdata->extstack_start;
   int size = stack_start[stackpos + 1] - stack_start[stackpos];
   const SCIP_Bool atInitialStar = extInitialCompIsStar(extdata) && (stackpos == 0);
   const SCIP_Bool atInitialGenStar = extInitialCompIsGenStar(extdata) && (stackpos == 0);

   if( atInitialStar )
   {
      size -= 1;
   }
   else if( atInitialGenStar )
   {
      size -= 2;
   }

   assert(extdata->extstack_state[stackpos] != EXT_STATE_NONE);
   assert(size > 0 && size < STP_EXT_MAXGRAD);

   return size;
}
