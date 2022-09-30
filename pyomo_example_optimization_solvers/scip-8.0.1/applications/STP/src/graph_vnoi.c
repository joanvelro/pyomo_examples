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

/**@file   graph_vnoi.c
 * @brief  Voronoi graph algorithms for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file encompasses various Voronoi diagram algorithms.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include "portab.h"
#include "graph.h"
#include "reduce.h"


/* todo deprecated replace */
inline static
void correct(
   int* RESTRICT heap,
   int* RESTRICT state,
   int* RESTRICT count,    /* pointer to store the number of elements on the heap */
   PATH* RESTRICT path,
   int    l,
   int    k,
   int    e,
   SCIP_Real cost)
{
   int    t;
   int    c;
   int    j;

   path[l].dist = path[k].dist + cost;
   path[l].edge = e;

   /* new node? */
   if( state[l] == UNKNOWN )
   {
      heap[++(*count)] = l;
      state[l]      = (*count);
   }

   /* Heap shift up */
   j = state[l];
   c = j / 2;
   while( (j > 1) && path[heap[c]].dist > path[heap[j]].dist )
   {
      t              = heap[c];
      heap[c]        = heap[j];
      heap[j]        = t;
      state[heap[j]] = j;
      state[heap[c]] = c;
      j              = c;
      c              = j / 2;
   }
}

/* todo deprecated, replace */
inline static
int nearest(
   int* RESTRICT heap,
   int* RESTRICT state,
   int* RESTRICT count,    /* pointer to store the number of elements on the heap */
   const PATH* path)
{
   int   k;
   int   t;
   int   c;
   int   j;
   k              = heap[1];
   j              = 1;
   c              = 2;
   heap[1]        = heap[(*count)--];
   state[heap[1]] = 1;

   if ((*count) > 2)
      if (LT(path[heap[3]].dist, path[heap[2]].dist))
         c++;

   while((c <= (*count)) && GT(path[heap[j]].dist, path[heap[c]].dist))
   {
      t              = heap[c];
      heap[c]        = heap[j];
      heap[j]        = t;
      state[heap[j]] = j;
      state[heap[c]] = c;
      j              = c;
      c             += c;

      if ((c + 1) <= (*count))
         if (LT(path[heap[c + 1]].dist, path[heap[c]].dist))
            c++;
   }
   return(k);
}


/** initializes */
static
void vnoiInit(
   const GRAPH*          g,                  /**< graph data structure */
   DHEAP*                dheap,              /**< heap */
   VNOI*                 vnoi                /**< vnoi data structure */
)
{
   const int nnodes = graph_get_nNodes(g);
   SCIP_Real* RESTRICT nodes_dist = vnoi->nodes_dist;
   int* RESTRICT nodes_pred = vnoi->nodes_predEdge;
   int* RESTRICT nodes_base = vnoi->nodes_base;

   assert(graph_heap_isClean(dheap));

   for( int i = 0; i < nnodes; i++ )
   {
      assert(UNKNOWN == dheap->position[i]);

      if( Is_term(g->term[i]) )
      {
         nodes_base[i] = i;
         nodes_dist[i] = 0.0;

         graph_heap_correct(i, 0.0, dheap);
      }
      else
      {
         nodes_base[i] = UNKNOWN;
         nodes_dist[i] = FARAWAY;
      }

      nodes_pred[i] = UNKNOWN;
   }
}


/** builds a Voronoi region w.r.t. shortest paths for all terminals */
static
void vnoiCompute(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   DHEAP*                dheap,              /**< heap */
   VNOI*                 vnoi                /**< Voronoi data structure */
   )
{
   SCIP_Real* RESTRICT nodes_dist = vnoi->nodes_dist;
   int* RESTRICT nodes_pred = vnoi->nodes_predEdge;
   int* RESTRICT nodes_base = vnoi->nodes_base;
   int* const state = dheap->position;
   const SCIP_Real* const gCost = g->cost;
   const int* const gOeat = g->oeat;
   const int* const gHead = g->head;

   assert(dheap->size > 0);
   assert(graph_get_nNodes(g) > 1);

   /* until the heap is empty */
   while( dheap->size > 0 )
   {
      const int k = graph_heap_deleteMinReturnNode(dheap);
      assert(CONNECT == state[k]);

      for( int e = g->outbeg[k]; e >= 0; e = gOeat[e] )
      {
         const int m = gHead[e];

         if( state[m] != CONNECT )
         {
            const SCIP_Real newdist = nodes_dist[k] + gCost[e];

            /* check whether the path (to m) including k is shorter than the so far best known */
            if( GT(nodes_dist[m], newdist) )
            {
               graph_heap_correct(m, newdist, dheap);
               nodes_pred[m] = e;
               nodes_dist[m] = newdist;
               nodes_base[m] = nodes_base[k];
            }
         }
      }
   }
}


/** build a Voronoi region w.r.t. implied shortest paths for all terminals */
static
void vnoiComputeImplied(
   const GRAPH*          g,                  /**< graph data structure */
   const SDPROFIT*       sdprofit,           /**< SD profit */
   DHEAP*                dheap,              /**< heap */
   VNOI*                 vnoi                /**< Voronoi data structure */
)
{
   SCIP_Real* RESTRICT nodes_dist = vnoi->nodes_dist;
   int* RESTRICT nodes_pred = vnoi->nodes_predEdge;
   int* RESTRICT nodes_base = vnoi->nodes_base;
   int* const state = dheap->position;
   const SCIP_Real* const gCost = g->cost;
   const int* const gOeat = g->oeat;
   const int* const gHead = g->head;

   assert(dheap->size > 0);
   assert(graph_get_nNodes(g) > 1);

   /* until the heap is empty */
   while( dheap->size > 0 )
   {
      const int k = graph_heap_deleteMinReturnNode(dheap);
      const int k_predNode = (nodes_pred[k] >= 0) ? g->tail[nodes_pred[k]] : -1;
      const SCIP_Real k_dist = nodes_dist[k];

      assert(CONNECT == state[k]);

      for( int e = g->outbeg[k]; e >= 0; e = gOeat[e] )
      {
         const int m = gHead[e];

         if( state[m] != CONNECT )
         {
            const SCIP_Real profit = reduce_sdprofitGetProfit(sdprofit, k, k_predNode, m);
            const SCIP_Real bias = MIN(gCost[e], profit);
            const SCIP_Real newdist = k_dist + gCost[e] - MIN(k_dist, bias);

            /* check whether the path (to m) including k is shorter than the so far best known */
            if( GT(nodes_dist[m], newdist) )
            {
               graph_heap_correct(m, newdist, dheap);
               nodes_pred[m] = e;
               nodes_dist[m] = newdist;
               nodes_base[m] = nodes_base[k];
            }
         }
      }
   }
}


/** extend a Voronoi region until all neighbouring terminals are spanned */
SCIP_RETCODE graph_voronoiExtend(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      cost,               /**< edgecosts */
   PATH*                 path,               /**< shortest paths data structure */
   SCIP_Real**           distarr,            /**< array to store distance from each node to its base */
   int**                 basearr,            /**< array to store the bases */
   int**                 edgearr,            /**< array to store the ancestor edge */
   STP_Bool*             termsmark,          /**< array to mark terminal */
   int*                  reachednodes,       /**< array to mark reached nodes */
   int*                  nreachednodes,      /**< pointer to number of reached nodes */
   int*                  nodenterms,         /**< array to store number of terimals to each node */
   int                   nneighbterms,       /**< number of neighbouring terminals */
   int                   base,               /**< voronoi base */
   int                   countex             /**< count of heap elements */
   )
{
   int   k;
   int   m;
   int   i;
   int* heap;
   int* state;
   int count = countex;

   assert(g      != NULL);
   assert(g->path_heap   != NULL);
   assert(g->path_state  != NULL);
   assert(path   != NULL);
   assert(cost   != NULL);

   if( g->knots == 0 || nneighbterms <= 0 )
      return SCIP_OKAY;

   heap = g->path_heap;
   state = g->path_state;

   if( g->knots > 1 )
   {
      while( count > 0 && nneighbterms > 0 )
      {
         k = nearest(heap, state, &count, path);
         state[k] = CONNECT;
         reachednodes[(*nreachednodes)++] = k;
         distarr[k][nodenterms[k]] = path[k].dist;
         edgearr[k][nodenterms[k]] = path[k].edge;
         basearr[k][nodenterms[k]] = base;

         nodenterms[k]++;

         if( termsmark[k] == TRUE )
         {
            termsmark[k] = FALSE;
            if( --nneighbterms == 0 )
            {
               while( count > 0 )
                  reachednodes[(*nreachednodes)++] = heap[count--];

               return SCIP_OKAY;
            }
         }
         else if( !Is_term(g->term[k]) )
         {
            /* iterate over all outgoing edges of vertex k */
            for( i = g->outbeg[k]; i != EAT_LAST; i = g->oeat[i] )
            {
               m = g->head[i];

               /* check whether the path (to m) including (k, m) is shorter than the so far best known */
               if( (state[m]) && (g->mark[m]) && (GT(path[m].dist, (path[k].dist + cost[i]))) )
                  correct(heap, state, &count, path, m, k, i, cost[i]);
            }
         }
      }
      assert(nneighbterms == 0);
   }
   return SCIP_OKAY;
}


/** build a Voronoi region, w.r.t. shortest paths, for a given set of bases */
void graph_voronoi(
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      cost,               /**< edge costs */
   const SCIP_Real*      costrev,            /**< reversed edge costs (or NULL if symmetric) */
   const STP_Bool*       basemark,           /**< array to indicate whether a vertex is a Voronoi base */
   int*                  vbase,              /**< voronoi base to each vertex */
   PATH*                 path                /**< path data struture (leading to respective Voronoi base) */
   )
{
   int* RESTRICT heap;
   int* RESTRICT state;
   int root;
   int count = 0;
   int nbases = 0;

   assert(g && cost && basemark && vbase && path);
   assert(g->path_heap != NULL);
   assert(g->path_state != NULL);

   if( g->knots == 0 )
      return;

   root = g->source;
   heap = g->path_heap;
   state = g->path_state;

   /* initialize */
   for( int i = 0; i < g->knots; i++ )
   {
      /* set the base of vertex i */
      if( basemark[i] )
      {
         nbases++;
         if( g->knots > 1 )
            heap[++count] = i;
         vbase[i] = i;
         path[i].dist = 0.0;
         path[i].edge = UNKNOWN;
         state[i] = count;
      }
      else
      {
         vbase[i] = UNKNOWN;
         path[i].dist = FARAWAY;
         path[i].edge = UNKNOWN;
         state[i] = UNKNOWN;
      }
   }
   assert(nbases > 0);

   if( g->knots > 1 )
   {
      /* with reversed costs? */
      if( costrev )
      {
         /* until the heap is empty */
         while( count > 0 )
         {
            /* get the next (i.e. a nearest) vertex of the heap */
            const int k = nearest(heap, state, &count, path);

            /* mark vertex k as scanned */
            state[k] = CONNECT;

            for( int i = g->outbeg[k]; i >= 0; i = g->oeat[i] )
            {
               const int m = g->head[i];

               /* check whether the path (to m) including k is shorter than the so far best known */
               if( (state[m]) && GT(path[m].dist, path[k].dist + ((vbase[k] == root)? cost[i] : costrev[i])) )
               {
                  assert(!basemark[m]);
                  correct(heap, state, &count, path, m, k, i, ((vbase[k] == root)? cost[i] : costrev[i]));
                  vbase[m] = vbase[k];
               }
            }
         }
      }
      else
      {
         while( count > 0 )
         {
            const int k = nearest(heap, state, &count, path);
            state[k] = CONNECT;

            for( int i = g->outbeg[k]; i >= 0; i = g->oeat[i] )
            {
               const int m = g->head[i];
               if( (state[m]) && GT(path[m].dist, path[k].dist + cost[i]) )
               {
                  assert(!basemark[m]);
                  correct(heap, state, &count, path, m, k, i, cost[i]);
                  vbase[m] = vbase[k];
               }
            }
         }
      }
   }
}




/** build a voronoi region, w.r.t. shortest paths, for all terminal
 * NOTE: uses bias for PC! */
void graph_voronoiTerms(
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Bool*      nodes_isTerm,       /**< for each node: is terminal? */
   int* RESTRICT         vbase,              /**< array containing Voronoi base to each node */
   PATH* RESTRICT        path                /**< array containing Voronoi paths data */
   )
{
   int* RESTRICT heap;
   int* RESTRICT state;
   int heapsize;
   const int nnodes = graph_get_nNodes(g);

   assert(nodes_isTerm && vbase && path);

   heap = g->path_heap;
   state = g->path_state;
   assert(heap && state);

   heapsize = 0;

   /* initialize */
   for( int i = 0; i < nnodes; i++ )
   {
      /* set the base of vertex i */
      if( nodes_isTerm[i] && g->mark[i] )
      {
         if( nnodes > 1 )
            heap[++heapsize] = i;
         vbase[i] = i;
         state[i] = heapsize;
         path[i].dist = 0.0;
      }
      else
      {
         vbase[i] = UNKNOWN;
         state[i] = UNKNOWN;
         path[i].dist = FARAWAY;
      }
      path[i].edge = UNKNOWN;
   }

   if( nnodes > 1 )
   {
      const SCIP_Bool isPc = graph_pc_isPc(g);

      /* until the heap is empty */
      while( heapsize > 0 )
      {
         /* get the next (i.e. a nearest) vertex of the heap */
         const int k = nearest(heap, state, &heapsize, path);

         /* mark vertex k as scanned */
         state[k] = CONNECT;

         /* iterate over all outgoing edges of vertex k */
         for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         {
            SCIP_Real costbiased;
            const int m = g->head[e];

            costbiased = g->cost[e];

            if( isPc && !nodes_isTerm[k] )
               costbiased -= g->prize[k];

            assert(GE(costbiased, 0.0) && LE(costbiased, g->cost[e]));

            if( state[m] && g->mark[m] && GT(path[m].dist, path[k].dist + costbiased) )
            {
               correct(heap, state, &heapsize, path, m, k, e, costbiased);
               vbase[m] = vbase[k];
            }
         }
      }
   }
}


/** build a Voronoi region, w.r.t. shortest paths, for all positive vertices */
void graph_voronoiMw(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      costrev,            /**< reversed edge costs */
   PATH*                 path,               /**< path data structure (leading to respective Voronoi base) */
   int*                  vbase,              /**< Voronoi base to each vertex */
   int*                  heap,               /**< array representing a heap */
   int*                  state               /**< array to mark the state of each node during calculation */
   )
{
   int k;
   int m;
   int i;
   int count = 0;
   int nbases = 0;
   int nnodes;

   assert(g      != NULL);
   assert(path   != NULL);
   assert(costrev   != NULL);
   assert(heap   != NULL);
   assert(state   != NULL);

   nnodes = g->knots;

   /* initialize */
   for( i = 0; i < nnodes; i++ )
   {
      /* set the base of vertex i */
      if( Is_term(g->term[i]) && g->mark[i] )
      {
         nbases++;
         if( nnodes > 1 )
            heap[++count] = i;
         vbase[i] = i;
         path[i].dist = -g->prize[i];
         path[i].edge = UNKNOWN;
         state[i] = count;
      }
      else
      {
         vbase[i] = UNKNOWN;
         path[i].dist = FARAWAY;
         path[i].edge = UNKNOWN;
         state[i]     = UNKNOWN;
      }
   }
   if( nbases == 0 )
      return;
   if( nnodes > 1 )
   {
      /* until the heap is empty */
      while( count > 0 )
      {
         /* get the next (i.e. a nearest) vertex of the heap */
         k = nearest(heap, state, &count, path);

         /* mark vertex k as scanned */
         state[k] = CONNECT;

         /* iterate over all outgoing edges of vertex k */
         for( i = g->outbeg[k]; i != EAT_LAST; i = g->oeat[i] )
         {
            m = g->head[i];

            /* check whether the path (to m) including k is shorter than the so far best known */
            if( (state[m]) && SCIPisGT(scip, path[m].dist, path[k].dist + costrev[i]) && g->mark[m] && !Is_term(g->term[m]) )
            {
               assert(vbase[m] != m);
               correct(heap, state, &count, path, m, k, i, costrev[i]);
               vbase[m] = vbase[k];
            }
         }
      }
   }
   return;
}


/** build a voronoi region, w.r.t. shortest paths, for all terminal and the distance */
SCIP_RETCODE graph_voronoiWithDist(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Bool*      nodes_isTerm,       /**< for each node: is terminal? */
   const SCIP_Real*      cost,               /**< edge costs */
   SCIP_Real*            distance,           /**< array storing path from a terminal over shortest
                                                incident edge to nearest terminal */
   int*                  edges_isMinedgePred,        /**< shortest edge predecessor array */
   int*                  vbase,              /**< array containing Voronoi base to each node */
   int*                  minarc,             /**< array to mark whether an edge is one a path corresponding to 'distance' */
   int*                  distnode,           /**< array to store terminal corresponding to distance stored in distance array */
   PATH*                 path                /**< array containing Voronoi paths data */
   )
{
   int* RESTRICT heap;
   int* RESTRICT state;
   int count;
   int nbases;
   const int nnodes = graph_get_nNodes(g);

   assert(path     != NULL);
   assert(cost     != NULL);
   assert(distance != NULL);
   assert(nodes_isTerm);

   heap = g->path_heap;
   state = g->path_state;
   assert(heap);
   assert(state);

   count = 0;
   nbases = 0;

   if( distnode != NULL )
   {
      for( int i = 0; i < nnodes; i++ )
         distnode[i] = UNKNOWN;
   }

   /* initialize */
   for( int i = 0; i < nnodes; i++ )
   {
      distance[i] = FARAWAY;

      /* set the base of vertex i */
      if( nodes_isTerm[i] && g->mark[i] )
      {
         nbases++;
         if( nnodes > 1 )
            heap[++count] = i;
         vbase[i] = i;
         state[i] = count;
         path[i].dist = 0.0;
      }
      else
      {
         vbase[i] = UNKNOWN;
         state[i] = UNKNOWN;
         path[i].dist = FARAWAY;
      }
      path[i].edge = UNKNOWN;
   }

   /* initialize predecessor array */

   for( int e = 0; e < g->edges; e++ )
      edges_isMinedgePred[e] = FALSE;

   for( int k = 0; k < nbases; k++ )
   {
      if( minarc[k] == UNKNOWN )
      {
         assert(graph_pc_isPc(g));
         continue;
      }
      assert(graph_edge_isInRange(g, minarc[k]));
      edges_isMinedgePred[minarc[k]] = TRUE;
   }

   /* if graph contains more than one vertices, start main loop */
   if( nnodes > 1 )
   {
      const SCIP_Bool isPc = graph_pc_isPc(g);
      /* until the heap is empty */
      while( count > 0 )
      {
         /* get the next (i.e. a nearest) vertex of the heap */
         const int k = nearest(heap, state, &count, path);
         const int prededge = path[k].edge;

         /* mark vertex k as scanned */
         state[k] = CONNECT;

         if( prededge != UNKNOWN )
         {
            const int pred = g->tail[prededge];

            assert(vbase[k] != UNKNOWN);
            assert(vbase[pred] != UNKNOWN);
            assert(vbase[pred] == vbase[k]);
            assert(g->head[prededge] == k);

            if( !nodes_isTerm[pred] && g->mark[pred] )
            {
               assert(path[pred].edge != UNKNOWN);
               edges_isMinedgePred[prededge] = edges_isMinedgePred[path[pred].edge];
            }
         }

         /* iterate over all outgoing edges of vertex k */
         for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         {
            SCIP_Real costbiased;
            const int m = g->head[e];

            costbiased = cost[e];

            if( isPc && !nodes_isTerm[k] )
               costbiased -= g->prize[k];

            assert(GE(costbiased, 0.0) && LE(costbiased, cost[e]));

            if( state[m] == CONNECT && vbase[m] != vbase[k] )
            {
               assert(g->mark[m]);

               if( edges_isMinedgePred[e] || (prededge != UNKNOWN && edges_isMinedgePred[prededge] ) )
               {
                  SCIP_Real distnew = path[k].dist + costbiased + path[m].dist;
                  if( isPc && !nodes_isTerm[m] )
                     distnew -= g->prize[m];

                  if( SCIPisLT(scip, distnew, distance[vbase[k]]) )
                  {
                     if( distnode != NULL )
                        distnode[vbase[k]] = vbase[m];

                     distance[vbase[k]] = distnew;
                  }
               }
               if( edges_isMinedgePred[Edge_anti(e)] || (path[m].edge != UNKNOWN && edges_isMinedgePred[path[m].edge] ) )
               {
                  SCIP_Real distnew = path[m].dist + costbiased + path[k].dist;
                  if( isPc && !nodes_isTerm[m] )
                     distnew -= g->prize[m];

                  if( SCIPisLT(scip, distnew, distance[vbase[m]]) )
                  {
                     if( distnode != NULL )
                        distnode[vbase[m]] = vbase[k];

                     distance[vbase[m]] = distnew;
                  }
               }
            }

            /* Check whether the path (to m) including k is shorter than the so far best known.
               In case of equality, also update if k is sucessor of minedge. */
            if( state[m] && g->mark[m] &&
               (SCIPisGT(scip, path[m].dist, path[k].dist + costbiased) ||
               (prededge != UNKNOWN && edges_isMinedgePred[prededge] && SCIPisEQ(scip, path[m].dist, path[k].dist + costbiased))) )
            {
               /* NOTE guard against cycling */
               if( isPc && EQ(path[m].dist, path[k].dist + costbiased) && EQ(costbiased, 0.0) )
                  continue;

               correct(heap, state, &count, path, m, k, e, costbiased);
               vbase[m] = vbase[k];
            }
         }
      }
   }

   return SCIP_OKAY;
}

/** build voronoi regions, w.r.t. shortest paths, for all terminals and compute the radii */
SCIP_RETCODE graph_voronoiWithRadius(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   GRAPH*                adjgraph,           /**< graph data structure */
   PATH*                 path,               /**< array containing Voronoi paths data */
   SCIP_Real*            rad,                /**< array storing shortest way from a terminal out of its Voronoi region */
   SCIP_Real*            cost,               /**< edge costs */
   SCIP_Real*            costrev,            /**< reversed edge costs */
   int*                  vbase,              /**< array containing Voronoi base of each node */
   int*                  heap,               /**< array representing a heap */
   int*                  state               /**< array to mark state of each node during calculation */
   )
{
   int* nodesid;
   int count = 0;
   int nterms = 0;
   const int root = graph->source;
   const int nnodes = graph->knots;
   const STP_Bool mw = (graph->stp_type == STP_MWCSP);
   const STP_Bool pc = ((graph->stp_type == STP_PCSPG) || (graph->stp_type == STP_RPCSPG));

   assert(graph != NULL);
   assert(heap   != NULL);
   assert(state  != NULL);
   assert(path   != NULL);
   assert(cost   != NULL);
   assert(costrev   != NULL);
   assert(rad != NULL);
   assert(vbase != NULL);
   assert(!graph->extended);

   if( graph->terms == 0 )
      return SCIP_OKAY;

   SCIP_CALL( SCIPallocBufferArray(scip, &nodesid, nnodes) );

   /* initialize */
   for( int i = 0; i < nnodes; i++ )
   {
      rad[i] = FARAWAY;

      /* set the base of vertex i */
      if( Is_term(graph->term[i]) && graph->mark[i] )
      {
         if( nnodes > 1 )
            heap[++count] = i;

         if( !mw && adjgraph != NULL )
         {
            adjgraph->mark[adjgraph->knots] = TRUE;
            graph_knot_add(adjgraph, -1);
         }

         nodesid[i] = nterms++;
         vbase[i] = i;

         if( mw )
            assert(SCIPisGE(scip, graph->prize[i], 0.0));
         if( mw )
            path[i].dist = -graph->prize[i];
         else
            path[i].dist = 0.0;

         path[i].edge = UNKNOWN;
         state[i] = count;
      }
      else
      {
         vbase[i] = UNKNOWN;
         nodesid[i] = UNKNOWN;
         path[i].dist = FARAWAY;
         path[i].edge = UNKNOWN;
         state[i]     = UNKNOWN;
      }
   }

   if( nnodes > 1 )
   {
      SCIP_Real ecost;

      /* until the heap is empty */
      while( count > 0 )
      {
         /* get the next (i.e. a nearest) vertex of the heap */
         const int k = nearest(heap, state, &count, path);

         /* mark vertex k as scanned */
         state[k] = CONNECT;

         /* iterate over all outgoing edges of vertex k */
         for( int i = graph->outbeg[k]; i != EAT_LAST; i = graph->oeat[i] )
         {
            const int m = graph->head[i];
            const int vbm = vbase[m];
            const int vbk = vbase[k];

            if( state[m] == CONNECT && vbm != vbk && graph->mark[m] )
            {
               assert(graph->mark[vbm]);
               assert(graph->mark[vbk]);
               if( mw )
               {
                  if( SCIPisGT(scip, rad[vbk], path[k].dist) )
                     rad[vbk] = path[k].dist;
                  if( SCIPisGT(scip, rad[vbm], path[m].dist) )
                     rad[vbm] = path[m].dist;
               }
               else
               {

                  if( SCIPisGT(scip, rad[vbk], path[k].dist + ((vbk == root) ? cost[i] : costrev[i])) )
                     rad[vbk] = path[k].dist + ((vbk == root) ? cost[i] : costrev[i]);

                  if( SCIPisGT(scip, rad[vbm], path[m].dist + ((vbm == root) ? costrev[i] : cost[i])) )
                     rad[vbm] = path[m].dist + ((vbm == root) ? costrev[i] : cost[i]);

                  if( adjgraph != NULL )
                  {
                     int ne;

                     if( graph->stp_type == STP_DHCSTP )
                     {
                        SCIP_Real c1;
                        SCIP_Real c2;

                        if( m == root )
                           c1 = path[m].dist + costrev[i];
                        else
                           c1 = path[m].dist + cost[i];
                        if( k == root )
                           c2 = path[k].dist + cost[i];
                        else
                           c2 = path[k].dist + costrev[i];

                        if( SCIPisGT(scip, c1, c2) )
                           ecost = c2;
                        else
                           ecost = c1;
                     }
                     else
                     {
                        if( SCIPisGT(scip, path[m].dist, path[k].dist) )
                           ecost = path[k].dist + cost[i];
                        else
                           ecost = path[m].dist + cost[i];
                     }

                     if( pc )
                     {
                        if( SCIPisGT(scip, ecost, graph->prize[vbm]) && root != vbm && !graph_pc_knotIsFixedTerm(graph, vbm) )
                           ecost = graph->prize[vbm];

                        if( SCIPisGT(scip, ecost, graph->prize[vbk]) && root != vbk && !graph_pc_knotIsFixedTerm(graph, vbk) )
                           ecost = graph->prize[vbk];
                     }

                     /* find edge in adjgraph */
                     for( ne = adjgraph->outbeg[nodesid[vbk]]; ne != EAT_LAST; ne = adjgraph->oeat[ne] )
                        if( adjgraph->head[ne] == nodesid[vbm] )
                           break;

                     /* edge exists? */
                     if( ne != EAT_LAST )
                     {
                        assert(ne >= 0);
                        assert(adjgraph->head[ne] == nodesid[vbm]);
                        assert(adjgraph->tail[ne] == nodesid[vbk]);
                        if( SCIPisGT(scip, adjgraph->cost[ne], ecost) )
                        {
                           adjgraph->cost[ne] = ecost;
                           adjgraph->cost[Edge_anti(ne)] = ecost;
                        }
                     }
                     else
                     {
                        graph_edge_add(scip, adjgraph, nodesid[vbm], nodesid[vbk], ecost, ecost);
                     }
                  }
               }
            }

            /* check whether the path (to m) including k is shorter than the so far best known */
            if( state[m] && graph->mark[m] && !Is_term(graph->term[m]) && SCIPisGT(scip, path[m].dist, path[k].dist + ((vbk == root)? cost[i] : costrev[i])) )
            {
               correct(heap, state, &count, path, m, k, i, ((vbk == root)? cost[i] : costrev[i]));
               vbase[m] = vbk;
            }
         }
      }
   }
   SCIPfreeBufferArray(scip, &nodesid);

   return SCIP_OKAY;
}

/** Build partition of graph for MWCSP into regions around the positive vertices.
 * Store the weight of a minimum weight center-boundary path for each region
 * in the radius array (has to be reverted to obtain the final r() value).
 */
void graph_voronoiWithRadiusMw(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   PATH*                 path,               /**< array containing graph decomposition data */
   const SCIP_Real*      cost,               /**< edge costs */
   SCIP_Real*            radius,             /**< array storing shortest way from a positive vertex out of its region */
   int*                  vbase,              /**< array containing base of each node */
   int*                  heap,               /**< array representing a heap */
   int*                  state               /**< array to mark state of each node during calculation */
   )
{
   SCIP_Real* nodeweight;
   int i;
   int k;
   int m;
   int count;
   int nbases;
   int nnodes;
   int nterms;

   assert(g != NULL);
   assert(heap   != NULL);
   assert(state  != NULL);
   assert(path   != NULL);
   assert(cost   != NULL);
   assert(radius != NULL);
   assert(vbase != NULL);

   count = 0;
   nbases = 0;
   nnodes = g->knots;
   nterms = g->terms - 1;
   nodeweight = g->prize;

   assert(nodeweight != NULL);

   if( nnodes == 0 || nterms <= 0 )
      return;

   assert(!g->mark[g->source]);

   /* initialize data */
   for( i = 0; i < nnodes; i++ )
   {
      radius[i] = FARAWAY;

      /* set the base of vertex i */
      if( Is_term(g->term[i]) && g->mark[i] )
      {
         nbases++;
         if( nnodes > 1 )
            heap[++count] = i;
         vbase[i] = i;
         path[i].dist = 0.0;
         path[i].edge = UNKNOWN;
         state[i] = count;
      }
      else
      {
         vbase[i] = UNKNOWN;
         path[i].dist = FARAWAY;
         path[i].edge = UNKNOWN;
         state[i]     = UNKNOWN;
      }
   }

   if( nbases == 0 )
      return;

   if( nnodes > 1 )
   {
      int basem;
      int basek;

      /* until the heap is empty */
      while( count > 0 )
      {
         /* get the next (i.e. a nearest) vertex of the heap */
         k = nearest(heap, state, &count, path);

         /* mark vertex k as scanned */
         state[k] = CONNECT;
         basek = vbase[k];

         assert(g->mark[basek]);

         /* iterate over all outgoing edges of vertex k */
         for( i = g->outbeg[k]; i != EAT_LAST; i = g->oeat[i] )
         {
            m = g->head[i];

            if( !(g->mark[m]) )
               continue;

            basem = vbase[m];

            /* are both m and k finally in different regions? */
            if( basem != basek && (state[m] == CONNECT || vbase[m] == m ||
               SCIPisGE(scip, path[k].dist, nodeweight[basek])) )
            {
               if( SCIPisGT(scip, radius[basek], path[k].dist) )
                  radius[basek] = path[k].dist;
               if( (state[m] == CONNECT || vbase[m] == m) && SCIPisGT(scip, radius[basem], path[m].dist) )
               {
                  assert(g->mark[basem]);
                  radius[basem] = path[m].dist;
               }
            }

            /* is the distance of vertex k smaller than the weight of its base node and smaller than the temp. radius? */
            if( SCIPisLT(scip, path[k].dist, nodeweight[basek]) && SCIPisLT(scip, path[k].dist, radius[basek]) )
            {
               /* check whether the path (to m) including k is shorter than the so far best known */
               if( (state[m]) && SCIPisGT(scip, path[m].dist, path[k].dist + cost[i]) )
               {
                  assert(vbase[m] != m);
                  correct(heap, state, &count, path, m, k, i, cost[i]);
                  vbase[m] = basek;
               }
            }
         }
      }
   }
   return;
}

/** repair a Voronoi diagram for a given set of base nodes */
void graph_voronoiRepair(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      cost,               /**< edge costs (possibly biased) */
   const SCIP_Real*      cost_org,           /**< original edge costs (only needed for PC) */
   int*                  nheapelems,         /**< pointer to number of heap elements */
   int*                  vbase,              /**< array containing Voronoi base of each node */
   PATH*                 path,               /**< Voronoi paths data struture */
   int*                  newedge,            /**< the new edge */
   int                   crucnode,           /**< the current crucial node */
   UF*                   uf                  /**< union find data structure */
   )
{
   const int nnodes = graph_get_nNodes(g);
   int boundaryedge = UNKNOWN;
   SCIP_Real boundarydist = FARAWAY;

   assert(cost && scip && nheapelems && newedge && vbase && uf);

   *newedge = UNKNOWN;

   if( nnodes > 1 )
   {
      int* const heap = g->path_heap;
      int* const state = g->path_state;
      const SCIP_Bool isPcMw = graph_pc_isPcMw(g);
      const SCIP_Bool isPc = graph_pc_isPc(g);

      assert(heap && state);

      /* until the heap is empty */
      while( *nheapelems > 0 )
      {
         /* get the next (i.e. a nearest) vertex from the heap */
         const int k = nearest(heap, state, nheapelems, path);

         /* mark vertex k as scanned */
         state[k] = CONNECT;

         assert(vbase[k] >= 0);
         assert(g->mark[k] && g->mark[vbase[k]]);

         /* iterate over all outgoing edges of vertex k */
         for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
         {
            const int m = g->head[e];

            /* check whether the path (to m) including k is shorter than the so far best known */
            if( (state[m]) && (SCIPisGT(scip, path[m].dist, path[k].dist + cost[e])) )
            {
               assert(g->mark[m]);
               correct(heap, state, nheapelems, path, m, k, e, cost[e]);
               vbase[m] = vbase[k];
            }
            /* check whether there is a better new boundary edge adjacent to vertex k */
            else if( state[m] == CONNECT && g->mark[m] )
            {
               const int node1 = SCIPStpunionfindFind(uf, vbase[m]);
               const int node2 = SCIPStpunionfindFind(uf, vbase[k]);

               if( (node1 == crucnode) == (node2 == crucnode) )
                  continue;

               if( !g->mark[node1] || !g->mark[node2] || !g->mark[vbase[m]] )
                  continue;

               /* now to the actual checks */

               if( boundaryedge == UNKNOWN )
               {
                  boundaryedge = e;
               }
               else
               {
                  SCIP_Real dist_new = path[k].dist + path[m].dist;

                  if( isPcMw )
                  {
                     if( isPc )
                     {
                        assert(cost_org);
                        dist_new += cost_org[e];
                     }
                  }
                  else
                  {
                     assert(!cost_org);
                     dist_new += cost[e];
                  }

                  if( dist_new < boundarydist )
                  {
                     boundarydist = dist_new;
                     boundaryedge = e;
                  }
               }
            } /* check for boundary edge */
         }
      }
   }

   *newedge = boundaryedge;
}


/** repair the Voronoi diagram for a given set nodes */
void graph_voronoiRepairMult(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      cost,               /**< edge costs */
   const STP_Bool*       nodesmark,          /**< array to mark temporarily discarded nodes */
   int* RESTRICT         count,              /**< pointer to number of heap elements */
   int* RESTRICT         vbase,              /**< array containing Voronoi base of each node */
   int* RESTRICT         boundedges,         /**< boundary edges */
   int* RESTRICT         nboundedges,        /**< number of boundary edges */
   UF* RESTRICT          uf,                 /**< union find data structure */
   PATH* RESTRICT        path                /**< Voronoi paths data structure */
   )
{
   assert(scip && g && cost && nodesmark && count && vbase && boundedges && nboundedges && uf && path);
   assert(g->path_heap && g->path_state);

   if( g->knots > 1 )
   {
      int node1;
      int node2;
      int* const heap = g->path_heap;
      int* const state = g->path_state;

      /* until the heap is empty */
      while( (*count) > 0 )
      {
         /* get the next (i.e. a nearest) vertex from the heap */
         const int k = nearest(heap, state, count, path);

         /* mark vertex k as scanned */
         state[k] = CONNECT;

         /* iterate all outgoing edges of vertex k */
         for( int i = g->outbeg[k]; i != EAT_LAST; i = g->oeat[i] )
         {
            const int m = g->head[i];

            if( !(g->mark[m]) )
               continue;

            /* check whether the path (to m) including k is shorter than the so far best known */
            if( state[m] && (SCIPisGT(scip, path[m].dist, path[k].dist + cost[i])) )
            {
               correct(heap, state, count, path, m, k, i, cost[i]);
               vbase[m] = vbase[k];
            }
            /* check whether there is a new boundary edge adjacent to vertex k */
            else if( (state[m] == CONNECT) && ((node1 = SCIPStpunionfindFind(uf, vbase[m])) != (node2 = SCIPStpunionfindFind(uf, vbase[k])))
               && g->mark[node1] && g->mark[node2] && (nodesmark[node1] || nodesmark[node2]) )
            {
               boundedges[(*nboundedges)++] = i;
            }
         }
      }
   }
}


/** initializes VNOI structure */
SCIP_RETCODE graph_vnoiInit(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure to work on */

   SCIP_Bool             useBufferArrays,    /**< should buffer arrays be used? */
   VNOI**                vnoi                /**< the VNOI */
)
{
   VNOI* vnoi_d;
   const int nnodes = graph_get_nNodes(graph);

   assert(scip);

   SCIP_CALL( SCIPallocMemory(scip, vnoi) );

   vnoi_d = *vnoi;
   vnoi_d->nnodes = nnodes;
   vnoi_d->usingBufferArrays = useBufferArrays;

   if( useBufferArrays )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &(vnoi_d->nodes_dist), nnodes) );
      SCIP_CALL( SCIPallocBufferArray(scip, &(vnoi_d->nodes_predEdge), nnodes) );
      SCIP_CALL( SCIPallocBufferArray(scip, &(vnoi_d->nodes_base), nnodes) );
   }
   else
   {
      SCIP_CALL( SCIPallocMemoryArray(scip, &(vnoi_d->nodes_dist), nnodes) );
      SCIP_CALL( SCIPallocMemoryArray(scip, &(vnoi_d->nodes_predEdge), nnodes) );
      SCIP_CALL( SCIPallocMemoryArray(scip, &(vnoi_d->nodes_base), nnodes) );
   }

   return SCIP_OKAY;
}


/** computes Voronoi region on given graph */
SCIP_RETCODE graph_vnoiCompute(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure to work on */
   VNOI*                 vnoi                /**< the VNOI */
)
{
   DHEAP* dheap;
   const int nnodes = graph_get_nNodes(graph);
   assert(vnoi->nnodes == graph->knots);

   SCIP_CALL( graph_heap_create(scip, nnodes, NULL, NULL, &(dheap) ));
   vnoiInit(graph, dheap, vnoi);
   vnoiCompute(scip, graph, dheap, vnoi);

   graph_heap_free(scip, TRUE, TRUE, &dheap);

   return SCIP_OKAY;
}


/** computes implied Voronoi region on given graph  */
SCIP_RETCODE graph_vnoiComputeImplied(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          graph,              /**< graph data structure to work on */
   const SDPROFIT*       sdprofit,           /**< SD profit */
   VNOI*                 vnoi                /**< the VNOI */
)
{
   DHEAP* dheap;

   const int nnodes = graph_get_nNodes(graph);

   assert(vnoi->nnodes == graph->knots);

   SCIP_CALL( graph_heap_create(scip, nnodes, NULL, NULL, &(dheap) ));

   vnoiInit(graph, dheap, vnoi);
   vnoiComputeImplied(graph, sdprofit, dheap, vnoi);

   graph_heap_free(scip, TRUE, TRUE, &dheap);

   return SCIP_OKAY;
}


/** frees VNOI structure */
void graph_vnoiFree(
   SCIP*                 scip,               /**< SCIP */
   VNOI**                vnoi                /**< the VNOI */
)
{
   VNOI* vnoi_d;
   assert(scip && vnoi);

   vnoi_d = *vnoi;
   assert(vnoi_d);

   if( vnoi_d->usingBufferArrays )
   {
      SCIPfreeBufferArray(scip, &(vnoi_d->nodes_base));
      SCIPfreeBufferArray(scip, &(vnoi_d->nodes_predEdge));
      SCIPfreeBufferArray(scip, &(vnoi_d->nodes_dist));
   }
   else
   {
      SCIPfreeMemoryArray(scip, &(vnoi_d->nodes_base));
      SCIPfreeMemoryArray(scip, &(vnoi_d->nodes_predEdge));
      SCIPfreeMemoryArray(scip, &(vnoi_d->nodes_dist));
   }

   SCIPfreeMemory(scip, vnoi);
}
