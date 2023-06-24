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

/**@file   graph_pcbase.c
 * @brief  includes several methods for prize-collecting Steiner problem graphs
 * @author Daniel Rehfeldt
 *
 * This file contains several basic methods to process prize-collecting Steiner problem graphs and kinsmen
 * such as the maximum-weight connected subgraph problem.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

//#define SCIP_DEBUG
#include "scip/misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "solstp.h"
#include "portab.h"
#include "graph.h"



/*
 * local functions
 */


/** remove non-leaf terminals by edge weight shifting (call before graph transformation was performed)  */
static inline
void markNonLeafTerms_2trans(
   SCIP*                 scip,               /**< SCIP */
   GRAPH*                g                   /**< the graph */
)
{
   const int nnodes = g->knots;

   assert(!g->extended);

   for( int k = 0; k < nnodes; k++ )
   {
      if( !Is_term(g->term[k]) )
         continue;

      if( graph_pc_termIsNonLeafTerm(g, k) )
      {
         graph_knot_chg(g, k, STP_TERM_NONLEAF);
      }
   }
}


/** gets original edge costs, when in extended mode */
static
void setCostToOrgPc(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
)
{
   const int nedges = graph->edges;
   const SCIP_Real* const cost_org = graph->cost_org_pc;
   SCIP_Real* const RESTRICT edgecosts = graph->cost;

   assert(scip && edgecosts && cost_org);
   assert(graph->extended && graph_pc_isPcMw(graph));

   assert(graph_pc_transOrgAreConistent(scip, graph, TRUE));

   for( int e = 0; e < nedges; ++e )
   {
      assert(graph_edge_isBlocked(graph, e) == EQ(edgecosts[e], BLOCKED_MINOR) || EQ(edgecosts[e], BLOCKED));

      if( !EQ(edgecosts[e], BLOCKED_MINOR) && !EQ(edgecosts[e], BLOCKED) )
         edgecosts[e] = cost_org[e];
   }
}


/** gets original edge costs, when in extended mode and in presolving state */
static
void setCostToOrgPcPreState(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
)
{
   const int nedges = graph->edges;
   const SCIP_Real* const cost_org = graph->cost_org_pc;
   SCIP_Real* const RESTRICT edgecosts = graph->cost;

   assert(scip && edgecosts && cost_org);
   assert(graph->extended && graph_pc_isPcMw(graph));

   assert(graph_pc_transOrgAreConistent(scip, graph, TRUE));

#ifndef NDEBUG
   for( int e = 0; e < nedges; ++e )
   {
      assert(!graph_edge_isBlocked(graph, e));
   }
#endif
   BMScopyMemoryArray(edgecosts, cost_org, nedges);
}



/* deletes dummy terminal to given node and edge from pseudo-root (if existent).
 * Furthermore, makes i a non-terminal, if makeNonTerminal is set */
static
void termDeleteExtension(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int                   i,                  /**< index of the terminal */
   SCIP_Bool             makeNonTerminal     /**< make i a non-terminal?*/
   )
{
   int e;
   int dummyterm;
   const SCIP_Bool has_pseudoroot = !graph_pc_isRootedPcMw(g);

   assert(g && scip);
   assert(g->term2edge && g->prize);
   assert((!g->extended && Is_term(g->term[i])) || (g->extended && Is_pseudoTerm(g->term[i])));
   assert(!graph_pc_knotIsFixedTerm(g, i));
   assert(i != g->source);

   /* get edge from i to its artificial terminal */
   e = g->term2edge[i];
   assert(e >= 0);

   dummyterm = g->head[e];
   assert(dummyterm != g->source);
   assert(g->grad[dummyterm] == 2);

   /* delete edge and unmark artificial terminal */
   graph_knot_chg(g, dummyterm, STP_TERM_NONE);
   graph_edge_del(scip, g, e, TRUE);
   g->term2edge[dummyterm] = TERM2EDGE_NOTERM;

   /* delete remaining incident edge of artificial terminal */
   e = g->inpbeg[dummyterm];

   assert(e != EAT_LAST);
   assert(g->source == g->tail[e]);
   assert(SCIPisEQ(scip, g->prize[i], g->cost[e]));

#ifndef NDEBUG
   g->cost[e] = 0.0;
#endif

   graph_edge_del(scip, g, e, TRUE);

   assert(g->inpbeg[dummyterm] == EAT_LAST && g->grad[dummyterm] == 0);

   if( has_pseudoroot )
   {
      const int edgeRoot2i = graph_pc_getRoot2PtermEdge(g, i);

#ifndef NDEBUG
   g->cost[edgeRoot2i] = 0.0;
#endif
      assert(SCIPisEQ(scip, g->cost[edgeRoot2i], 0.0));
      graph_edge_del(scip, g, edgeRoot2i, TRUE);
   }

   if( makeNonTerminal )
   {
      graph_knot_chg(g, i, STP_TERM_NONE);
      g->term2edge[i] = TERM2EDGE_NOTERM;
      g->prize[i] = 0.0;
   }
}

/** is given terminal the last terminal? */
static inline
SCIP_Bool isLastTerm(
   GRAPH*                g,                  /**< the graph */
   int                   t                   /**< terminal */
)
{
   assert(graph_pc_isPcMw(g));
   assert(g && g->term2edge);
   assert(!g->extended);
   assert(Is_term(g->term[t]));
   assert(!graph_pc_knotIsFixedTerm(g, t) && !graph_pc_knotIsNonLeafTerm(g, t));

   if( !graph_pc_isRootedPcMw(g) && g->grad[g->source] <= 2 )
   {
      return TRUE;
   }

   return FALSE;
}



/** changes incident edges after prize of node was changed */
static inline
void mwKnotUpdateIncEdges(
   GRAPH*                g,                  /**< the graph */
   int                   node                /**< the node */
   )
{
   const SCIP_Real newprize = g->prize[node];
   const SCIP_Real incost = (newprize > 0.0) ? 0.0 : -newprize;

   assert(!graph_pc_knotIsDummyTerm(g, node));

   for( int e = g->inpbeg[node]; e >= 0; e = g->ieat[e] )
   {
      const int tail = g->tail[e];
      if( !graph_pc_knotIsDummyTerm(g, tail) )
      {
         g->cost[e] = incost;
      }
   }
}


/** contract an edge of rooted prize-collecting Steiner tree problem or maximum-weight connected subgraph problem
 *  such that this edge is incident to least one fixed terminal */
static
SCIP_RETCODE contractEdgeWithFixedEnd(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int*                  solnode,            /**< solution nodes or NULL */
   int                   t,                  /**< tail node to be contracted (surviving node) */
   int                   s,                  /**< head node to be contracted */
   int                   ets                 /**< edge from t to s */
)
{
   assert(ets >= 0);
   assert(g->tail[ets] == t && g->head[ets] == s);
   assert(graph_pc_isRootedPcMw(g));
   assert(!g->extended);
   assert(graph_pc_knotIsFixedTerm(g, s) || graph_pc_knotIsFixedTerm(g, t));

   SCIP_CALL(graph_fixed_moveNodePc(scip, s, g));
   SCIP_CALL(graph_fixed_addEdge(scip, ets, g));

   if( !graph_pc_knotIsFixedTerm(g, s) )
   {
      assert(graph_pc_knotIsFixedTerm(g, t));

      if( Is_term(g->term[s]) )
         graph_pc_termToNonTerm(scip, g, s);
   }
   else
   {
      if( !graph_pc_knotIsFixedTerm(g, t) )
      {
         assert(!graph_pc_isMw(g)); // currently does not work due to offset issue in graph_pc_knotTofixedTerm!
         assert(g->source != t);
         assert(SCIPisEQ(scip, g->prize[s], FARAWAY));

         graph_pc_knotToFixedTerm(scip, g, t, NULL);
      }

      graph_pc_fixedTermToNonTerm(scip, g, s);
   }

   /* contract s into t */
   SCIP_CALL(graph_knot_contract(scip, g, solnode, t, s));

   return SCIP_OKAY;
}



/** contract an edge of (rooted) prize-collecting Steiner tree problem or maximum-weight connected subgraph problem
 *  such that this edge is NOT incident to least one fixed terminal */
static
SCIP_RETCODE contractEdgeNoFixedEnd(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int*                  solnode,            /**< solution nodes or NULL */
   int                   t,                  /**< tail node to be contracted (surviving node) */
   int                   s,                  /**< head node to be contracted */
   int                   ets,                /**< edge from t to s */
   int                   term4offset         /**< terminal to add offset to */
)
{
   const SCIP_Bool isMw = graph_pc_isMw(g);

   assert(ets >= 0);
   assert(g->tail[ets] == t && g->head[ets] == s);
   assert(Is_term(g->term[term4offset]));
   assert(!graph_pc_knotIsFixedTerm(g, s) && !graph_pc_knotIsFixedTerm(g, t));

   SCIP_CALL( graph_pc_contractNodeAncestors(scip, g, t, s, ets) );

   /* are both end-points of the edge to be contracted terminals? */
   if( Is_term(g->term[t]) && Is_term(g->term[s]) )
   {
      const SCIP_Real prize_s = g->prize[s];

      graph_pc_termToNonTerm(scip, g, s);

      if( !graph_pc_knotIsFixedTerm(g, term4offset) )
         graph_pc_subtractPrize(scip, g, g->cost[ets] - prize_s, term4offset);
   }
   else
   {
      if( !graph_pc_knotIsFixedTerm(g, term4offset) )
      {
         if( isMw )
            graph_pc_subtractPrize(scip, g, -(g->prize[s]), term4offset);
         else
            graph_pc_subtractPrize(scip, g, g->cost[ets], term4offset);
      }

      if( Is_term(g->term[s]) )
         graph_pc_termToNonTerm(scip, g, s);
   }

   /* contract s into t */
   SCIP_CALL( graph_knot_contract(scip, g, solnode, t, s) );

   if( isMw )
   {
      if( Is_term(g->term[term4offset]) && LE(g->prize[term4offset], 0.0) )
         graph_pc_termToNonTerm(scip, g, t);

      return SCIP_OKAY;
   }

   if( Is_term(g->term[t]) )
   {
      if( graph_pc_evalTermIsNonLeaf(scip, g, t) || g->grad[t] == 0 )
      {
         if( g->term2edge[t] != TERM2EDGE_NONLEAFTERM )
         {
            assert(g->term2edge[t] >= 0);

            graph_pc_termToNonLeafTerm(scip, g, t, FALSE);
         }
      }
      else
      {
         assert(TERM2EDGE_NONLEAFTERM != g->term2edge[t] && "currently not supported");
         /* NOTE: this case could happen, but not with current contraction calls; would need to keep edges for extension in this case */
      }
   }
   else
   {
      assert(g->term2edge[t] == TERM2EDGE_NOTERM);
   }

   return SCIP_OKAY;
}




/** initializes biased data */
static
void getBiasedPc(
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_Real* RESTRICT   costbiased,         /**< biased costs */
   SCIP_Real* RESTRICT   prizebiased         /**< biased prizes */
)
{
   const int nnodes = graph_get_nNodes(graph);
   const SCIP_Bool isRpc = graph_pc_isRootedPcMw(graph);
   const int root = graph->source;

   assert(graph_pc_isPc(graph));

   BMScopyMemoryArray(costbiased, graph->cost, graph->edges);

   for( int k = 0; k < nnodes; k++ )
   {
      if( Is_pseudoTerm(graph->term[k]) && graph->grad[k] != 0 )
      {
         SCIP_Real mincost = FARAWAY;

         /* get the minimum cost among all incoming arcs */
         for( int e = graph->inpbeg[k]; e != EAT_LAST; e = graph->ieat[e] )
         {
            const int tail = graph->tail[e];

            if( !isRpc && tail == root )
               continue;

            if( graph->cost[e] < mincost )
            {
               assert(!Is_term(graph->term[tail]) || graph_pc_knotIsFixedTerm(graph, tail));
               assert(!graph_pc_knotIsDummyTerm(graph, tail));
               assert(tail != root || isRpc);

               mincost = graph->cost[e];
            }
         }

         mincost = MIN(mincost, graph->prize[k]);

         /** apply the minimum */
         for( int e = graph->inpbeg[k]; e != EAT_LAST; e = graph->ieat[e] )
         {
            const int tail = graph->tail[e];

            if( !isRpc && tail == root )
               continue;

            if( GE(graph->cost[e], FARAWAY) )
            {
               assert(Is_term(graph->term[tail]));
               continue;
            }

            assert(!Is_term(graph->term[tail]) || graph_pc_knotIsFixedTerm(graph, tail));
            assert(!graph_pc_knotIsDummyTerm(graph, tail));

            costbiased[e] -= mincost;
            assert(GE(costbiased[e], 0.0));
         }

         prizebiased[k] = graph->prize[k] - mincost;
         assert(GE(prizebiased[k], 0.0));
      }
      else
      {
         if( isRpc && graph_pc_knotIsFixedTerm(graph, k) )
            prizebiased[k] = graph->prize[k];
         else
            prizebiased[k] = 0.0;
      }
   }
}


/** initializes biased data */
static
void getBiasedMw(
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_Real* RESTRICT   costbiased,         /**< biased costs */
   SCIP_Real* RESTRICT   prizebiased         /**< biased prizes */
)
{
   const int nnodes = graph_get_nNodes(graph);
   const SCIP_Bool isRmw = graph_pc_isRootedPcMw(graph);
   const int root = graph->source;

   assert(graph_pc_isMw(graph));

   BMScopyMemoryArray(costbiased, graph->cost, graph->edges);

   for( int k = 0; k < nnodes; k++ )
   {
      if( Is_pseudoTerm(graph->term[k]) && graph->grad[k] != 0 )
      {
         SCIP_Real mincost = FARAWAY;

         /* get the minimum cost among all outgoing arcs */
         for( int e = graph->outbeg[k]; e != EAT_LAST; e = graph->oeat[e] )
         {
            const int head = graph->head[e];

            if( graph_pc_knotIsDummyTerm(graph, head) )
               continue;

            if( !isRmw && head == root )
               continue;

            if( graph->cost[e] < mincost )
            {
               assert(head != root || isRmw);
               mincost = graph->cost[e];
            }
         }

         mincost = MIN(mincost, graph->prize[k]);

         /** apply the minimum */
         for( int e = graph->inpbeg[k]; e != EAT_LAST; e = graph->ieat[e] )
         {
            const int tail = graph->tail[e];

            if( !isRmw && tail == root )
               continue;

            assert(EQ(costbiased[e], graph->cost[e]));

            if( GE(costbiased[e], FARAWAY) )
            {
               assert(Is_term(graph->term[tail]));
               continue;
            }

            assert(!graph_pc_knotIsDummyTerm(graph, tail));
            assert(GE(mincost, 0.0) && LT(mincost, FARAWAY));
            assert(EQ(costbiased[e], 0.0));

            costbiased[e] = -mincost;
         }

         prizebiased[k] = graph->prize[k] - mincost;
         assert(GE(prizebiased[k], 0.0));
      }
      else
      {
         if( isRmw && graph_pc_knotIsFixedTerm(graph, k) )
         {
            assert(EQ(graph->prize[k], FARAWAY));
            prizebiased[k] = FARAWAY;
         }
         else
            prizebiased[k] = 0.0;
      }
   }
}


/*
 * Interface methods
 */


#if 0
/** transforms an MWCSP to an SAP */
SCIP_RETCODE graph_MwcsToSap(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< the graph */
   SCIP_Real*            maxweights          /**< array containing the weight of each node */
   )
{
   int e;
   int i;
   int nnodes;
   int nterms = 0;

   assert(maxweights != NULL);
   assert(scip != NULL);
   assert(graph != NULL);
   assert(graph->cost != NULL);
   assert(graph->terms == 0);

   nnodes = graph->knots;

   /* count number of terminals, modify incoming edges for non-terminals */
   for( i = 0; i < nnodes; i++ )
   {
      if( SCIPisLT(scip, maxweights[i], 0.0) )
      {
         for( e = graph->inpbeg[i]; e != EAT_LAST; e = graph->ieat[e] )
         {
            graph->cost[e] -= maxweights[i];
         }
      }
      else
      {
         graph_knot_chg(graph, i, 0);
         nterms++;
      }
   }
   nterms = 0;
   for( i = 0; i < nnodes; i++ )
   {
      graph->prize[i] = maxweights[i];
      if( Is_term(graph->term[i]) )
      {
         assert(SCIPisGE(scip, maxweights[i], 0.0));
         nterms++;
      }
      else
      {
         assert(SCIPisLT(scip, maxweights[i], 0.0));
      }
   }
   assert(nterms == graph->terms);
   graph->stp_type = STP_MWCSP;

   SCIP_CALL( graph_PcToSap(scip, graph) );
   assert(graph->stp_type == STP_MWCSP);
   return SCIP_OKAY;
}


/** alters the graph for prize collecting problems */
SCIP_RETCODE graph_PcToSap(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
   )
{
   SCIP_Real* prize;
   int k;
   int root;
   int node;
   int nnodes;
   int nterms;
   int pseudoroot;

   assert(graph != NULL);
   assert(graph->prize != NULL);
   assert(graph->knots == graph->ksize);
   assert(graph->edges == graph->esize);

   prize = graph->prize;
   nnodes = graph->knots;
   nterms = graph->terms;
   graph->norgmodelknots = nnodes;
   graph->norgmodeledges = graph->edges;

   /* for each terminal, except for the root, one node and three edges (i.e. six arcs) are to be added */
   SCIP_CALL( graph_resize(scip, graph, (graph->ksize + graph->terms + 2), (graph->esize + graph->terms * 8) , -1) );

   /* create a new nodes */
   for( k = 0; k < nterms; ++k )
      graph_knot_add(graph, -1);

   /* new pseudo-root */
   pseudoroot = graph->knots;
   graph_knot_add(graph, -1);

   /* new root */
   root = graph->knots;
   graph_knot_add(graph, 0);

   nterms = 0;
   for( k = 0; k < nnodes; ++k )
   {
      /* is the kth node a terminal other than the root? */
      if( Is_term(graph->term[k]) )
      {
         /* the copied node */
         node = nnodes + nterms;
         nterms++;

         /* switch the terminal property, mark k */
         graph_knot_chg(graph, k, -2);
         graph_knot_chg(graph, node, 0);
         assert(SCIPisGE(scip, prize[k], 0.0));

         /* add one edge going from the root to the 'copied' terminal and one going from the former terminal to its copy */
         graph_edge_add(scip, graph, root, k, BLOCKED, FARAWAY);
         graph_edge_add(scip, graph, pseudoroot, node, prize[k], FARAWAY);
         graph_edge_add(scip, graph, k, node, 0.0, FARAWAY);
         graph_edge_add(scip, graph, k, pseudoroot, 0.0, FARAWAY);
      }
      else if( graph->stp_type != STP_MWCSP )
      {
         prize[k] = 0;
      }
   }
   graph->source = root;
   graph->extended = TRUE;
   assert((nterms + 1) == graph->terms);
   if( graph->stp_type != STP_MWCSP )
      graph->stp_type = STP_PCSPG;

   return SCIP_OKAY;
}
#endif


/** shift costs of non-leaf terminals (call right after transformation to extended has been performed)  */
void graph_pc_shiftNonLeafCosts(
   SCIP*                 scip,               /**< SCIP */
   GRAPH*                g                   /**< the graph */
)
{
   const int nnodes = g->knots;
   SCIP_Real* const cost = g->cost;

#ifndef NDEBUG
   assert(g->cost_org_pc);
   assert(graph_pc_isPc(g));
   assert(g->extended);

   for( int e = 0; e < g->edges; e++ )
   {
      if( g->oeat[e] == EAT_LAST )
         continue;

      assert(SCIPisEQ(scip, cost[e], cost[flipedge(e)]) || SCIPisGE(scip, cost[e], FARAWAY) || SCIPisGE(scip, cost[flipedge(e)], FARAWAY));
   }
#endif

   for( int k = 0; k < nnodes; k++ )
   {
      if( Is_nonleafTerm(g->term[k]) )
      {
         const SCIP_Real prize = g->prize[k];

         assert(SCIPisGE(scip, prize, 0.0));

         for( int e = g->inpbeg[k]; e != EAT_LAST; e = g->ieat[e] )
         {
            assert(SCIPisLT(scip, cost[e], FARAWAY));
            assert(SCIPisLT(scip, cost[flipedge(e)], FARAWAY));

            if( graph_edge_isBlocked(g, e) )
               continue;

            cost[e] -= prize;
            assert(SCIPisGE(scip, cost[e], 0.0));

            if( cost[e] < 0.0 )
               cost[e] = 0.0;
         }
      }
   }
}


/** initializes term2edge array */
SCIP_RETCODE graph_pc_initTerm2Edge(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int                   size                /**< the size */
   )
{
   assert(scip && g);
   assert(!g->term2edge);
   assert(size > 0);

   SCIP_CALL(SCIPallocMemoryArray(scip, &(g->term2edge), size));

   for( int i = 0; i < size; i++ )
      g->term2edge[i] = TERM2EDGE_NOTERM;

   return SCIP_OKAY;
}


/** allocates prizes array for PC and MW problems */
SCIP_RETCODE graph_pc_initPrizes(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int                   sizeprize           /**< size of prize array to allocate (or -1) */
   )
{
   assert(scip != NULL);
   assert(g != NULL);
   assert(NULL == g->prize);
   assert(sizeprize > 0);

   SCIP_CALL( SCIPallocMemoryArray(scip, &(g->prize), sizeprize) );

   for( int i = 0; i < sizeprize; i++ )
      g->prize[i] = -FARAWAY;

   return SCIP_OKAY;
}


// todo this method should be static once subgraph method is finished
/** allocates and initializes arrays for subgraph PC/MW */
SCIP_RETCODE graph_pc_initSubgraph(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                subgraph            /**< the subgraph */
   )
{
   int ksize;
   int esize;

   assert(scip && subgraph);
   assert(graph_pc_isPcMw(subgraph));

   ksize = subgraph->ksize;
   esize = subgraph->esize;

   assert(ksize > 0 && ksize >= subgraph->knots);
   assert(esize > 0 && esize >= subgraph->edges);

   SCIP_CALL( graph_pc_initPrizes(scip, subgraph, ksize) );
   SCIP_CALL( graph_pc_initTerm2Edge(scip, subgraph, ksize) );

   assert(!subgraph->cost_org_pc);

   if( graph_pc_isPc(subgraph) )
   {
      SCIP_CALL( SCIPallocMemoryArray(scip, &(subgraph->cost_org_pc), esize) );
   }

   return SCIP_OKAY;
}

// todo this method should be static once subgraph method is finished
/** allocates and initializes arrays for subgraph PC/MW */
SCIP_RETCODE graph_pc_finalizeSubgraph(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                subgraph            /**< the subgraph */
   )
{
   if( graph_pc_isPcMw(subgraph) )
   {
      assert(scip);
      assert(subgraph->term2edge && subgraph->prize);
      assert(subgraph->extended);
      assert(subgraph->source >= 0);
      assert(!graph_pc_isPcMw(subgraph) || Is_term(subgraph->term[subgraph->source]));
      assert((graph_pc_isPc(subgraph)) == (subgraph->cost_org_pc != NULL));
   }

   return SCIP_OKAY;
}


/** changes graph of PC and MW problems needed for presolving routines */
SCIP_RETCODE graph_pc_presolInit(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
   )
{
   int prev;
   const int root = g->source;
   const int nedges = g->edges;

   if( g->stp_type == STP_RPCSPG )
      return SCIP_OKAY;

   assert(scip != NULL && g != NULL);
   assert(g->rootedgeprevs == NULL);
   assert(nedges > 0 && g->grad[root] > 0);

   SCIP_CALL( SCIPallocMemoryArray(scip, &(g->rootedgeprevs), nedges) );

   for( int e = 0; e < nedges; e++ )
      g->rootedgeprevs[e] = -1;

   prev = g->outbeg[root];
   assert(prev != EAT_LAST);

   for( int e = g->oeat[prev]; e != EAT_LAST; e = g->oeat[e] )
   {
      g->rootedgeprevs[e] = prev;
      prev = e;
   }

   prev = g->inpbeg[root];
   assert(prev != EAT_LAST);

   for( int e = g->ieat[prev]; e != EAT_LAST; e = g->ieat[e] )
   {
      g->rootedgeprevs[e] = prev;
      prev = e;
   }

   return SCIP_OKAY;
}

/** changes graph of PC and MW problems needed after exiting presolving routines */
void graph_pc_presolExit(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
   )
{
   assert(scip != NULL && g != NULL);

   if( g->stp_type == STP_RPCSPG )
      return;

   assert(g->rootedgeprevs != NULL);

   SCIPfreeMemoryArray(scip, &(g->rootedgeprevs));
}

/** checks consistency of term2edge array  */
SCIP_Bool graph_pc_term2edgeIsConsistent(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g                   /**< the graph */
)
{
   const int root = g->source;
   const SCIP_Bool rooted = graph_pc_isRootedPcMw(g);
   const SCIP_Bool isExtended = g->extended;

   assert(scip && g && g->term2edge);
   assert(graph_pc_isPcMw(g));

   if( g->term2edge[root] != TERM2EDGE_FIXEDTERM )
   {
      SCIPdebugMessage("term2edge root consistency \n");
      return FALSE;
   }

   for( int i = 0; i < g->knots; i++ )
   {
      if( rooted && graph_pc_knotIsFixedTerm(g, i) && !SCIPisEQ(scip, g->prize[i], FARAWAY) )
      {
         SCIPdebugMessage("inconsistent prize for fixed terminal %d \n", i);
         return FALSE;
      }

      if( i == root )
         continue;

      if( !isExtended && Is_term(g->term[i]) && graph_pc_realDegree(g, i, graph_pc_knotIsFixedTerm(g, i)) )
      {
         const SCIP_Bool isNonLeaf = (g->term2edge[i] == TERM2EDGE_NONLEAFTERM);

         if( isNonLeaf )
         {
            const SCIP_Bool isNonLeaf_evaluate = graph_pc_evalTermIsNonLeaf(scip, g, i);
            if( !isNonLeaf_evaluate )
            {
               SCIPdebugMessage("term2edge consistency fail0 %d \n", i);
               graph_knot_printInfo(g, i);
               printf("isNonLeaf=%ud isNonLeaf_evaluate=%ud \n", isNonLeaf, isNonLeaf_evaluate);

               return FALSE;
            }
         }
      }

      if( Is_anyTerm(g->term[i])
            && !graph_pc_knotIsFixedTerm(g, i)
            && !graph_pc_knotIsNonLeafTerm(g, i)
            && g->term2edge[i] < 0 )
      {
         SCIPdebugMessage("term2edge consistency fail1 %d \n", i);
         return FALSE;
      }

      if( !Is_anyTerm(g->term[i]) && g->term2edge[i] != TERM2EDGE_NOTERM )
      {
         SCIPdebugMessage("term2edge consistency fail2 %d \n", i);
         return FALSE;
      }

      if( isExtended && Is_nonleafTerm(g->term[i]) && g->term2edge[i] != TERM2EDGE_NONLEAFTERM )
      {
         SCIPdebugMessage("term2edge consistency fail2b %d \n", i);
         return FALSE;
      }

      if( !isExtended && g->term2edge[i] == TERM2EDGE_NONLEAFTERM && !Is_term(g->term[i]) )
      {
         SCIPdebugMessage("term2edge consistency fail2c %d \n", i);
         return FALSE;
      }

      if( Is_pseudoTerm(g->term[i]) )
      {
         int k = -1;
         int e;

         for( e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
         {
            k = g->head[e];
            if( Is_term(g->term[k]) && !graph_pc_knotIsFixedTerm(g, k) )
               break;
         }
         assert(e != EAT_LAST);
         assert(k >= 0);

         if( g->term2edge[i] != e )
         {
            SCIPdebugMessage("term2edge consistency fail3 %d \n", i);
            return FALSE;
         }

         if( g->term2edge[k] != flipedge(e) )
         {
            SCIPdebugMessage("term2edge consistency fail4 %d \n", i);
            return FALSE;
         }
      }
   }
   return TRUE;
}


/** transformed problem consistent to original one? Call only for extended graph */
SCIP_Bool graph_pc_transOrgAreConistent(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< the graph */
   SCIP_Bool             verbose             /**< be verbose? */
   )
{
   const int nedges = graph->edges;
   const SCIP_Real* const cost = graph->cost;
   const SCIP_Real* const cost_org = graph->cost_org_pc;

   assert(graph->cost_org_pc && graph->prize);
   assert(graph->extended);
   assert(graph_pc_isPcMw(graph));

   for( int e = 0; e < nedges; e++ )
   {
      int head;

      if( graph->oeat[e] == EAT_FREE )
         continue;

      if( graph_edge_isBlocked(graph, e) )
         continue;

      head = graph->head[e];

      if( Is_nonleafTerm(graph->term[head]) )
      {
         const SCIP_Real prize = graph->prize[head];
         assert(SCIPisGE(scip, prize, 0.0));
         assert(SCIPisLT(scip, cost_org[e], FARAWAY));

         if( !SCIPisEQ(scip, cost_org[e], cost[e] + prize) )
         {
            if( verbose )
            {
               graph_edge_printInfo(graph, e);
               printf("cost_org=%f cost=%f prize=%f \n", cost_org[e], cost[e],
                     prize);
            }

            return FALSE;
         }
      }
      else
      {
         if( !SCIPisEQ(scip, cost_org[e], cost[e]) )
         {
            if( verbose )
            {
               graph_edge_printInfo(graph, e);
               printf("cost_org=%f cost=%f \n", cost_org[e], cost[e]);
            }

            return FALSE;
         }
      }
   }

   return TRUE;
}


/** change property of node to be a non-terminal; prize is not changed! */
void graph_pc_knotToNonTermProperty(
   GRAPH*                g,                  /**< the graph */
   int                   node                /**< node to be changed */
   )
{
   assert(g);
   assert(node >= 0 && node < g->knots);

   assert(graph_pc_isPcMw(g));
   assert(g->term2edge);

   g->term2edge[node] = TERM2EDGE_NOTERM;

   graph_knot_chg(g, node, STP_TERM_NONE);
}


/** change property of node to be a terminal; prize is changed, but no edges are deleted! */
void graph_pc_knotToFixedTermProperty(
   GRAPH*                g,                  /**< the graph */
   int                   node                /**< node to be changed */
   )
{
   assert(g);
   assert(node >= 0 && node < g->knots);
   assert(g->term2edge && g->prize);

   g->term2edge[node] = TERM2EDGE_FIXEDTERM;
   g->prize[node] = FARAWAY;

   graph_knot_chg(g, node, STP_TERM);
}


/** Makes a node a fixed terminal */
void graph_pc_knotToFixedTerm(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int                   node,               /**< node */
   SCIP_Real*            offset              /**< offset needed for RMW, or NULL */
   )
{
   const SCIP_Bool isMw = graph_pc_isMw(g);
   assert(scip);
   assert(g->prize && g->term2edge);
   assert(node != g->source);
   assert(!graph_pc_knotIsFixedTerm(g, node));
   assert(graph_pc_isRootedPcMw(g));

   if( Is_term(g->term[node]) )
   {
      assert(!g->extended);

      if( isMw || !graph_pc_termIsNonLeafTerm(g, node) )
         termDeleteExtension(scip, g, node, TRUE);
   }
   else if( Is_pseudoTerm(g->term[node]) )
   {
      assert(g->extended);

      termDeleteExtension(scip, g, node, TRUE);
   }

   graph_knot_chg(g, node, STP_TERM_NONE);
   g->term2edge[node] = TERM2EDGE_NOTERM;

   graph_pc_nonTermToFixedTerm(g, node, offset);
}


/** makes a non-terminal node a fixed terminal */
void graph_pc_nonTermToFixedTerm(
   GRAPH*                g,                  /**< graph data structure */
   int                   node,               /**< node */
   SCIP_Real*            offset              /**< offset needed for RMW, or NULL */
   )
{
   const SCIP_Bool isMw = graph_pc_isMw(g);
   assert(g->prize && g->term2edge);
   assert(!graph_pc_knotIsFixedTerm(g, node));
   assert(graph_pc_isRootedPcMw(g));
   assert(!Is_term(g->term[node]));
   assert(!Is_pseudoTerm(g->term[node]));
   assert(!graph_pc_knotIsDummyTerm(g, node));
   assert(g->term2edge[node] == TERM2EDGE_NOTERM);
   assert(g->term[node] == STP_TERM_NONE);

   if( isMw && g->grad[node] > 0 )
   {
      const SCIP_Real incost = g->cost[g->inpbeg[node]];

      if( offset != NULL && node != g->source  )
      {
         (*offset) += incost;
      }

      for( int e = g->inpbeg[node]; e != EAT_LAST; e = g->ieat[e] )
      {
         assert(EQ(g->cost[e], incost));

         g->cost[e] = 0.0;
      }
   }

   graph_knot_chg(g, node, STP_TERM);
   g->prize[node] = FARAWAY;
   g->term2edge[node] = TERM2EDGE_FIXEDTERM;

   assert(graph_pc_knotIsFixedTerm(g, node));
}


/** Makes a non-fixed terminal a non-terminal.
 *  Also sets the prize to 0.0 for (R)PC! */
void graph_pc_termToNonTerm(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int                   term                /**< terminal to be unfixed */
   )
{
   assert(graph_pc_isPcMw(g));
   assert(term != g->source);
   assert(!graph_pc_knotIsFixedTerm(g, term));
   assert(!g->extended);
   assert(Is_anyTerm(g->term[term]));

   if( graph_pc_isPc(g) )
   {
      if( !graph_pc_termIsNonLeafTerm(g, term) )
         termDeleteExtension(scip, g, term, FALSE);

      g->prize[term] = 0.0;
   }
   else
   {
      termDeleteExtension(scip, g, term, FALSE);
   }

   graph_pc_knotToNonTermProperty(g, term);

   assert(!Is_anyTerm(g->term[term]));
}


/** Makes a fixed terminal a non-terminal */
void graph_pc_fixedTermToNonTerm(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int                   term                /**< terminal to be unfixed */
   )
{
   assert(graph_pc_isPcMw(g));
   assert(term != g->source);
   assert(graph_pc_knotIsFixedTerm(g, term));

   graph_knot_chg(g, term, STP_TERM_NONE);
   g->term2edge[term] = TERM2EDGE_NOTERM;
   g->prize[term] = 0.0;

   assert(!graph_pc_knotIsFixedTerm(g, term));
   assert(!Is_anyTerm(g->term[term]));
}


/** change property of (non-fixed) terminal to be a non-leaf terminal
 *  NOTE: if force == FALSE, then nothing is done if term is the last terminal   */
void graph_pc_termToNonLeafTerm(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int                   term,               /**< terminal to be changed */
   SCIP_Bool             force               /**< force the transformation? Should usually be FALSE */
   )
{
   assert(graph_pc_isPcMw(g));
   assert(g && g->term2edge);
   assert(!g->extended);
   assert(Is_term(g->term[term]));
   assert(!graph_pc_knotIsFixedTerm(g, term) && !graph_pc_knotIsNonLeafTerm(g, term));

   if( force || !isLastTerm(g, term) )
   {
      termDeleteExtension(scip, g, term, FALSE);
      g->term2edge[term] = TERM2EDGE_NONLEAFTERM;
   }
}


/** is the edge part of the extended graph? */
SCIP_Bool graph_pc_edgeIsExtended(
   const GRAPH*          g,                  /**< the graph */
   int                   edge                /**< edge to be checked */
   )
{
   const int tail = g->tail[edge];
   const int head = g->head[edge];

   assert(graph_pc_isPcMw(g));
   assert(edge >= 0 && edge < g->edges);

   if( graph_pc_knotIsDummyTerm(g, tail) || graph_pc_knotIsDummyTerm(g, head) )
   {
      assert(EQ(g->cost[edge], FARAWAY) || EQ(g->cost[flipedge(edge)], FARAWAY));

      return TRUE;
   }

   assert(LT(g->cost[edge], FARAWAY) && LT(g->cost[flipedge(edge)], FARAWAY));

   return FALSE;
}


/** check whether node is fixed terminal */
SCIP_Bool graph_pc_knotIsFixedTerm(
   const GRAPH*          g,                  /**< the graph */
   int                   node                /**< node to be checked */
   )
{
   assert(g);
   assert(node >= 0 && node < g->knots);
   assert(g->term2edge);
   assert(g->prize);

#ifndef NDEBUG
   if( TERM2EDGE_FIXEDTERM == g->term2edge[node] )
   {
      assert(Is_term(g->term[node]));
      assert(node == g->source || g->prize[node] == FARAWAY);
   }

   if( (TERM2EDGE_FIXEDTERM == g->term2edge[node]) && node != g->source && graph_pc_isMw(g) )
   {
      for( int e = g->inpbeg[node]; e != EAT_LAST; e = g->ieat[e] )
      {
         assert(EQ(g->cost[e], 0.0));
      }
   }
#endif

   return (TERM2EDGE_FIXEDTERM == g->term2edge[node]);
}


/** check whether node is proper potential terminal */
SCIP_Bool graph_pc_knotIsPropPotTerm(
   const GRAPH*          g,                  /**< the graph */
   int                   node                /**< node to be checked */
   )
{
   assert(g);
   assert(graph_pc_isPcMw(g));
   assert(node >= 0 && node < g->knots);
   assert(g->term2edge && g->prize);

   if( g->extended )
   {
      return Is_pseudoTerm(g->term[node]);
   }

   if( g->term2edge[node] >= 0 && Is_term(g->term[node]) )
   {
      assert(!graph_pc_knotIsDummyTerm(g, node));

      return TRUE;
   }

   return FALSE;
}


/** is there no vertex of higher prize? */
SCIP_Bool graph_pc_knotHasMaxPrize(
   const GRAPH*          g,                  /**< graph data structure */
   int                   i                   /**< the node to be checked */
   )
{
   SCIP_Real maxprize = -1.0;
   const int nnodes = graph_get_nNodes(g);
   const SCIP_Real* const prize = g->prize;

   assert(i >= 0 && i < nnodes);
   assert(graph_pc_isPc(g));
   assert(!graph_pc_knotIsDummyTerm(g, i));

   if( g->stp_type == STP_RPCSPG )
   {
      return (graph_pc_knotIsFixedTerm(g, i));
   }

   for( int k = 0; k < nnodes; k++ )
   {
      if( prize[k] > maxprize && LT(prize[k], FARAWAY) )
         maxprize = prize[k];
   }

   return (EQ(g->prize[i], maxprize));
}


/** check whether node is a dummy (pseudo) terminal */
SCIP_Bool graph_pc_knotIsDummyTerm(
   const GRAPH*          g,                  /**< the graph */
   int                   node                /**< node to be checked */
   )
{
   assert(g      != NULL);
   assert(node   >= 0);
   assert(node   < g->knots);
   assert(graph_pc_isPcMw(g));
   assert(graph_pc_knotIsFixedTerm(g, g->source));

   if( node == g->source && !graph_pc_isRootedPcMw(g) )
      return TRUE;

   if( g->extended )
   {
      if( Is_term(g->term[node]) && !graph_pc_knotIsFixedTerm(g, node) )
      {
         assert(g->grad[node] == 2 );

         return TRUE;
      }
   }
   else
   {
      if( Is_pseudoTerm(g->term[node]) )
      {
         assert(g->grad[node] == 2 );
         assert(!graph_pc_knotIsFixedTerm(g, node));

         return TRUE;
      }
   }

   return FALSE;
}


/** check whether node is a terminal AND is not a leaf (or not contained) in at least one optimal tree */
SCIP_Bool graph_pc_knotIsNonLeafTerm(
   const GRAPH*          g,                  /**< the graph */
   int                   node                /**< node to be checked */
   )
{
   assert(g);
   assert(node >= 0 && node < g->knots);

   if( !Is_anyTerm(g->term[node]) )
      return FALSE;

   return graph_pc_termIsNonLeafTerm(g, node);
}


/** changes prize of a node */
void graph_pc_knotChgPrize(
   GRAPH*                g,                  /**< the graph */
   SCIP_Real             newprize,           /**< new prize */
   int                   node                /**< the node */
   )
{
   const SCIP_Bool isMw = (g->stp_type == STP_MWCSP || g->stp_type == STP_RMWCSP);
   assert(graph_pc_isPcMw(g));
   assert(isMw || Is_term(g->term[node]));

   if( Is_term(g->term[node]) )
   {
      const int twin = graph_pc_getTwinTerm(g, node);
      const int root2edge = graph_pc_getRoot2PtermEdge(g, twin);

      assert(GE(newprize, 0.0));
      assert(graph_pc_knotIsPropPotTerm(g, node));
      assert(EQ(g->prize[node], g->cost[root2edge]));

      g->cost[root2edge] = newprize;
   }

   g->prize[node] = newprize;

   if( isMw )
   {
      mwKnotUpdateIncEdges(g, node);
   }
}


/** check whether terminal is not a leaf (or not contained) in at least one optimal tree */
SCIP_Bool graph_pc_termIsNonLeafTerm(
   const GRAPH*          g,                  /**< the graph */
   int                   term                /**< terminal to be checked */
   )
{
   SCIP_Bool isNonLeafTerm = FALSE;

   assert(g && g->term2edge);
   assert(term >= 0 && term < g->knots);
   assert(Is_anyTerm(g->term[term]));

   if( graph_pc_knotIsFixedTerm(g, term) )
   {
      isNonLeafTerm = FALSE;
   }
   else if( g->extended )
   {
      isNonLeafTerm = Is_nonleafTerm(g->term[term]);
   }
   else
   {
      /* original graph: */
      assert(Is_term(g->term[term]) || Is_pseudoTerm(g->term[term]));

      isNonLeafTerm = (g->term2edge[term] == TERM2EDGE_NONLEAFTERM);

      assert(!(Is_pseudoTerm(g->term[term]) && isNonLeafTerm));
   }

   assert(!isNonLeafTerm || g->term2edge[term] == TERM2EDGE_NONLEAFTERM);

   return isNonLeafTerm;
}


/** is terminal a non-leaf? Checked by evaluation of the current graph */
SCIP_Bool graph_pc_evalTermIsNonLeaf(
   SCIP*                 scip,               /**< SCIP */
   const GRAPH*          g,                  /**< the graph */
   int                   term                /**< terminal to be checked */
   )
{
   SCIP_Bool isNonLeaf = TRUE;

   assert(graph_pc_isPcMw(g));
   assert(!g->extended);
   assert(Is_term(g->term[term]));
   assert(!graph_pc_knotIsFixedTerm(g, term));

   if( graph_pc_realDegree(g, term, FALSE) == 0 )
      return FALSE;

   for( int e = g->inpbeg[term]; e != EAT_LAST; e = g->ieat[e] )
   {
      const int neighbor = g->tail[e];

      if( !graph_pc_knotIsDummyTerm(g, neighbor) && SCIPisLT(scip, g->cost[e], g->prize[term]) )
      {
         assert(neighbor != g->source || graph_pc_isRootedPcMw(g));
         isNonLeaf = FALSE;
         break;
      }
   }

   return isNonLeaf;
}


/** check whether terminal is not a leaf in at least one optimal tree */
void graph_pc_termMarkProper(
   const GRAPH*          g,                  /**< the graph */
   int*                  termmark            /**< terminal mark (2 for proper terminal, 1 for non-proper terminal, 0 otherwise) */
)
{
   const int nnodes = g->knots;

   assert(!g->extended);

   assert(g && termmark);

   for( int i = 0; i < nnodes; i++ )
   {
      if( Is_term(g->term[i]) )
      {
         if( graph_pc_termIsNonLeafTerm(g, i) )
            termmark[i] = 1;
         else
            termmark[i] = 2;
      }
      else
      {
         termmark[i] = 0;
      }
   }
}


/** Enforces given pseudo-terminal without deleting edges.
 *  I.e. the terminal is part of any optimal solution. */
void graph_pc_enforcePseudoTerm(
   SCIP*           scip,               /**< SCIP data */
   GRAPH*          graph,              /**< graph */
   int             pterm               /**< the pseudo-terminal */
)
{
   const int dummyterm = graph_pc_getTwinTerm(graph, pterm);
   const int root2term = graph_pc_getRoot2PtermEdge(graph, dummyterm);

   assert(scip && graph);
   assert(!graph_pc_knotIsDummyTerm(graph, pterm));
   assert(graph_pc_knotIsDummyTerm(graph, dummyterm));
   assert(graph_pc_knotIsPropPotTerm(graph, pterm));
   assert(SCIPisEQ(scip, graph->cost[root2term], graph->prize[pterm]));

   /* don't change because of weird prize sum in reduce.c */
   if( SCIPisLT(scip, graph->prize[pterm], BLOCKED_MINOR) )
   {
      graph->prize[pterm] = BLOCKED_MINOR;
      graph->cost[root2term] = BLOCKED_MINOR;
   }
}


/** Enforces non-leaf terminal without deleting edges.
 *  I.e. the terminal is part of any optimal solution. */
void graph_pc_enforceNonLeafTerm(
   SCIP*           scip,               /**< SCIP data */
   GRAPH*          graph,              /**< graph */
   int             nonleafterm         /**< the terminal */
)
{
   assert(scip && graph);
   assert(graph_pc_isPc(graph) && graph->extended);
   assert(graph_pc_knotIsNonLeafTerm(graph, nonleafterm));

   if( graph_pc_isRootedPcMw(graph) )
   {
      assert(!graph_pc_isMw(graph));

      /* make it a proper fixed terminal */
      graph_pc_knotToFixedTermProperty(graph, nonleafterm);

      if( graph_pc_isPc(graph) )
      {
         assert(graph->cost_org_pc);

         for( int e = graph->inpbeg[nonleafterm]; e != EAT_LAST; e = graph->ieat[e] )
            graph->cost[e] = graph->cost_org_pc[e];
      }
   }
   else if( SCIPisLT(scip, graph->prize[nonleafterm], BLOCKED) )
   {
      /* don't change because of weird prize sum in reduce_base.c */
      // graph->prize[nonleafterm] = BLOCKED_MINOR; // todo quite hacky, because it destroys the invariant of non-leaf terms!
   }
}

/** is non-leaf term enforced? */
SCIP_Bool graph_pc_nonLeafTermIsEnforced(
   SCIP*           scip,               /**< SCIP data */
   const GRAPH*    graph,              /**< graph */
   int             nonleafterm         /**< the terminal */
)
{
   assert(scip && graph);
   assert(graph_pc_isPcMw(graph) && graph->extended);
   assert(graph_pc_knotIsNonLeafTerm(graph, nonleafterm));

   return SCIPisEQ(scip, graph->prize[nonleafterm], BLOCKED_MINOR);
}


/** Tries to enforce node without deleting or adding edges.
 *  I.e. the terminal is part of any optimal solution.
 *  Is not always possible! */
void graph_pc_enforceNode(
   SCIP*           scip,               /**< SCIP data */
   GRAPH*          graph,              /**< graph */
   int             term,               /**< the terminal */
   SCIP_Real*      offset              /**< pointer to the offset, for RMW only */
)
{
   assert(graph_pc_isPcMw(graph));

   if( graph->extended )
   {
      /* nothing to enforce? */
      if( Is_term(graph->term[term]) )
         return;

      if( Is_pseudoTerm(graph->term[term]) )
      {
         graph_pc_enforcePseudoTerm(scip, graph, term);
      }
      else if( graph_pc_isRootedPcMw(graph) && !graph_pc_knotIsNonLeafTerm(graph, term) )
      {
         graph_pc_nonTermToFixedTerm(graph, term, offset);
      }
   }
   else
   {
      if( graph_pc_knotIsFixedTerm(graph, term) )
         return;

      if( !graph_pc_isMw(graph) && graph_pc_knotIsNonLeafTerm(graph, term) )
      {
         if( graph_pc_isRootedPcMw(graph) )
            graph_pc_knotToFixedTermProperty(graph, term);

         return;
      }

      if( Is_term(graph->term[term]) )
      {
         assert(graph_pc_knotIsPropPotTerm(graph, term));

         graph_pc_enforcePseudoTerm(scip, graph, term);
      }
      else if( graph_pc_isRootedPcMw(graph) )
      {
         assert(!graph_pc_knotIsDummyTerm(graph, term));

         graph_pc_nonTermToFixedTerm(graph, term, offset);
      }
   }
}


/** Updates prize-collecting data for an edge added to subgraph of given graph 'orggraph'.
 *  Needs to be called right before corresponding edge is added */
void graph_pc_updateSubgraphEdge(
   const GRAPH*          orggraph,           /**< original graph */
   const int*            nodemapOrg2sub,     /**< node mapping from original to subgraph */
   int                   orgedge,            /**< original edge */
   GRAPH*                subgraph            /**< the subgraph */
)
{
   const int orgtail = orggraph->tail[orgedge];
   const int orghead = orggraph->head[orgedge];
   const int newtail = nodemapOrg2sub[orgtail];
   const int newhead = nodemapOrg2sub[orghead];

   assert(subgraph);
   assert(subgraph->term2edge);
   assert(orggraph->term2edge);
   assert(newtail >= 0);
   assert(newhead >= 0);
   assert(orggraph->extended);
   assert(subgraph->extended);

   if( orggraph->term2edge[orgtail] >= 0 && orggraph->term2edge[orghead] >= 0 && orggraph->term[orgtail] != orggraph->term[orghead] )
   {
      assert(Is_anyTerm(subgraph->term[newtail]) && Is_anyTerm(subgraph->term[newhead]));
      assert(Is_anyTerm(orggraph->term[orgtail]) && Is_anyTerm(orggraph->term[orghead]));
      assert(orggraph->source != orgtail && orggraph->source != orghead);
      assert(flipedge(subgraph->edges) == subgraph->edges + 1);

      subgraph->term2edge[newtail] = subgraph->edges;
      subgraph->term2edge[newhead] = subgraph->edges + 1;
   }

#ifndef NDEBUG
   if( TERM2EDGE_NOTERM == orggraph->term2edge[orgtail] )
      assert(TERM2EDGE_NOTERM == subgraph->term2edge[newtail]);

   if( TERM2EDGE_NOTERM == orggraph->term2edge[orghead] )
      assert(TERM2EDGE_NOTERM == subgraph->term2edge[newhead]);
#endif

   /* now save the terminal states if there are any */

   if( orggraph->term2edge[orgtail] < 0 )
      subgraph->term2edge[newtail] = orggraph->term2edge[orgtail];

   if( orggraph->term2edge[orghead] < 0 )
      subgraph->term2edge[newhead] = orggraph->term2edge[orghead];

   assert(subgraph->edges + 1 < subgraph->esize);
   assert(subgraph->edges + 1 < orggraph->edges);

   /* now save the original costs */
   if( graph_pc_isPc(orggraph) )
   {
      assert(subgraph->cost_org_pc);
      subgraph->cost_org_pc[subgraph->edges] = orggraph->cost_org_pc[orgedge];
      subgraph->cost_org_pc[subgraph->edges + 1] = orggraph->cost_org_pc[flipedge(orgedge)];
   }
}


/** gets number of undeleted, original edges (not going to dummy terminals) */
int graph_pc_getNorgEdges(
   const GRAPH*          graph               /**< the graph */
)
{
   int norgedges = 0;
   const int nedges = graph_get_nEdges(graph);

   for( int e = 0; e < nedges; ++e )
   {
      const int tail = graph->tail[e];
      const int head = graph->head[e];

      /* edge deleted? */
      if( graph->oeat[e] == EAT_FREE )
         continue;

      if( !graph_pc_knotIsDummyTerm(graph, tail) && !graph_pc_knotIsDummyTerm(graph, head) )
      {
         assert(LT(graph->cost[e], FARAWAY));

         norgedges++;
      }
   }

   return norgedges;
}


/** gets ratio of remaining nodes/edges */
void graph_pc_getReductionRatios(
  const GRAPH*          graph,              /**< graph */
  SCIP_Real*	         ratio_nodes,        /**< nodes ratio */
  SCIP_Real*	         ratio_edges         /**< edges ratio */
)
{
   int nnodes_real = 0;
   int nedges_real = 0;
   const SCIP_Bool isRooted = graph_pc_isRootedPcMw(graph);
   const int nnodes = graph->knots;
   const int norgmodelknots = graph->norgmodelknots;
   const int norgmodeledges = graph->norgmodeledges;
   const int pseudoroot = (isRooted ? -1 : graph->source);

   assert(ratio_nodes && ratio_edges);
   assert(graph_pc_isPcMw(graph));
   assert(graph->extended);

   /* todo somewhat hacky...but otherwise there is a problem if the graph vanished during presolving, because term2edge
    * array does not exist */
   if( graph->terms == 1 )
   {
      assert(graph->edges == 0);

      *ratio_nodes = 0.0;
      *ratio_edges = 0.0;
      return;
   }

   for( int i = 0; i < nnodes; i++ )
   {
	   if( pseudoroot == i )
		  continue;

	   if( graph_pc_knotIsDummyTerm(graph, i) )
		  continue;

	   if( graph->grad[i] == 0 )
		  continue;

	   assert(!Is_term(graph->term[i]) || isRooted);

	   nnodes_real++;
	   nedges_real += graph->grad[i];

	   if( Is_pseudoTerm(graph->term[i]) )
	   {
		  if( isRooted )
		     nedges_real -= 1;
		  else
			 nedges_real -= 2;
	   }

	   if( isRooted && i == graph->source )
	   {
	      nedges_real -= graph_pc_nNonFixedTerms(graph);
	   }
   }

   assert(1 <= nnodes_real && nnodes_real <= norgmodelknots);
   assert(nedges_real <= norgmodeledges);
   assert(2 <= norgmodeledges);
   assert(norgmodeledges % 2 == 0);
   assert(nedges_real % 2 == 0);

#ifdef SCIP_DEBUG
   printf("norgmodelknots=%d \n", norgmodelknots);
   printf("norgmodeledges=%d \n", norgmodeledges);
   printf("nnodes_real=%d \n", nnodes_real);
   printf("nedges_real=%d \n", nedges_real);
#endif

   *ratio_nodes = (SCIP_Real) nnodes_real / (SCIP_Real) norgmodelknots;
   *ratio_edges = (SCIP_Real) nedges_real / (SCIP_Real) norgmodeledges;
}


/** gets original edge costs, when in extended mode */
void graph_pc_getOrgCosts(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< the graph */
   SCIP_Real*            edgecosts           /**< original costs to be filled */
)
{
   const int nedges = graph_get_nEdges(graph);
   const SCIP_Real* const cost_org = graph->cost_org_pc;

   assert(scip && edgecosts);
   assert(graph->extended && graph_pc_isPcMw(graph));

   assert(graph_pc_transOrgAreConistent(scip, graph, TRUE));

   if( SCIPgetStage(scip) < SCIP_STAGE_INITSOLVE )
   {
      BMScopyMemoryArray(edgecosts, cost_org, nedges);

#ifndef NDEBUG
      for( int e = 0; e < nedges; ++e )
      {
         assert(!graph_edge_isBlocked(graph, e));
      }
#endif
   }
   else
   {
      BMScopyMemoryArray(edgecosts, graph->cost, nedges);

      for( int e = 0; e < nedges; ++e )
      {
         const SCIP_Bool edgeIsBlocked = EQ(graph->cost[e], BLOCKED_MINOR) || EQ(graph->cost[e], BLOCKED);
         assert(edgeIsBlocked == graph_edge_isBlocked(graph, e));

         if( !edgeIsBlocked )
            edgecosts[e] = cost_org[e];
      }
   }
}


/** gets original edge costs for CSR, when in extended mode */
void graph_pc_getOrgCostsCsr(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< the graph */
   CSR*                  csr                 /**< CSR */
)
{
   const SCIP_Real* const gCosts = g->cost;
   const SCIP_Real* const gCosts_org = g->cost_org_pc;
   SCIP_Real* RESTRICT csrCosts = csr->cost;
   const int* const edgeid = csr->edge_id;
   const int nnodes = graph_get_nNodes(g);
   const int nedges = csr->start[nnodes];

   assert(edgeid && gCosts && gCosts_org);
   assert(nnodes >= 1 && nedges >= 0);
   assert(csr->nnodes == nnodes);
   assert(nedges <= csr->nedges_max);
   assert(nedges <= graph_get_nEdges(g));
   assert(g->extended && graph_pc_isPc(g));
   assert(graph_pc_transOrgAreConistent(scip, g, TRUE));

   for( int i = 0; i < nedges; i++ )
   {
      const int edge_g = edgeid[i];
      const SCIP_Bool edgeIsBlocked = EQ(gCosts[edge_g], BLOCKED_MINOR) || EQ(gCosts[edge_g], BLOCKED);

      assert(0 <= edge_g && edge_g < g->edges);
      assert(edgeIsBlocked == graph_edge_isBlocked(g, edge_g));

      if( edgeIsBlocked )
      {
         csrCosts[i] = gCosts[edge_g];
      }
      else
      {
         csrCosts[i] = gCosts_org[edge_g];
      }
   }
}


/** are the given costs equal to the original edge costs? */
SCIP_Bool graph_pc_costsEqualOrgCosts(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< the graph */
   const SCIP_Real*      edgecosts           /**< edge costs to be checked */
)
{
   SCIP_Real* costorg;
   const int nedges = graph_get_nEdges(graph);
   SCIP_Bool isEqual = TRUE;

   assert(scip && edgecosts);

   SCIP_CALL_ABORT( SCIPallocMemoryArray(scip, &costorg, nedges) );
   graph_pc_getOrgCosts(scip, graph, costorg);

   for( int e = 0; e < nedges; ++e )
   {
      if( !EQ(edgecosts[e], costorg[e]) )
      {
         isEqual = FALSE;
         break;
      }
   }

   SCIPfreeMemoryArray(scip, &costorg);

   return isEqual;
}


/** mark original graph (without dummy terminals) */
void graph_pc_markOrgGraph(
   GRAPH*                g                   /**< the graph */
)
{
   const int root = g->source;
   const int nnodes = g->knots;

   assert(g != NULL);
   assert(graph_pc_isPcMw(g));
   assert(g->extended);

   for( int k = 0; k < nnodes; k++ )
      g->mark[k] = (g->grad[k] > 0);

   for( int e = g->outbeg[root]; e != EAT_LAST; e = g->oeat[e] )
   {
      const int head = g->head[e];

      if( graph_pc_knotIsDummyTerm(g, head) )
      {
         g->mark[head] = FALSE;
         assert(g->grad[head] == 2);
         assert(GT(g->cost[e], 0.0));
      }
   }

   if( !graph_pc_isRootedPcMw(g) )
      g->mark[root] = FALSE;
}


/** mark terminals and switch terminal property to original terminals */
void graph_pc_2org(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
   )
{
   const int nnodes = graph_get_nNodes(graph);

   assert(scip);
   assert(graph->extended && graph_pc_isPcMw(graph));

   /* restore original edge weights */
   if( graph_pc_isPc(graph) )
   {
      if( (SCIPgetStage(scip) < SCIP_STAGE_INITSOLVE) )
         setCostToOrgPcPreState(scip, graph);
      else
         setCostToOrgPc(scip, graph);
   }

   /* swap terminal properties and mark original graph */
   for( int k = 0; k < nnodes; k++ )
   {
      if( Is_pseudoTerm(graph->term[k]) || Is_nonleafTerm(graph->term[k]) )
      {
         assert(!Is_term(graph->term[k]));

         graph_knot_chg(graph, k, STP_TERM);
      }
      else if( Is_term(graph->term[k]) && !graph_pc_knotIsFixedTerm(graph, k) )
      {
         assert(k != graph->source);

         graph_knot_chg(graph, k, STP_TERM_PSEUDO);
      }
   }

   graph->extended = FALSE;

   graph_mark(graph);
}

/** mark transformed graph and adapt terminal properties to transformed graph */
void graph_pc_2trans(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
   )
{
   const int nnodes = graph->knots;;

   assert(scip);
   assert(!graph->extended);
   assert(graph_pc_isPcMw(graph));

   /* adapt terminal properties for non-leaf terminals (so far not explicitly marked) */
   if( graph_pc_isPc(graph) )
      markNonLeafTerms_2trans(scip, graph);

   /* adapt terminal properties and mark transformed graph */
   for( int k = 0; k < nnodes; k++ )
   {
      if( Is_pseudoTerm(graph->term[k]) )
      {
         graph_knot_chg(graph, k, STP_TERM);
      }
      else if( Is_term(graph->term[k]) && !graph_pc_knotIsFixedTerm(graph, k) )
      {
         assert(k != graph->source);
         graph_knot_chg(graph, k, STP_TERM_PSEUDO);
      }
   }

   graph->extended = TRUE;

   graph_mark(graph);

   /* restore transformed edge weights (shift) after storing original ones */
   if( graph_pc_isPc(graph) )
   {
      assert(graph->cost_org_pc);
      BMScopyMemoryArray(graph->cost_org_pc, graph->cost, graph->edges);

      graph_pc_shiftNonLeafCosts(scip, graph);
   }
}

/** graph_pc_2org if extended */
void graph_pc_2orgcheck(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
   )
{
   assert(graph && scip);

   if( !graph->extended )
      return;

   graph_pc_2org(scip, graph);
}

/** graph_pc_2trans if not extended */
void graph_pc_2transcheck(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< the graph */
   )
{
   assert(graph && scip);

   if( graph->extended )
      return;

   graph_pc_2trans(scip, graph);
}


/* returns sum of positive vertex weights */
SCIP_Real graph_pc_getPosPrizeSum(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph               /**< the graph */
   )
{
   SCIP_Real prizesum = 0.0;

   assert(scip != NULL);
   assert(graph != NULL);
   assert(graph->prize != NULL);
   assert(!graph->extended);

   for( int i = 0; i < graph->knots; i++ )
      if( Is_term(graph->term[i]) && i != graph->source && SCIPisLT(scip, graph->prize[i], BLOCKED) )
         prizesum += graph->prize[i];

   return prizesum;
}


/* returns degree of non-root vertex in non-extended graph */
int graph_pc_realDegree(
   const GRAPH*          g,                  /**< graph data structure */
   int                   i,                  /**< the vertex to be checked */
   SCIP_Bool             fixedterm           /**< fixed terminal? */
)
{
   const int ggrad = g->grad[i];
   const SCIP_Bool isRooted = graph_pc_isRootedPcMw(g);
   const SCIP_Bool isMw = graph_pc_isMw(g);
   int newgrad;

   assert(g != NULL);
   assert(graph_pc_isPcMw(g));
   assert(!g->extended);
   assert(!Is_pseudoTerm(g->term[i]));
   assert(i != g->source || graph_pc_isRootedPcMw(g));
   assert(fixedterm == graph_pc_knotIsFixedTerm(g, i));

   if( !Is_term(g->term[i]) || fixedterm )
   {
      newgrad = ggrad;
   }
   else if( !isMw && graph_pc_knotIsNonLeafTerm(g, i) )
   {
      newgrad = ggrad;
   }
   else if( isRooted )
   {
      assert(Is_term(g->term[i]));

      newgrad = ggrad - 1;
   }
   else
   {
      assert(Is_term(g->term[i]));

      newgrad = ggrad - 2;
   }

   return newgrad;
}


/** adapts SAP deriving from PCST or MWCS problem with new big M */
void graph_pc_adaptSap(
   SCIP_Real             bigM,               /**< new big M value */
   GRAPH*                graph,              /**< the SAP graph */
   SCIP_Real*            offset              /**< the offset */
   )
{
   SCIP_Real oldbigM;
   const int root = graph->source;

   assert(bigM > 0.0);
   assert(graph != NULL && offset != NULL);
   assert(graph->outbeg[root] >= 0);

   oldbigM = graph->cost[graph->outbeg[root]];
   assert(oldbigM > 0.0);

   *offset += (oldbigM - bigM);

   SCIPdebugMessage("new vs old %f, %f \n", bigM, oldbigM);

   for( int e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
   {
      assert(EQ(graph->cost[e], oldbigM));
      graph->cost[e] = bigM;
   }
}


/** initializes biased data structures */
void graph_pc_getBiased(
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_Real* RESTRICT   costbiased,         /**< biased costs */
   SCIP_Real* RESTRICT   prizebiased         /**< biased prizes */
)
{
   assert(graph && costbiased && prizebiased);
   assert(graph->extended);
   assert(graph_pc_knotIsFixedTerm(graph, graph->source));
   assert(graph_pc_isPcMw(graph));

   if( graph_pc_isPc(graph) )
   {
      getBiasedPc(graph, costbiased, prizebiased);
   }
   else
   {
      assert(graph_pc_isMw(graph));
      getBiasedMw(graph, costbiased, prizebiased);
   }
}


/** returns offset generated by non-leaf terminals */
SCIP_Real graph_pc_getNonLeafTermOffset(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph               /**< graph data structure */
)
{
   SCIP_Real offset = 0.0;
   const int nnodes = graph_get_nNodes(graph);

   assert(scip);
   assert(graph->stp_type == STP_RPCSPG || graph->stp_type == STP_PCSPG);

   for( int i = 0; i < nnodes; ++i )
   {
      if( graph_pc_knotIsNonLeafTerm(graph, i) )
      {
         assert(SCIPisGT(scip, graph->prize[i], 0.0));
         offset += graph->prize[i];
      }
   }

   return offset;
}


/** Deletes a terminal for a (rooted) prize-collecting problem.
 *  Note that the prize of the terminal is not changed! */
int graph_pc_deleteTerm(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   int                   term,               /**< terminal to be deleted */
   SCIP_Real*            offset              /**< pointer to the offset */
   )
{
   int grad = g->grad[term];

   assert(g && scip && offset);
   assert(g->term2edge && g->prize);
   assert(graph_pc_isPcMw(g));
   assert(Is_term(g->term[term]));
   assert(!graph_pc_knotIsFixedTerm(g, term));

   assert(term != g->source);

   if( !g->extended && graph_pc_termIsNonLeafTerm(g, term) )
   {
      *offset += g->prize[term];
      graph_pc_knotToNonTermProperty(g, term);
      graph_knot_del(scip, g, term, TRUE);
      g->prize[term] = 0.0;
   }
   else
   {
      int e;
      int twin = UNKNOWN;

      /* delete terminal */

      while( (e = g->outbeg[term]) != EAT_LAST )
      {
         const int i1 = g->head[e];

         if( Is_pseudoTerm(g->term[i1]) && g->source != i1 )
            twin = g->head[e];

         graph_edge_del(scip, g, e, TRUE);
      }

      assert(g->grad[term] == 0);
      assert(twin != UNKNOWN);
      assert(twin == graph_pc_getTwinTerm(g, term));

      if( g->extended )
      {
         assert(SCIPisZero(scip, g->prize[term]));

         *offset += g->prize[twin];
         g->prize[twin] = 0.0;
      }
      else
      {
         assert(SCIPisZero(scip, g->prize[twin]));

         *offset += g->prize[term];
         g->prize[term] = 0.0;
      }

      graph_pc_knotToNonTermProperty(g, term);

      /* delete twin */

      graph_pc_knotToNonTermProperty(g, twin);
      g->mark[twin] = FALSE;
      grad += g->grad[twin] - 1;

      graph_knot_del(scip, g, twin, TRUE);
   }

   g->mark[term] = FALSE;

   assert(SCIPisZero(scip, g->prize[term]));

   return grad;
}


/** subtract a given sum from the prize of a terminal */
void graph_pc_subtractPrize(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   SCIP_Real             cost,               /**< cost to be subtracted */
   int                   i                   /**< the terminal */
   )
{
   assert(scip && g);
   assert(!g->extended);
   assert(Is_term(g->term[i]));
   assert(!graph_pc_knotIsFixedTerm(g, i) && i != g->source);

   g->prize[i] -= cost;

   assert(SCIPisGE(scip, g->prize[i], 0.0) || graph_pc_isPcMw(g));

   /* do we need to adapt edge cost as well? */
   if( graph_pc_isMw(g) || !graph_pc_termIsNonLeafTerm(g, i) )
   {
      const int twinterm = graph_pc_getTwinTerm(g, i);
      const int root2twin = graph_pc_getRoot2PtermEdge(g, twinterm);

      assert(!g->mark[twinterm]);
      assert(g->tail[root2twin] == g->source && g->head[root2twin] == twinterm);

      g->cost[root2twin] -= cost;

      assert(SCIPisEQ(scip, g->prize[i], g->cost[root2twin]));
   }
   else
   {
      assert(graph_pc_isPc(g));
   }
}


/** contract ancestors of an edge of (rooted) prize-collecting Steiner tree problem or maximum-weight connected subgraph problem */
SCIP_RETCODE graph_pc_contractNodeAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int                   t,                  /**< tail node to be contracted (surviving node) */
   int                   s,                  /**< head node to be contracted */
   int                   ets                 /**< edge from t to s or -1 */
   )
{
   assert(g != NULL);
   assert(scip != NULL);

   if( ets < 0 )
   {
      assert(ets == -1);

      for( ets = g->outbeg[t]; ets != EAT_LAST; ets = g->oeat[ets] )
         if( g->head[ets] == s )
            break;
   }

   assert(ets >= 0 && ets < g->edges);

   SCIP_CALL( SCIPintListNodeAppendCopy(scip, &(g->pcancestors[t]), g->pcancestors[s], NULL) );
   SCIP_CALL( SCIPintListNodeAppendCopy(scip, &(g->pcancestors[s]), g->pcancestors[t], NULL) );

   SCIP_CALL(SCIPintListNodeAppendCopy(scip, &(g->pcancestors[s]), graph_edge_getAncestors(g, ets), NULL));
   SCIP_CALL(SCIPintListNodeAppendCopy(scip, &(g->pcancestors[t]), graph_edge_getAncestors(g, ets), NULL));

   return SCIP_OKAY;
}


/** contract an edge of (rooted) prize-collecting Steiner tree problem or maximum-weight connected subgraph problem */
SCIP_RETCODE graph_pc_contractEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int*                  solnode,            /**< solution nodes or NULL */
   int                   t,                  /**< tail node to be contracted (surviving node) */
   int                   s,                  /**< head node to be contracted */
   int                   term4offset         /**< terminal to add offset to */
   )
{
   int ets;

   assert(scip && g);
   assert(Is_term(g->term[term4offset]));
   assert(!g->extended);
   assert(g->source != s);

   /* get edge from t to s */
   for( ets = g->outbeg[t]; ets != EAT_LAST; ets = g->oeat[ets] )
   {
      if( g->head[ets] == s )
         break;
   }

   assert(ets != EAT_LAST);

   if( graph_pc_knotIsFixedTerm(g, s) || graph_pc_knotIsFixedTerm(g, t) )
   {
      SCIP_CALL( contractEdgeWithFixedEnd(scip, g, solnode, t, s, ets) );
   }
   else
   {
      assert(!graph_pc_isRootedPcMw(g) || (s != g->source && t != g->source));
      SCIP_CALL( contractEdgeNoFixedEnd(scip, g, solnode, t, s, ets, term4offset) );
   }

   assert(g->grad[s] == 0);
   assert(TERM2EDGE_NOTERM == g->term2edge[s]);
   assert(!Is_anyTerm(g->term[s]));
   assert(SCIPisEQ(scip, g->prize[s], 0.0) || graph_pc_isMw(g));

   if( graph_pc_isMw(g) )
   {
      g->prize[s] = 0.0;
      mwKnotUpdateIncEdges(g, t);
   }

   SCIPdebugMessage("PcMw contraction: %d into %d, saved in %d \n", s, t, term4offset);

   return SCIP_OKAY;
}


/** contract an edge of (rooted) prize-collecting Steiner tree problem or maximum-weight connected subgraph problem;
 *  method decides whether to contract s into t or vice-versa. Offset is added to surviving node */
SCIP_RETCODE graph_pc_contractEdgeUnordered(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< the graph */
   int*                  solnode,            /**< solution nodes or NULL */
   int                   t,                  /**< tail node to be contracted */
   int                   s                   /**< head node to be contracted */
   )
{
   assert(g);

   if( graph_pc_termIsNonLeafTerm(g, t) )
      SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, s, t, s) );
   else if( graph_pc_termIsNonLeafTerm(g, s) )
      SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, t, s, t) );
   else if( (g->grad[s] >= g->grad[t] || s == g->source) && t != g->source )
      SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, s, t, s) );
   else
      SCIP_CALL( graph_pc_contractEdge(scip, g, solnode, t, s, t) );

   return SCIP_OKAY;
}


/** is this graph a prize-collecting variant? */
SCIP_Bool graph_pc_isPc(
   const GRAPH*          g                   /**< the graph */
)
{
   const int type = g->stp_type;
   assert(g != NULL);

   return (type == STP_PCSPG || type == STP_RPCSPG);
}


/** is this graph a maximum-weight variant? */
SCIP_Bool graph_pc_isMw(
   const GRAPH*          g                   /**< the graph */
)
{
   const int type = g->stp_type;
   assert(g != NULL);

   return (type == STP_MWCSP || type == STP_RMWCSP || type == STP_BRMWCSP);
}


/** is this graph a prize-collecting or maximum-weight variant? */
SCIP_Bool graph_pc_isPcMw(
   const GRAPH*          g                   /**< the graph */
)
{
   const int type = g->stp_type;
   assert(g != NULL);

   return (type == STP_PCSPG || type == STP_RPCSPG || type == STP_MWCSP || type == STP_RMWCSP || type == STP_BRMWCSP);
}


/** get edge from root to (pseudo) terminal */
int graph_pc_getRoot2PtermEdge(
   const GRAPH*          graph,               /**< the graph */
   int                   pseudoterm           /**< the pseudo terminal  */
)
{
   int rootedge = -1;
   const int root = graph->source;

   assert(graph != NULL);
   assert(pseudoterm >= 0 && pseudoterm < graph->knots);

   for( int e = graph->inpbeg[pseudoterm]; e != EAT_LAST; e = graph->ieat[e] )
   {
      if( graph->tail[e] == root )
      {
         rootedge = e;
         break;
      }
   }

   assert(rootedge >= 0);

   return rootedge;
}


/** get number of fixed terminals (not counting the aritificial root) */
int graph_pc_nFixedTerms(
   const GRAPH*          graph                /**< the graph */
)
{
   int nfixterms = 0;
   const int nnodes = graph->knots;
   assert(graph != NULL);
   assert(graph_pc_isPcMw(graph));

   if( !graph_pc_isRootedPcMw(graph) )
      return 0;

   for( int k = 0; k < nnodes; k++ )
      if( graph_pc_knotIsFixedTerm(graph, k) )
         nfixterms++;

   return nfixterms;
}


/** Returns number of non-fixed terminals.
 *  Note that it is equal to the number of the proper potential terminals
 *  if g->extended, because in this case the non-leaf terminals are not marked explicitly. */
int graph_pc_nNonFixedTerms(
   const GRAPH*          graph                /**< the graph */
)
{
   assert(graph != NULL);
   assert(graph_pc_isPcMw(graph));

   if( !graph_pc_isRootedPcMw(graph) )
      return (graph->terms - 1);

   return (graph->terms - graph_pc_nFixedTerms(graph));
}


/** returns number of non-leaf terminals */
int graph_pc_nNonLeafTerms(
   const GRAPH*          graph                /**< the graph */
)
{
   const int nnodes = graph_get_nNodes(graph);
   int nnonleafs = 0;

   assert(graph_pc_isPcMw(graph));

   for( int i = 0; i < nnodes; ++i )
      if( graph_pc_knotIsNonLeafTerm(graph, i) )
         nnonleafs++;

   return nnonleafs;
}


/** returns number of proper potential terminals (potential terminals excluding non-leaf terminals) */
int graph_pc_nProperPotentialTerms(
   const GRAPH*          graph                /**< the graph */
)
{
   int nppterms;

   assert(graph != NULL);
   assert(graph_pc_isPcMw(graph));

   if( graph->extended )
      nppterms = graph_pc_nNonFixedTerms(graph);
   else
      nppterms = graph_pc_nNonFixedTerms(graph) - graph_pc_nNonLeafTerms(graph);

   assert(nppterms >= 0);

   return nppterms;
}


/** compute solution value for given edge-solution array (CONNECT/UNKNOWN) and offset, takes prizes into account! */
SCIP_Real graph_pc_solGetObj(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< the graph */
   const int*            soledge,            /**< solution */
   SCIP_Real             offset              /**< offset */
   )
{
   const int nnodes = graph_get_nNodes(g);
   const int nedges = graph_get_nEdges(g);
   const SCIP_Real* const edgecost = g->cost;
   SCIP_Real obj = offset;

   assert(graph_pc_isPcMw(g));

   for( int e = 0; e < nedges; e++ )
      if( soledge[e] == CONNECT )
         obj += edgecost[e];

   /* there are no non-leaf terminals for MWCSP, so return already */
   if( !graph_pc_isPc(g) )
      return obj;

   if( g->extended )
   {
      obj += graph_pc_getNonLeafTermOffset(scip, g);
   }
   else
   {
      STP_Bool* solnode;

      SCIP_CALL_ABORT( SCIPallocBufferArray(scip, &solnode, nnodes) );

      solstp_setVertexFromEdge(g, soledge, solnode);

      for( int i = 0; i < nnodes; ++i )
      {
         if( !solnode[i] && graph_pc_knotIsNonLeafTerm(g, i) )
         {
            assert(SCIPisGT(scip, g->prize[i], 0.0));
            obj += g->prize[i];
         }
      }

      SCIPfreeBufferArray(scip, &solnode);
   }

   return obj;
}

/** get twin-terminal */
int graph_pc_getTwinTerm(
   const GRAPH*          g,                  /**< the graph */
   int                   vertex              /**< the vertex  */
)
{
   assert(g && g->term2edge);
   assert(graph_pc_isPcMw(g));
   assert(Is_anyTerm(g->term[vertex]));
   assert(g->term2edge[vertex] >= 0 && g->term2edge[vertex] < g->edges);
   assert(g->tail[g->term2edge[vertex]] == vertex);

   return g->head[g->term2edge[vertex]];
}


/** is this graph a un-rooted prize-collecting or rooted maximum-weight variant? */
SCIP_Bool graph_pc_isUnrootedPcMw(
   const GRAPH*          g                   /**< the graph */
)
{
   const int type = g->stp_type;
   assert(g != NULL);

   return (type == STP_PCSPG || type == STP_MWCSP);
}


/** is this graph a rooted prize-collecting or rooted maximum-weight variant? */
SCIP_Bool graph_pc_isRootedPcMw(
   const GRAPH*          g                   /**< the graph */
)
{
   const int type = g->stp_type;
   assert(g != NULL);

   return (type == STP_RPCSPG || type == STP_RMWCSP || type == STP_BRMWCSP);
}
