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

/**@file   dualascent.c
 * @brief  Dual-ascent dual heuristic for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file includes dual-ascent for classic Steiner tree and some variants.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include "scip/cons_linear.h"
#include "scip/cons_logicor.h"
#include "scip/cons_setppc.h"
#include "dualascent.h"
#include "probdata_stp.h"
#include "graph.h"
#include "heur_ascendprune.h"
#include "scip/scip.h"
#include "scip/misc.h"



#define DEFAULT_DAMAXDEVIATION  0.25  /**< max deviation for dual ascent */
#define DA_MAXDEVIATION_LOWER   0.01  /**< lower bound for max deviation for dual ascent */
#define DA_MAXDEVIATION_UPPER   0.9   /**< upper bound for max deviation for dual ascent */
#define DA_EPS                  (5e-7)
#define DAPATHS_HITLIMIT_PCMW 20
#define DAPATHS_HITLIMIT      5

/* do depth-first search */
#define DFS


#ifdef BITFIELDSARRAY
#define ARRLENGTH 32
#define SetBit(Arr, pos)     ( Arr[(pos/ARRLENGTH)] |= (1 << (pos%ARRLENGTH)) )
#define CleanBit(Arr, pos)   ( Arr[(pos/ARRLENGTH)] &= ~(1 << (pos%ARRLENGTH)) )
#define BitTrue(Arr, pos)    ( Arr[(pos/ARRLENGTH)] & (1 << (pos%ARRLENGTH)) )
#endif

enum DACONS_Type { dacons_linear = 0, dacons_logicor = 1, dacons_setppc = 2 };


/** internal data for path based dual-ascent */
typedef struct dual_ascent_paths
{
   DIJK*                 dijklimited;        /**< Dijkstra data */
   int*                  startnodes;         /**< start nodes */
   int*                  nodes_hits;         /**< counts how often a node has been hit */
   SCIP_Bool*            nodes_abort;        /**< nodes to abort at */
   SCIP_Real             maxdist;            /**< current max distance */
   int                   nstartnodes;        /**< number of */
   int                   centernode;         /**< node for PC/MW */
   SCIP_Bool             isUnrootedPcMw;     /**< un-rooted PC or MW? */
} DAPATHS;


/**@name Local methods
 *
 * @{
 */

/** creates empty constraint */
static
SCIP_RETCODE daconsCreateEmpty(
   SCIP*                 scip,               /**< SCIP data structure */
   enum DACONS_Type      constype,           /**< constraint type to be used */
   SCIP_Bool             consUseInital,      /**< use dual-ascent cuts as initial constraints? */
   SCIP_CONS**           cons                /**< to be initialized */
   )
{
   // todo PcMw did not propagate before, keep it?

   if( constype == dacons_linear )
   {
      SCIP_CALL( SCIPcreateConsLinear(scip, cons, "da", 0, NULL, NULL, 1.0, SCIPinfinity(scip),
         consUseInital, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, FALSE) );
   }
   else if( constype == dacons_logicor )
   {
      SCIP_CALL( SCIPcreateConsLogicor(scip, cons, "da", 0, NULL,
         consUseInital, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, FALSE) );
   }
   else
   {
      SCIP_CALL( SCIPcreateConsSetcover(scip, cons, "da", 0, NULL,
         consUseInital, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, FALSE) );
   }

   return SCIP_OKAY;
}


/** gets parameter */
static
SCIP_RETCODE daconsGetParams(
   SCIP*                 scip,               /**< SCIP data structure */
   enum DACONS_Type*     constype,           /**< pointer: constraint type to be used (OUT) */
   SCIP_Bool*            consUseInital       /**< pointer: use dual-ascent cuts as initial constraints? (OUT) */
   )
{
   int type;
   SCIP_CALL( SCIPgetBoolParam(scip, "stp/usedacutsinitial", consUseInital) );
   SCIP_CALL( SCIPgetIntParam(scip, "stp/dacutstype", &type) );

   if( type == 0 )
   {
      *constype = dacons_linear;
   }
   else if( type == 1 )
   {
      *constype = dacons_logicor;
   }
   else
   {
      assert(type == 2);
      *constype = dacons_setppc;
   }

   return SCIP_OKAY;

}


/** sorts according to distance in solution */
static
SCIP_RETCODE dapathsSortStarts(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph */
   const int*            result,             /**< solution array */
   DAPATHS*              dapaths             /**< to be initialized */
   )
{
   SCIP_Real* RESTRICT starts_prio;
   SCIP_Real* RESTRICT nodes_dist;
   SCIP_Bool* RESTRICT nodes_visisted;
   int* RESTRICT nodes_startid;
   int* RESTRICT stackarr;
   int* const startnodes = dapaths->startnodes;
   const int nstartnodes = dapaths->nstartnodes;
   const int nnodes = graph_get_nNodes(graph);
   int stacksize;
   int startcount = 0;

   assert(result);
   assert(nstartnodes > 0);

   SCIP_CALL( SCIPallocBufferArray(scip, &starts_prio, nstartnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_dist, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_visisted, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_startid, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &stackarr, nnodes) );

   BMSclearMemoryArray(nodes_visisted, nnodes);
   nodes_visisted[graph->source] = TRUE;
   nodes_dist[graph->source] = 0.0;

#ifndef NDEBUG
   for( int i = 0; i < nstartnodes; i++ )
      starts_prio[i] = FARAWAY;
#endif

   for( int i = 0; i < nnodes; i++ )
   {
      if( Is_term(graph->term[i]) && i != graph->source )
      {
         nodes_startid[i] = startcount++;
      }
      else
      {
         nodes_startid[i] = -1;
      }
   }
   assert(startcount == nstartnodes);

   stacksize = 0;
   stackarr[stacksize++] = graph->source;

   /* DFS loop */
   while( stacksize != 0 )
   {
      const int node = stackarr[--stacksize];

      for( int a = graph->outbeg[node]; a != EAT_LAST; a = graph->oeat[a] )
      {
         if( result[a] == CONNECT )
         {
            const int head = graph->head[a];

            if( !nodes_visisted[head] )
            {
               nodes_dist[head] = nodes_dist[node] + graph->cost[a];
               nodes_visisted[head] = TRUE;
               stackarr[stacksize++] = head;

               if( Is_term(graph->term[head]) )
               {
                  const int start = nodes_startid[head];
                  assert(start >= 0 && start < nstartnodes);

                  starts_prio[start] = -nodes_dist[head];
               }
            }
         }
      }
   }

#ifndef NDEBUG
   for( int i = 0; i < nstartnodes; i++ )
   {
      assert(!EQ(starts_prio[i], FARAWAY));
   }
#endif

#ifdef SCIP_DEBUG
   for( int i = 0; i < nstartnodes; i++ )
   {
      graph_knot_printInfo(graph, startnodes[i]);
      printf("...%f \n", starts_prio[i]);
   }
#endif

   SCIPsortDownRealInt(starts_prio, startnodes, nstartnodes);

#ifdef SCIP_DEBUG
   printf("after \n");
   for( int i = 0; i < nstartnodes; i++ )
   {
      graph_knot_printInfo(graph, startnodes[i]);
      printf("...%f \n", starts_prio[i]);
   }
#endif

   SCIPfreeBufferArray(scip, &stackarr);
   SCIPfreeBufferArray(scip, &nodes_startid);
   SCIPfreeBufferArray(scip, &nodes_visisted);
   SCIPfreeBufferArray(scip, &nodes_dist);
   SCIPfreeBufferArray(scip, &starts_prio);

   return SCIP_OKAY;
}


/** sets shortest path parameters: start node and abort nodes */
static
void dapathsSetRunParams(
   const GRAPH*          graph,              /**< graph */
   DAPATHS*              dapaths             /**< to be initialized */
   )
{
   SCIP_Bool* RESTRICT nodes_abort = dapaths->nodes_abort;
   int* RESTRICT startnodes = dapaths->startnodes;
   const int nnodes = graph_get_nNodes(graph);
   const int root = graph->source;
   int centernode = UNKNOWN;
   int nstartnodes = 0;

   BMSclearMemoryArray(nodes_abort, nnodes);
   nodes_abort[root] = TRUE;

   for( int i = 0; i < nnodes; i++ )
   {
      if( !Is_term(graph->term[i]) )
         continue;

      if( i == root )
         continue;

      if( dapaths->isUnrootedPcMw )
      {
         assert(graph->grad[i] == 2);

         if( nstartnodes == 0 )
         {
            for( int e = graph->inpbeg[i]; e != EAT_LAST; e = graph->ieat[e] )
            {
               if( GT(graph->cost[e], 0.0) )
               {
                  centernode = graph->tail[e];
                  assert(graph->grad[centernode] == 2 * (graph->terms - 1));
                  nodes_abort[centernode] = TRUE;
               }
            }
         }
      }

      startnodes[nstartnodes++] = i;
   }

   assert(nstartnodes > 0);
   assert(centernode >= 0 || !dapaths->isUnrootedPcMw);

   dapaths->nstartnodes = nstartnodes;
   dapaths->maxdist = -FARAWAY;
   dapaths->centernode = centernode;
   assert(graph->outbeg[root] >= 0);
}


/** initializes */
static
SCIP_RETCODE dapathsInit(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph */
   DAPATHS*              dapaths             /**< to be initialized */
   )
{
   const int nnodes = graph_get_nNodes(graph);
   SCIP_CALL( graph_dijkLimited_init(scip, graph, &(dapaths->dijklimited)) );

   SCIP_CALL( SCIPallocBufferArray(scip, &(dapaths->nodes_hits), nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &(dapaths->nodes_abort), nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &(dapaths->startnodes), nnodes) );

   BMSclearMemoryArray(dapaths->nodes_hits, nnodes);

   dapathsSetRunParams(graph, dapaths);

   return SCIP_OKAY;
}


/** initializes reduced costs */
static
void dapathsInitRedCosts(
   const GRAPH*          graph,              /**< graph; possibly transformed SAP graph */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs */
   SCIP_Real*            objval              /**< pointer to store (dual) objective value */
)
{
   const int nedges = graph_get_nEdges(graph);

   BMScopyMemoryArray(redcost, graph->cost, nedges);
   *objval = 0.0;
}


/** updates reduced costs and hit count */
static
void dapathsUpdate(
   const GRAPH*          g,                  /**< graph */
   const DAPATHS*        dapaths,            /**< DA paths data */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs */
   SCIP_Real*            objval              /**< pointer to store (dual) objective value */
)
{
   const DIJK* dijklimited = dapaths->dijklimited;
   const SCIP_Real* const node_distance = dijklimited->node_distance;
   const int* const visitlist = dijklimited->visitlist;
   const SCIP_Real maxdist = dapaths->maxdist;
   const int nvisits = dijklimited->nvisits;
   int* RESTRICT nodes_hits = dapaths->nodes_hits;
   SCIP_Bool* RESTRICT nodes_abort = dapaths->nodes_abort;
   const int hitlimit = dapaths->isUnrootedPcMw ? DAPATHS_HITLIMIT_PCMW : DAPATHS_HITLIMIT;

   assert(GE(maxdist, 0.0));
   assert(GE(*objval, 0.0));
   assert(nvisits >= 0);

   // go over all visited...in and out separated.. todo maybe use visited edge list if slow
   for( int k = 0; k < nvisits; k++ )
   {
      const int node_i = visitlist[k];
      const SCIP_Real dist_i = MIN(node_distance[node_i], maxdist);

      assert(graph_knot_isInRange(g, node_i));

      if( nodes_hits[node_i] > hitlimit )
      {
         nodes_abort[node_i] = TRUE;
      }
      else
      {
         nodes_hits[node_i]++;
      }

      for( int e = g->outbeg[node_i]; e >= 0; e = g->oeat[e] )
      {
         const int node_j = g->head[e];
         const SCIP_Real dist_j = node_distance[node_j];

         if( LT(dist_j, maxdist) )
         {
            const SCIP_Real offset = MAX(0.0, dist_i - dist_j);
            assert(LE_FEAS_EPS(offset, redcost[e], EPSILON));

            redcost[e] -= offset;

            if( redcost[e] < 0.0 )
               redcost[e] = 0.0;
         }
      }
   }

   *objval += maxdist;
}


/** runs */
static
void dapathsRunShortestPaths(
   const GRAPH*          graph,              /**< graph; possibly transformed SAP graph */
   DAPATHS*              dapaths,            /**< to be initialized */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs */
   SCIP_Real*            objval              /**< objective */
   )
{
   const int* startnodes = dapaths->startnodes;
   SCIP_Bool* nodes_abort = dapaths->nodes_abort;
   DIJK* dijklimited = dapaths->dijklimited;
   const int nstarts = dapaths->nstartnodes;

   dapathsInitRedCosts(graph, redcost, objval);

   for( int i = 0; i < nstarts; i++ )
   {
      const int start = startnodes[i];
      assert(graph_knot_isInRange(graph, start) && Is_term(graph->term[start]));
      assert(nodes_abort[graph->source]);

      graph_pathInLimitedExec(graph, redcost, nodes_abort, start, dijklimited, &(dapaths->maxdist));
      dapathsUpdate(graph, dapaths, redcost, objval);

      graph_dijkLimited_reset(graph, dijklimited);
   }

   /* for PC/MW we make sure that we reach the root */
   if( dapaths->isUnrootedPcMw )
   {
      assert(nodes_abort[dapaths->centernode]);

      BMSclearMemoryArray(dapaths->nodes_abort, graph->knots);
      nodes_abort[graph->source] = TRUE;
      graph_pathInLimitedExec(graph, redcost, nodes_abort, dapaths->centernode, dijklimited, &(dapaths->maxdist));
      dapathsUpdate(graph, dapaths, redcost, objval);
   }
}


/** frees */
static
void dapathsFreeMembers(
   SCIP*                 scip,               /**< SCIP data structure */
   DAPATHS*              dapaths             /**< to be initialized */
   )
{
   SCIPfreeBufferArray(scip, &(dapaths->startnodes));
   SCIPfreeBufferArray(scip, &(dapaths->nodes_abort));
   SCIPfreeBufferArray(scip, &(dapaths->nodes_hits));
   graph_dijkLimited_free(scip, &(dapaths->dijklimited));
}


/** returns whether node realtail is active or leads to active node other than dfsbase */
static inline
SCIP_Bool daNodeIsActive(
   const int*            active,             /**< active nodes array */
   int                   realtail,           /**< vertex to start from */
   int                   dfsbase             /**< DFS source vertex */
   )
{
   int curr;

   for( curr = active[realtail]; curr != 0 && curr != dfsbase + 1; curr = active[curr - 1] )
   {
      assert(curr >= 0);
   }

   return (curr == 0);
}


/** initializes */
static
void daInitRescaps(
   const GRAPH*          g,                  /**< graph data structure */
   const int*            edgemap,            /**< CSR ancestor edge array */
   int                   ncsredges,          /**< number of CSR edges */
   SCIP_Real* RESTRICT   rescap,             /**< residual capacity */
   SCIP_Real*            dualobj             /**< dual objective */
)
{
   for( int i = 0; i < ncsredges; i++ )
      rescap[i] = g->cost[edgemap[i]];

   *dualobj = 0.0;
}


/** updates */
static
SCIP_RETCODE daUpdateRescaps(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   const int*            edgemap,            /**< CSR ancestor edge array */
   int                   ncsredges,          /**< number of CSR edges */
   SCIP_Real*            rescap              /**< residual capacity */
)
{
   const int nedges = graph_get_nEdges(g);
   SCIP_Real* rescaps_org;

   SCIP_CALL( SCIPallocBufferArray(scip, &rescaps_org, nedges) );
   BMScopyMemoryArray(rescaps_org, rescap, nedges);

   for( int i = 0; i < ncsredges; i++ )
   {
      const int edgeorg = edgemap[i];
      assert(graph_edge_isInRange(g, edgeorg));

      rescap[i] = rescaps_org[edgeorg];
   }

   SCIPfreeBufferArray(scip, &rescaps_org);

   return SCIP_OKAY;
}


/** initializes */
static
SCIP_RETCODE daInit(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph data structure */
   int                   root,               /**< the root */
   SCIP_Bool             is_pseudoroot,      /**< is the root a pseudo root? */
   int*                  gmark,              /**< array for marking nodes */
   int* RESTRICT         active,             /**< active vertices mark */
   SCIP_PQUEUE*          pqueue,             /**< priority queue */
   GNODE*                gnodearr,           /**< array containing terminal nodes*/
   int*                  augmentingcomponent /**< augmenting component */
)
{
   const int nnodes = g->knots;

   *augmentingcomponent = -1;

   /* mark terminals as active, add all except root to pqueue */
   for( int i = 0, termcount = 0; i < nnodes; i++ )
   {
      if( !Is_term(g->term[i]) )
      {
         active[i] = -1;
         continue;
      }

      active[i] = 0;
      assert(g->grad[i] > 0);

      if( i != root )
      {
         assert(termcount < g->terms - 1);
         assert(gnodearr);

         gnodearr[termcount].number = i;
         gnodearr[termcount].dist = g->grad[i];

         /* for variants with dummy terminals */
         if( g->grad[i] == 2 )
         {
            int a;

            for( a = g->inpbeg[i]; a != EAT_LAST; a = g->ieat[a] )
               if( SCIPisZero(scip, g->cost[a]) )
                  break;

            if( a != EAT_LAST )
            {
               const int tail = g->tail[a];
               gnodearr[termcount].dist += g->grad[tail] - 1;

               if( is_pseudoroot )
               {
                  for( a = g->inpbeg[tail]; a != EAT_LAST; a = g->ieat[a] )
                  {
                     if( SCIPisZero(scip, g->cost[a]) )
                     {
                        gnodearr[termcount].dist += g->grad[g->tail[a]] - 1;
                     }
                  }
               }
            }

            assert(gnodearr[termcount].dist > 0);
         }

         SCIP_CALL(SCIPpqueueInsert(pqueue, &(gnodearr[termcount])));

         if( *augmentingcomponent == -1 )
            *augmentingcomponent = i;

         termcount++;
      }
   }

   assert(*augmentingcomponent >= 0);

   for( int i = 0; i < nnodes + 1; i++ )
      gmark[i] = FALSE;

   return SCIP_OKAY;
}


/** dual ascent heuristic */
static
SCIP_RETCODE daExec(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const int*            result,             /**< solution array or NULL */
   const DAPARAMS*       daparams,           /**< parameter */
   SCIP_Bool             updateRescaps,      /**< update? */
   SCIP_Real* RESTRICT   rescap,             /**< residual capacities aka reduced costs */
   SCIP_Real*            objval              /**< pointer to store objective value */
)
{
   SCIP_CONSHDLR* conshdlr = NULL;
   SCIP_PQUEUE* pqueue;
   SCIP_VAR** vars;
   GNODE* gnodearr;
   int* RESTRICT edgearr;
   int* RESTRICT tailarr;
   int* RESTRICT start;
   int* RESTRICT stackarr;
   int* RESTRICT cutverts;
   int* RESTRICT unsatarcs;
   int* RESTRICT unsattails;
   int* RESTRICT gmark;
   int* RESTRICT active;
   SCIP_Real dualobj;
   SCIP_Real currscore;
   const SCIP_Real maxdeviation = (daparams->damaxdeviation > 0.0) ? daparams->damaxdeviation : DEFAULT_DAMAXDEVIATION;
   const int nnodes = g->knots;
   const int nterms = g->terms;
   const int nedges = g->edges;
   int ncsredges;
   int norgcutverts;
   int stacklength;
   int augmentingcomponent;
   const SCIP_Bool addconss = (SCIPgetStage(scip) < SCIP_STAGE_INITSOLVE);
   int root = daparams->root;
   const SCIP_Bool addcuts = daparams->addcuts;
   const SCIP_Bool is_pseudoroot = daparams->is_pseudoroot;
   const SCIP_Bool withInfinityArcs = graph_typeIsDirected(g) || g->stp_type == STP_DCSTP;
   SCIP_Bool consUseInital = TRUE;
   enum DACONS_Type constype = dacons_logicor;

   assert(rescap);
   assert(addconss || !addcuts);  /* should currently not  be activated */
   assert(maxdeviation >= DA_MAXDEVIATION_LOWER && maxdeviation <= DA_MAXDEVIATION_UPPER);
   assert(daparams->damaxdeviation == -1.0 || daparams->damaxdeviation > 0.0);

   /* if specified root is not a terminal, take default root */
   if( root < 0 || !Is_term(g->term[root]) )
      root = g->source;


   if( addcuts )
   {
      SCIP_CALL( daconsGetParams(scip, &constype, &consUseInital) );

      vars = SCIPprobdataGetVars(scip);
      assert(vars != NULL);

      if( !addconss )
      {
         conshdlr = SCIPfindConshdlr(scip, "stp");
         assert(conshdlr != NULL);
      }
   }
   else
   {
      vars = NULL;
   }

#ifdef BITFIELDSARRAY
   u_int32_t* bitarr;
   SCIP_CALL( SCIPallocBufferArray(scip, &bitarr, nedges / ARRLENGTH + 1) );
#endif

   stacklength = 0;

   if( nterms > 1 )
      SCIP_CALL( SCIPallocMemoryArray(scip, &gnodearr, nterms - 1) );
   else
      gnodearr = NULL;

   SCIP_CALL( SCIPpqueueCreate(&pqueue, nterms, 2.0, GNODECmpByDist, NULL) );

   SCIP_CALL( SCIPallocMemoryArray(scip, &active, nnodes) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &edgearr, nedges) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &tailarr, nedges) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &start, nnodes + 1) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &gmark, nnodes + 1) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &stackarr, nnodes) );

   /* fill auxiliary adjacent vertex/edges arrays */
   graph_getCsr(g, edgearr, tailarr, start, &ncsredges);

   if( updateRescaps )
   {
      dualobj = *objval;
      SCIP_CALL( daUpdateRescaps(scip, g, edgearr, ncsredges, rescap) );
   }
   else
   {
      daInitRescaps(g, edgearr, ncsredges, rescap, &dualobj);
   }

   SCIP_CALL( SCIPallocBufferArray(scip, &unsattails, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cutverts, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &unsatarcs, nedges) );

   SCIP_CALL( daInit(scip, g, root, is_pseudoroot, gmark, active, pqueue,
         gnodearr, &augmentingcomponent) );

   /* mark whether an arc is satisfied (has capacity 0) */
   for( int i = 0; i < ncsredges; i++ )
   {
#ifdef BITFIELDSARRAY
      if( SCIPisZero(scip, rescap[i]) )
         SetBit(bitarr, i);
      else
         CleanBit(bitarr, i);
#else
      if( EQ(rescap[i], 0.0) )
      {
         rescap[i] = 0.0;

         if( active[tailarr[i] - 1] == 0 )
            tailarr[i] = 0;
         else
            tailarr[i] *= -1;
      }
#endif
   }

   norgcutverts = 0;

   /* (main) dual ascent loop */
   while( SCIPpqueueNElems(pqueue) > 0 && !SCIPisStopped(scip) )
   {
      /* get active vertex of minimum score */
      GNODE* const gnodeact = (GNODE*) SCIPpqueueRemove(pqueue);
      const SCIP_Real prio1 = gnodeact->dist;
      const SCIP_Real prio2 = (SCIPpqueueNElems(pqueue) > 0) ? ((GNODE*) SCIPpqueueFirst(pqueue))->dist : FARAWAY;
      const int v = gnodeact->number;
      SCIP_Real degsum = g->grad[v];
      int ncutverts = 0;
      int nunsatarcs = 0;

      SCIP_Bool firstrun = TRUE;

      SCIPdebugMessage("DA: START WITH v %d prio1 %f prio2 %f \n", v, prio1, prio2);

      /* perform augmentation as long as priority of root component does not exceed max deviation */
      for( ; ; )
      {
         assert(stacklength == 0);

         /* 1. step: BFS from v (or connected component) on saturated, incoming arcs */

         if( firstrun )
         {
            firstrun = FALSE;
            gmark[v + 1] = TRUE;
            cutverts[ncutverts++] = v;
            assert(stacklength < nnodes);
            stackarr[stacklength++] = v;
         }
         /* not in first processing of root component: */
         else
         {
            for( int i = norgcutverts; i < ncutverts; i++ )
            {
               const int s = cutverts[i];

               assert(gmark[s + 1]);
               assert(active[s] != 0);
               assert(stacklength < nnodes);

               stackarr[stacklength++] = s;
            }
         }
#ifdef DFS
         while( stacklength )
         {
            const int node = stackarr[--stacklength];
#else
         for( int n = 0; n < stacklength; n++ )
         {
            int end;

            assert(n < nnodes);
            node = stackarr[n];
#endif

            /* traverse incoming arcs */
            for( int i = start[node], end = start[node + 1]; i != end; i++ )
            {
               int tail = tailarr[i];

               /* zero reduced-cost arc? */
               if( tail <= 0 )
               {
                  tail *= -1;
                  if( !gmark[tail] )
                  {
                     /* if an active vertex has been hit (other than v), break */
                     if( 0 == tail )
                     {
                        const int realtail = g->tail[edgearr[i]];

                        /* v should not be processed */
                        if( realtail == v )
                           continue;

                        /* is realtail active or does realtail lead to an active vertex other than v? */
                        if( daNodeIsActive(active, realtail, v) )
                        {
                           active[v] = realtail + 1;
                           stacklength = 0;
                           goto ENDOFLOOP;
                        }

                        tail = realtail + 1;

                        /* have we processed tail already? */
                        if( gmark[tail] )
                           continue;
                     }

                     assert(tail > 0);

                     gmark[tail] = TRUE;
                     tail--;
                     cutverts[ncutverts++] = tail;
                     degsum += g->grad[tail];

                     assert(stacklength < nnodes);
                     stackarr[stacklength++] = tail;
                  } /* marked */
               } /* zero reduced-cost arc */
               else if( !gmark[tail] )
               {
                  unsattails[nunsatarcs] = tail;
                  unsatarcs[nunsatarcs++] = i;
               }
            }
         }
#ifndef DFS
         stacklength = 0;
#endif
         currscore = degsum - (ncutverts - 1);

         /* guiding solution provided? */
         if( result != NULL )
         {
            int nsolarcs = 0;
            for( int i = 0; i < nunsatarcs; i++ )
            {
               const int a = unsatarcs[i];

               assert(tailarr[a] > 0);

               if( !(gmark[tailarr[a]]) )
               {
                  if( result[edgearr[a]] == CONNECT )
                     nsolarcs++;
               }
            }

            assert(nsolarcs > 0);
            assert(currscore <= nedges);

            if( nsolarcs > 1 )
              currscore += (SCIP_Real) ((nsolarcs - 1) * (g->knots * 2.0));
         }
         else
         {
            assert(SCIPisGE(scip, currscore, prio1));
         }

         SCIPdebugMessage("DA: deviation %f \n", (currscore - prio1) / prio1);
         SCIPdebugMessage("DA: currscore %f prio1 %f prio2 %f \n", currscore, prio1, prio2);

         /* augmentation criteria met? */
         if( ((currscore - prio1) / prio1) <= maxdeviation || currscore <= prio2 )
         {
            SCIP_CONS* cons = NULL;
            SCIP_ROW* row = NULL;

            int shift = 0;
            SCIP_Real min = FARAWAY;
            SCIP_Bool isactive = FALSE;

            /* 2. step: get minimum residual capacity among cut-arcs */

            /* adjust array of unsatisfied arcs */

            for( int i = 0; i < nunsatarcs; i++ )
            {
               const int tail = unsattails[i];

               if( gmark[tail] )
               {
                  shift++;
               }
               else
               {
                  const int a = unsatarcs[i];

                  assert(tailarr[a] > 0);
                  assert(rescap[a] > 0);

                  if( rescap[a] < min )
                     min = rescap[a];
                  if( shift )
                  {
                     unsattails[i - shift] = tail;
                     unsatarcs[i - shift] = a;
                  }
               }
            }

            assert(SCIPisLT(scip, min, FARAWAY));
            nunsatarcs -= shift;

            norgcutverts = ncutverts;

            /* 3. step: perform augmentation */

            /* create constraints/cuts ? */
            if( addcuts )
            {
               if( addconss )
               {
                  SCIP_CALL( daconsCreateEmpty(scip, constype, consUseInital, &cons) );
               }
               else
               {
                  SCIP_CALL( SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "da", 1.0,
                        SCIPinfinity(scip), FALSE, FALSE, TRUE) );

                  SCIP_CALL( SCIPcacheRowExtensions(scip, row) );
               }
            }

            shift = 0;

            /* update (dual) objective */
            dualobj += min;

            for( int i = 0; i < nunsatarcs; i++ )
            {
               const int a = unsatarcs[i];
               assert(a >= 0);

               if( addcuts )
               {
                  assert(vars != NULL);

                  if( !withInfinityArcs || LT(g->cost[edgearr[a]], FARAWAY) )
                  {
                     assert(addconss);
                     if( constype == dacons_linear )
                        SCIP_CALL( SCIPaddCoefLinear(scip, cons, vars[edgearr[a]], 1.0) );
                     else if( constype == dacons_logicor )
                        SCIP_CALL( SCIPaddCoefLogicor(scip, cons, vars[edgearr[a]]) );
                     else
                        SCIP_CALL( SCIPaddCoefSetppc(scip, cons, vars[edgearr[a]]) );

#ifdef SCIP_DISABLED
                     if( addconss )
                        SCIP_CALL( SCIPaddCoefLogicor(scip, cons, vars[edgearr[a]]) );//, 1.0) );
                     else
                        SCIP_CALL( SCIPaddVarToRow(scip, row, vars[edgearr[a]], 1.0) );
#endif
                  }
               }
               rescap[a] -= min;

               assert(SCIPisGE(scip, rescap[a], 0.0));

               if( rescap[a] <= DA_EPS )
               {
                  int tail = unsattails[i];

                  rescap[a] = 0.0;

                  assert(tail > 0);
                  assert(tailarr[a] > 0);

                  tailarr[a] *= -1;

                  if( active[tail - 1] >= 0 && daNodeIsActive(active, tail - 1, v) )
                  {
                     assert(tail - 1 != v);
                     tailarr[a] = 0;
                     if( !isactive )
                     {
                        isactive = TRUE;
                        active[v] = tail;
                     }
                  }


                  if( !(gmark[tail])  )
                  {
                     assert(tail != 0);

                     gmark[tail] = TRUE;
                     tail--;
                     degsum += g->grad[tail];
                     cutverts[ncutverts++] = tail;
                  }

                  shift++;
               }
               else if( shift )
               {
                  unsattails[i - shift] = unsattails[i];
                  unsatarcs[i - shift] = a;
               }
            }

            if( addcuts )
            {
               if( addconss )
               {
                  SCIP_CALL( SCIPaddCons(scip, cons) );
                  SCIP_CALL( SCIPreleaseCons(scip, &cons) );
               }
               else
               {
                  SCIP_Bool infeasible;

                  SCIP_CALL( SCIPflushRowExtensions(scip, row) );
                  SCIP_CALL( SCIPaddRow(scip, row, FALSE, &infeasible) );
                  SCIP_CALL( SCIPreleaseRow(scip, &row) );

                  assert(!infeasible);
               }
            }

            if( isactive )
            {
               stacklength = 0;
               goto ENDOFLOOP;
            }
            nunsatarcs -= shift;

            continue;
         }
         else
         {
            SCIP_Bool insert = TRUE;

            if( is_pseudoroot )
            {
               int i = start[v];
               const int end = start[v + 1];

               assert(end - i == 2);

               for( ; i != end; i++ )
                  if( rescap[i] != 0.0 )
                     break;

               if( i == end )
               {
                  if( augmentingcomponent == -1 )
                     augmentingcomponent = v;

                  if( augmentingcomponent != v )
                     insert = FALSE;
               }
            }

            if( insert )
            {
               /* reinsert active vertex */
               gnodeact->dist = currscore;
               SCIP_CALL( SCIPpqueueInsert(pqueue, gnodeact) );
            }
         }

         ENDOFLOOP:

         for( int i = 0; i < ncutverts; i++ )
            gmark[cutverts[i] + 1] = FALSE;

         for( int i = 0; i < nnodes + 1; i++ )
         {
            assert(!gmark[i]);
         }

         break;
      } /* augmentation loop */
   } /* dual ascent loop */

   SCIPdebugMessage("DA: dualglobal: %f \n", dualobj);
   *objval = dualobj;

   for( int i = ncsredges; i < nedges; i++ )
   {
      edgearr[i] = i;
      rescap[i] = g->cost[i];
   }

   /* re-extend rescap array */
   for( int i = 0; i < ncsredges; i++ )
   {
      if( edgearr[i] != i  )
      {
         SCIP_Real bufferedval = rescap[i];
         int a = i;

         rescap[i] = g->cost[i];
         while( edgearr[a] != a )
         {
            const int shift = edgearr[a];
            const SCIP_Real min = rescap[shift];

            rescap[shift] = bufferedval;
            bufferedval = min;
            edgearr[a] = a;
            a = shift;
         }
      }
   }

#ifdef BITFIELDSARRAY
   SCIPfreeBufferArray(scip, &bitarr);
#endif

   SCIPfreeMemoryArray(scip, &stackarr);
   SCIPfreeMemoryArray(scip, &gmark);
   SCIPfreeMemoryArray(scip, &start);
   SCIPfreeMemoryArray(scip, &tailarr);
   SCIPfreeMemoryArray(scip, &edgearr);
   SCIPfreeMemoryArray(scip, &active);

   SCIPpqueueFree(&pqueue);
   SCIPfreeMemoryArrayNull(scip, &gnodearr);

   /* call Ascend-And-Prune? */
   if( daparams->ascendandprune )
   {
       SCIP_Bool success;

       SCIP_CALL( SCIPStpHeurAscendPruneRun(scip, NULL, g, rescap, unsatarcs, root, &success, TRUE) );
   }

   SCIPfreeBufferArray(scip, &unsatarcs);
   SCIPfreeBufferArray(scip, &cutverts);
   SCIPfreeBufferArray(scip, &unsattails);

   assert(dualascent_allTermsReachable(scip, g, root, rescap));

   return SCIP_OKAY;
}


/**@} */

/**@name Interface methods
 *
 * @{
 */



/** dual ascent heuristic */
SCIP_RETCODE dualascent_exec(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const int*            result,             /**< solution array or NULL */
   const DAPARAMS*       daparams,           /**< parameter */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs or NULL */
   SCIP_Real*            objval              /**< pointer to store objective value */
)
{
   SCIP_Real* RESTRICT rescap;

   assert(scip && g && daparams && objval);

   if( g->knots == 1 )
   {
      *objval = 0.0;
      return SCIP_OKAY;
   }

   if( redcost == NULL )
      SCIP_CALL( SCIPallocBufferArray(scip, &rescap, g->edges) );
   else
      rescap = redcost;

   SCIP_CALL( daExec(scip, g, result, daparams, FALSE, rescap, objval) );

   if( redcost == NULL )
      SCIPfreeBufferArray(scip, &rescap);

   return SCIP_OKAY;
}


/** updates reduced costs with dual ascent heuristic */
SCIP_RETCODE dualascent_update(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const int*            result,             /**< solution array or NULL */
   const DAPARAMS*       daparams,           /**< parameter */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs */
   SCIP_Real*            objval              /**< pointer to store objective value */
)
{
   assert(scip && g && daparams && objval && redcost);
   assert(GE(*objval, 0.0));

   if( g->knots == 1 )
      return SCIP_OKAY;

   assert(!dualascent_allTermsReachable(scip, g, daparams->root, redcost));

   printf("dual bound before %f \n", *objval);

   SCIP_CALL( daExec(scip, g, result, daparams, TRUE, redcost, objval) );

   printf("dual bound after %f \n", *objval);

   assert(dualascent_allTermsReachable(scip, g, daparams->root, redcost));

   return SCIP_OKAY;
}



/** dual ascent heuristic for degree constrained problem */
SCIP_RETCODE dualascent_execDegCons(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< graph data structure */
   const int*            result,             /**< solution array or NULL */
   const DAPARAMS*       daparams,           /**< parameter */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs or NULL */
   SCIP_Real*            objval              /**< pointer to store objective value */
)
{
   SCIP_Real* orgcosts;
   SCIP_Real* RESTRICT rescap;
   const int nnodes = graph_get_nNodes(g);
   const int nedges = graph_get_nEdges(g);
   const int root = daparams->root;

   assert(scip && g && daparams && objval);
   assert(g->stp_type == STP_DCSTP);
   assert(g->maxdeg);

   if( nnodes == 1 )
   {
      *objval = 0.0;
      return SCIP_OKAY;
   }

   assert(root == g->source || !daparams->addcuts);
   assert(graph_knot_isInRange(g, root) && Is_term(g->term[root]));

   SCIP_CALL( SCIPallocBufferArray(scip, &orgcosts, nedges) );
   BMScopyMemoryArray(orgcosts, g->cost, nedges);

   for( int k = 0; k < nnodes; k++ )
   {
      if( g->maxdeg[k] != 1 )
         continue;

      if( !Is_term(g->term[k]) || k == root )
         continue;

      for( int e = g->outbeg[k]; e != EAT_LAST; e = g->oeat[e] )
      {
         g->cost[e] = FARAWAY;
      }
   }

   if( redcost == NULL )
      SCIP_CALL( SCIPallocBufferArray(scip, &rescap, nedges) );
   else
      rescap = redcost;

   SCIP_CALL( daExec(scip, g, result, daparams, FALSE, rescap, objval) );

   if( redcost == NULL )
      SCIPfreeBufferArray(scip, &rescap);

   BMScopyMemoryArray(g->cost, orgcosts, nedges);
   SCIPfreeBufferArray(scip, &orgcosts);

   return SCIP_OKAY;
}


/** dual ascent heuristic for PCSPG and MWCSP */
SCIP_RETCODE dualascent_execPcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   SCIP_Real*            redcost,            /**< array to store reduced costs or NULL */
   SCIP_Real*            objval,             /**< pointer to store objective value */
   SCIP_Bool             addcuts,            /**< should dual-ascent add Steiner cuts? */
   SCIP_Bool             ascendandprune,     /**< perform ascend-and-prune and add solution? */
   int                   nruns               /**< number of dual ascent runs */
   )
{
   SCIP_CONSHDLR* conshdlr = NULL;
   SCIP_PQUEUE* pqueue;
   SCIP_VAR** vars;
   GRAPH* transgraph;
   SCIP_Real min;
   SCIP_Real prio1;
   SCIP_Real offset;
   SCIP_Real dualobj;
   SCIP_Real currscore;
   SCIP_Real maxdeviation;
   SCIP_Real* rescap;
   GNODE* gnodeact;
   GNODE** gnodearr;
   int s;
   int i;
   int k;
   int v;
   int a;
   int tail;
   int pnode;
   int shift;
   int root;
   int nnodes;
   int nterms;
   int nedges;
   int degsum;
   int ncutverts;
   int pseudoroot;
   int nunsatarcs;
   int stacklength;
   int norgcutverts;
   int* cutverts;
   int* stackarr;
   STP_Bool* origedge;
   int* unsatarcs;
   STP_Bool firstrun;
   STP_Bool* sat;
   STP_Bool* active;
   const SCIP_Bool addconss = (SCIPgetStage(scip) < SCIP_STAGE_INITSOLVE);
   SCIP_Bool consUseInital = TRUE;
   enum DACONS_Type constype = dacons_logicor;

   /* should currently not  be activated */
   assert(addconss || !addcuts);

   assert(g != NULL);
   assert(scip != NULL);
   assert(nruns >= 0);
   assert(objval != NULL);

   if( g->knots == 1 )
   {
      *objval = 0.0;
      return SCIP_OKAY;
   }


   if( addcuts )
   {
      SCIP_CALL( daconsGetParams(scip, &constype, &consUseInital) );

      vars = SCIPprobdataGetVars(scip);
      assert(vars != NULL);
      if( !addconss )
      {
         conshdlr = SCIPfindConshdlr(scip, "stp");
         assert(conshdlr != NULL);
      }
   }
   else
   {
      vars = NULL;
   }

   root = g->source;
   degsum = 0;
   offset = 0.0;
   dualobj = 0.0;

   ncutverts = 0;
   norgcutverts = 0;
   maxdeviation = DEFAULT_DAMAXDEVIATION;

   SCIP_CALL( graph_transPcGetSap(scip, g, &transgraph, &offset) );

   nnodes = transgraph->knots;
   nedges = transgraph->edges;
   nterms = transgraph->terms;
   pseudoroot = nnodes - 1;

   if( redcost == NULL )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &rescap, nedges) );
   }
   else
   {
      rescap = redcost;
   }

   stacklength = 0;
   SCIP_CALL( SCIPallocBufferArray(scip, &stackarr, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &sat, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &active, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cutverts, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &gnodearr, nterms - 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &unsatarcs, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &origedge, nedges) );

   for( i = 0; i < nedges; i++ )
      if( !Is_term(transgraph->term[transgraph->tail[i]]) && transgraph->head[i] == pseudoroot )
         origedge[i] = FALSE;
      else if( transgraph->tail[i] == pseudoroot && !Is_term(transgraph->term[transgraph->head[i]])  )
         origedge[i] = FALSE;
      else
         origedge[i] = TRUE;

   for( i = 0; i < nterms - 1; i++ )
   {
      SCIP_CALL( SCIPallocBuffer(scip, &gnodearr[i]) ); /*lint !e866*/
   }

   SCIP_CALL( SCIPpqueueCreate( &pqueue, nnodes, 2.0, GNODECmpByDist, NULL) );

   k = 0;
   /* mark terminals as active, add all except root to pqueue */
   for( i = 0; i < nnodes; i++ )
   {
      if( Is_term(transgraph->term[i]) )
      {
         active[i] = TRUE;
         assert(transgraph->grad[i] > 0);
         if( i != root  )
         {
            gnodearr[k]->number = i;
            gnodearr[k]->dist = transgraph->grad[i];

            for( a = transgraph->inpbeg[i]; a != EAT_LAST; a = transgraph->ieat[a] )
               if( SCIPisEQ(scip, transgraph->cost[a], 0.0) )
                  break;

            if( a != EAT_LAST )
               gnodearr[k]->dist += transgraph->grad[transgraph->tail[a]] - 1;

            assert(gnodearr[k]->dist > 0);

            SCIP_CALL( SCIPpqueueInsert(pqueue, gnodearr[k++]) );
         }
      }
      else
      {
         active[i] = FALSE;
      }
      transgraph->mark[i] = FALSE;
   }

   for( i = 0; i < nedges; i++ )
   {
      rescap[i] = transgraph->cost[i];
      if( SCIPisZero(scip, rescap[i]) )
         sat[i] = TRUE;
      else
         sat[i] = FALSE;
   }

   /* dual ascent loop */
   while( SCIPpqueueNElems(pqueue) > 0 && !SCIPisStopped(scip) )
   {
      /* get active vertex of minimum score */
      gnodeact = (GNODE*) SCIPpqueueRemove(pqueue);

      v = gnodeact->number;
      prio1 = gnodeact->dist;

      firstrun = TRUE;
      nunsatarcs = 0;

      /* perform augmentation as long as ... */
      for( ; ; )
      {
         assert(stacklength == 0);
         /* 1. step: BFS from v (or connected component) on saturated, incoming arcs */

         if( firstrun )
         {
            degsum = transgraph->grad[v];
            ncutverts = 0;
            firstrun = FALSE;
            nunsatarcs = 0;
            transgraph->mark[v] = TRUE;
            cutverts[ncutverts++] = v;
            stackarr[stacklength++] = v;
         }
         /* not in first processing of root component: */
         else
         {
            for( i = norgcutverts; i < ncutverts; i++ )
            {
               s = cutverts[i];
               assert(transgraph->mark[s]);
               if( active[s] )
               {
                  active[v] = FALSE;
                  stacklength = 0;
                  goto ENDOFLOOP;
               }

               stackarr[stacklength++] = s;
            }
         }

         while( stacklength )
         {
            pnode = stackarr[--stacklength];

            /* traverse incoming arcs */
            for( a = transgraph->inpbeg[pnode]; a != EAT_LAST; a = transgraph->ieat[a] )
            {
               tail = transgraph->tail[a];
               if( sat[a] )
               {
                  if( !transgraph->mark[tail] )
                  {
                     /* if an active vertex has been hit, break */
                     if( active[tail] )
                     {
                        active[v] = FALSE;
                        stacklength = 0;
                        goto ENDOFLOOP;
                     }

                     degsum += transgraph->grad[tail];
                     transgraph->mark[tail] = TRUE;
                     cutverts[ncutverts++] = tail;
                     stackarr[stacklength++] = tail;
                  }
               }
               else if( !transgraph->mark[tail] )
               {
                  unsatarcs[nunsatarcs++] = a;
               }
            }
         }

         currscore = degsum - (ncutverts - 1);

         assert(SCIPisGE(scip, currscore, prio1));

         /* augmentation criteria met? */
         if( SCIPisLE(scip, (currscore - prio1) / prio1, maxdeviation) || (SCIPpqueueNElems(pqueue) == 0) )
         {
            SCIP_Bool in = FALSE;
            SCIP_ROW* row;
            SCIP_CONS* cons = NULL;

            /* 2. pass: get minimum residual capacity among cut-arcs */

            /* adjust array of unsatisfied arcs */
            min = FARAWAY;
            shift = 0;

            for( i = 0; i < nunsatarcs; i++ )
            {
               a = unsatarcs[i];
               if( transgraph->mark[transgraph->tail[a]] )
               {
                  shift++;
               }
               else
               {

                  assert(!sat[a]);
                  if( SCIPisLT(scip, rescap[a], min) )
                     min = rescap[a];
                  if( shift != 0 )
                     unsatarcs[i - shift] = a;
               }
            }

            assert(SCIPisLT(scip, min, FARAWAY));
            nunsatarcs -= shift;

            if( nunsatarcs > 0)
               assert(!transgraph->mark[transgraph->tail[unsatarcs[nunsatarcs-1]]]);

            norgcutverts = ncutverts;


            /* 3. pass: perform augmentation */


            /* create constraint/row */

            if( addcuts )
            {
               if( addconss )
               {
                  SCIP_CALL( daconsCreateEmpty(scip, constype, consUseInital, &cons) );
               }
               else
               {
                  SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "da", 1.0, SCIPinfinity(scip), FALSE, FALSE, TRUE));
                  SCIP_CALL(SCIPcacheRowExtensions(scip, row));
               }
            }

            dualobj += min;
            for( i = 0; i < nunsatarcs; i++ )
            {
               a = unsatarcs[i];
               if( a == -1 )
                  continue;

               if( addcuts && origedge[a] )
               {
                  assert(vars && cons);
                  assert(addconss);

                  if( g->tail[a] == root && g->head[a] == v )
                     in = TRUE;

                  if( constype == dacons_linear )
                     SCIP_CALL( SCIPaddCoefLinear(scip, cons, vars[a], 1.0) );
                  else if( constype == dacons_logicor )
                     SCIP_CALL( SCIPaddCoefLogicor(scip, cons, vars[a]) );
                  else
                     SCIP_CALL( SCIPaddCoefSetppc(scip, cons, vars[a]) );

#ifdef SCIP_DISABLED
                  if( addconss )
                     SCIP_CALL( SCIPaddCoefLinear(scip, cons, vars[a], 1.0) );
                  else
                     SCIP_CALL( SCIPaddVarToRow(scip, row, vars[a], 1.0) );
#endif
               }
               rescap[a] -= min;

               assert(SCIPisGE(scip, rescap[a], 0.0));

               if( SCIPisEQ(scip, rescap[a], 0.0) )
               {
                  sat[a] = TRUE;
                  if( !(transgraph->mark[transgraph->tail[a]]) )
                  {
                     tail = transgraph->tail[a];
                     degsum += transgraph->grad[tail];
                     transgraph->mark[tail] = TRUE;
                     cutverts[ncutverts++] = tail;
                  }
               }
            }

            if( addcuts )
            {
               assert(vars != NULL);

               if( !in )
               {
                  for( i = g->outbeg[root]; i != EAT_LAST; i = g->oeat[i] )
                  {
                     if( g->head[i] == v )
                     {
                        if( constype == dacons_linear )
                           SCIP_CALL( SCIPaddCoefLinear(scip, cons, vars[i], 1.0) );
                        else if( constype == dacons_logicor )
                           SCIP_CALL( SCIPaddCoefLogicor(scip, cons, vars[i]) );
                        else
                           SCIP_CALL( SCIPaddCoefSetppc(scip, cons, vars[i]) );

#ifdef SCIP_DISABLED
                        if( addconss )
                           SCIP_CALL( SCIPaddCoefLinear(scip, cons, vars[i], 1.0) );
                        else
                           SCIP_CALL( SCIPaddVarToRow(scip, row, vars[i], 1.0) );
#endif
                     }
                  }
               }

               if( addconss )
               {
                  SCIP_CALL( SCIPaddCons(scip, cons) );
                  SCIP_CALL( SCIPreleaseCons(scip, &cons) );
               }
               else
               {
                  SCIP_Bool infeasible;
                  assert(row != NULL);

                  SCIP_CALL( SCIPflushRowExtensions(scip, row) );
                  SCIP_CALL( SCIPaddRow(scip, row, FALSE, &infeasible) );
                  SCIP_CALL( SCIPreleaseRow(scip, &row) );

                  assert(!infeasible);
               }
            }

            continue;
         }
         else
         {
            /* reinsert active vertex */
            gnodeact->dist = currscore;
            SCIP_CALL( SCIPpqueueInsert(pqueue, gnodeact) );
         }

      ENDOFLOOP:

         for( i = 0; i < ncutverts; i++ )
            transgraph->mark[cutverts[i]] = FALSE;

         break;
      } /* augmentation loop */
   } /* dual ascent loop */


   *objval = dualobj + offset;
   SCIPdebugMessage("DA: dualglobal: %f \n", *objval + SCIPprobdataGetOffset(scip));

   /* call dual Ascend-And-Prune? */
   if( ascendandprune )
   {
      SCIP_Bool success;
      SCIP_CALL( SCIPStpHeurAscendPruneRun(scip, NULL, g, rescap, unsatarcs, -1, &success, TRUE));
   }

   /* free memory */
   SCIPpqueueFree(&pqueue);

   for( i = nterms - 2; i >= 0; i-- )
      SCIPfreeBuffer(scip, &gnodearr[i]);

   SCIPfreeBufferArray(scip, &origedge);
   SCIPfreeBufferArray(scip, &unsatarcs);
   SCIPfreeBufferArray(scip, &cutverts);
   SCIPfreeBufferArray(scip, &gnodearr);
   SCIPfreeBufferArray(scip, &active);
   SCIPfreeBufferArray(scip, &sat);
   SCIPfreeBufferArray(scip, &stackarr);

   assert(dualascent_allTermsReachable(scip, transgraph, root, rescap));

   if( redcost == NULL )
      SCIPfreeBufferArray(scip, &rescap);

   graph_free(scip, &transgraph, TRUE);

   return SCIP_OKAY;
}


/** path based dual ascent heuristic */
SCIP_RETCODE dualascent_pathsPcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          transgraph,         /**< transformed SAP graph */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs */
   SCIP_Real*            objval,             /**< pointer to store (dual) objective value */
   const int*            result              /**< solution array or NULL */
)
{
   DAPATHS dapaths = { NULL, NULL, NULL, NULL, -1.0, -1, -1, .isUnrootedPcMw = TRUE };

   assert(scip && transgraph && redcost && objval);

   SCIP_CALL( dapathsInit(scip, transgraph, &dapaths) );

   dapathsRunShortestPaths(transgraph, &dapaths, redcost, objval);

   dapathsFreeMembers(scip, &dapaths);

   return SCIP_OKAY;
}


/** path based dual ascent heuristic */
SCIP_RETCODE dualascent_paths(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph */
   SCIP_Real* RESTRICT   redcost,            /**< array to store reduced costs */
   SCIP_Real*            objval,             /**< pointer to store (dual) objective value */
   const int*            result              /**< solution array or NULL */
)
{
   DAPATHS dapaths = { NULL, NULL, NULL, NULL, -1.0, -1, -1, .isUnrootedPcMw = FALSE };

   assert(scip && graph && redcost && objval);
   assert(!graph_pc_isPcMw(graph) || graph_pc_isRootedPcMw(graph));

   if( graph_pc_isPcMw(graph) )
      graph_pc_2transcheck(scip, graph);

   SCIP_CALL( dapathsInit(scip, graph, &dapaths) );

   if( result )
   {
      SCIP_CALL( dapathsSortStarts(scip, graph, result, &dapaths) );
   }

   dapathsRunShortestPaths(graph, &dapaths, redcost, objval);

   dapathsFreeMembers(scip, &dapaths);

   return SCIP_OKAY;
}


/** can all terminal be reached via reduced costs from given root? */
SCIP_Bool dualascent_allTermsReachable(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< graph */
   int                   root,               /**< root for reduced costs */
   const SCIP_Real*      redcost             /**< reduced costs */
   )
{
   int* RESTRICT queue;
   STP_Bool* RESTRICT scanned;
   int qsize;
   const int nnodes = graph_get_nNodes(g);
   int termscount;

   assert(scip && redcost);
   assert(graph_knot_isInRange(g, root));
   assert(Is_term(g->term[root]));

   SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &queue, nnodes ) );
   SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &scanned, nnodes) );

   BMSclearMemoryArray(scanned, nnodes);

   termscount = 1;
   qsize = 0;
   scanned[root] = TRUE;
   queue[qsize++] = root;

   /* DFS */
   while( qsize > 0 )
   {
      const int k = queue[--qsize];
      scanned[k] = TRUE;

      for( int a = g->outbeg[k]; a != EAT_LAST; a = g->oeat[a] )
      {
         const int head = g->head[a];

         if( SCIPisZero(scip, redcost[a]) )
         {
            /* vertex not visited yet? */
            if( !scanned[head] )
            {
               scanned[head] = TRUE;
               queue[qsize++] = head;

               if( Is_term(g->term[head]) )
                  termscount++;
            }
         }
      }
   }

   SCIPfreeMemoryArray(scip, &scanned);
   SCIPfreeMemoryArray(scip, &queue);

   return (termscount == g->terms);
}



/**@} */
