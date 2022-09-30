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

/**@file   reduce_alt.c
 * @brief  Altenative-based reduction tests for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file implements alternative-based reduction techniques for several Steiner problems.
 * Most tests can be found in "Combining NP-Hard Reduction Techniques and Strong Heuristics in an Exact Algorithm for the
 * Maximum-Weight Connected Subgraph Problem" by Daniel Rehfeldt and Thorsten Koch,
 * or in "Reduction Techniques for the Prize-Collecting Steiner Tree Problem and the Maximum-Weight Connected Subgraph Problem"
 * by Daniel Rehfeldt et al.
 * Note that special distance tests as well as extending (alternative) reduction techniques can
 * be found in separate files.
 *
 * A list of all interface methods can be found in reduce.h.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

//#define SCIP_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "graph.h"
#include "reduce.h"
#include "misc_stp.h"
#include "probdata_stp.h"
#include "scip/scip.h"
#include "portab.h"


#define VERTEX_CONNECT      0
#define VERTEX_TEMPNEIGHBOR 1
#define VERTEX_NEIGHBOR     2
#define VERTEX_OTHER        3
#define STP_RED_CNSNN       25
#define STP_RED_ANSMAXCANDS 500
#define STP_RED_ANSEXMAXCANDS 50
#define STP_RED_ANSMAXNEIGHBORS 25


/** NSV test data */
typedef struct nearest_special_distance_test_data
{
   const SD*             sdistance;         /**< NON-OWNED! */
   SCIP_Real* RESTRICT   candidates_bottleneck;
   SCIP_Real* RESTRICT   candidates_taildist;
   SCIP_Real* RESTRICT   candidates_headdist;
   int* RESTRICT         candidates_edge;
   SCIP_Bool* RESTRICT   candidates_isLink;
   int* RESTRICT         candidates_tail;
   int* RESTRICT         candidates_head;
   int* RESTRICT         nodes_isBlocked;
   STP_Vectype(int)      edgesrecord;        /**< NON-OWNED! */
   int* RESTRICT         solnode;            /**< NON-OWNED! */
   SCIP_Real*            fixed;              /**< NON-OWNED offset pointer */
   int                   ncandidates;
   SCIP_Bool             useRecording;
} NSV;


/*
 * Local methods
 */


/** ans subtest */
static inline
void ansDeleteVertex(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int* RESTRICT         marked,             /**< nodes array */
   int* RESTRICT         nelims,             /**< pointer to number of reductions */
   int                   vertex              /**< vertex */
)
{
   assert(vertex >= 0 && vertex < g->knots);
   assert(!Is_term(g->term[vertex]));
   assert(!graph_pc_knotIsDummyTerm(g, vertex));

#ifdef SCIP_DEBUG
   printf("delete node with ANS: ");
   graph_knot_printInfo(g, vertex);
#endif

   (*nelims) += g->grad[vertex];

   graph_knot_del(scip, g, vertex, TRUE);
   g->mark[vertex] = FALSE;
   marked[vertex] = FALSE;
}

/** un-marks */
static inline
void ansUnmark(
   const GRAPH*          g,                  /**< graph data structure */
   int                   basenode,           /**< base node */
   const int*            neighbarr,          /**< array of neighbors */
   int                   nNeigbors,          /**< nNeigbors */
   int* RESTRICT         marked              /**< nodes array */
)
{
   const int* const gHead = g->head;
   const int* const gOeat = g->oeat;

   assert(neighbarr || nNeigbors == 0);

#ifndef NDEBUG
   assert(marked[basenode]);

   for( int x = 0; x < nNeigbors; x++ )
   {
      const int neighbor = neighbarr[x];
      assert(neighbor >= 0 && neighbor < g->knots);
      assert(marked[neighbor]);
      assert(g->grad[neighbor] > 0);
   }
#endif

   for( int e = g->outbeg[basenode]; e >= 0; e = gOeat[e] )
      marked[gHead[e]] = FALSE;

   for( int l = 0; l < nNeigbors; l++ )
   {
      const int neighbor = neighbarr[l];

      for( int e = g->outbeg[neighbor]; e >= 0; e = gOeat[e] )
         marked[gHead[e]] = FALSE;
   }

   marked[basenode] = FALSE;

#ifndef NDEBUG
      for( int k2 = 0; k2 < g->knots; k2++ )
         assert(!marked[k2]);
#endif
}

/** ANS submethod for the case that the candidate vertex has exactly one non-dominated neighbor
 *  and both vertices combined are dominated */
static inline
void ansProcessCandidateWithBridge(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int* RESTRICT         marked,             /**< nodes array */
   int* RESTRICT         nelims,             /**< pointer to number of reductions */
   SCIP_Real             maxprize,           /**< value to not surpass */
   int                   candvertex,         /**< candidate */
   int                   bridgeedge          /**< edge to neighbor */
)
{
   /* NOTE: terminals can be leafs in an optimal solution! Thus we need to be careful with terminals. */
   int e3;
   const int neighbor = g->head[bridgeedge];
   const SCIP_Real setprize = g->prize[candvertex] + g->prize[neighbor];

   assert(LE(maxprize, 0.0));

   if( SCIPisGT(scip, setprize, maxprize) )
      return;

   for( e3 = g->outbeg[neighbor]; e3 >= 0; e3 = g->oeat[e3] )
   {
      const int head = g->head[e3];

      if( !marked[head] )
         break;
   }

   /* is {candvertex, neighbor} dominated? */
   if( e3 == EAT_LAST )
   {
      const SCIP_Bool candvertexIsSmall = SCIPisLE(scip, g->prize[candvertex], maxprize);
      const SCIP_Bool neighborIsSmall = SCIPisLE(scip, g->prize[neighbor], maxprize);

      /* delete both vertices? */
      if( candvertexIsSmall && neighborIsSmall )
      {
         SCIPdebugMessage("delete set of two nodes with ANS: %d %d \n", candvertex, neighbor);

         ansDeleteVertex(scip, g, marked, nelims, candvertex);
         ansDeleteVertex(scip, g, marked, nelims, neighbor);
      }
      else
      {
         SCIPdebugMessage("delete edge with ANS: %d->%d\n", neighbor, candvertex);

         graph_edge_del(scip, g, bridgeedge, TRUE);

         /* now both candvertex and neighbor are dominated */

         if( candvertexIsSmall )
            ansDeleteVertex(scip, g, marked, nelims, candvertex);

         if( neighborIsSmall )
            ansDeleteVertex(scip, g, marked, nelims, neighbor);
      }
   }
}

/** ANS subtest */
static
void ansProcessCandidate(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int* RESTRICT         marked,             /**< nodes array */
   int* RESTRICT         nelims,             /**< pointer to number of reductions */
   SCIP_Real             maxprize,           /**< value to not surpass */
   int                   candvertex          /**< candidate */
)
{
   int bridgeedge = -1;
   unsigned misses = 0;

   assert(g->mark[candvertex]);
   assert(!graph_pc_knotIsDummyTerm(g, candvertex));
   assert(LE(maxprize, 0.0));
   assert(!Is_term(g->term[candvertex]));

   for( int e2 = g->outbeg[candvertex]; e2 >= 0; e2 = g->oeat[e2] )
   {
      if( !marked[g->head[e2]] )
      {
         misses++;

         if( misses >= 2 )
            break;

         bridgeedge = e2;
      }
   }

   /* neighbors of candvertex subset of those of k? */
   if( misses == 0 && SCIPisLE(scip, g->prize[candvertex], maxprize) )
   {
      SCIPdebugMessage("candvertex %d is fully dominated \n", candvertex);
      ansDeleteVertex(scip, g, marked, nelims, candvertex);
   }
   else if( misses == 1 )
   {
      ansProcessCandidateWithBridge(scip, g, marked, nelims, maxprize, candvertex, bridgeedge);
   }
}


/** initializes NSV test data */
static
SCIP_RETCODE nsvInitData(
   SCIP*                 scip,               /**< SCIP data structure */
   const SD*             sdistance,          /**< special distances storage */
   const GRAPH*          g,                  /**< graph structure */
   int*                  solnode,            /**< node array to mark whether an node is part of a given solution (CONNECT) */
   SCIP_Real*            fixed,              /**< offset pointer */
   NSV*                  nsv                 /**< NSV */
)
{
   const BLCTREE* const blctree = sdistance->blctree;
   SCIP_Real* RESTRICT candidates_bottleneck;
   SCIP_Real* RESTRICT candidates_taildist;
   SCIP_Real* RESTRICT candidates_headdist;
   int* RESTRICT candidates_edge;
   int* RESTRICT candidates_tail;
   int* RESTRICT candidates_head;
   int* RESTRICT nodes_isBlocked;
   SCIP_Bool* RESTRICT candidates_isLink;
   int ncandidates;
   const int nnodes = graph_get_nNodes(g);


   assert(blctree);
   assert(sdistance->sdprofit);

   ncandidates = reduce_blctreeGetMstNedges(blctree);
   assert(ncandidates > 0);

   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_bottleneck, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_taildist, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_headdist, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_edge, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_isLink, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_tail, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &candidates_head, ncandidates) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_isBlocked, nnodes) );

   reduce_blctreeGetMstEdges(g, blctree, candidates_edge);
   reduce_blctreeGetMstEdgesState(g, blctree, candidates_isLink);
   reduce_blctreeGetMstEdgesToCutDist(g, blctree, candidates_taildist, candidates_headdist);
   reduce_blctreeGetMstBottlenecks(g, blctree, candidates_bottleneck);

   for( int i = 0; i < ncandidates; i++  )
   {
      const int edge = candidates_edge[i];
      candidates_tail[i] = g->tail[edge];
      candidates_head[i] = g->head[edge];
   }

   for( int i = 0; i < nnodes; i++ )
      nodes_isBlocked[i] = FALSE;

   nsv->sdistance = sdistance;
   nsv->candidates_bottleneck = candidates_bottleneck;
   nsv->candidates_taildist = candidates_taildist;
   nsv->candidates_headdist = candidates_headdist;
   nsv->candidates_edge = candidates_edge;
   nsv->candidates_isLink = candidates_isLink;
   nsv->candidates_tail = candidates_tail;
   nsv->candidates_head = candidates_head;
   nsv->nodes_isBlocked = nodes_isBlocked;
   nsv->solnode = solnode;
   nsv->fixed = fixed;
   nsv->useRecording = FALSE;
   nsv->edgesrecord = NULL;
   nsv->ncandidates = ncandidates;

   return SCIP_OKAY;
}


/** initializes NSV recordings */
static
void nsvInitRecording(
   STP_Vectype(int)      edgesrecord,        /**< edges record */
   NSV*                  nsv                 /**< NSV */
)
{
   assert(nsv);
   assert(!nsv->edgesrecord);
   assert(!nsv->useRecording);

   nsv->useRecording = TRUE;
   nsv->edgesrecord = edgesrecord;
}

/** frees NSV test data */
static
void nsvFreeData(
   SCIP*                 scip,               /**< SCIP data structure */
   NSV*                  nsv                 /**< NSV */
)
{
   SCIPfreeBufferArray(scip, &(nsv->nodes_isBlocked));
   SCIPfreeBufferArray(scip, &(nsv->candidates_head));
   SCIPfreeBufferArray(scip, &(nsv->candidates_tail));
   SCIPfreeBufferArray(scip, &(nsv->candidates_isLink));
   SCIPfreeBufferArray(scip, &(nsv->candidates_edge));
   SCIPfreeBufferArray(scip, &(nsv->candidates_headdist));
   SCIPfreeBufferArray(scip, &(nsv->candidates_taildist));
   SCIPfreeBufferArray(scip, &(nsv->candidates_bottleneck));
}


/** edge in NSV test can possibly still be contracted? */
static inline
SCIP_Bool nsvEdgeIsValid(
   const GRAPH*          g,                  /**< graph structure */
   const NSV*            nsv,                /**< NSV data */
   int                   edge               /**< the edge */
   )
{
   assert(graph_edge_isInRange(g, edge));

   if( graph_edge_isDeleted(g, edge) )
   {
      return FALSE;
   }
   else
   {
      const int* const nodes_isBlocked = nsv->nodes_isBlocked;
      const int tail = g->tail[edge];
      const int head = g->head[edge];

      if( nodes_isBlocked[tail] || nodes_isBlocked[head] )
      {
         return FALSE;
      }
   }

   return TRUE;
}


/** get special implied distances to terminals from both endpoints of given edge */
static inline
void nsvEdgeGetTermDists(
   const GRAPH*          g,                  /**< graph structure */
   const NSV*            nsv,                /**< NSV data */
   int                   edge,               /**< the edge */
   int                   candidate_id,       /**< id of candidate */
   SCIP_Real*            dist_tail,          /**< distance from tail */
   SCIP_Real*            dist_head           /**< distance from head */
   )
{
   SCIP_Real termdist[4];
   int firstedges[4];
   int neighbterms[4];
   const SD* sd = nsv->sdistance;
   int nnterms;
   const int tail = g->tail[edge];
   const int head = g->head[edge];
   int term_tail = -1;
   const SCIP_Real bottleneckdist = nsv->candidates_bottleneck[candidate_id];
   const SCIP_Real upper_sdbound = bottleneckdist - g->cost[edge];

   assert(GT(bottleneckdist, 0.0));
   assert(GE(upper_sdbound, 0.0));
   assert(nsv->candidates_edge[candidate_id] == edge);

   *dist_tail = FARAWAY;
   *dist_head = FARAWAY;

   if( Is_term(g->term[tail]) )
   {
      term_tail = tail;
      *dist_tail = 0.0;
   }
   else
   {
      const SCIP_Real* const cutdists_tail = nsv->candidates_taildist;

      graph_tpathsGet4CloseTermsLE(g, sd->terminalpaths, tail, upper_sdbound, neighbterms, firstedges,
            termdist, &nnterms);

      for( int i = 0; i < nnterms; i++ )
      {
         const int term = neighbterms[i];
         assert(term != tail);

         if( term == head )
            continue;

         if( (firstedges[i] / 2) == (edge / 2) )
            continue;

         if( termdist[i] < *dist_tail )
         {
            *dist_tail = termdist[i];
            term_tail = term;
         }
      }

      if( cutdists_tail[candidate_id] < *dist_tail )
         *dist_tail = cutdists_tail[candidate_id];
   }

   if( Is_term(g->term[head]) )
   {
      *dist_head = 0.0;
   }
   else
   {
      const SCIP_Real* const cutdists_head = nsv->candidates_headdist;

      graph_tpathsGet4CloseTermsLE(g, sd->terminalpaths, head, upper_sdbound, neighbterms, firstedges,
            termdist, &nnterms);

      for( int i = 0; i < nnterms; i++ )
      {
         const int term = neighbterms[i];
         assert(term != head);

         if( term == tail || term == term_tail )
            continue;

         if( (firstedges[i] / 2) == (edge / 2) )
            continue;
         if( termdist[i] < *dist_head )
            *dist_head = termdist[i];
      }

      if( cutdists_head[candidate_id] < *dist_head )
         *dist_head = cutdists_head[candidate_id];
   }

   assert(GE(*dist_tail, 0.0));
   assert(GE(*dist_head, 0.0));
}


/** contract edge in NSV test */
static inline
SCIP_RETCODE nsvEdgeContract(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge,               /**< the edge */
   int                   end_remain,         /**< survivor end node of edge */
   int                   end_killed,         /**< other end node */
   GRAPH*                g,                  /**< graph structure */
   NSV*                  nsv,                /**< NSV data */
   int*                  nelims              /**< number of eliminations */
)
{
   int* RESTRICT nodes_isBlocked = nsv->nodes_isBlocked;

#ifdef SCIP_DEBUG
   graph_edge_printInfo(g, edge);
#endif

   nodes_isBlocked[end_remain] = TRUE;
   nodes_isBlocked[end_killed] = TRUE;

   if( nsv->useRecording )
   {
      assert(!nelims);
      StpVecPushBack(scip, nsv->edgesrecord, edge);
   }
   else
   {
      assert(nelims);
      *(nsv->fixed) += g->cost[edge];
      SCIP_CALL( graph_knot_contractFixed(scip, g, nsv->solnode, edge, end_remain, end_killed) );
      graph_knot_chg(g, end_remain, STP_TERM);
      (*nelims)++;
   }

   return SCIP_OKAY;
}

/** executes actual NSV test */
static
SCIP_RETCODE nsvExec(
   SCIP*                 scip,               /**< SCIP data structure */
   NSV*                  nsv,                /**< NSV */
   GRAPH*                g,                  /**< graph structure */
   int*                  nelims              /**< number of eliminations */
)
{
   const SDPROFIT* const sdprofit = nsv->sdistance->sdprofit;
   const int ncandidates = nsv->ncandidates;
   const int* const candidate_edges = nsv->candidates_edge;
   const SCIP_Real* const candidate_bottlenecks = nsv->candidates_bottleneck;
   const SCIP_Bool* const candidate_isLink = nsv->candidates_isLink;
   assert(sdprofit);

   for( int i = 0; i < ncandidates; ++i )
   {
      const int edge = candidate_edges[i];

      if( nsvEdgeIsValid(g, nsv, edge) )
      {
         assert(graph_edge_isInRange(g, edge));
         assert(LE(g->cost[edge], candidate_bottlenecks[i]));

         /* cut edge? */
         if( EQ(candidate_bottlenecks[i], FARAWAY) )
         {
            SCIP_Bool pruned;
            SCIP_CALL( reduce_cutEdgeTryPrune(scip, edge, g, &pruned) );

            if( !pruned )
            {
               SCIP_CALL( nsvEdgeContract(scip, edge, g->tail[edge], g->head[edge], g, nsv, nelims) );
            }

            continue;
         }

         if( candidate_isLink[i] )
         {
            const SCIP_Real edgecost = g->cost[edge];
            const int tail = g->tail[edge];
            const int head = g->head[edge];
            const SCIP_Real profit_tail = reduce_sdprofitGetProfit(sdprofit, tail, head, -1);
            const SCIP_Real profit_head = reduce_sdprofitGetProfit(sdprofit, head, tail, -1);
            SCIP_Real dist_tail;
            SCIP_Real dist_head;

            if( Is_term(g->term[tail]) && GE(profit_head, edgecost) )
            {
               SCIPdebugMessage("NSV contract implied profit end (%f >= %f) ... ", profit_head, edgecost);
               SCIP_CALL( nsvEdgeContract(scip, edge, tail, head, g, nsv, nelims) );
               continue;
            }

            if( Is_term(g->term[head]) && GE(profit_tail, edgecost) )
            {
               SCIPdebugMessage("NSV contract implied profit end (%f >= %f) ... ", profit_tail, edgecost);
               SCIP_CALL( nsvEdgeContract(scip, flipedge(edge), head, tail, g, nsv, nelims) );
               continue;
            }

            nsvEdgeGetTermDists(g, nsv, edge, i, &dist_tail, &dist_head);

            if( LE(dist_tail + edgecost + dist_head, candidate_bottlenecks[i]) )
            {
               SCIPdebugMessage("NSV contract default ... ");
               SCIPdebugMessage("%f + %f + %f <= %f ... ", dist_tail, edgecost, dist_head, candidate_bottlenecks[i]);

               SCIP_CALL( nsvEdgeContract(scip, edge, tail, head, g, nsv, nelims) );
            }
         }
      }
   }

   assert(graph_valid(scip, g));


   return SCIP_OKAY;
}


/*
 * Interface methods
 */

/** implied version of NSV test */
SCIP_RETCODE reduce_nsvImplied(
   SCIP*                 scip,               /**< SCIP data structure */
   const SD*             sdistance,          /**< special distances storage */
   GRAPH*                g,                  /**< graph structure */
   int*                  solnode,            /**< node array to mark whether an node is part of a given solution (CONNECT) */
   SCIP_Real*            fixed,              /**< offset pointer */
   int*                  nelims              /**< number of eliminations */
)
{
   NSV nsv;
   assert(scip && sdistance && nelims && fixed);
   assert(*nelims >= 0);

   SCIP_CALL( nsvInitData(scip, sdistance, g, solnode, fixed, &nsv) );
   SCIP_CALL( nsvExec(scip, &nsv, g, nelims) );
   nsvFreeData(scip, &nsv);

   return SCIP_OKAY;
}


/** implied version of NSV test. Does not perform the reductions, but rather records them */
SCIP_RETCODE reduce_nsvImpliedRecord(
   SCIP*                 scip,               /**< SCIP data structure */
   const SD*             sdistance,          /**< special distances storage */
   GRAPH*                g,                  /**< graph structure */
   STP_Vectype(int)*     edgerecord          /**< keeps number edges */
)
{
   NSV nsv;
   assert(scip && sdistance && edgerecord);

   SCIP_CALL( nsvInitData(scip, sdistance, g, NULL, NULL, &nsv) );
   nsvInitRecording(*edgerecord, &nsv);
   SCIP_CALL( nsvExec(scip, &nsv, g, NULL) );

   *edgerecord = nsv.edgesrecord;
   nsvFreeData(scip, &nsv);

   return SCIP_OKAY;
}


/** shortest link reduction */
SCIP_RETCODE reduce_sl(
   SCIP*                 scip,               /**< SCIP data structure */
   const int*            edgestate,          /**< for propagation or NULL */
   GRAPH*                g,                  /**< graph data structure */
   PATH*                 vnoi,               /**< Voronoi data structure */
   SCIP_Real*            fixed,              /**< offset pointer */
   int*                  vbase,              /**< Voronoi/shortest path bases array */
   int*                  vrnodes,            /**< Voronoi/shortest path array  */
   STP_Bool*             visited,            /**< Voronoi/shortest path array */
   int*                  solnode,            /**< node array to mark whether an node is part of a given solution (CONNECT),
                                                  or NULL */
   int*                  nelims              /**< pointer to store number of eliminations */
   )
{
   STP_Vectype(int) queue = NULL;
   SCIP_Bool* nodes_isTerm;
   STP_Bool foundterms;
   STP_Bool* forbidden;
   STP_Bool* newterm;
   const SCIP_Bool isPc = graph_pc_isPc(g);
   const int nnodes = g->knots;

   assert(vnoi != NULL);
   assert(vbase != NULL);
   assert(vrnodes != NULL);
   assert(visited != NULL);

   *nelims = 0;
   foundterms = FALSE;

   if( g->terms <= 1 )
      return SCIP_OKAY;

   if( g->terms <= 2 && graph_pc_isUnrootedPcMw(g) )
      return SCIP_OKAY;

   SCIP_CALL( SCIPallocClearBufferArray(scip, &forbidden, nnodes) );
   SCIP_CALL( SCIPallocClearBufferArray(scip, &newterm, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_isTerm, nnodes) );

   BMSclearMemoryArray(visited, nnodes);
   graph_mark(g);
   graph_getIsTermArray(g, nodes_isTerm);
   graph_voronoiTerms(g, nodes_isTerm, vbase, vnoi);

#ifndef NDEBUG
   for( int i = 0; i < nnodes; i++ )
   {
      assert(!newterm[i]);
      assert(!forbidden[i]);
      assert(!visited[i]);
   }
#endif

   for( int baseterm = 0; baseterm < nnodes; baseterm++ )
   {
      if( nodes_isTerm[baseterm] && g->mark[baseterm] && !forbidden[baseterm] )
      {
         SCIP_Real mincost2 = FARAWAY;
         SCIP_Real mincost3 = FARAWAY;
         SCIP_Real ttdist;
         int mintail;
         int minhead;
         int minedge = UNKNOWN;
         int vrcount = 1;

         assert(StpVecGetSize(queue) == 0);

         StpVecPushBack(scip, queue, baseterm);
         vrnodes[0] = baseterm;
         visited[baseterm] = TRUE;

         /* traverse Voronoi-region of "baseterm" */
         while( StpVecGetSize(queue) != 0 )
         {
            const int qnode = queue[StpVecGetSize(queue) - 1];
            StpVecPopBack(queue);

            for( int e = g->outbeg[qnode]; e != EAT_LAST; e = g->oeat[e] )
            {
               int headbase;
               minhead = g->head[e];

               if( !g->mark[minhead] )
                  continue;

               headbase = vbase[minhead];
               assert(headbase != UNKNOWN);

               if( !visited[minhead] && headbase == baseterm )
               {
                  visited[minhead] = TRUE;
                  vrnodes[vrcount++] = minhead;
                  StpVecPushBack(scip, queue, minhead);
               }
               else if( headbase != baseterm )
               /* update shortest and second shortest edge (cost) leaving the voronoi region */
               {
                  const SCIP_Real edgecost = g->cost[e];
                  if( minedge == UNKNOWN )
                  {
                     minedge = e;
                  }
                  else if( LE(edgecost, g->cost[minedge]) )
                  {
                     mincost3 = mincost2;
                     mincost2 = g->cost[minedge];
                     minedge = e;
                  }
                  else if( LE(edgecost, mincost2) )
                  {
                     mincost3 = mincost2;
                     mincost2 = g->cost[e];
                  }
                  else if( LT(edgecost, mincost3) )
                  {
                     mincost3 = g->cost[e];
                  }
               }
            }
         }
         for( int j = 0; j < vrcount; j++ )
            visited[vrnodes[j]] = FALSE;

#ifndef NDEBUG
         for( int j = 0; j < nnodes; j++ )
            assert(!visited[j]);
#endif

         if( minedge == UNKNOWN )
            continue;

         mintail = g->tail[minedge];
         minhead = g->head[minedge];
         assert(vbase[mintail] == baseterm);

         ttdist = vnoi[mintail].dist + g->cost[minedge] + vnoi[minhead].dist;

         if( isPc )
         {
            if( !nodes_isTerm[mintail] )
               ttdist -= g->prize[mintail];

            if( !nodes_isTerm[minhead] )
               ttdist -= g->prize[minhead];

            assert(GE(ttdist, 0.0) && LE(ttdist, vnoi[mintail].dist + g->cost[minedge] + vnoi[minhead].dist));
         }

         /* check whether minedge can be removed */
         if( SCIPisGE(scip, mincost2, ttdist) && (edgestate == NULL || edgestate[minedge] != EDGE_BLOCKED) )
         {
            int contractnode_in;
            int contractnode_out;
            int old;

            if( isPc )
            {
               assert(g->stp_type != STP_RPCSPG || EQ(g->prize[g->source], FARAWAY));

               if( !SCIPisLE(scip, ttdist, g->prize[vbase[minhead]]) )
                  continue;

               if( baseterm == mintail )
               {
                  if( !SCIPisLE(scip, ttdist, g->prize[baseterm]) )
                     continue;
               }
               else
               {
                  if( !SCIPisLT(scip, ttdist, g->prize[baseterm]) )
                     continue;
               }

               if( nodes_isTerm[minhead] && nodes_isTerm[mintail] )
                  continue;
            }

            assert(g->mark[mintail] && g->mark[minhead]);
            assert(!Is_pseudoTerm(g->term[mintail]) && !Is_pseudoTerm(g->term[minhead]));

            if( nodes_isTerm[minhead] || (isPc && graph_pc_knotIsNonLeafTerm(g, mintail) ) )
            {
               contractnode_in = minhead;
               contractnode_out = mintail;
            }
            else
            {
               contractnode_in = mintail;
               contractnode_out = minhead;
            }

            // NOTE: currently not supported for contraction operation
            // todo
            if( isPc && graph_pc_knotIsNonLeafTerm(g, contractnode_in) )
               continue;

            *fixed += g->cost[minedge];
            old = g->grad[contractnode_in] + g->grad[contractnode_out] - 1;

            // todo
#ifdef SCIP_DEBUG
            printf("SL contract edge: \n");
            graph_edge_printInfo(g, minedge);
#endif

            if( isPc )
            {
               SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, contractnode_in, contractnode_out, baseterm) );
               assert(g->grad[contractnode_in] > 0);
               if( graph_pc_knotIsFixedTerm(g, baseterm) && !nodes_isTerm[contractnode_in] )
               {
                  assert(!Is_anyTerm(g->term[contractnode_in]));
                  newterm[contractnode_in] = TRUE;
                  foundterms = TRUE;
               }
            }
            else
            {
               SCIP_CALL( graph_knot_contractFixed(scip, g, solnode, minedge, contractnode_in, contractnode_out) );

               assert(g->grad[contractnode_out] == 0 && g->grad[contractnode_in] >= 0);

               if( !nodes_isTerm[contractnode_in] )
               {
                  newterm[contractnode_in] = TRUE;
                  foundterms = TRUE;
               }
            }

            assert(old - g->grad[contractnode_in] - g->grad[contractnode_out] > 0);
            (*nelims) += old - g->grad[contractnode_in] - g->grad[contractnode_out];
            forbidden[vbase[contractnode_in]] = TRUE;
            forbidden[vbase[contractnode_out]] = TRUE;
         }
      }
   }

   if( foundterms )
   {
      for( int i = 0; i < nnodes; i++ )
      {
         if( newterm[i] && g->grad[i] > 0 )
         {
            if( isPc )
            {
               if( !graph_pc_knotIsFixedTerm(g, i) )
                  graph_pc_knotToFixedTerm(scip, g, i, NULL);
            }
            else if( !nodes_isTerm[i] )
            {
               graph_knot_chg(g, i, STP_TERM);
            }
         }
      }
   }

   StpVecFree(scip, queue);
   SCIPfreeBufferArray(scip, &nodes_isTerm);
   SCIPfreeBufferArray(scip, &newterm);
   SCIPfreeBufferArray(scip, &forbidden);

   return SCIP_OKAY;
}


/* NV reduction from T. Polzin's "Algorithms for the Steiner problem in networks" */
SCIP_RETCODE reduce_nv(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   PATH*                 vnoi,               /**< Voronoi data structure */
   SCIP_Real*            fixed,              /**< offset pointer */
   int*                  edgearrint,         /**< edge int array for internal computations */
   int*                  vbase,              /**< array for internal computations */
   int*                  solnode,            /**< node array to mark whether an node is part of a given solution (CONNECT),
                                                  or NULL */
   int* nelims                               /**< pointer to store number of eliminations */
   )
{
   SCIP_Real* distance;
   SCIP_Real  min1;
   SCIP_Real  min2;
   SCIP_Real  pi;
   SCIP_Real  pt;
   SCIP_Real  ttdist;
   int* term;
   int*    minedge1;
   int*    distnode;
   int     i;
   int     l;
   int     k;
   int     e;
   int     t;
   int     old;
   int     edge1;
   int     nnodes;
   int     nterms;
   int     mingrad;
   int     termcount;
   SCIP_Bool pc;
   assert(g != NULL);
   assert(vnoi != NULL);
   assert(vbase != NULL);

   t = 0;
   termcount = 0;
   *nelims = 0;
   pi = 0;
   pt = 0;

   nnodes = g->knots;
   nterms = g->terms;
   pc = (g->stp_type == STP_PCSPG) || (g->stp_type == STP_RPCSPG);
   SCIP_CALL( SCIPallocBufferArray(scip, &term, nterms) );
   SCIP_CALL( SCIPallocBufferArray(scip, &minedge1, nterms) );
   SCIP_CALL( SCIPallocBufferArray(scip, &distance, nnodes) );

   /* minimal grad of a vertex to be scrutinized */
   if( pc )
   {
      if( g->stp_type == STP_RPCSPG )
         mingrad = 3;
      else
         mingrad = 4;

      SCIP_CALL( SCIPallocBufferArray(scip, &distnode, nnodes) );
   }
   else
   {
      mingrad = 2;
      distnode = NULL;
      for( i = 0; i < nnodes; i++ )
         g->mark[i] = (g->grad[i] > 0);
   }

   for( i = 0; i < nnodes; i++ )
   {
      if( Is_term(g->term[i]) && g->mark[i] && g->grad[i] > 0 )
      {
         /* compute shortest incident edge */
         edge1 = UNKNOWN;
         if( g->grad[i] >= 1 )
         {
            min1  = FARAWAY;

            for( e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
            {
               if( !g->mark[g->head[e]] )
                  continue;
               if( SCIPisLE(scip, g->cost[e], min1) )
               {
                  edge1 = e;
                  min1 = g->cost[e];
               }
            }
         }
         minedge1[termcount] = edge1;
         term[termcount++] = i;
      }
   }

   /* compute Voronoi regions and distances */
   assert(0 && "does currently not work");
   SCIP_CALL( graph_voronoiWithDist(scip, g, NULL, g->cost, distance, edgearrint, vbase, minedge1, distnode, vnoi) );

   for( l = 0; l < termcount; l++ )
   {
      /* get l'th terminal */
      i = term[l];

      if( g->grad[i] < mingrad )
         continue;

      assert(minedge1[l] != UNKNOWN);
      /* get shortest two edges */
      edge1 = UNKNOWN;
      min2 = FARAWAY;
      min1 = FARAWAY;
      for( e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
      {
         if( !g->mark[g->head[e]] )
            continue;
         if( SCIPisLE(scip, g->cost[e], min1) )
         {
            edge1 = e;
            min2 = min1;
            min1 = g->cost[e];
         }
         else if( SCIPisLE(scip, g->cost[e], min2) )
         {
            min2 = g->cost[e];
         }
      }

      assert(edge1 != UNKNOWN);
      assert(i == g->tail[edge1]);
      k = g->head[edge1];

      /* covered in degree test */
      if( Is_term(g->term[k]) )
         continue;

      if( vbase[k] != i )
      {
         if( pc )
            t = vbase[k];
         ttdist = g->cost[edge1] + vnoi[k].dist;
      }
      else
      {
         if( distnode != NULL )
            t = distnode[i];
         ttdist = distance[i];
      }
      if( pc )
      {
         if( i != g->source )
            pi = g->prize[i];
         else
            pi = FARAWAY;

         if( t != g->source )
            pt = g->prize[t];
         else
            pt = FARAWAY;
      }

      if( SCIPisGE(scip, min2, ttdist)
         && (!pc || (SCIPisLE(scip, g->cost[edge1], pi) && SCIPisLE(scip, ttdist, pt))) )
      {
         old = g->grad[i] + g->grad[k] - 1;
         *fixed += g->cost[edge1];

         if( pc )
         {
            SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, i, k, i) );
         }
         else
         {
            SCIP_CALL( graph_knot_contractFixed(scip, g, solnode, edge1, i, k) );
         }
         assert(old - g->grad[i] - g->grad[k] > 0);
         (*nelims) += old - g->grad[i] - g->grad[k];
      }
   }

   SCIPfreeBufferArrayNull(scip, &distnode);
   SCIPfreeBufferArray(scip, &distance);
   SCIPfreeBufferArray(scip, &minedge1);
   SCIPfreeBufferArray(scip, &term);

   return SCIP_OKAY;
}


/* advanced NV reduction */
SCIP_RETCODE reduce_nvAdv(
   SCIP*                 scip,               /**< SCIP data structure */
   const int*            edgestate,          /**< for propagation or NULL */
   GRAPH*                g,                  /**< graph data structure */
   PATH*                 vnoi,               /**< Voronoi data structure */
   SCIP_Real*            distance,           /**< nodes-sized distance array */
   SCIP_Real*            fixed,              /**< offset pointer */
   int*                  edgearrint,         /**< edges-sized array */
   int*                  vbase,              /**< Voronoi base array  */
   int*                  distnode,           /**< nodes-sized distance array  */
   int*                  solnode,            /**< node array to mark whether an node is part of a given solution (CONNECT),
                                                  or NULL */
   int*                  nelims              /**< pointer to store number of eliminations */
   )
{
   SCIP_Real  baseprize;
   SCIP_Real  nextprize;
   SCIP_Real  ttdist;
   int* term;
   int* minedge1;
   SCIP_Bool* nodes_isTerm;
   int     head1;
   int     nextterm;
   int     mingrad;
   int     termcount;
   SCIP_Bool contract;
   const int nnodes = graph_get_nNodes(g);
   const int nterms = graph_get_nTerms(g);
   const SCIP_Bool pc = (g->stp_type == STP_PCSPG) || (g->stp_type == STP_RPCSPG);

   assert(vnoi != NULL);
   assert(vbase != NULL);

   baseprize = 0;
   nextprize = 0;
   *nelims = 0;
   termcount = 0;

   if( nterms <= 2 )
      return SCIP_OKAY;

   SCIP_CALL( SCIPallocBufferArray(scip, &term, nterms) );
   SCIP_CALL( SCIPallocBufferArray(scip, &minedge1, nterms) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_isTerm, nnodes) );

   graph_getIsTermArray(g, nodes_isTerm);

   /* set minimal grad of a vertex to be scrutinized */
   if( pc )
   {
      if( g->stp_type == STP_RPCSPG )
         mingrad = 3;
      else
         mingrad = 4;

      assert(distnode != NULL);
      assert(!g->extended);
   }
   else
   {
      mingrad = 2;
   }

   graph_mark(g);

   /* compute shortest incident edge to each terminal */
   for( int i = 0; i < nnodes; i++ )
   {
      if( nodes_isTerm[i] && g->mark[i] )
      {
         /* compute shortest incident edge */
         int edge1 = UNKNOWN;

         if( g->grad[i] >= 1 )
         {
            SCIP_Real min1 = FARAWAY;

            for( int e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
            {
               if( g->mark[g->head[e]] && LE(g->cost[e], min1) )
               {
                  edge1 = e;
                  min1 = g->cost[e];
               }
            }
         }

         assert(!pc || !graph_pc_knotIsNonLeafTerm(g, i));

         minedge1[termcount] = edge1;
         term[termcount++] = i;
      }
   }

   assert(termcount <= nterms);

   if( termcount <= 1 )
   {
      assert(pc);
      SCIPfreeBufferArray(scip, &nodes_isTerm);
      SCIPfreeBufferArray(scip, &minedge1);
      SCIPfreeBufferArray(scip, &term);

      return SCIP_OKAY;
   }

   /* compute Voronoi regions and distances */
   SCIP_CALL( graph_voronoiWithDist(scip, g, nodes_isTerm, g->cost, distance, edgearrint, vbase, minedge1, distnode, vnoi) );

   /* main loop: try to contract (shortest) edges into terminals */
   for( int l = 0; l < termcount; l++ )
   {
      /* get l'th terminal */
      const int baseterm = term[l];
      SCIP_Real min1;
      SCIP_Real min2;
      SCIP_Real min3;
      int edge1;
      int edge2;

      if( g->grad[baseterm] < mingrad )
         continue;

      /* get shortest two edges */

      min3 = FARAWAY;
      min2 = FARAWAY;
      min1 = FARAWAY;
      edge1 = UNKNOWN;
      edge2 = UNKNOWN;

      for( int e = g->outbeg[baseterm]; e != EAT_LAST; e = g->oeat[e] )
      {
         if( !g->mark[g->head[e]] )
            continue;

         if( SCIPisLE(scip, g->cost[e], min1) )
         {
            edge2 = edge1;
            edge1 = e;

            min3 = min2;
            min2 = min1;
            min1 = g->cost[e];
         }
         else if( SCIPisLE(scip, g->cost[e], min2) )
         {
            edge2 = e;

            min3 = min2;
            min2 = g->cost[e];
         }
         else if( SCIPisLE(scip, g->cost[e], min3) )
         {
            min3 = g->cost[e];
         }
      }

      assert(edge1 != UNKNOWN);
      assert(baseterm == g->tail[edge1]);

      head1 = g->head[edge1];

      /* covered in degree test */
      if( nodes_isTerm[head1] )
         continue;

      if( vbase[head1] != baseterm )
      {
         ttdist = g->cost[edge1] + vnoi[head1].dist;

         if( pc )
         {
            if( !nodes_isTerm[head1] )
            {
               ttdist -= g->prize[head1];
               assert(GE(ttdist, 0.0) && LE(ttdist, g->cost[edge1] + vnoi[head1].dist));
            }
            nextterm = vbase[head1];
         }
      }
      else
      {
         if( pc )
            nextterm = distnode[baseterm];

         ttdist = distance[baseterm];
      }

      if( pc )
      {
         assert(baseterm != g->source || g->stp_type == STP_RPCSPG);

         if( nextterm == UNKNOWN )
            continue;

         if( g->grad[nextterm] == 0 )
         {
            assert(*nelims > 0);
            continue;
         }

         baseprize = g->prize[baseterm];

         if( nextterm == UNKNOWN )
            nextprize = -1.0;
         else
            nextprize = g->prize[nextterm];
      }

      contract = FALSE;

      if( edgestate != NULL && edgestate[edge1] == EDGE_BLOCKED )
      {
         contract = FALSE;
      }
      else if( SCIPisGE(scip, min2, ttdist) )
      {
         contract = TRUE;
      }
      else if( edge2 != UNKNOWN && !nodes_isTerm[g->head[edge2]] && SCIPisGE(scip, min3, ttdist) )
      {
         int e;
         nextterm = g->head[edge2];
         for( e = g->outbeg[nextterm]; e != EAT_LAST; e = g->oeat[e] )
         {
            if( e != flipedge(edge2) && SCIPisLT(scip, g->cost[e], ttdist) )/*&& !neighb[g->head[e]] ) */
               break;
         }

         if( e == EAT_LAST )
            contract = TRUE;
      }

      if( contract && (!pc || (SCIPisLE(scip, g->cost[edge1], baseprize) && SCIPisLE(scip, ttdist, nextprize))) )
      {
         (*nelims)++;
         SCIPdebugMessage("nvAdv contract \n");

         *fixed += g->cost[edge1];

         if( pc )
         {
            SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, baseterm, head1, baseterm) );
         }
         else
         {
            SCIP_CALL( graph_knot_contractFixed(scip, g, solnode, edge1, baseterm, head1) );
         }
      }
   }

   SCIPfreeBufferArray(scip, &nodes_isTerm);
   SCIPfreeBufferArray(scip, &minedge1);
   SCIPfreeBufferArray(scip, &term);

   return SCIP_OKAY;
}


// todo finish
#ifdef SCIP_DISABLED
/** domination vertex reduction for the SPG */
void reduce_alt_dv(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   STP_Bool*             marked,             /**< nodes array */
   int*                  count               /**< pointer to number of reductions */
   )
{
   const int nnodes = g->knots;
   int nreds = 0;

   assert(scip   != NULL);
   assert(g      != NULL);
   assert(count  != NULL);
   assert(marked != NULL);
   assert(g->stp_type == STP_SPG);

   BMSclearMemoryArray(marked, nnodes);

   /* main loop */
   for( int k = 0; k < nnodes; k++ )
   {
      const SCIP_Real maxgrad = g->grad[k];

      if( maxgrad < 3 )
         continue;

      assert(g->mark[k]);

      /* mark adjacent vertices and k*/
      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         marked[g->head[e]] = TRUE;

      marked[k] = TRUE;

      /* check all other nodes */
      for( int i = 0; i < nnodes; i++ )
      {
         /* valid candidate? */
         if( !Is_term(g->term[i]) && g->grad[i] <= maxgrad && g->mark[i] && k != i )
         {
            SCIP_Real min;
            int e2;
            for( e2 = g->outbeg[i]; e2 != EAT_LAST; e2 = g->oeat[e2] )
               if( !marked[g->head[e2]] )
                  break;

            /* neighbors of j subset of those of k? */
            if( e2 == EAT_LAST )
            {
#if 0
               int maxe = g->outbeg[i];
               for( e2 = g->outbeg[i]; e2 != EAT_LAST; e2 = g->oeat[e2] )
                  if( g->cost[e] > g->cost[maxe] )
                     maxe = e;

               min = 0.0;




               (*count) += g->grad[i];
               while( g->outbeg[i] != EAT_LAST )
                  graph_edge_del(scip, g, g->outbeg[j] TRUE);

               g->mark[i] = FALSE;
               marked[i] = FALSE;
#endif
            }
         }
      }

      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         marked[g->head[e]] = FALSE;

      marked[k] = FALSE;

      for( int j = 0; j < nnodes; j++ )
         assert(marked[j] == FALSE);
   }

   *count += nreds;
}

#endif

/** adjacent neighbourhood reduction for the MWCSP */
SCIP_RETCODE reduce_ans(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int*                  nelims              /**< pointer to number of reductions */
   )
{
   int* marked;
   const int nnodes = graph_get_nNodes(g);

   assert(scip   != NULL);
   assert(nelims  != NULL);
   assert(graph_pc_isMw(g));
   assert(graph_valid(scip, g));

   *nelims = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &marked, nnodes) );
   graph_mark(g);

   /* unmark all nodes */
   for( int k = 0; k < nnodes; k++ )
      marked[k] = FALSE;

   /* check neighborhood of all nodes */
   for( int k = 0; k < nnodes; k++ )
   {
      SCIP_Real min;
      int e;
      int neighborcount = 0;

      if( !g->mark[k] )
      {
         assert(graph_pc_knotIsDummyTerm(g, k) || 0 == g->grad[k]);
         continue;
      }

      /* mark adjacent vertices and k*/
      for( e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         marked[g->head[e]] = TRUE;

      marked[k] = TRUE;

      if( LT(g->prize[k], 0.0) )
         min = g->prize[k];
      else
         min = 0.0;

      SCIPdebugMessage("check ANS base node %d \n", k);

      /* check all neighbors of k */
      e = g->outbeg[k];
      while( e != EAT_LAST && neighborcount++ < STP_RED_ANSMAXNEIGHBORS )
      {
         const int j = g->head[e];
         e = g->oeat[e];

         /* valid candidate? */
         if( g->grad[j] <= g->grad[k] && !Is_term(g->term[j]) && g->mark[j] )
         {
            ansProcessCandidate(scip, g, marked, nelims, min, j);
         }
      }

      ansUnmark(g, k, NULL, 0, marked);
   }

   assert(graph_valid(scip, g));

   SCIPfreeBufferArray(scip, &marked);

   return SCIP_OKAY;
}

/** advanced adjacent neighbourhood reduction for the MWCSP */
SCIP_RETCODE reduce_ansAdv(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int*                  nelims,             /**< pointer to number of performed reductions */
   SCIP_Bool             extneigbhood        /**< use extended neighbour hood */
   )
{
   int* marked;
   int candidates[MAX(STP_RED_ANSMAXCANDS, STP_RED_ANSEXMAXCANDS)];
   int neighbarr[STP_RED_CNSNN];
   const int nnodes = graph_get_nNodes(g);
   int* const RESTRICT gHead = g->head;
   int* const RESTRICT gOeat = g->oeat;

   assert(scip   != NULL);
   assert(nelims  != NULL);
   assert(graph_pc_isMw(g));

   *nelims = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &marked, nnodes) );

   for( int k = 0; k < nnodes; k++ )
      marked[k] = FALSE;

   /* check neighborhood of all non-terminals of degree >= 2 */
   for( int k = 0; k < nnodes; k++ )
   {
      SCIP_Real maxprize;
      int nNeigbors;
      int nCands;
      int maxgrad;

      if( (!(g->mark[k])) || (g->grad[k] < 2) || Is_term(g->term[k]) )
         continue;

      nNeigbors = 0;

      /* mark adjacent vertices and k */
      for( int e = g->outbeg[k]; e >= 0; e = gOeat[e] )
      {
         const int j = gHead[e];
         marked[j] = TRUE;

         if( SCIPisGT(scip, g->prize[j], 0.0) && nNeigbors < STP_RED_CNSNN )
         {
            assert(Is_term(g->term[j]));
            assert(!graph_pc_knotIsDummyTerm(g, j));

            neighbarr[nNeigbors++] = j;
         }
      }

      marked[k] = TRUE;
      maxgrad = g->grad[k];
      nCands = 0;

      /* mark neighbors of the neighbors */
      for( int n = 0; n < nNeigbors; n++ )
      {
         for( int e = g->outbeg[neighbarr[n]]; e >= 0; e = gOeat[e] )
         {
            marked[gHead[e]] = TRUE;
         }
         maxgrad += g->grad[neighbarr[n]];
      }

      assert(SCIPisLE(scip, g->prize[k], 0.0));

      maxprize = g->prize[k];

      if( extneigbhood )
      {
         assert(0 && "implement me");
      }
      else
      {
         for( int e = g->outbeg[k]; e != EAT_LAST; e = gOeat[e] )
         {
            const int neighbor = gHead[e];

            if( g->grad[neighbor] <= maxgrad && !Is_term(g->term[neighbor]) )
            {
               assert(g->mark[neighbor]);

               candidates[nCands++] = neighbor;
               if( nCands >= STP_RED_ANSMAXCANDS )
               {
                  SCIPdebugMessage("REACHED ANS LIMIT %d \n", nCands);
                  break;
               }
            }
         }
      }

      /* now check all neighbors of k for elimination */
      for( int l = 0; l < nCands; l++ )
         ansProcessCandidate(scip, g, marked, nelims, maxprize, candidates[l]);

      /* clean-up */
      ansUnmark(g, k, neighbarr, nNeigbors, marked);
   }

   assert(graph_valid(scip, g));

   SCIPfreeBufferArray(scip, &marked);

   return SCIP_OKAY;
}


/** alternative advanced adjacent neighbourhood reduction for the MWCSP */
SCIP_RETCODE reduce_ansAdv2(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int*                  nelims              /**< pointer to number of reductions */
   )
{
   int neighbarr[STP_RED_CNSNN + 1];
   int* marked;
   SCIP_Real min;
   const int nnodes = graph_get_nNodes(g);

   assert(scip   != NULL);
   assert(nelims  != NULL);
   assert(graph_pc_isMw(g));

   *nelims = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &marked, nnodes) );

   for( int k = 0; k < nnodes; k++ )
      marked[k] = FALSE;

   /* check neighborhood of all non-terminals of degree >= 2 */
   for( int k = 0; k < nnodes; k++ )
   {
      SCIP_Real maxSpecialNeighborPrize;
      int specialNeigborRound0;

      if( (!(g->mark[k])) || (g->grad[k] < 2) || Is_term(g->term[k]) )
         continue;

      maxSpecialNeighborPrize = g->prize[k];
      specialNeigborRound0 = -1;

      for( int run = 0; run < 2; run++ )
      {
         int e;
         int nNeighbors = 0;
         int specialNeighbor = UNKNOWN;
         int maxgrad = g->grad[k];

         /* mark (limited number of) adjacent vertices and k */
         for( e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         {
            const int neighbor = g->head[e];
            marked[neighbor] = TRUE;
            if( SCIPisGE(scip, g->prize[neighbor], 0.0) && nNeighbors < STP_RED_CNSNN - 1 )
            {
               assert(!graph_pc_knotIsDummyTerm(g, neighbor));

               neighbarr[nNeighbors++] = neighbor;
            }
            else if( GT(g->prize[neighbor], maxSpecialNeighborPrize) && neighbor != specialNeigborRound0 )
            {
               assert(g->mark[neighbor]);

               maxSpecialNeighborPrize = g->prize[neighbor];
               specialNeighbor = neighbor;
            }
         }

         marked[k] = TRUE;

         /* we already take the special neighbor in round 1; it might still be updated in round 2 */
         if( run == 0 && specialNeighbor != UNKNOWN )
            neighbarr[nNeighbors++] = specialNeighbor;

         /* find a neighbor of the neighbors to add to the neighbor set */
         for( int l = 0; l < nNeighbors; l++ )
         {
            const int neighbor = neighbarr[l];
            for( e = g->outbeg[neighbor]; e != EAT_LAST; e = g->oeat[e] )
            {
               const int neighborNeighbor = g->head[e];
               if( run == 1 && g->mark[neighborNeighbor] && !Is_term(g->term[neighborNeighbor])
                     && GT(g->prize[neighborNeighbor], maxSpecialNeighborPrize) && neighborNeighbor != specialNeigborRound0 )
               {
                  maxSpecialNeighborPrize = g->prize[neighborNeighbor];
                  specialNeighbor = neighborNeighbor;
               }
               marked[neighborNeighbor] = TRUE;
            }

            marked[neighbor] = VERTEX_NEIGHBOR;
            maxgrad += g->grad[neighbor];
         }

         if( run == 1 && specialNeighbor != UNKNOWN )
         {
            maxgrad += g->grad[specialNeighbor];
            neighbarr[nNeighbors++] = specialNeighbor;

            for( e = g->outbeg[specialNeighbor]; e != EAT_LAST; e = g->oeat[e] )
               marked[g->head[e]] = TRUE;
         }

         assert(SCIPisLE(scip, g->prize[k], 0.0));

         min = g->prize[k];
         if( specialNeighbor != UNKNOWN )
         {
            marked[specialNeighbor] = VERTEX_NEIGHBOR;
            specialNeigborRound0 = specialNeighbor;

            if( LT(g->prize[specialNeighbor], 0.0) )
               min += g->prize[specialNeighbor];
         }

         /* check all neighbors of k */
         e = g->outbeg[k];
         while( e != EAT_LAST )
         {
            const int neighbor = g->head[e];
            e = g->oeat[e];

            /* valid candidate? */
            if( g->grad[neighbor] <= maxgrad && !Is_term(g->term[neighbor]) && marked[neighbor] != VERTEX_NEIGHBOR )
            {
               assert(g->mark[neighbor]);

               ansProcessCandidate(scip, g, marked, nelims, min, neighbor);
            }
         }

         ansUnmark(g, k, neighbarr, nNeighbors, marked);
      }
   }

   assert(graph_valid(scip, g));

   SCIPfreeBufferArray(scip, &marked);

   return SCIP_OKAY;
}


/** advanced connected neighborhood subset reduction test for the MWCSP */
SCIP_RETCODE reduce_cnsAdv(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int*                  marked,             /**< nodes array for internal use */
   int*                  count               /**< pointer to number of reductions */
   )
{
   SCIP_Real min;
   SCIP_Real kprize;
   int neighbarr[STP_RED_CNSNN + 1];
   int neighbarr2[STP_RED_CNSNN + 1];
   int j;
   int e;
   int k2;
   int e2;
   int l;
   int nn;
   int nn2;
   int k2grad;
   int maxgrad;
   const int nnodes = graph_get_nNodes(g);

   assert(scip   != NULL);
   assert(g      != NULL);
   assert(count  != NULL);
   assert(marked != NULL);
   assert(g->stp_type == STP_MWCSP);

   k2grad = 0;
   *count = 0;

   /* unmark all nodes */
   for( int k = 0; k < nnodes; k++ )
      marked[k] = VERTEX_OTHER;

   /* first run: consider node plus adjacent terminals */

   /* check neighborhood of all nodes */
   for( int k = 0; k < nnodes; k++ )
   {
      if( !(g->mark[k]) || (g->grad[k] < 2) )
         continue;

      nn = 0;
      k2 = UNKNOWN;
      nn2 = 0;
      kprize = g->prize[k];
      maxgrad = g->grad[k];

      /* mark adjacent vertices and k */
      for( e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
      {
         j = g->head[e];

         if( !g->mark[j] )
            continue;

         if( GE(g->prize[j], 0.0) && nn < STP_RED_CNSNN - 1 )
         {
            neighbarr[nn++] = j;
            marked[j] = VERTEX_CONNECT;
         }
         else
         {
            marked[j] = VERTEX_NEIGHBOR;
         }
      }

      marked[k] = VERTEX_CONNECT;

      /* traverse all connected non-negative nodes and mark their neighbors */
      for( l = 0; l < nn; l++ )
      {
         for( e = g->outbeg[neighbarr[l]]; e != EAT_LAST; e = g->oeat[e] )
         {
            j = g->head[e];
            if( !g->mark[j] )
               continue;

            if( marked[j] == VERTEX_OTHER )
               marked[j] = VERTEX_NEIGHBOR;
         }
         maxgrad += g->grad[neighbarr[l]] - 1;
      }

      if( Is_term(g->term[k]) )
         min = 0.0;
      else
         min = g->prize[k];

      /* traverse all vertices (main loop) */
      for( j = 0; j < nnodes; j++ )
      {
         /* vertex part of the current connected subset? Or terminal? Or belonging to the extension of the graph? */
         if( marked[j] != VERTEX_CONNECT && g->mark[j] && !Is_term(g->term[j])
               &&
               /* valid candidate? */
               g->grad[j] <= maxgrad && SCIPisLE(scip, g->prize[j], min) )
         {
            for( e2 = g->outbeg[j]; e2 != EAT_LAST; e2 = g->oeat[e2] )
               if( marked[g->head[e2]] == VERTEX_OTHER )
                  break;

            /* neighbors of j subset of those of k? */
            if( e2 == EAT_LAST )
            {
               /* yes, delete vertex */
               while( g->outbeg[j] != EAT_LAST )
               {
                  e2 = g->outbeg[j];
                  (*count)++;
                  graph_edge_del(scip, g, e2, TRUE);
               }
               g->mark[j] = FALSE;
               marked[j] = VERTEX_OTHER;
            }
         }
      }

      for( e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         marked[g->head[e]] = VERTEX_OTHER;

      for( l = 0; l < nn; l++ )
         for( e = g->outbeg[neighbarr[l]]; e != EAT_LAST; e = g->oeat[e] )
            marked[g->head[e]] = VERTEX_OTHER;

      marked[k] = VERTEX_OTHER;

#ifdef DEBUG
      for( l = 0; l < nnodes; l++ )
      assert(marked[l] == VERTEX_OTHER);
#endif

   }
    /* second run: consider the same plus an additional (non-positive) vertex  */

   for( int k = 0; k < nnodes; k++ )
   {
      if( !(g->mark[k]) || g->grad[k] < 2 || Is_term(g->term[k]) )
         continue;

      nn = 0;
      k2 = UNKNOWN;
      nn2 = 0;
      kprize = g->prize[k];
      maxgrad = g->grad[k];

      /* mark adjacent vertices and k */
      for( e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
      {
         j = g->head[e];

         if( !g->mark[j] )
            continue;

         if( SCIPisGE(scip, g->prize[j], 0.0) && nn < STP_RED_CNSNN - 1 )
         {
            neighbarr[nn++] = j;
            marked[j] = VERTEX_CONNECT;
         }
         else if( (SCIPisGT(scip, g->prize[j], kprize) && nn2 < STP_RED_CNSNN)
               || (SCIPisGE(scip, g->prize[j], kprize) && j > k && nn2 < 3) )
         {
            neighbarr2[nn2++] = j;
            marked[j] = VERTEX_NEIGHBOR;
         }
         else
         {
            marked[j] = VERTEX_NEIGHBOR;
         }
      }

      marked[k] = VERTEX_CONNECT;

      /* traverse all connected non-negative nodes and mark their neighbors */
      for( l = 0; l < nn; l++ )
      {
         for( e = g->outbeg[neighbarr[l]]; e != EAT_LAST; e = g->oeat[e] )
         {
            j = g->head[e];
            if( !g->mark[j] )
               continue;

            if( marked[j] == VERTEX_OTHER && nn2 < STP_RED_CNSNN
                  && (SCIPisGT(scip, g->prize[j], kprize)
                        || (SCIPisGE(scip, g->prize[j], kprize) && j > k
                              && nn2 < 3)) )
            {
               neighbarr2[nn2++] = j;
               marked[j] = VERTEX_NEIGHBOR;
            }
            else if( marked[j] == VERTEX_OTHER )
            {
               marked[j] = VERTEX_NEIGHBOR;
            }
         }
         maxgrad += g->grad[neighbarr[l]] - 1;
      }

      if( Is_term(g->term[k]) )
         min = 0.0;
      else
         min = g->prize[k];

      for( l = 0; l < nn2; l++ )
      {
         k2 = neighbarr2[l];

         if( !g->mark[k2] )
            continue;

         for( e = g->outbeg[k2]; e != EAT_LAST; e = g->oeat[e] )
            if( marked[g->head[e]] == VERTEX_OTHER && g->mark[g->head[e]] )
               marked[g->head[e]] = VERTEX_TEMPNEIGHBOR;
         min += g->prize[k2];
         k2grad = g->grad[k2];
         maxgrad += k2grad - 1;
         assert(SCIPisLE(scip, g->prize[k2], 0.0));

         /* traverse all vertices (main loop) */
         for( j = 0; j < nnodes; j++ )
         {
            /* vertex part of the current connected subset? Or terminal? Or belonging to the extension of the graph? */
            if( marked[j] != VERTEX_CONNECT && g->mark[j]
                  && !Is_term(g->term[j]) &&
                  /* valid candidate? */
                  g->grad[j] <= maxgrad && SCIPisLE(scip, g->prize[j], min) )
            {
               for( e2 = g->outbeg[j]; e2 != EAT_LAST; e2 = g->oeat[e2] )
                  if( marked[g->head[e2]] == VERTEX_OTHER )
                     break;

               /* neighbors of j subset of those of k? */
               if( e2 == EAT_LAST )
               {
                  /* yes, delete vertex */
                  while( g->outbeg[j] != EAT_LAST )
                  {
                     e2 = g->outbeg[j];
                     (*count)++;
                     graph_edge_del(scip, g, e2, TRUE);
                  }
                  g->mark[j] = FALSE;
                  marked[j] = VERTEX_OTHER;
               }
            }
         }
         for( e = g->outbeg[k2]; e != EAT_LAST; e = g->oeat[e] )
            if( marked[g->head[e]] == VERTEX_TEMPNEIGHBOR
                  && g->mark[g->head[e]] )
               marked[g->head[e]] = VERTEX_OTHER;
         min -= g->prize[k2];
         maxgrad -= k2grad - 1;

      }
      for( e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         marked[g->head[e]] = VERTEX_OTHER;

      for( l = 0; l < nn; l++ )
         for( e = g->outbeg[neighbarr[l]]; e != EAT_LAST; e = g->oeat[e] )
            marked[g->head[e]] = VERTEX_OTHER;

      marked[k] = VERTEX_OTHER;
#ifdef DEBUG
      for( l = 0; l < nnodes; l++ )
      assert(marked[l] == VERTEX_OTHER);
#endif
   }

   return SCIP_OKAY;
}


/** non-positive vertex reduction for the MWCSP */
SCIP_RETCODE reduce_npv(
   SCIP*                 scip,
   GRAPH*                g,
   PATH*                 pathtail,
   int*                  heap,
   int*                  statetail,
   int*                  statehead,
   int*                  nelims,
   int                   limit
   )
{
   GRAPH* auxg;
   PATH mst[5];
   PATH* pathhead;
   int adjverts[5];
   int incedges[5];
   int* memlbltail;
   int* memlblhead;
   SCIP_Real prize;
   const int nnodes = graph_get_nNodes(g);

   assert(scip  != NULL);
   assert(pathtail != NULL);
   assert(heap != NULL);
   assert(statetail != NULL);
   assert(statehead != NULL);
   assert(nelims != NULL);
   assert(limit > 0);

   /* NOTE: necessary because of erroneous compiler warning */
   incedges[0] = incedges[1] = incedges[2] = -1;

   *nelims = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &pathhead, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &memlbltail, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &memlblhead, nnodes + 1) );

   graph_mark(g);

   /* initialize arrays */
   for( int i = 0; i < nnodes; i++ )
   {
      statetail[i]     = UNKNOWN;
      pathtail[i].dist = FARAWAY;
      pathtail[i].edge = UNKNOWN;
      statehead[i]     = UNKNOWN;
      pathhead[i].dist = FARAWAY;
      pathhead[i].edge = UNKNOWN;
   }


   /* --- NPV3 test --- */

   /* try to eliminate non-positive vertices of degree 3 */
   for( int i = 0; i < nnodes; i++ )
   {
      int k;
      SCIP_Real sdist0;
      SCIP_Real sdist1;
      SCIP_Real sdist2;

      assert(g->grad[i] >= 0);

      /* only non-positive vertices of degree 3 */
      if( g->grad[i] != 3 || !g->mark[i] || Is_term(g->term[i]) )
         continue;

      k = 0;
      for( int e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
      {
         assert(k < 3);
         assert(g->mark[g->head[e]]);

         incedges[k] = e;
         adjverts[k++] = g->head[e];
      }

      assert(k == 3);

      g->mark[i] = FALSE;

      prize = g->prize[i];
      SCIP_CALL( reduce_getSdByPaths(scip, g, pathtail, pathhead, &sdist0, -prize, heap, statetail, statehead, memlbltail, memlblhead, adjverts[0], adjverts[1], limit, FALSE, TRUE) );
      SCIP_CALL( reduce_getSdByPaths(scip, g, pathtail, pathhead, &sdist1, -prize, heap, statetail, statehead, memlbltail, memlblhead, adjverts[1], adjverts[2], limit, FALSE, TRUE) );
      SCIP_CALL( reduce_getSdByPaths(scip, g, pathtail, pathhead, &sdist2, -prize, heap, statetail, statehead, memlbltail, memlblhead, adjverts[2], adjverts[0], limit, FALSE, TRUE) );

      /* can vertex be deleted? */
      if( (SCIPisGE(scip, -sdist0 - sdist1, prize) && SCIPisGE(scip, -sdist2, prize))
         || (SCIPisGE(scip, -sdist1 - sdist2, prize) && SCIPisGE(scip, -sdist0, prize))
         || (SCIPisGE(scip, -sdist2 - sdist0, prize) && SCIPisGE(scip, -sdist1, prize))
         )
      {
         SCIPdebugMessage("npv3Reduction delete: %d (prize: %f) \n", i,  g->prize[i]);
         (*nelims) +=  g->grad[i];

         graph_knot_del(scip, g, i, TRUE);
      }
      else
      {
         if( SCIPisGE(scip, -sdist0 - sdist1, prize) )
         {
            SCIPdebugMessage("npv3Reduction delete edge: %d \n", incedges[1]);
            graph_edge_del(scip, g, incedges[1], TRUE);
            (*nelims) += 1;
         }
         else if( SCIPisGE(scip, -sdist1 - sdist2, prize) )
         {
            SCIPdebugMessage("npv3Reduction delete edge: %d \n", incedges[2]);
            graph_edge_del(scip, g, incedges[2], TRUE);
            (*nelims) += 1;
         }
         else if( SCIPisGE(scip, -sdist2 - sdist0, prize) )
         {
            SCIPdebugMessage("npv3Reduction delete edge: %d \n", incedges[0]);
            graph_edge_del(scip, g, incedges[0], TRUE);
            (*nelims) += 1;
         }

         g->mark[i] = TRUE;
         assert(g->grad[i] >= 2);
      }
   }

   /* --- NPV4 test --- */

   /* initialize mst struct and new graph for further tests */
   SCIP_CALL( graph_init(scip, &auxg, 5, 40, 1) );

   for( int k = 0; k < 4; k++ )
      graph_knot_add(auxg, -1);
   for( int k = 0; k < 4; k++ )
      for( int k2 = k + 1; k2 < 4; k2++ )
         graph_edge_add(scip, auxg, k, k2, 1.0, 1.0);

   SCIP_CALL( graph_path_init(scip, auxg) );

   /* try to eliminate non-positive vertices of degree 4 */
   for( int i = 0; i < nnodes; i++ )
   {
      int e;
      int k;

      /* only non-positive vertices of degree 4 */
      if( g->grad[i] != 4 || !g->mark[i] || Is_term(g->term[i]) )
         continue;

      k = 0;

      /* store neighbours */
      for( e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
         adjverts[k++] = g->head[e];

      assert(k == 4);

      g->mark[i] = FALSE;
      prize = g->prize[i];

      /* compute mw bottleneck distance to each pair of neighbours */
      for( k = 0; k < 4; k++ )
      {
         auxg->mark[k] = TRUE;
         for( e = auxg->outbeg[k]; e != EAT_LAST; e = auxg->oeat[e] )
         {
            const int k2 = auxg->head[e];
            if( k2 > k )
            {
               SCIP_Real sdist0;
               SCIP_CALL( reduce_getSdByPaths(scip, g, pathtail, pathhead, &(sdist0), -prize, heap, statetail, statehead, memlbltail, memlblhead, adjverts[k], adjverts[k2], limit, FALSE, TRUE) );
               auxg->cost[e] = sdist0;
               if( SCIPisGT(scip, prize, -auxg->cost[e]) )
                  break;

               auxg->cost[flipedge(e)] = auxg->cost[e];
            }
         }
         if( e != EAT_LAST )
            break;
      }

      k = UNKNOWN;
      if( e == EAT_LAST )
      {
         SCIP_Real sdist0 = 0.0;

         /* compute mst on all neighbours */
         graph_path_exec(scip, auxg, MST_MODE, 0, auxg->cost, mst);

         /* calculate mst cost */
         for( int l = 1; l < 4; l++ )
            sdist0 += mst[l].dist;

         if( SCIPisLE(scip, prize, -sdist0) )
         {
            /* compute subset msts on all neighbours */
            for( k = 0; k < 4; k++ )
            {
               auxg->mark[k] = FALSE;
               sdist0 = 0.0;
               if( k != 0 )
               {
                  graph_path_exec(scip, auxg, MST_MODE, 0, auxg->cost, mst);
                  for( int l = 1; l < 4; l++ )
                     if( auxg->mark[l] )
                        sdist0 += mst[l].dist;
               }
               else
               {
                  graph_path_exec(scip, auxg, MST_MODE, 1, auxg->cost, mst);
                  for( int l = 2; l < 4; l++ )
                     sdist0 += mst[l].dist;
               }
               auxg->mark[k] = TRUE;
               if( SCIPisGT(scip, prize, -sdist0) )
                  break;
            }
         }
      }

      /* can node be eliminated? */
      if( k == 4 )
      {
         SCIPdebugMessage("npv4Reduction delete: %d (prize: %f) \n", i,  g->prize[i]);
         (*nelims) += g->grad[i];

         graph_knot_del(scip, g, i, TRUE);
      }
      else
      {
         g->mark[i] = TRUE;
      }
   }

   /* --- NPV5 test --- */

   /* enlarge graph for NPV5 test*/
   graph_knot_add(auxg, -1);
   for( int k = 0; k < 4; k++ )
      graph_edge_add(scip, auxg, k, 4, 1.0, 1.0);
   graph_path_exit(scip, auxg);
   SCIP_CALL( graph_path_init(scip, auxg) );

   /* try to eliminate non-positive vertices of degree 5 */
   for( int i = 0; i < nnodes; i++ )
   {
      int e;
      int k;

      /* only non-positive vertices of degree 5 */
      if( g->grad[i] != 5 || !g->mark[i] || Is_term(g->term[i]) )
         continue;
      k = 0;

      /* store neighbours */
      for( e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
         adjverts[k++] = g->head[e];

      assert(k == 5);

      g->mark[i] = FALSE;
      prize = g->prize[i];

      for( k = 0; k < 5; k++ )
      {
         auxg->mark[k] = TRUE;
         for( e = auxg->outbeg[k]; e != EAT_LAST; e = auxg->oeat[e] )
         {
            const int k2 = auxg->head[e];
            if( k2 > k )
            {
               SCIP_Real sdist0;
               SCIP_CALL( reduce_getSdByPaths(scip, g, pathtail, pathhead, &(sdist0), -prize, heap, statetail, statehead, memlbltail, memlblhead, adjverts[k], adjverts[k2], limit, FALSE, TRUE) );
               auxg->cost[e] = sdist0;
               if( SCIPisGT(scip, prize, -auxg->cost[e]) )
                  break;

               auxg->cost[flipedge(e)] = auxg->cost[e];
            }
         }
         if( e != EAT_LAST )
            break;
      }
      k = UNKNOWN;
      if( e == EAT_LAST )
      {
         SCIP_Real sdist0 = 0.0;

         graph_path_exec(scip, auxg, MST_MODE, 0, auxg->cost, mst);

         for( int l = 1; l < 5; l++ )
            sdist0 += mst[l].dist;

         if( SCIPisLE(scip, prize, -sdist0) )
         {
            for( k = 0; k < 5; k++ )
            {
               int k2 = UNKNOWN;
               auxg->mark[k] = FALSE;
               sdist0 = 0.0;
               if( k != 0 )
               {
                  graph_path_exec(scip, auxg, MST_MODE, 0, auxg->cost, mst);
                  for( int l = 1; l < 5; l++ )
                     if( auxg->mark[l] )
                        sdist0 += mst[l].dist;
               }
               else
               {
                  graph_path_exec(scip, auxg, MST_MODE, 1, auxg->cost, mst);
                  for( int l = 2; l < 5; l++ )
                     sdist0 += mst[l].dist;
               }

               if( SCIPisLE(scip, prize, -sdist0) )
               {
                  for( k2 = k + 1; k2 < 5; k2++ )
                  {
                     if( k2 == k )
                        continue;
                     auxg->mark[k2] = FALSE;
                     sdist0 = 0.0;
                     if( k2 != 0 && k != 0)
                     {
                        graph_path_exec(scip, auxg, MST_MODE, 0, auxg->cost, mst);
                        for( int l = 1; l < 5; l++ )
                           if( auxg->mark[l] )
                              sdist0 += mst[l].dist;
                     }
                     else
                     {
                        int s;
                        if( k != 1 && k2 != 1 )
                           s = 1;
                        else
                           s = 2;
                        graph_path_exec(scip, auxg, MST_MODE, s, auxg->cost, mst);
                        for( int l = 0; l < 5; l++ )
                           if( auxg->mark[l] && l != s  )
                              sdist0 += mst[l].dist;
                     }
                     auxg->mark[k2] = TRUE;
                     if( SCIPisGT(scip, prize, -sdist0) )
                        break;
                  }
               }
               auxg->mark[k] = TRUE;
               if( k2 != 5 )
                  break;
            }
         }
      }

      if( k == 5 )
      {
         SCIPdebugMessage(" \n npv5Reduction delete: %d (prize: %f) \n", i,  g->prize[i]);
         (*nelims) += g->grad[i];

         graph_knot_del(scip, g, i, TRUE);
      }
      else
      {
         g->mark[i] = TRUE;
      }
   }

   /* free memory*/
   graph_path_exit(scip, auxg);
   graph_free(scip, &auxg, TRUE);

   SCIPfreeBufferArray(scip, &memlblhead);
   SCIPfreeBufferArray(scip, &memlbltail);
   SCIPfreeBufferArray(scip, &pathhead);

   return SCIP_OKAY;
}


/** chain reduction (NPV_2) for the MWCSP */
SCIP_RETCODE reduce_chain2(
   SCIP*                 scip,
   GRAPH*                g,
   PATH*                 pathtail,
   int*                  heap,
   int*                  statetail,
   int*                  statehead,
   int*                  nelims,
   int                   limit
   )
{
   PATH* pathhead;
   int* memlbltail;
   int* memlblhead;
   const int nnodes = graph_get_nNodes(g);

   assert(scip  != NULL);
   assert(pathtail != NULL);
   assert(heap != NULL);
   assert(statetail != NULL);
   assert(statehead != NULL);
   assert(nelims != NULL);

   *nelims = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &pathhead, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &memlbltail, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &memlblhead, nnodes + 1) );

   /* initialize arrays */
   for( int i = 0; i < nnodes; i++ )
   {
      statetail[i]     = UNKNOWN;
      pathtail[i].dist = FARAWAY;
      pathtail[i].edge = UNKNOWN;
      statehead[i]     = UNKNOWN;
      pathhead[i].dist = FARAWAY;
      pathhead[i].edge = UNKNOWN;
   }

   /* main loop: try to eliminate non-positive vertices of degree two */
   for( int i = 0; i < nnodes; i++ )
   {
      SCIP_Real sdist;
      int i1;
      int i2;
      int e1;
      int e2;

      if( g->grad[i] != 2 || !g->mark[i] || Is_term(g->term[i]) )
         continue;

      /* non-positive chains */
      e1 = g->outbeg[i];
      e2 = g->oeat[e1];
      i1 = g->head[e1];
      i2 = g->head[e2];

      assert(e1 >= 0);
      assert(e2 >= 0);
      assert(g->mark[i1]);
      assert(g->mark[i2]);

      g->mark[i] = FALSE;

      SCIP_CALL( reduce_getSdByPaths(scip, g, pathtail, pathhead, &sdist, -(g->prize[i]), heap, statetail, statehead, memlbltail, memlblhead, i1, i2, limit, FALSE, TRUE) );

      if( SCIPisGE(scip, -sdist, g->prize[i]) )
      {
         SCIPdebugMessage("delete : %d prize: %f sd: %f \n", i,  g->prize[i], -sdist );
         graph_edge_del(scip, g, e1, TRUE);
         graph_edge_del(scip, g, e2, TRUE);
         (*nelims) += 2;
      }
      else
      {
         g->mark[i] = TRUE;
      }
   }

   SCIPfreeBufferArray(scip, &memlblhead);
   SCIPfreeBufferArray(scip, &memlbltail);
   SCIPfreeBufferArray(scip, &pathhead);

   return SCIP_OKAY;
}


/** non-negative path reduction for the MWCSP */
SCIP_RETCODE reduce_nnp(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int*                  nelims              /**< pointer to number of reductions */
   )
{
   int* marked;
   int nelims_local = 0;
   const int nnodes = graph_get_nNodes(g);

   assert(scip   != NULL);
   assert(nelims  != NULL);
   assert(graph_pc_isMw(g));

   SCIP_CALL( SCIPallocBufferArray(scip, &marked, nnodes) );

   for( int k = 0; k < nnodes; k++ )
      marked[k] = FALSE;

   /* check neighborhood of terminals */
   for( int k = 0; k < nnodes; k++ )
   {
      if( !g->mark[k] || LT(g->prize[k], 0.0) )
         continue;

      /* mark adjacent vertices of k */
      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         if( g->mark[g->head[e]] )
            marked[g->head[e]] = TRUE;

      /* ... and traverse them */
      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
      {
         const int j = g->head[e];

         if( marked[j] )
         {
            int e2 = g->outbeg[j];

            while( e2 != EAT_LAST )
            {
               const int candedge = e2;
               e2 = g->oeat[e2];
               if( marked[g->head[candedge]] )
               {
                  graph_edge_del(scip, g, candedge, TRUE);
                  nelims_local++;
               }
            }
         }
      }

      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         marked[g->head[e]] = FALSE;

#ifndef NDEBUG
      for( int j = 0; j < nnodes; j++ )
         assert(FALSE == marked[j]);
#endif
   }

   *nelims = nelims_local;

   assert(graph_valid(scip, g));
   SCIPfreeBufferArray(scip, &marked);

   return SCIP_OKAY;
}


/** Combined implied-profit based tests:
 *  First elimination tests are used, afterwards
 *  edge contraction test are applied.
 *  NOTE: The expensive part is to build the bottleneck distances,
 *  thus we always apply all other tests. */
SCIP_RETCODE reduce_impliedProfitBased(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edgelimit,          /**< limit for star test */
   GRAPH*                g,                  /**< graph structure */
   int*                  solnode,            /**< node array to mark whether an node is part of a given solution (CONNECT) */
   SCIP_Real*            fixed,              /**< offset pointer */
   int*                  nelims              /**< number of eliminations */
)
{
   SD* sdistance;
   int nelimsnew = 0;
   const int isPcMw = graph_pc_isPcMw(g);

   assert(scip && g && nelims && fixed);
   assert(*nelims >= 0);

   if( g->terms <= 2 )
      return SCIP_OKAY;

   /* NOTE: pruning is necessary, because we need remaining graph to be connected */
   if( isPcMw )
   {
      reduce_unconnectedRpcRmw(scip, g, fixed);
   }
   else
   {
      SCIP_CALL( reduce_unconnected(scip, g) );
   }

   SCIP_CALL( reduce_sdInitBiasedBottleneck(scip, g, &sdistance) );

   /* first call edge elimination tests */
   if( !isPcMw )
   {
      SCIP_CALL( reduce_sdBiased(scip, sdistance, g, &nelimsnew) );
   }
   SCIP_CALL( reduce_sdStarBiasedWithProfit(scip, edgelimit, sdistance->sdprofit, TRUE, g, &nelimsnew) );

   if( nelimsnew > 0 )
   {
      SCIP_CALL( reduce_sdprofitBuildFromBLC(scip, g, sdistance->blctree, FALSE, sdistance->sdprofit) );
      SCIP_CALL( graph_tpathsRecomputeBiased(sdistance->sdprofit, g, sdistance->terminalpaths) );
      *nelims += nelimsnew;
   }

   /* now call edge contraction tests */
   SCIP_CALL( reduce_nsvImplied(scip, sdistance, g, solnode, fixed, nelims) );

   reduce_sdFree(scip, &sdistance);

   return SCIP_OKAY;
}


/** similar to above, but for rooted-prize collecting Steiner tree problem */
SCIP_RETCODE reduce_impliedProfitBasedRpc(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph structure */
   REDSOLLOCAL*          redsollocal,        /**< primal bound info */
   SCIP_Real*            fixed,              /**< offset pointer */
   int*                  nelims              /**< number of eliminations */
)
{
   STP_Vectype(int) contractedges = NULL;
   GRAPH* graph_spg;
   SD* sdistance;
   int* edgemap_new2org;
   int* solnode = reduce_sollocalGetSolnode(redsollocal);
   SCIP_Real fixednew = 0.0;
   int nelimsnew = 0;
   SCIP_Real primalbound;

   assert(scip && g && nelims && fixed);

   assert(*nelims >= 0);
   assert(g->stp_type == STP_RPCSPG);

   if( g->terms <= 2 )
      return SCIP_OKAY;

   reduce_sollocalSetOffset(*fixed, redsollocal);
   SCIP_CALL( reduce_sollocalRebuildTry(scip, g, redsollocal) );
   primalbound = reduce_sollocalGetUpperBound(redsollocal);

   if( GE(primalbound, FARAWAY) )
      return SCIP_OKAY;

   if( !graph_transRpcToSpgIsStable(g, primalbound) )
      return SCIP_OKAY;

   /* NOTE: pruning is necessary, because we need remaining graph to be connected */
   reduce_unconnectedRpcRmw(scip, g, fixed);

   assert(graph_valid(scip, g));

   graph_mark(g);
   SCIP_CALL( graph_transRpcGetSpg(scip, g, primalbound, &fixednew, &edgemap_new2org, &graph_spg) );

   SCIP_CALL( graph_path_init(scip, graph_spg) );
   SCIP_CALL( reduce_sdInitBiasedBottleneck(scip, graph_spg, &sdistance) );
   SCIP_CALL( reduce_sdBiased(scip, sdistance, graph_spg, &nelimsnew) );

   if( nelimsnew > 0 )
   {
      for( int i = 0; i < graph_spg->edges; i += 2 )
      {
         if( graph_spg->oeat[i] == EAT_FREE )
         {
            const int orgedge = edgemap_new2org[i];
            if( orgedge == -1 )
            {
               assert(Is_term(graph_spg->term[graph_spg->tail[i]]) || Is_term(graph_spg->term[graph_spg->head[i]]));
               continue;
            }
            assert(graph_edge_isInRange(g, orgedge));

#ifdef SCIP_DEBUG
            printf("delete %d \n", orgedge);
            graph_edge_printInfo(g, orgedge);
#endif

            graph_edge_del(scip, g, orgedge, TRUE);
            (*nelims)++;
         }
      }

      assert(graph_valid(scip, g));

      SCIP_CALL( reduce_sdprofitBuildFromBLC(scip, graph_spg, sdistance->blctree, FALSE, sdistance->sdprofit) );
      SCIP_CALL( graph_tpathsRecomputeBiased(sdistance->sdprofit, graph_spg, sdistance->terminalpaths) );
   }

   /* now call edge contraction tests */
   SCIP_CALL( reduce_nsvImpliedRecord(scip, sdistance, graph_spg, &contractedges) );

   for( int i = 0; i < StpVecGetSize(contractedges); i++ )
   {
      const int edge = contractedges[i];
      assert(graph_edge_isInRange(graph_spg, edge));

      if( edgemap_new2org[edge] != -1 )
      {
         const int orgedge = edgemap_new2org[edge];
         const int orgtail = g->tail[orgedge];
         const int orghead = g->head[orgedge];

         assert(graph_edge_isInRange(g, orgedge));

#ifdef SCIP_DEBUG
         printf("try \n");
         graph_edge_printInfo(g, orgedge);
#endif

         if( (graph_pc_knotIsFixedTerm(g, orgtail) || graph_pc_knotIsPropPotTerm(g, orgtail)) && g->source != orghead )
         {
            SCIPdebugMessage("contract %d \n", orgedge);
            *fixed += g->cost[orgedge];
            SCIP_CALL(graph_pc_contractEdge(scip, g, solnode, orgtail, orghead, orgtail));
            (*nelims)++;

            continue;
         }

         if( graph_pc_knotIsFixedTerm(g, orghead) || graph_pc_knotIsPropPotTerm(g, orghead) )
         {
            SCIPdebugMessage("contract %d \n", orgedge);
            *fixed += g->cost[orgedge];
            SCIP_CALL(graph_pc_contractEdge(scip, g, solnode, orghead, orgtail, orghead));
            (*nelims)++;

            continue;
         }
      }
   }

   StpVecFree(scip, contractedges);

   SCIPfreeMemoryArray(scip, &edgemap_new2org);
   graph_path_exit(scip, graph_spg);
   graph_free(scip, &graph_spg, FALSE);
   reduce_sdFree(scip, &sdistance);

   return SCIP_OKAY;
}
