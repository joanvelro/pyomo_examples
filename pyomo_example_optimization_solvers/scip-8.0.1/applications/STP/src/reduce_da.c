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

/**@file   reduce_da.c
 * @brief  dual-ascent based reductions for Steiner tree problems
 * @author Daniel Rehfeldt
 *
 * This file implements dual-ascent based techniques for several Steiner problems.
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
#include "extreduce.h"
#include "heur_tm.h"
#include "heur_ascendprune.h"
#include "heur_lurkprune.h"
#include "heur_slackprune.h"
#include "heur_local.h"
#include "heur_rec.h"
#include "solpool.h"
#include "solstp.h"
#include "dualascent.h"
#include "probdata_stp.h"

#define BND_TMHEUR_NRUNS 100                  /**< number of runs of constructive heuristic */
#define BND_TMHEUR_NRUNS_RESTRICT 16          /**< number of runs of constructive heuristic */
#define BND_TMHEUR_NRUNS_RPC      16          /**< number of runs for RPC */
#define DEFAULT_DARUNS_DIRECTED   3
#define DEFAULT_DARUNS     8                  /**< number of runs for dual ascent heuristic */
#define DEFAULT_DARUNS_FAST     4             /**< number of runs for dual ascent heuristic */
#define DEFAULT_NMAXROOTS  8                  /**< max number of roots to use for new graph in dual ascent heuristic */
#define SOLPOOL_SIZE 20                       /**< size of presolving solution pool */
#define DARUNS_TERMRATIO_LOW   0.05
#define DARUNS_TERMRATIO_MED   0.075
#define DARUNS_TERMRATIO_HIGH   0.1
#define DARUNS_TERMRATIO_HUGE   0.2
#define STP_RED_MINBNDTERMS   750
#define STP_DABD_MAXDEGREE 5
#define STP_DABD_MAXDNEDGES 10
#define DAMAXDEVIATION_RANDOM_LOWER 0.20  /**< random upper bound for max deviation for dual ascent */
#define DAMAXDEVIATION_RANDOM_UPPER 0.30  /**< random upper bound for max deviation for dual ascent */
#define DAMAXDEVIATION_FAST         0.75


/** returns solution value for given edge-solution array (CONNECT/UNKNOWN) and offset, takes prizes into account! */
static
SCIP_Real getSolObj(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< the graph */
   const int*            soledge             /**< solution */
)
{
   SCIP_Real obj;
   if( graph_pc_isPcMw(g) )
      obj = graph_pc_solGetObj(scip, g, soledge, 0.0);
   else
      obj = solstp_getObjBounded(g, soledge, 0.0, g->edges);

   return obj;
}


/** computes dual solution with dual-ascent and guided solution (and possibly reroots given solution) */
static
SCIP_RETCODE computeDualSolutionGuided(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   SCIP_Real             damaxdeviation,     /**< maximum deviation for DA */
   REDCOST*              redcostdata,        /**< reduced cost data */
   int*                  result              /**< solution array */
)
{
   const int daroot = redcosts_getRootTop(redcostdata);
   SCIP_Real* const redcost = redcosts_getEdgeCostsTop(redcostdata);
   SCIP_Real dualobjval = -1.0;
   DAPARAMS daparams = { .addcuts = FALSE, .ascendandprune = FALSE, .root = daroot,
         .is_pseudoroot = FALSE, .damaxdeviation = damaxdeviation };

   /* solution might not be valid anymore */
   if( !solstp_isValid(scip, graph, result) )
   {
      SCIPdebugMessage("solution not valid; run normal dual-ascent \n");

      if( graph->stp_type == STP_DCSTP )
      {
         SCIP_CALL( dualascent_execDegCons(scip, graph, NULL, &daparams, redcost, &dualobjval) );
      }
      else
      {
         SCIP_CALL( dualascent_exec(scip, graph, NULL, &daparams, redcost, &dualobjval) );
      }
   }
   else
   {
      if( graph_typeIsUndirected(graph) )
      {
         SCIPdebugMessage("reroot solution and run guided dual-ascent \n");
         SCIP_CALL(solstp_reroot(scip, graph, result, daroot));
      }
      else
      {
         assert(daroot == graph->source);
         SCIPdebugMessage("run guided dual-ascent \n");
      }

      if( graph->stp_type == STP_DCSTP )
      {
         SCIP_CALL( dualascent_execDegCons(scip, graph, result, &daparams, redcost, &dualobjval));
      }
      else
      {
         SCIP_CALL( dualascent_exec(scip, graph, result, &daparams, redcost, &dualobjval) );
      }
   }

   if( STP_RPCSPG == graph->stp_type )
   {
      SCIPdebugMessage("RPC: add %f to dual objective \n", graph_pc_getNonLeafTermOffset(scip, graph));

      dualobjval += graph_pc_getNonLeafTermOffset(scip, graph);
   }

   redcosts_setDualBoundTop(dualobjval, redcostdata);

   return SCIP_OKAY;
}


/** computes dual solution with dual-ascent */
static
SCIP_RETCODE computeDualSolution(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   SCIP_Real             damaxdeviation,     /**< maximum deviation for DA */
   REDCOST*              redcostdata         /**< reduced cost data */
)
{
   const int daroot = redcosts_getRootTop(redcostdata);
   SCIP_Real* const redcost = redcosts_getEdgeCostsTop(redcostdata);
   SCIP_Real dualobjval = -1.0;
   DAPARAMS daparams = { .addcuts = FALSE, .ascendandprune = FALSE, .root = daroot,
         .is_pseudoroot = FALSE, .damaxdeviation = damaxdeviation };

   SCIPdebugMessage("no rerooting, run normal dual-ascent \n");

   if( graph->stp_type == STP_DCSTP )
   {
      SCIP_CALL( dualascent_execDegCons(scip, graph, NULL, &daparams, redcost, &dualobjval) );
   }
   else
   {
      SCIP_CALL( dualascent_exec(scip, graph, NULL, &daparams, redcost, &dualobjval) );

   }

   if( STP_RPCSPG == graph->stp_type )
   {
      SCIPdebugMessage("RPC: add %f to dual objective \n", graph_pc_getNonLeafTermOffset(scip, graph));

      dualobjval += graph_pc_getNonLeafTermOffset(scip, graph);
   }

   redcosts_setDualBoundTop(dualobjval, redcostdata);

   return SCIP_OKAY;
}


/** computes TM solution */
static
SCIP_RETCODE computeSteinerTreeTM(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   int*                  result,             /**< solution array */
   SCIP_Real*            bestobjval,         /**< pointer to the objective value */
   SCIP_Bool*            success             /**< success? */
)
{
   SCIP_Real* cost = NULL;
   SCIP_Real* costrev = NULL;
   int* startstm = NULL;
   const int nnodes = graph_get_nNodes(graph);
   const int nedges = graph_get_nEdges(graph);
   int runstm;
   SCIP_Real hopfactor = -1.0; /* automatic set to default */

   if( graph_typeIsDirected(graph) || graph->stp_type == STP_DCSTP )
   {
      runstm = BND_TMHEUR_NRUNS;
   }
   else if( graph_pc_isRootedPcMw(graph) )
   {
      runstm = BND_TMHEUR_NRUNS_RPC;
   }
   else
   {
      runstm = BND_TMHEUR_NRUNS_RESTRICT;
   }

   assert(graph->stp_type != STP_RPCSPG || !graph->extended);

   SCIP_CALL( SCIPallocBufferArray(scip, &startstm, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cost, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &costrev, nedges) );

   graph_getEdgeCosts(graph, cost, costrev);

   SCIPStpHeurTMCompStarts(graph, startstm, &runstm);

   SCIP_CALL( SCIPStpHeurTMRun(scip, pcmode_fromheurdata,
      graph, startstm, NULL, result, runstm, graph->source, cost, costrev, &hopfactor, NULL, success) );

   if( *success )
   {
      const SCIP_Real obj = getSolObj(scip, graph, result);

      SCIPdebugMessage("TM successful, obj=%f \n", obj);

      if( obj < *bestobjval )
         *bestobjval = obj;
   }
   else
   {
      assert(graph->stp_type == STP_DHCSTP || graph->stp_type == STP_DCSTP);
      SCIPdebugMessage("TM failed \n");
   }

   SCIPfreeBufferArray(scip, &costrev);
   SCIPfreeBufferArray(scip, &cost);
   SCIPfreeBufferArray(scip, &startstm);

   return SCIP_OKAY;
}



/** computes solution from reduced costs */
static
SCIP_RETCODE computeSteinerTreeRedCosts(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const REDCOST*        redcostdata,        /**< reduced cost data */
   SCIP_Bool             useSlackPrune,      /**< use slack prune? */
   SCIP_Bool             useRec,             /**< use recombination? */
   STPSOLPOOL*           pool,               /**< solution pool */
   int*                  result,             /**< result array */
   int*                  bestresult,         /**< result array */
   SCIP_Bool*            havebestsol,        /**< could best solution be stored in bestresult? */
   SCIP_Real*            bestobjval          /**< pointer to the objective value */
)
{
   const SCIP_Real* redcosts = redcosts_getEdgeCostsTop(redcostdata);
   const int daroot = redcosts_getRootTop(redcostdata);
   const int nedges = graph->edges;
   SCIP_Bool success;
   SCIP_Bool soladded;
   SCIP_Real objval;

   soladded = FALSE;

   SCIP_CALL( SCIPStpHeurAscendPruneRun(scip, NULL, graph, redcosts, result, daroot, &success, FALSE));
   assert(success && solstp_isValid(scip, graph, result));

   SCIP_CALL(SCIPStpHeurLocalRunFast(scip, graph, result));
   assert(solstp_isValid(scip, graph, result));

   objval = getSolObj(scip, graph, result);

   if( useRec )
      SCIP_CALL(solpool_addSol(scip, objval, result, pool, &soladded));

   /* should we try recombination? */
   if( useRec && soladded && pool->size >= 2 && LT(objval, *bestobjval) )
   {
      /* get index of just added solution */
      int solindex = pool->maxindex;
      SCIP_Bool solfound;

      SCIPdebugMessage("POOLSIZE %d \n", pool->size);

      SCIP_CALL(SCIPStpHeurRecRun(scip, pool, NULL, NULL, graph, NULL, &solindex, 1, pool->size, FALSE, &solfound));

      if( solfound )
      {
         const STPSOL* const sol = solpool_solFromIndex(pool, solindex);
         SCIP_Real solobjval;

         assert(sol != NULL);

         solobjval = sol->obj;

         if( graph->stp_type == STP_RPCSPG )
            solobjval += graph_pc_getNonLeafTermOffset(scip, graph);

         assert(SCIPisEQ(scip, getSolObj(scip, graph, sol->soledges), solobjval));

         SCIPdebugMessage("DA: rec found better solution with obj %f vs %f \n", sol->obj, objval);

         if( SCIPisLT(scip, solobjval, objval) )
         {
            assert(SCIPisLT(scip, getSolObj(scip, graph, sol->soledges), getSolObj(scip, graph, result)));

            BMScopyMemoryArray(result, sol->soledges, nedges);

            SCIP_CALL(SCIPStpHeurLocalRun(scip, graph, result));
            objval = getSolObj(scip, graph, result);

            assert(SCIPisLE(scip, objval, solobjval));

            if( objval < solobjval )
               SCIP_CALL(solpool_addSol(scip, objval, result, pool, &solfound));
         }
      }
   }

   if( useSlackPrune )
   {
      SCIP_Real upperbound_sp;
      SCIP_CALL( SCIPStpHeurSlackPruneRun(scip, NULL, graph, result, &success, FALSE, FALSE) );
      upperbound_sp = getSolObj(scip, graph, result);

      if( LT(upperbound_sp, objval) )
      {
          objval = upperbound_sp;
      }
   }

   if( SCIPisLE(scip, objval, *bestobjval) )
   {
      *havebestsol = TRUE;
      *bestobjval = objval;
      BMScopyMemoryArray(bestresult, result, nedges);
   }
   else if( *havebestsol )
   {
      *havebestsol = solstp_isUnreduced(scip, graph, bestresult);
   }

   assert(*havebestsol == FALSE || solstp_isValid(scip, graph, bestresult));

   return SCIP_OKAY;
}



/** computes solution from reduced costs for directed graph */
static
SCIP_RETCODE computeSteinerTreeRedCostsDirected(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const REDCOST*        redcostdata,        /**< reduced cost data */
   int*                  result,             /**< result array */
   int*                  bestresult,         /**< result array */
   SCIP_Bool*            havebestsol,        /**< could best solution be stored in bestresult? */
   SCIP_Real*            bestobjval          /**< pointer to the objective value */
)
{
   const SCIP_Real* redcosts = redcosts_getEdgeCostsTop(redcostdata);
   const int daroot = redcosts_getRootTop(redcostdata);
   const int nedges = graph->edges;
   SCIP_Bool success;
   SCIP_Real objval;

   assert(graph_typeIsDirected(graph) || graph->stp_type == STP_DCSTP);
   assert(daroot == graph->source || graph->stp_type == STP_DCSTP);

   SCIP_CALL( SCIPStpHeurAscendPruneRun(scip, NULL, graph, redcosts, result, daroot, &success, FALSE));

   if( !success )
   {
      assert(graph->stp_type == STP_DHCSTP || graph->stp_type == STP_DCSTP);
      *havebestsol = FALSE;

      SCIPdebugMessage("ascend-prune failed \n");

      SCIP_CALL( computeSteinerTreeTM(scip, graph, result, bestobjval, &success) );

      if( !success )
         return SCIP_OKAY;
   }

   objval = solstp_getObj(graph, result, 0.0);
   SCIPdebugMessage("old obj=%f ascend-prune obj=%f \n", *bestobjval, objval);

   assert(success && solstp_isValid(scip, graph, result));

   if( SCIPisLE(scip, objval, *bestobjval) )
   {
      *havebestsol = TRUE;
      *bestobjval = objval;
      BMScopyMemoryArray(bestresult, result, nedges);
   }
   else if( *havebestsol )
   {
      *havebestsol = solstp_isUnreduced(scip, graph, bestresult);
   }

   assert(*havebestsol == FALSE || solstp_isValid(scip, graph, bestresult));

   return SCIP_OKAY;
}


/** compute primal solution during dual-ascent routine for PCSTP or MWCSP based on reduced costs */
static
SCIP_RETCODE computeSteinerTreeRedCostsPcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   STPSOLPOOL*           pool,               /**< solution pool */
   const SCIP_Real*      cost,               /**< dual ascent costs */
   SCIP_Real*            upperbound,         /**< upperbound pointer */
   int*                  result1,            /**< sol int array corresponding to upper bound */
   int*                  result2,            /**< sol int array corresponding to best new solution (might be worse than upper bound) */
   int*                  pathedge,           /**< int array */
   STP_Bool*             nodearrchar,        /**< node array storing solution vertices */
   SCIP_Bool*            apsol               /**< ascend-prune sol? */
)
{
   SCIP_Real ub2;
   const int nedges = graph->edges;
   SCIP_Bool success;

   assert(graph_pc_isPcMw(graph));

   /* compute new solution and store it in result2 */

   SCIP_CALL( SCIPStpHeurAscendPruneRun(scip, NULL, graph, cost, result2, -1, &success, FALSE) );
   assert(success);

   SCIP_CALL( SCIPStpHeurLocalRun(scip, graph, result2) );

   assert(solstp_isValid(scip, graph, result2));

   ub2 = getSolObj(scip, graph, result2);
   SCIPdebugMessage("DA: first new sol value in computeSteinerTreeRedCostsPcMw: %f ... old value: %f \n", ub2, *upperbound);

   /* try recombination? */
   if( pool != NULL )
   {
      SCIPdebugMessage("ub %f vs best sol %f\n", ub2, pool->sols[0]->obj);
      SCIP_CALL( solpool_addSol(scip, ub2, result2, pool, &success) );

#ifdef SCIP_DEBUG
      for( int i = 0; i < pool->size; i++ )
         printf(" %f ", pool->sols[i]->obj);
      printf("\n ");
#endif

      if( success && pool->size >= 2 )
      {
         /* get index of just added solution */
         int solindex = pool->maxindex;

         SCIP_Bool solfound;

         SCIPdebugMessage("POOLSIZE %d \n", pool->size);

         SCIP_CALL( SCIPStpHeurRecRun(scip, pool, NULL, NULL, graph, NULL, &solindex, 3, pool->size, FALSE, &solfound) );

         if( solfound )
         {
            STPSOL* sol = solpool_solFromIndex(pool, solindex);
            SCIP_Real solobjval;

            assert(sol != NULL);
            solobjval = sol->obj;

            if( graph_pc_isPc(graph) )
               solobjval += graph_pc_getNonLeafTermOffset(scip, graph);

            assert(SCIPisEQ(scip, getSolObj(scip, graph, sol->soledges), solobjval));

            SCIPdebugMessage("DA: rec found better solution with obj %f vs %f \n", sol->obj, ub2);

            if( SCIPisLT(scip, solobjval, ub2) )
            {
               assert(SCIPisLT(scip, getSolObj(scip, graph, sol->soledges), getSolObj(scip, graph, result2)));

               BMScopyMemoryArray(result2, sol->soledges, nedges);

               SCIP_CALL( SCIPStpHeurLocalRun(scip, graph, result2) );

               assert(SCIPisLT(scip, getSolObj(scip, graph, result2), ub2));

               ub2 = getSolObj(scip, graph, result2);

               if( SCIPisLT(scip, ub2, sol->obj) )
                  SCIP_CALL( solpool_addSol(scip, ub2, result2, pool, &success) );
            }
         }
      }
   }

   if( SCIPisLE(scip, ub2, *upperbound) )
   {
      SCIPdebugMessage("DA: improved incumbent %f vs %f, return \n", ub2, *upperbound);

      *apsol = TRUE;
      *upperbound = ub2;
      BMScopyMemoryArray(result1, result2, nedges);
   }

   if( graph->stp_type != STP_MWCSP || !(*apsol) )
     return SCIP_OKAY;

#if 1
   SCIP_CALL( SCIPStpHeurRecExclude(scip, graph, result1, result2, pathedge, nodearrchar, &success) );

   if( success )
   {
      BMScopyMemoryArray(result1, result2, nedges);
      *upperbound = getSolObj(scip, graph, result1);
      SCIPdebugMessage("DA: afterLastExclusion %f \n", *upperbound);
   }
#endif

   return SCIP_OKAY;
}



/** collected terminals (fixed ones for RPC/RMW) */
static inline
void collectFixedTerminals(
   const GRAPH*          graph,              /**< graph data structure */
   int*                  terminals,          /**< terminals array (of size graph->terms) */
   int*                  nterms              /**< number of terminals (might differ for RPC) */
)
{
   int n = 0;
   const int nnodes = graph->knots;
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(graph);

   assert(graph->stp_type != STP_PCSPG && graph->stp_type != STP_MWCSP);
   assert(!isRpcmw || !graph->extended);

   for( int i = 0; i < nnodes; i++ )
   {
      if( Is_term(graph->term[i]) )
      {
         assert(graph->mark[i]);
         if( !isRpcmw || graph_pc_knotIsFixedTerm(graph, i) )
            terminals[n++] = i;
      }
   }

   assert(isRpcmw || graph->terms == n);
   *nterms = n;
}


/** updates node bounds for reduced cost fixings */
static
void updateNodeFixingBounds(
   SCIP_Real*            fixingbounds,       /**< fixing bounds */
   const GRAPH*          graph,              /**< graph data structure */
   const SCIP_Real*      pathdist,           /**< shortest path distances  */
   const PATH*           vnoi,               /**< Voronoi paths  */
   SCIP_Real             lpobjval,           /**< LP objective  */
   SCIP_Bool             initialize          /**< initialize fixing bounds? */
)
{
   const int nnodes = graph->knots;

   assert(graph->stp_type == STP_SPG || graph->stp_type == STP_RSMT || !graph->extended);

   if( initialize )
      for( int k = 0; k < nnodes; k++ )
         fixingbounds[k] = -FARAWAY;

   for( int k = 0; k < nnodes; k++ )
   {
      SCIP_Real fixbnd;

      if( !graph->mark[k] )
         continue;

      assert(!Is_pseudoTerm(graph->term[k]));

      fixbnd = pathdist[k] + vnoi[k].dist + lpobjval;

      if( fixbnd > fixingbounds[k] )
      {
         fixingbounds[k] = fixbnd;
      }
   }
}

/** updates node bounds for reduced cost replacement */
static
SCIP_RETCODE updateNodeReplaceBounds(
   SCIP*                 scip,               /**< SCIP */
   const REDCOST*        redcostdata,        /**< reduced cost data */
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_Real*            replacebounds,      /**< replacement bounds */
   SCIP_Real             upperbound,         /**< upper bound */
   SCIP_Bool             initialize,         /**< initialize fixing bounds? */
   SCIP_Bool             extendedsearch      /**< perform extended searching? */
)
{
   unsigned int* eqstack = NULL;
   SCIP_Bool* eqmark = NULL;
   int outedges[STP_DABD_MAXDEGREE];
   const int nnodes = graph->knots;
   const SCIP_Real lpobjval = redcosts_getDualBoundTop(redcostdata);
   const SCIP_Real cutoff = MAX(upperbound - lpobjval, 0.0);
   const int halfnedges = graph->edges / 2;
   const SCIP_Real *redcost = redcosts_getEdgeCostsTop(redcostdata);
   const SCIP_Real *pathdist = redcosts_getRootToNodeDistTop(redcostdata);
   const PATH *vnoi = redcosts_getNodeToTermsPathsTop(redcostdata);
   const int *vbase = redcosts_getNodeToTermsBasesTop(redcostdata);
   const int root = redcosts_getRootTop(redcostdata);

   assert(!graph_pc_isMw(graph));
   assert(SCIPisFeasGE(scip, upperbound, lpobjval));

   assert(graph->stp_type == STP_SPG || graph->stp_type == STP_RSMT || !graph->extended);
   assert(!extendedsearch && "deprecated!");

   if( initialize )
   {
      for( int k = 0; k < nnodes; k++ )
         replacebounds[k] = -FARAWAY;
   }

   if( extendedsearch )
   {
      SCIP_CALL( SCIPallocCleanBufferArray(scip, &eqmark, halfnedges) );
      SCIP_CALL( SCIPallocBufferArray(scip, &eqstack, halfnedges) );
   }

   for( int node = 0; node < nnodes; node++ )
   {
      const int degree = graph->grad[node];

      if( degree >= 3 && !Is_anyTerm(graph->term[node]) )
      {
         SCIP_Real fixbnd;

         /* bound already good enough? */
         if( SCIPisFeasLT(scip, upperbound, replacebounds[node]) )
               continue;

         fixbnd = pathdist[node] + vnoi[node].dist + vnoi[node + nnodes].dist + lpobjval;

         assert(!Is_pseudoTerm(graph->term[node]));

         /* Y-test for small degrees */
         if( degree <= STP_DABD_MAXDEGREE && !SCIPisFeasLT(scip, upperbound, fixbnd) )
         {
            int eqstack_size = 0;
            int edgecount = 0;

            fixbnd = FARAWAY;

            for( int e = graph->outbeg[node]; e != EAT_LAST; e = graph->oeat[e] )
               outedges[edgecount++] = e;

            assert(edgecount == degree);

            /* compute cost for each root and save minimum */
            for( int i = 0; i < degree; i++ )
            {
               const int tmproot = graph->head[outedges[i]];
               const int rootedge = flipedge(outedges[i]);

               /* loop over all combinations of Y with root tmproot */
               for( int j = i + 1; j <= i + degree - 2; j++ )
               {
                  for( int k = j + 1; k <= i + degree - 1; k++ )
                  {
                     const int outedge1 = outedges[j % degree];
                     const int outedge2 = outedges[k % degree];
                     const int leaf1 = graph->head[outedge1];
                     const int leaf2 = graph->head[outedge2];

                     SCIP_Real tmpcostY;

                     assert(leaf1 != leaf2 && tmproot != leaf1 && tmproot != leaf2);
                     assert(vbase[leaf1] >= 0 || vnoi[leaf1].dist >= FARAWAY);
                     assert(vbase[leaf2] >= 0 || vnoi[leaf2].dist >= FARAWAY);

                     if( leaf1 == root || leaf2 == root )
                        continue;

                     tmpcostY = pathdist[tmproot] + redcost[rootedge] + redcost[outedge1] + redcost[outedge2];

                     if( vbase[leaf1] != vbase[leaf2] )
                     {
                        tmpcostY += vnoi[leaf1].dist + vnoi[leaf2].dist;
                     }
                     else
                     {
                        /* also covers the case that leaf is a terminal */
                        const SCIP_Real leaf1far = vnoi[leaf1 + nnodes].dist;
                        const SCIP_Real leaf2far = vnoi[leaf2 + nnodes].dist;

                        assert(vbase[leaf1 + nnodes] >= 0 || leaf1far == FARAWAY);
                        assert(vbase[leaf2 + nnodes] >= 0 || leaf2far == FARAWAY);

                        tmpcostY += MIN(leaf1far + vnoi[leaf2].dist, vnoi[leaf1].dist + leaf2far);
                     }

                     if( tmpcostY < fixbnd )
                     {
                        if( extendedsearch && SCIPisLE(scip, tmpcostY, cutoff) )
                        {
                           int tree3outedges[2];
                           SCIP_Bool ruleout;
#ifndef NDEBUG
                           const SCIP_Real tmpcostYorg = tmpcostY;
#endif
                           tree3outedges[0] = outedge1;
                           tree3outedges[1] = outedge2;

                           SCIP_CALL( reduce_extendedCheck3Tree(scip, graph, root, redcost, pathdist, vnoi, vbase, cutoff, tree3outedges, rootedge,
                                       &tmpcostY, &ruleout, eqstack, &eqstack_size, eqmark) );

                           if( ruleout )
                              tmpcostY = FARAWAY;

#ifndef NDEBUG
                           assert(tmpcostY >= tmpcostYorg);
#endif
                        }

                        if( tmpcostY < fixbnd )
                           fixbnd = tmpcostY;
                     }
                  }
               } /* Y loop */
            } /* root loop */

            fixbnd += lpobjval;

            assert(SCIPisGE(scip, fixbnd, pathdist[node] + vnoi[node].dist + vnoi[node + nnodes].dist + lpobjval)
                  || fixbnd >= FARAWAY);

            if( extendedsearch )
            {
               for( int i = 0; i < eqstack_size; i++ )
                  eqmark[eqstack[i]] = FALSE;

               for( int i = 0; i < halfnedges; i++ )
                  assert(eqmark[i] == FALSE);
            }
         }

         if( fixbnd > replacebounds[node] )
            replacebounds[node] = fixbnd;
      }
   }
   if( extendedsearch )
   {
      assert(eqstack != NULL && eqmark != NULL);

      SCIPfreeBufferArray(scip, &eqstack);
      SCIPfreeCleanBufferArray(scip, &eqmark);
   }

   return SCIP_OKAY;
}


/** updates edge fixing bounds for reduced cost fixings */
static
void updateEdgeFixingBounds(
   SCIP_Real*            fixingbounds,       /**< fixing bounds */
   const GRAPH*          graph,              /**< graph data structure */
   const SCIP_Real*      cost,               /**< reduced costs */
   const SCIP_Real*      pathdist,           /**< shortest path distances  */
   const PATH*           vnoi,               /**< Voronoi paths  */
   SCIP_Real             lpobjval,           /**< LP objective  */
   int                   extnedges,          /**< number of edges for extended problem */
   SCIP_Bool             initialize,         /**< initialize fixing bounds? */
   SCIP_Bool             undirected          /**< only consider undirected edges */
)
{
   const int nnodes = graph->knots;

   assert(graph->stp_type == STP_SPG || graph->stp_type == STP_RSMT || !graph->extended);
   assert(extnedges > 0);

   if( initialize )
      for( int e = 0; e < extnedges; e++ )
         fixingbounds[e] = -FARAWAY;

   for( int k = 0; k < nnodes; k++ )
   {
      if( !graph->mark[k] )
         continue;

      assert(!Is_pseudoTerm(graph->term[k]));

      if( undirected )
      {
         for( int e = graph->outbeg[k]; e != EAT_LAST; e = graph->oeat[e] )
         {
            const int head = graph->head[e];

            if( graph->mark[head] )
            {
               const int erev = flipedge(e);
               const SCIP_Real fixbnd = pathdist[k] + cost[e] + vnoi[head].dist + lpobjval;
               const SCIP_Real fixbndrev = pathdist[head] + cost[erev] + vnoi[k].dist + lpobjval;
               const SCIP_Real minbnd = MIN(fixbnd, fixbndrev);

               assert(fixingbounds[e] == fixingbounds[erev]);

               if( minbnd > fixingbounds[e] )
               {
                  fixingbounds[e] = minbnd;
                  fixingbounds[erev] = minbnd;
               }
            }
         }
      }
      else
      {
         for( int e = graph->outbeg[k]; e != EAT_LAST; e = graph->oeat[e] )
         {
            const int head = graph->head[e];

            if( graph->mark[head] )
            {
               const SCIP_Real fixbnd = pathdist[k] + cost[e] + vnoi[head].dist + lpobjval;

               if( fixbnd > fixingbounds[e] )
                  fixingbounds[e] = fixbnd;
            }
         }
      }
   }
}

/** eliminate nodes by using fixing-bounds and reduced costs */
static
int reduceWithNodeFixingBounds(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   GRAPH*                transgraph,         /**< graph data structure or NULL */
   const SCIP_Real*      fixingbounds,       /**< fixing bounds */
   SCIP_Real             upperbound          /**< best upperbound */
)
{
   int nfixed = 0;
   const int nnodes = graph->knots;

   assert(graph->stp_type == STP_SPG || graph->stp_type == STP_RSMT || !graph->extended);

   graph_mark(graph);

   for( int k = 0; k < nnodes; k++ )
   {
      if( !graph->mark[k] || Is_term(graph->term[k]) )
         continue;

      assert(!Is_pseudoTerm(graph->term[k]));

      if( SCIPisFeasLT(scip, upperbound, fixingbounds[k]) )
      {
         SCIPdebugMessage("delete knot %d %f < %f %d\n", k, upperbound, fixingbounds[k], graph->grad[k]);
         nfixed += graph->grad[k];

         graph_knot_del(scip, graph, k, TRUE);

         if( transgraph != NULL )
            graph_knot_del(scip, transgraph, k, FALSE);
      }
   }

   return nfixed;
}


/** eliminate edges by using fixing-bounds and reduced costs */
static
int reduceWithEdgeFixingBounds(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   GRAPH*                transgraph,         /**< graph data structure or NULL */
   const SCIP_Real*      fixingbounds,       /**< fixing bounds */
   const int*            result,             /**< solution */
   SCIP_Real             upperbound          /**< best upperbound */
)
{
   int nfixed = 0;
   int nnodes = graph->knots;
   const SCIP_Bool solgiven = (result != NULL);
   const SCIP_Bool probIsDirected = graph_typeIsDirected(graph);

   assert(graph->stp_type == STP_SPG || graph->stp_type == STP_RSMT || !graph->extended);
   assert(!solgiven || SCIPisEQ(scip, upperbound, getSolObj(scip, graph, result)));
   assert(!solgiven || solstp_isValid(scip, graph, result));

   for( int k = 0; k < nnodes; k++ )
   {
      int e;
      if( !graph->mark[k] )
         continue;

      assert(!Is_pseudoTerm(graph->term[k]));

      e = graph->outbeg[k];
      while( e != EAT_LAST )
      {
         const int enext = graph->oeat[e];

         if( graph->mark[graph->head[e]] )
         {
            const int erev = flipedge(e);
            SCIP_Bool deleteEdge;

            if( !solgiven || result[e] == CONNECT || result[erev] == CONNECT )
               deleteEdge = (SCIPisFeasLT(scip, upperbound, fixingbounds[e]) && SCIPisFeasLT(scip, upperbound, fixingbounds[erev]));
            else
               deleteEdge = (SCIPisLE(scip, upperbound, fixingbounds[e]) && SCIPisLE(scip, upperbound, fixingbounds[erev]));

            if( deleteEdge )
            {
               assert(EQ(graph->cost[e], graph->cost[erev]) || graph_pc_isMw(graph) || probIsDirected);

               SCIPdebugMessage("delete edge %d (upperbound=%f fixingbound=%f) \n", e, upperbound, fixingbounds[e]);

               graph_edge_del(scip, graph, e, TRUE);

               if( transgraph != NULL )
                  graph_edge_del(scip, transgraph, e, FALSE);

               nfixed++;
            }
            else if( probIsDirected && LT(graph->cost[e], FARAWAY) )
            {
               SCIP_Bool deleteArc;
               if( !solgiven || result[e] == CONNECT )
                  deleteArc = (SCIPisFeasLT(scip, upperbound, fixingbounds[e]) );
               else
                  deleteArc = (SCIPisLE(scip, upperbound, fixingbounds[e]) );

               if( deleteArc )
               {
                  graph->cost[e] = FARAWAY;
               }
            }
         }

         e = enext;
      }
   }

   return nfixed;
}



/** eliminate edges with extended reductions */
static
SCIP_RETCODE reduceWithEdgeExtReds(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             upperbound,         /**< best upperbound */
   EXTPERMA*             extperma,           /**< extension data */
   GRAPH*                graph,              /**< graph data structure */
   int*                  ndeletions_run      /**< all deleted edges */
)
{
   REDCOST* const redcostdata = extperma->redcostdata;
   const int nlevels = redcosts_getNlevels(redcostdata);
   int nextdeleted = 0;

   assert(*ndeletions_run >= 0);

   for( int i = 0; i < nlevels; i++ )
   {
      redcosts_setCutoffFromBound(upperbound, i, redcostdata);

#ifdef SCIP_DEBUG
      {
         const SCIP_Real cutoff = redcosts_getCutoff(redcostdata, i);
         printf("Edge ext. reds., cutoff for level %d: %f \n", i, cutoff);
      }
#endif
   }

   SCIP_CALL( extreduce_deleteEdges(scip, extperma, graph, &nextdeleted) );

//#define EXT_WRITE
//   graph_printInfo(graph);  printf("newly fixedSECOND =%d \n", *nextdeleted);
#ifdef EXT_WRITE
      FILE *fp;
      //char filename[SCIP_MAXSTRLEN];
      //(void) SCIPsnprintf(filename, SCIP_MAXSTRLEN,"/nfs/optimi/kombadon/bzfrehfe/projects/scip/applications/STP/x%d_pred.stp", node);
      fp = fopen("/nfs/optimi/kombadon/bzfrehfe/projects/scip/applications/STP/STATS/stpall_hash.txt", "a+");
      fprintf(fp, "%s %d \n", SCIPgetProbName(scip), extfixed);
      fclose(fp);
      exit(1);
#endif

    *ndeletions_run += nextdeleted;

    return SCIP_OKAY;
}




/* marks nodes that can be pseudo-eliminated */
static
void markPseudoDeletablesFromBounds(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const SCIP_Real*      replacebounds,      /**< replacement bounds */
   SCIP_Real             upperbound,         /**< best upper bound */
   SCIP_Bool*            pseudoDelNodes      /**< pseudo deletable nodes */
)
{
   const int nnodes = graph->knots;
   const SCIP_Bool rpc = (graph->stp_type == STP_RPCSPG);

   for( int k = 0; k < nnodes; k++ )
      pseudoDelNodes[k] = FALSE;

   /* main loop */
   for( int degree = 3; degree <= STP_DABD_MAXDEGREE; degree++ )
   {
      for( int k = 0; k < nnodes; k++ )
      {
         if( rpc && degree == 3 && graph_pc_knotIsNonLeafTerm(graph, k) && graph->grad[k] == 3 )
         {
            SCIPdebugMessage("found rpc deg3 candidate %d \n", k);
         }
         else if( (degree != graph->grad[k] || Is_anyTerm(graph->term[k])) )
         {
            continue;
         }

         if( SCIPisFeasLT(scip, upperbound, replacebounds[k]))
         {
            pseudoDelNodes[k] = TRUE;
         }
      }
   }
}


/** submethod for daFindRoots */
static
void findRootsMark(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const GRAPH*          transgraph,         /**< graph data structure */
   const int*            termmark,           /**< terminal mark (2 for proper terminal, 1 for non-proper terminal, 0 otherwise) */
   const SCIP_Real*      cost,               /**< da reduced cost */
   const SCIP_Real*      bestcost,           /**< best incumbent da reduced cost */
   SCIP_Real             lpobjval,           /**< da lower bound */
   SCIP_Real             bestlpobjval,       /**< best da lower bound */
   SCIP_Real             upperbound,         /**< da upper bound */
   SCIP_Bool             rerun,              /**< not the first run? */
   SCIP_Bool             probrooted,         /**< is transgraph a rooted RMW or RPC? */
   int                   pseudoterm,         /**< pseudo terminal */
   int                   pseudoedge,         /**< pseudo terminal edge */
   STP_Bool*             isfixedterm,        /**< bool array to indicate fixed terminals */
   int*                  roots,              /**< root array */
   int*                  rootscount,         /**< number of roots */
   int*                  pathedge,           /**< array */
   STP_Bool*             visited,            /**< stores whether a node has been visited */
   SCIP_Real*            dist                /**< distances array, initially set to FARAWAY */
)
{
   int realterm = -1;
   SCIP_Bool mark = FALSE;
   const SCIP_Real objgap = MAX(upperbound - lpobjval, 0.0);
   const SCIP_Real objgap_best = MAX(upperbound - bestlpobjval, 0.0);

   assert(!graph->extended && transgraph->extended);
   assert(graph->grad[pseudoterm] == 2);

   if( probrooted )
   {
      assert(transgraph->tail[transgraph->term2edge[pseudoterm]] == pseudoterm);

      realterm = transgraph->head[transgraph->term2edge[pseudoterm]];
   }
   else
   {
      for( int e2 = transgraph->inpbeg[pseudoterm]; e2 != EAT_LAST; e2 = transgraph->ieat[e2] )
      {
         if( transgraph->cost[e2] == 0.0 )
            realterm = transgraph->tail[e2];
         else
            assert(pseudoedge == e2); /* that holds because of correspondence between graph and transgraph for the pseudo-terminal edge */
      }
   }

   assert(realterm >= 0 && graph->mark[realterm]);
   assert(realterm != graph->source && realterm != transgraph->source);
   assert(Is_pseudoTerm(transgraph->term[realterm]) && Is_term(graph->term[realterm]));

   if( rerun && isfixedterm[realterm] )
      return;

   if( SCIPisGT(scip, cost[pseudoedge], objgap) || SCIPisGT(scip, bestcost[pseudoedge], objgap_best) )
   {
      mark = TRUE;
   }
   else
   {
      /* get terminals that imply realterm and add corresponding reduced costs up */
      int nvisits;
      double costsum = cost[pseudoedge];
      double bestcostsum = bestcost[pseudoedge];

      assert(graph->path_heap != NULL);
      mark = graph_sdWalksConnected(scip, graph, termmark, graph->cost, isfixedterm, realterm, 1500, dist, pathedge, &nvisits,
            visited, TRUE);

#ifndef NDEBUG
      for( int k = 0; k < graph->knots; k++ )
         assert(graph->path_state[k] == UNKNOWN && visited[k] == FALSE && dist[k] == FARAWAY);
#endif

      if( !mark )
      {
         for( int k = 0; k < nvisits; k++ )
         {
            const int node = pathedge[k];

            assert((termmark[node] == 2) == (Is_term(graph->term[node]) && !graph_pc_termIsNonLeafTerm(graph, node)));

            if( termmark[node] == 2 && node != realterm )
            {
               const int nodepterm = graph_pc_getTwinTerm(graph, node);
               const int rootedge = graph_pc_getRoot2PtermEdge(graph, nodepterm);

               assert(graph->mark[node]);
               assert(!graph_pc_knotIsFixedTerm(graph, node));
               assert(graph->grad[nodepterm] == 2);
               assert(rootedge >= 0);
               assert(graph->cost[rootedge] == transgraph->cost[rootedge]);
               assert(graph->cost[rootedge] == graph->prize[node]);

               costsum += cost[rootedge];
               bestcostsum += bestcost[rootedge];
            }
         }

         if( SCIPisGT(scip, costsum, objgap) || SCIPisGT(scip, bestcostsum, objgap_best) )
            mark = TRUE;
      }
   }

   if( mark )
   {
      assert(realterm >= 0);

      assert(SCIPisPositive(scip, graph->prize[realterm]));
      assert((*rootscount) < graph->terms);

      roots[(*rootscount)++] = realterm;
      isfixedterm[realterm] = TRUE;
   }
}


/** special method for RPC/RMW that deletes incident edges of terminal, but not the terminal and the extension itself */
static
void daRpcmwDeleteTermIncidents(
   SCIP*                 scip,               /**< SCIP data structure */
   const PATH*           vnoi,               /**< Voronoi data structure */
   int                   term,               /**< the terminal */
   GRAPH*                graph,              /**< graph data structure */
   int*                  incidents,          /**< int array */
   int*                  nfixedp             /**< number of fixed edges pointer */
)
{
   const int twinterm = graph_pc_getTwinTerm(graph, term);
   int incidcount = 0;

#ifndef NDEBUG
   const int termedge = graph->term2edge[term];
   assert(termedge >= 0 && Is_pseudoTerm(graph->term[twinterm]) && graph->cost[termedge] == 0.0);
   assert(graph_pc_isRootedPcMw(graph));
#endif

   for( int e = graph->outbeg[term]; e != EAT_LAST; e = graph->oeat[e] )
      incidents[incidcount++] = e;

   assert(incidcount == graph->grad[term]);
   (*nfixedp) += graph->grad[term] - 1;

   for( int e = 0; e < incidcount; e++ )
   {
      const int edge = incidents[e];

      assert(graph->tail[edge] == term);

      if( graph->head[edge] == twinterm )
         continue;

      graph_edge_del(scip, graph, edge, TRUE);
   }

   assert(graph->grad[term] == 1);
   assert(graph->outbeg[term] == graph->term2edge[term] && twinterm == graph_pc_getTwinTerm(graph, term));
}

/** find roots for PC and MW during DA reduction */
static
SCIP_RETCODE daPcFindRoots(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const GRAPH*          transgraph,         /**< graph data structure */
   const SCIP_Real*      cost,               /**< da reduced cost */
   const SCIP_Real*      bestcost,           /**< best incumbent da reduced cost */
   SCIP_Real             lpobjval,           /**< da lower bound */
   SCIP_Real             bestlpobjval,       /**< best da lower bound */
   SCIP_Real             upperbound,         /**< da upper bound */
   SCIP_Bool             rerun,              /**< not the first run? */
   SCIP_Bool             probrooted,         /**< is transgraph a rooted RMW or RPC? */
   PATH*                 vnoi,               /**< SP array */
   int*                  pathedge,           /**< array */
   int*                  vbase,              /**< array */
   STP_Bool*             isfixedterm,        /**< bool array */
   int*                  roots,              /**< roots (fixed terminals) array */
   int*                  rootscount          /**< number of roots */
)
{
   SCIP_Real* dist;
   STP_Bool* visited;
   int* termmark;
   int* const state = graph->path_state;
   const int root = graph->source;
   const int nnodes = graph->knots;
   const SCIP_Bool graphextended = graph->extended;
   int nroots = *rootscount;
   int nvisits;

   assert(state);
   assert(graph_pc_isPcMw(graph));
   assert(!graph_pc_isRootedPcMw(graph));

   SCIP_CALL(SCIPallocBufferArray(scip, &dist, nnodes));
   SCIP_CALL(SCIPallocBufferArray(scip, &visited, nnodes));
   SCIP_CALL(SCIPallocBufferArray(scip, &termmark, nnodes));

   for( int i = 0; i < nnodes; i++ )
   {
      visited[i] = FALSE;
      state[i] = UNKNOWN;
      dist[i] = FARAWAY;
   }

   assert(transgraph->extended);

   if( graphextended )
      graph_pc_2org(scip, graph);

   graph_pc_termMarkProper(graph, termmark);

   assert(rerun || nroots == 0);

   /*
    * get possible roots
    */

   BMSclearMemoryArray(isfixedterm, nnodes);

   if( rerun )
   {
      for( int i = 0; i < nroots; i++ )
         isfixedterm[roots[i]] = TRUE;
   }

   SCIPdebugMessage("before findDaRootsMark: all roots: %d, nodes: %d edges: %d terms: %d \n",
         nroots, nnodes, graph->edges, graph->terms);

   /* has transgraph non-artificial root (and arcs to pseudo-terminals)? */
   if( probrooted )
   {
      const int transroot = transgraph->source;

      assert(transgraph->term2edge != NULL);

      for( int e = transgraph->outbeg[transroot]; e != EAT_LAST; e = transgraph->oeat[e] )
      {
         const int pseudoterm = transgraph->head[e];

         if( Is_term(transgraph->term[pseudoterm]) && transgraph->term2edge[pseudoterm] >= 0 )
         {
            findRootsMark(scip, graph, transgraph, termmark, cost, bestcost, lpobjval, bestlpobjval, upperbound, rerun, probrooted, pseudoterm, e,
                  isfixedterm, roots, &nroots, pathedge, visited, dist);
         }
      }
   }
   /* transgraph has artificial root, so no arcs to pseudo-terminals */
   else
   {
      for( int e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
      {
         const int pseudoterm = graph->head[e];

         if( Is_pseudoTerm(graph->term[pseudoterm]) )
         {
            findRootsMark(scip, graph, transgraph, termmark, cost, bestcost, lpobjval, bestlpobjval, upperbound, rerun, probrooted, pseudoterm, e,
                  isfixedterm, roots, &nroots, pathedge, visited, dist);
         }
      }
   }

   SCIPdebugMessage("...after: new roots in rerun %d all roots: %d, nodes: %d edges: %d terms: %d \n\n", nroots - *rootscount,
         nroots, nnodes, graph->edges, graph->terms);

   /* could more roots be found? */
   if( nroots > *rootscount && graph->terms > 2 )
   {
      /*
       * try to find additional roots by connecting walks
       */
      SCIP_Bool rerunwalks = TRUE;

      for( int rounds = 0; rounds < 3 && rerunwalks; rounds++ )
      {
         rerunwalks = FALSE;

         for( int i = 0; i < nnodes; i++ )
         {
            SCIP_Bool connected;

            if( !Is_term(graph->term[i]) || isfixedterm[i] || graph_pc_knotIsFixedTerm(graph, i) )
               continue;

            if( graph->grad[i] == 0 )
            {
               assert(graph_pc_isPcMw(graph) && graph_pc_knotIsNonLeafTerm(graph, i));
               continue;
            }

            connected = graph_sdWalksConnected(scip, graph, termmark, graph->cost, isfixedterm, i, 1500, dist, pathedge, &nvisits,
                  visited, TRUE);

            if( connected )
            {
               assert(nroots < graph->terms);

               roots[nroots++] = i;
               isfixedterm[i] = TRUE;
               rerunwalks = TRUE;

               SCIPdebugMessage("WALK-CONNECT: added new root %d prize: %f  \n", i, graph->prize[i]);
            }

#ifndef NDEBUG
            for( int k = 0; k < nnodes; k++ )
            {
               assert(state[k] == UNKNOWN);
               assert(visited[k] == FALSE);
               assert(dist[k] == FARAWAY);
            }
#endif
         }
      } /* for rounds < 3 */

      SCIPdebugMessage("number of terminals found by DA: %d \n", nroots);

   } /* look for additional roots */

   SCIPdebugMessage("new roots in rerun %d all roots: %d, nodes: %d  \n", nroots - *rootscount, nroots, nnodes);

   *rootscount = nroots;

   if( graphextended )
      graph_pc_2trans(scip, graph);

   SCIPfreeBufferArray(scip, &termmark);
   SCIPfreeBufferArray(scip, &visited);
   SCIPfreeBufferArray(scip, &dist);

   return SCIP_OKAY;
}


/** computes TM solution and adds it too pool */
static
SCIP_RETCODE daPcAddTmSolToPool(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   int*                  result,             /**< solution */
   STPSOLPOOL*           pool                /**< the pool */
)
{
   SCIP_Bool success;
   SCIP_Real ub;

   /* compute second solution and add to pool */
   SCIP_CALL( SCIPStpHeurTMRun(scip, pcmode_fromheurdata,
      graph, NULL, NULL, result, BND_TMHEUR_NRUNS_RPC, graph->source, graph->cost, graph->cost, NULL, NULL, &success) );
   assert(success);

   SCIP_CALL( SCIPStpHeurLocalRun(scip, graph, result) );
   ub = getSolObj(scip, graph, result);

   SCIP_CALL( solpool_addSol(scip, ub, result, pool, &success) );
   SCIPdebugMessage("added initial TM sol to pool? %d , ub %f \n", success, ub);

   return SCIP_OKAY;
}


/** set prize of marked terminals to blocked */
static
void daPcMarkRoots(
   SCIP*                 scip,               /**< SCIP data structure */
   const int*            roots,              /**< root array */
   int                   nrootsold,          /**< old number of roots */
   int                   nrootsnew,          /**< new number of roots */
   SCIP_Real             prizesum,           /**< sum of positive prizes */
   GRAPH*                graph,              /**< graph data structure */
   SCIP_Bool*            userec,             /**< recombination? */
   STPSOLPOOL**          solpool             /**< solution pool */
)
{
   const int root = graph->source;
   const SCIP_Bool graphextended = graph->extended;

   if( graphextended )
      graph_pc_2org(scip, graph);

   if( *userec && *solpool != NULL )
   {
      *userec = FALSE;
      solpool_free(scip, solpool);
      *solpool = NULL;
   }

   assert(graph != NULL);
   assert(roots != NULL);
   assert(!graph->extended);
   assert(nrootsnew >= 0 && nrootsold >= 0);

   for( int i = nrootsold; i < nrootsnew; i++ )
   {
      int e;
      const int term = roots[i];
      const int pterm = graph_pc_getTwinTerm(graph, term);

      assert(Is_term(graph->term[term]));
      assert(SCIPisPositive(scip, graph->prize[term]));
      assert(pterm >= 0);
      assert(Is_pseudoTerm(graph->term[pterm]));

      for( e = graph->inpbeg[pterm]; e != EAT_LAST; e = graph->ieat[e] )
         if( root == graph->tail[e] )
            break;

      assert(e != EAT_LAST);
      assert(SCIPisEQ(scip, graph->prize[term], graph->cost[e]));

      graph->prize[term] = prizesum;
      graph->cost[e] = prizesum;
   }

   if( graphextended )
      graph_pc_2trans(scip, graph);
}

/** are the reduced costs still valid, i.e. are there zero cost paths from the root to all terminals? */
static
SCIP_Bool daRedCostIsValid(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                transgraph,         /**< graph data structure */
   const SCIP_Real*      cost,               /**< dual ascent costs */
   int*                  nodearrint,         /**< int array */
   STP_Bool*             nodearrbool         /**< bool array */
)
{
   int* const queue = nodearrint;
   STP_Bool*  visited = nodearrbool;
   int qsize;
   const int root = transgraph->source;
   const int nnodes = transgraph->knots;

   /*
    * construct new graph corresponding to zero cost paths from the root to all terminals
    */

   BMSclearMemoryArray(visited, nnodes);

   qsize = 0;
   visited[root] = TRUE;
   queue[qsize++] = root;

   /* DFS */
   while( qsize )
   {
      const int k = queue[--qsize];

      /* traverse outgoing arcs */
      for( int a = transgraph->outbeg[k]; a != EAT_LAST; a = transgraph->oeat[a] )
      {
         const int head = transgraph->head[a];

         if( !visited[head] && SCIPisZero(scip, cost[a]) )
         {
            visited[head] = TRUE;
            queue[qsize++] = head;
         }
      }
      assert(qsize <= nnodes);
   }

   for( int k = 0; k < nnodes; k++ )
      if( Is_term(transgraph->term[k]) && !visited[k] )
      {
         return FALSE;
      }

   return TRUE;
}


/** returns maximum allowed deviation for dual-ascent */
static
SCIP_Real daGetMaxDeviation(
   const RPDA*           paramsda,           /**< parameters */
   SCIP_RANDNUMGEN*      randnumgen          /**< random number generator */
)
{
   SCIP_Real damaxdeviation;

   assert(paramsda && randnumgen);

#if 1
   damaxdeviation = SCIPrandomGetReal(randnumgen, DAMAXDEVIATION_RANDOM_LOWER, DAMAXDEVIATION_RANDOM_UPPER);
#else
   if( paramsda->damode > 0 )
      damaxdeviation = SCIPrandomGetReal(randnumgen, DAMAXDEVIATION_RANDOM_LOWER, DAMAXDEVIATION_RANDOM_UPPER);
   else
      damaxdeviation = -1.0;
#endif

   return damaxdeviation;
}


/** decide whether guided DA is promising */
static
SCIP_Bool daGuidedIsPromising(
   const GRAPH*          graph,              /**< graph structure */
   int                   run,                /**< number of current run */
   SCIP_RANDNUMGEN*      randnumgen          /**< random number generator */
)
{
   assert(run >= 0);

   if( !graph_typeIsUndirected(graph) )
   {
      assert(run < DEFAULT_DARUNS_DIRECTED);
      if( run > 0 )
         return TRUE;
   }

   return ((run > 1) && (SCIPrandomGetInt(randnumgen, 0, 2) < 2));
}

/** order roots */
static
SCIP_RETCODE daOrderRoots(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph structure */
   int*                  terms,              /**< sol int array corresponding to upper bound */
   int                   nterms,             /**< number of terminals */
   SCIP_Bool             randomize,          /**< randomize? */
   SCIP_RANDNUMGEN*      randnumgen          /**< random number generator */
)
{
   int* termdegs;
   int maxdeg = 0;
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(graph);

   assert(terms != NULL);
   assert(nterms > 0);

   if( graph_typeIsDirected(graph) )
   {
      for( int i = 0; i < nterms; i++ )
         terms[i] = graph->source;

      return SCIP_OKAY;
   }

   SCIP_CALL( SCIPallocBufferArray(scip, &termdegs, nterms) );

   for( int i = 0; i < nterms; i++ )
   {
      const int grad = graph->grad[terms[i]];
      assert(terms[i] >= 0);

      termdegs[i] = grad;

      if( grad > maxdeg )
         maxdeg = termdegs[i];

      if( isRpcmw )
      {
         assert(graph_pc_knotIsFixedTerm(graph, terms[i] ));

         /* make sure root is selected for RPC/RMW */
         if( terms[i] == graph->source )
            termdegs[i] = 2 * graph->knots;
      }
   }

   if( randomize )
   {
      assert(randnumgen);
      for( int i = 0; i < nterms; i++ )
         termdegs[i] += SCIPrandomGetInt(randnumgen, 0, 2 * maxdeg);
   }

   SCIPsortDownIntInt(termdegs, terms, nterms);

   SCIPfreeBufferArray(scip, &termdegs);

   return SCIP_OKAY;
}


/** initializes reduced costs data */
static
SCIP_RETCODE daRedcostsInit(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const RPDA*           paramsda,           /**< parameters */
   int                   nLevels,            /**< number of levels for extended reductions */
   REDCOST**             redcostdata         /**< reduced cost data */
)
{
   const int nnodes = graph_get_nNodes(graph);
   const int nedges = graph_get_nEdges(graph);
   RCPARAMS rcparams = { .cutoff = -1.0, .nLevels = 1, .nCloseTerms = 3,
                         .nnodes = nnodes, .nedges = nedges, .redCostRoot = UNKNOWN };

   if( paramsda->extredMode != extred_none )
   {
      rcparams.nLevels = nLevels;
   }

   SCIP_CALL( redcosts_initFromParams(scip, &rcparams, redcostdata) );

   return SCIP_OKAY;
}


/** frees reduced costs data */
static
void daRedcostsExit(
   SCIP*                 scip,               /**< SCIP data structure */
   REDCOST**             redcostdata         /**< reduced cost data */
)
{
   redcosts_free(scip, redcostdata);
}


/** number of runs */
static
int daGetNruns(
   const GRAPH*          graph,              /**< graph structure */
   const RPDA*           paramsda,           /**< parameters */
   int                   nterms              /**< number of terminals */
)
{
   int nruns;

   if( paramsda->damode == STP_DAMODE_FAST )
   {
	   nruns = MIN(nterms, DEFAULT_DARUNS_FAST);
   }
   else
   {
	   nruns = MIN(nterms, DEFAULT_DARUNS);

	   if( graph_typeIsSpgLike(graph) )
	   {
	      SCIP_Real ratio;
	      int nnodes;
	      graph_get_nVET(graph, &nnodes, NULL, NULL);
	      ratio = (SCIP_Real) graph->terms / (SCIP_Real) nnodes;

         if( ratio >= DARUNS_TERMRATIO_LOW )
            nruns--;
         if( ratio >= DARUNS_TERMRATIO_MED )
            nruns--;
         if( ratio >= DARUNS_TERMRATIO_HIGH )
            nruns--;
         if( ratio >= DARUNS_TERMRATIO_HUGE )
            nruns--;
	   }
   }

   if( !graph_typeIsUndirected(graph) )
   {
      nruns = MIN(nruns, DEFAULT_DARUNS_DIRECTED);
   }

   if( paramsda->damode == STP_DAMODE_HOPS )
   {
      assert(graph->stp_type == STP_DHCSTP);
      nruns = MIN(nruns, 2);
   }

   nruns = MAX(nruns, 1);

   return nruns;
}


/** initializes DA round */
static
SCIP_RETCODE daRoundInit(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             upperbound,         /**< upper bound */
   GRAPH*                graph,              /**< graph structure */
   REDCOST*              redcostdata,        /**< reduced cost data */
   STP_Bool*             arcsdeleted,        /**< array */
   SCIP_Real*            cutoffbound         /**< cut-off bound */
)
{
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(graph);

   redcosts_setAndReturnCutoffFromBoundTop(upperbound, redcostdata, cutoffbound);

   if( isRpcmw )
      graph_pc_2org(scip, graph);
   else
      graph_mark(graph);

   if( graph_typeIsUndirected(graph) )
   {
      const int nedges = graph_get_nEdges(graph);

      /* NOTE: because we change the root node for dual-ascent, the deleted arcs don't
       * stay valid between rounds */
      for( int e = 0; e < nedges; e++ )
         arcsdeleted[e] = FALSE;
   }

   SCIP_CALL( redcosts_initializeDistancesTop(scip, graph, redcostdata) );

   return SCIP_OKAY;
}


/** for exiting DA round */
static
void daRoundExit(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   ndeletions_run,     /**< number of deletions */
   GRAPH*                graph,              /**< graph structure */
   int*                  nelims              /**< number of eliminations */
)
{
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(graph);

   assert(scip && nelims);
   assert(ndeletions_run >= 0 && *nelims >= 0);

   if( ndeletions_run > 0 && !isRpcmw )
   {
      if( graph_typeIsDirected(graph) )
         reduce_unconnectedForDirected(scip, graph);
      else
         reduce_unconnected(scip, graph);
   }

   assert(graph_valid(scip, graph));

   if( !isRpcmw )
      graph_mark(graph);
   else
      graph_pc_2trans(scip, graph);

   *nelims += ndeletions_run;
   SCIPdebugMessage("new deletions: %d \n", ndeletions_run);
}


/** try to improve both dual and primal solution */
static
SCIP_RETCODE computePertubedSol(
   SCIP* scip,
   GRAPH* graph,
   GRAPH* transgraph,
   STPSOLPOOL* pool,
   PATH* vnoi,
   SCIP_Real* cost,
   SCIP_Real* bestcost,
   SCIP_Real* pathdist,
   int* pathedge,
   int* result,
   int* result2,
   int* transresult,
   STP_Bool* nodearrchar,
   SCIP_Real* upperbound,
   SCIP_Real* lpobjval,
   SCIP_Real* bestlpobjval,
   SCIP_Real* minpathcost,
   SCIP_Bool* apsol,
   SCIP_Real offset,
   int extnedges
   )
{
   SCIP_Real lb;
   int e;
   const int root = graph->source;
   const int nedges = graph->edges;
   const int transnedges = transgraph->edges;

   assert(graph_pc_isPcMw(graph));

   graph_pc_2transcheck(scip, graph);

   SCIP_CALL( computeSteinerTreeRedCostsPcMw(scip, graph, pool, cost, upperbound, result, result2, pathedge, nodearrchar, apsol) );

   /* does result not contain a valid solution? */
   if( !(*apsol) )
      BMScopyMemoryArray(result, result2, nedges);

   SCIPdebugMessage("DA: pertubated sol value %f \n", *upperbound);

   /* compute guiding solution */

   BMScopyMemoryArray(transresult, result, nedges);

   for( e = nedges; e < transnedges; e++ )
       transresult[e] = UNKNOWN;

   /* non-trivial solution */
   for( e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
      if( Is_term(graph->term[graph->head[e]]) && result[e] != CONNECT )
         break;

   if( e != EAT_LAST)
   {
      int k;
      for( e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
         if( !Is_term(graph->term[graph->head[e]]) && result[e] == CONNECT )
            break;

      assert(e != EAT_LAST);
      k = graph->head[e];

      for( e = transgraph->outbeg[k]; e != EAT_LAST; e = transgraph->oeat[e] )
      {
         if( transgraph->head[e] == graph->knots )
         {
            transresult[e] = CONNECT;
            break;
         }
      }
   }

   assert(!(*apsol) || solstp_isValid(scip, graph, result));
   assert(graph_valid(scip, transgraph));
   assert(root == transgraph->source);

   {
      DAPARAMS daparams = { .addcuts = FALSE, .ascendandprune = FALSE, .root = root,
                  .is_pseudoroot = TRUE, .damaxdeviation = -1.0 };

      SCIP_CALL( dualascent_exec(scip, transgraph, transresult, &daparams, cost, &lb) );
   }

   lb += offset;
   *lpobjval = lb;

   SCIP_CALL( computeSteinerTreeRedCostsPcMw(scip, graph, pool, cost, upperbound, result, result2, pathedge, nodearrchar, apsol) );

   assert(!(*apsol) || solstp_isValid(scip, graph, result));

   if( SCIPisGE(scip, lb, *bestlpobjval) )
   {
      *bestlpobjval = lb;
      BMScopyMemoryArray(bestcost, cost, extnedges);
   }

   /* the required reduced path cost to be surpassed */
   *minpathcost = *upperbound - *lpobjval;

   return SCIP_OKAY;
}

/** compute Voronoi region for dual-ascent elimination for PC/MW */
static
void computeTransVoronoi(
    SCIP* scip,
    GRAPH* transgraph,
    PATH* vnoi,
    const SCIP_Real* cost,
    SCIP_Real* costrev,
    SCIP_Real* pathdist,
    int* vbase,
    int* pathedge
)
{
   const int root = transgraph->source;
   const int transnnodes = transgraph->knots;
   const int transnedges = transgraph->edges;

   for( int k = 0; k < transnnodes; k++ )
      transgraph->mark[k] = (transgraph->grad[k] > 0);

   /* distance from root to all nodes */
   graph_path_execX(scip, transgraph, root, cost, pathdist, pathedge);

   for( int k = 0; k < transnnodes; k++ )
      if( Is_term(transgraph->term[k]) )
         assert(SCIPisEQ(scip, pathdist[k], 0.0));

   for( int e = 0; e < transnedges; e++ )
      costrev[e] = cost[flipedge(e)];

   /* no paths should go back to the root */
   for( int e = transgraph->outbeg[root]; e != EAT_LAST; e = transgraph->oeat[e] )
      costrev[e] = FARAWAY;

   /* build Voronoi diagram wrt incoming arcs */
   graph_add1stTermPaths(transgraph, costrev, vnoi, vbase, transgraph->path_state);
}


/** reduce graph with non-artificial root (SPG, RPC ...) based on information from dual ascent and given upper bound  */
static
SCIP_RETCODE reduceRootedProb(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   STP_Bool*             marked,             /**< edge array to mark which (directed) edge can be removed */
   const REDCOST*        redcostdata,        /**< reduced cost data */
   const int*            result,             /**< sol int array */
   SCIP_Bool             solgiven,           /**< is sol given? */
   int*                  nfixedp             /**< number of fixed edges pointer */
)
{
   const PATH *vnoi = redcosts_getNodeToTermsPathsTop(redcostdata);
   const SCIP_Real *cost = redcosts_getEdgeCostsTop(redcostdata);
   const SCIP_Real *pathdist = redcosts_getRootToNodeDistTop(redcostdata);
   const SCIP_Real cutoffbound = redcosts_getCutoffTop(redcostdata);
   int* incidents = NULL;
   STP_Bool* nodearrchar = NULL;
   const int nnodes = graph->knots;
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(graph);
   const SCIP_Bool keepsol = (solgiven && SCIPisZero(scip, cutoffbound));
   const SCIP_Bool isDirected = !graph_typeIsUndirected(graph);

   assert(GE(cutoffbound, 0.0));

   if( isRpcmw )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &incidents, nnodes) );

#ifndef NDEBUG
      assert(!graph->extended);
      for( int k = 0; k < nnodes; k++ )
      {
         if( Is_pseudoTerm(graph->term[k]) )
            assert(!graph->mark[k] && graph->grad[k] == 2);
         else
            assert(graph->mark[k] || graph->grad[k] == 0);
      }
#endif
   }

   if( solgiven )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &nodearrchar, nnodes) );
      solstp_setVertexFromEdge(graph, result, nodearrchar);
   }

   /* main loop: try to reduce */
   for( int k = 0; k < nnodes; k++ )
   {
      SCIP_Real redcost;
      int e;

      if( !graph->mark[k] )
         continue;

      assert(!Is_pseudoTerm(graph->term[k]));

      redcost = pathdist[k] + vnoi[k].dist;

      if( isRpcmw && Is_term(graph->term[k]) && !graph_pc_knotIsFixedTerm(graph, k) && !graph_pc_termIsNonLeafTerm(graph, k)
         && SCIPisGT(scip, redcost, cutoffbound) )
      {
         daRpcmwDeleteTermIncidents(scip, vnoi, k, graph, incidents, nfixedp);
         continue;
      }

      /* note: if we want to keep the solution we cannot just delete vertices */
      if( !Is_term(graph->term[k]) && !keepsol &&
         (SCIPisFeasGT(scip, redcost, cutoffbound) || (solgiven && SCIPisEQ(scip, redcost, cutoffbound) && !nodearrchar[k])) )
      {
         (*nfixedp) += graph->grad[k];
         graph_knot_del(scip, graph, k, TRUE);
         continue;
      }

      e = graph->outbeg[k];
      while( e != EAT_LAST )
      {
         const int head = graph->head[e];
         const int enext = graph->oeat[e];

         /* for rpc no artificial terminal arcs should be deleted */
         if( (isRpcmw && !graph->mark[head])
          || (keepsol && (result[e] == CONNECT || result[flipedge(e)] == CONNECT)) )
         {
            e = enext;
            continue;
         }

         redcost = pathdist[k] + cost[e] + vnoi[head].dist;

         if( SCIPisGT(scip, redcost, cutoffbound)
            || (solgiven && SCIPisEQ(scip, redcost, cutoffbound) && result[e] != CONNECT && result[flipedge(e)] != CONNECT) )
         {
            if( marked[flipedge(e)] )
            {
               graph_edge_del(scip, graph, e, TRUE);
               (*nfixedp)++;
            }
            else
            {
               marked[e] = TRUE;
               if( isDirected )
                  graph->cost[e] = FARAWAY;
            }
         }
         e = enext;
      }
   }

   SCIPfreeBufferArrayNull(scip, &nodearrchar);
   SCIPfreeBufferArrayNull(scip, &incidents);

   return SCIP_OKAY;
}

/** reduce PCSTP or MWCS graph based on information from dual ascent and given upper bound  */
static
int reducePcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   GRAPH*                transgraph,         /**< graph data structure */
   const PATH*           vnoi,               /**< Voronoi data structure */
   const SCIP_Real*      cost,               /**< dual ascent costs */
   const SCIP_Real*      pathdist,           /**< distance array from shortest path calculations */
   SCIP_Real             minpathcost,        /**< the required reduced path cost to be surpassed */
   const int*            result,             /**< sol int array */
   STP_Bool*             marked,             /**< edge array to mark which (directed) edge can be removed */
   STP_Bool*             nodearrchar,        /**< node array storing solution vertices */
   SCIP_Bool             solgiven,           /**< is sol given? */
   SCIP_Bool             deleteTransEdges    /**< delete edges of transformed graph */
)
{
   const int nnodes = graph_get_nNodes(graph);
   int nfixed;
   SCIP_Real tmpcost;
   SCIP_Bool keepsol = FALSE;
   STP_Vectype(int) edges_delete = NULL;

   assert(SCIPisGE(scip, minpathcost, 0.0));

   if( minpathcost < 0.0 )
      minpathcost = 0.0;

   nfixed = 0;

   graph_pc_2orgcheck(scip, graph);

   if( solgiven )
   {
      assert(solstp_isValid(scip, graph, result));

      solstp_setVertexFromEdge(graph, result, nodearrchar);

      if( SCIPisZero(scip, minpathcost) )
         keepsol = TRUE;
   }

   /* try to eliminate vertices and edges */
   for( int k = 0; k < nnodes; k++ )
   {
      if( !graph->mark[k] )
         continue;

      assert(!Is_pseudoTerm(graph->term[k]));

      if( Is_term(graph->term[k]) )
      {
         int e = graph->outbeg[k];
         while( e != EAT_LAST )
         {
            const int etmp = graph->oeat[e];
            tmpcost = pathdist[k] + cost[e] + vnoi[graph->head[e]].dist;

            if( graph->mark[graph->head[e]] &&
               ((SCIPisGT(scip, tmpcost, minpathcost) && (!keepsol || (result[e] != CONNECT && result[flipedge(e)] != CONNECT))) ||
                  (solgiven && tmpcost >= minpathcost && result[e] != CONNECT && result[flipedge(e)] != CONNECT)) )
            {
               if( marked[flipedge(e)] )
               {
                  assert(!Is_pseudoTerm(graph->term[graph->head[e]]));

                  StpVecPushBack(scip, edges_delete, e);

           //       graph_edge_del(scip, graph, e, TRUE);
           //       graph_edge_del(scip, transgraph, e, FALSE);
               }
               else
               {
                  marked[e] = TRUE;
               }
            }
            e = etmp;
         }
         continue;
      }

      tmpcost = pathdist[k] + vnoi[k].dist;

      if( (SCIPisGT(scip, tmpcost, minpathcost) && !keepsol) ||
         (solgiven && tmpcost >= minpathcost && !nodearrchar[k]))
      {
         for( int e = transgraph->outbeg[k]; e != EAT_LAST; e = transgraph->oeat[e] )
         {
            StpVecPushBack(scip, edges_delete, e);

            //graph_edge_del(scip, transgraph, e, FALSE);
            //graph_edge_del(scip, graph, e, TRUE);
         }
      }
      else
      {
         int e = transgraph->outbeg[k];
         while( e != EAT_LAST )
         {
            const int etmp = transgraph->oeat[e];
            tmpcost = pathdist[k] + cost[e] + vnoi[transgraph->head[e]].dist;

            if( (SCIPisGT(scip, tmpcost, minpathcost) && (!keepsol || (result[e] != CONNECT && result[flipedge(e)] != CONNECT))) ||
               (solgiven && tmpcost >= minpathcost && result[e] != CONNECT && result[flipedge(e)] != CONNECT) )
            {
               if( marked[flipedge(e)] )
               {
                //  graph_edge_del(scip, graph, e, TRUE);
                //  graph_edge_del(scip, transgraph, e, FALSE);
                  StpVecPushBack(scip, edges_delete, e);
               }
               else
               {
                  marked[e] = TRUE;
               }
            }
            e = etmp;
         }
      }
   }

   for( int i = 0; i < StpVecGetSize(edges_delete); i++ )
   {
      const int e = edges_delete[i];

      if( graph->oeat[e] == EAT_FREE )
         continue;

      graph_edge_del(scip, graph, e, TRUE);
      nfixed++;

      if( deleteTransEdges )
         graph_edge_del(scip, transgraph, e, FALSE);
   }

   StpVecFree(scip, edges_delete);
   SCIPdebugMessage("DA: eliminations %d \n", nfixed);

   return nfixed;
}

/** try to run reduction method for best known reduced costs (if they are valid) */
static
int reducePcMwTryBest(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   GRAPH*                transgraph,         /**< graph data structure */
   PATH*                 vnoi,               /**< Voronoi data structure */
   const SCIP_Real*      cost,               /**< dual ascent costs */
   SCIP_Real*            costrev,            /**< SCIP_Real array */
   SCIP_Real*            bestcost,           /**< best dual ascent costs */
   SCIP_Real*            pathdist,           /**< distance array from shortest path calculations */
   SCIP_Real*            upperbound,         /**< upper bound */
   SCIP_Real*            lpobjval,           /**< reduced cost value */
   SCIP_Real*            bestlpobjval,       /**< best reduced cost value */
   SCIP_Real*            minpathcost,        /**< the required reduced path cost to be surpassed */
   SCIP_Real             oldupperbound,      /**< old upper bound */
   const int*            result,             /**< sol int array */
   int*                  vbase,              /**< array for Voronoi bases */
   int*                  state,              /**< int 4 * nnodes array for internal computations */
   int*                  pathedge,           /**< array for predecessor edge on a path */
   STP_Bool*             marked,             /**< edge array to mark which (directed) edge can be removed */
   STP_Bool*             nodearrchar,        /**< node array storing solution vertices */
   SCIP_Bool*            solgiven,           /**< is sol given? */
   int                   extnedges           /**< number of edges for transgraph */
)
{
   const SCIP_Bool dualisvalid = daRedCostIsValid(scip, transgraph, bestcost, state, nodearrchar);

   if( !dualisvalid )
   {
      *bestlpobjval = *lpobjval;
      BMScopyMemoryArray(bestcost, cost, extnedges);
   }

   /* has upperbound and changed, and is best reduced cost valid and different from cost? */
   if( SCIPisGT(scip, *bestlpobjval, *lpobjval) && SCIPisLT(scip, *upperbound, oldupperbound) )
   {
      *solgiven = *solgiven && solstp_isUnreduced(scip, graph, result);

      assert(!(*solgiven) || solstp_isValid(scip, graph, result));

      *minpathcost = *upperbound - *bestlpobjval;
      assert(SCIPisGE(scip, *minpathcost, 0.0));

      computeTransVoronoi(scip, transgraph, vnoi, bestcost, costrev, pathdist, vbase, pathedge);

      return reducePcMw(scip, graph, transgraph, vnoi, bestcost, pathdist, *minpathcost, result, marked, nodearrchar, *solgiven, TRUE);
   }
   return 0;
}

/** promising? */
static
SCIP_Bool dapathsIsPromising(
   const GRAPH*                g             /**< graph data structure */
)
{
   SCIP_Bool isPromising;
   int nterms;
   int nnodes;

   assert(g);

   graph_get_nVET(g, &nnodes, NULL, &nterms);

   isPromising = ((SCIP_Real) nterms / (SCIP_Real) nnodes < 0.1);

   return isPromising;
}


/** deletes edges */
static
SCIP_RETCODE dapathsDeleteEdges(
   SCIP*                 scip,               /**< SCIP data structure */
   const REDCOST*        redcostdata,        /**< reduced cost data */
   const int*            result,             /**< solution */
   GRAPH*                g,                  /**< graph data structure */
   int*                  nelims              /**< pointer to store number of reduced edges */
   )
{
   STP_Bool* edges_isDeletable;

   const int nedges = graph_get_nEdges(g);

   SCIP_CALL( SCIPallocBufferArray(scip, &edges_isDeletable, nedges) );
   BMSclearMemoryArray(edges_isDeletable, nedges);

   SCIP_CALL( reduceRootedProb(scip, g, edges_isDeletable, redcostdata, result, TRUE, nelims) );

   SCIPfreeBufferArray(scip, &edges_isDeletable);

   return SCIP_OKAY;
}


/** pseudo-eliminates nodes */
static
SCIP_RETCODE dapathsReplaceNodes(
   SCIP*                 scip,               /**< SCIP data structure */
   REDCOST*              redcostdata,        /**< reduced cost data */
   const int*            result,             /**< solution */
   SCIP_Real             objbound_upper,     /**< objective */
   GRAPH*                g,                  /**< graph data structure */
   SCIP_Real*            offsetp,            /**< pointer to store offset */
   int*                  nelims              /**< pointer to store number of reduced edges */
   )
{
   SCIP_Real* nodereplacebounds;
   SCIP_Bool* pseudoDelNodes;
   int nreplacings = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &nodereplacebounds, g->knots) );
   SCIP_CALL( SCIPallocBufferArray(scip, &pseudoDelNodes, g->knots) );

   SCIP_CALL( updateNodeReplaceBounds(scip, redcostdata, g, nodereplacebounds, objbound_upper, TRUE, FALSE));
   markPseudoDeletablesFromBounds(scip, g, nodereplacebounds, objbound_upper, pseudoDelNodes);
   SCIP_CALL( reduce_applyPseudoDeletions(scip, pseudoDelNodes, redcostdata, g, offsetp, &nreplacings) );

   SCIPfreeBufferArray(scip, &pseudoDelNodes);
   SCIPfreeBufferArray(scip, &nodereplacebounds);

   *nelims += nreplacings;

   return SCIP_OKAY;
}

/** eliminates RPC/RMW potential-terminals */
static
SCIP_RETCODE dapathsFixPotTerms(
   SCIP*                 scip,               /**< SCIP data structure */
   const REDCOST*        redcostdata,        /**< reduced cost data */
   GRAPH*                g,                  /**< graph data structure */
   SCIP_Real*            offsetp,            /**< pointer to store offset */
   int*                  nelims              /**< pointer to store number of reduced edges */
   )
{
   int* fixlist;
   const SCIP_Real* const redEdgeCost = redcosts_getEdgeCostsTop(redcostdata);
   const SCIP_Real cutoff = redcosts_getCutoffTop(redcostdata);
   int nfixes = 0;
   const int root = g->source;

   assert(graph_pc_isRootedPcMw(g));
   assert(!g->extended);

   SCIP_CALL( SCIPallocBufferArray(scip, &fixlist, g->terms) );

   for( int e = g->outbeg[root]; e != EAT_LAST; e = g->oeat[e] )
   {
      const int head = g->head[e];

      if( graph_pc_knotIsDummyTerm(g, head) )
      {
         if( GT(redEdgeCost[e], cutoff) )
         {
            assert(nfixes < g->terms);

            fixlist[nfixes++] = head;
         }
      }
   }

   for( int i = 0; i < nfixes; ++i )
   {
      const int pterm = fixlist[i];
      const int twin = graph_pc_getTwinTerm(g, pterm);
      assert(g->grad[pterm] == 2);
      assert(Is_term(g->term[twin]));

#ifdef SCIP_DEBUG
      printf("dapath fixes: ");
      graph_knot_printInfo(g, twin);
#endif

      graph_pc_knotToFixedTerm(scip, g, twin, offsetp);
   }

   *nelims += nfixes;

   SCIPfreeBufferArray(scip, &fixlist);

   return SCIP_OKAY;
}


/** dual ascent path based reductions */
SCIP_RETCODE reduce_dapaths(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   SCIP_Real*            offsetp,            /**< pointer to store offset */
   int*                  nelims              /**< pointer to store number of reduced edges */
   )
{
   REDCOST* redcostdata;
   const int nedges = graph_get_nEdges(g);
   int* RESTRICT result;
   SCIP_Real objbound_upper = FARAWAY;
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(g);
   SCIP_Real dualobjval = -1.0;
   SCIP_Bool solFound;

   assert(scip && offsetp && nelims);

   *nelims = 0;

   if( g->terms <= 2 || !dapathsIsPromising(g) )
      return SCIP_OKAY;

   SCIP_CALL( SCIPallocBufferArray(scip, &result, nedges) );

   graph_mark(g);
   SCIP_CALL( redcosts_init(scip, g->knots, nedges, FARAWAY, g->source, &redcostdata) );
   SCIP_CALL( computeSteinerTreeTM(scip, g, result, &objbound_upper, &solFound) );
   SCIP_CALL( dualascent_paths(scip, g, redcosts_getEdgeCostsTop(redcostdata), &dualobjval, result) );
   redcosts_setDualBoundTop(dualobjval, redcostdata);
   SCIP_CALL( redcosts_initializeDistancesTop(scip, g, redcostdata) );

   assert(graph_isMarked(g));

   redcosts_setCutoffFromBoundTop(objbound_upper, redcostdata);
   SCIP_CALL( dapathsDeleteEdges(scip, redcostdata, result, g, nelims) );

   if( *nelims > 0 && solstp_isUnreduced(scip, g, result) )
   {
      if( isRpcmw )
         graph_pc_2trans(scip, g);

      SCIP_CALL( SCIPStpHeurLocalRun(scip, g, result) );
      objbound_upper = solstp_getObj(g, result, 0.0);
      redcosts_setCutoffFromBoundTop(objbound_upper, redcostdata);

      if( isRpcmw )
         graph_pc_2org(scip, g);
      else
         graph_mark(g);

      assert(graph_isMarked(g));

      SCIP_CALL( dapathsDeleteEdges(scip, redcostdata, result, g, nelims) );
   }

   SCIP_CALL( dapathsReplaceNodes(scip, redcostdata, result, objbound_upper, g, offsetp, nelims) );

   if( graph_pc_isRootedPcMw(g) )
   {
      SCIP_CALL( dapathsFixPotTerms(scip, redcostdata, g, offsetp, nelims) );
   }

   redcosts_free(scip, &redcostdata);
   SCIPfreeBufferArray(scip, &result);

   SCIP_CALL( reduce_unconnected(scip, g) );

   assert(!graph_pc_isPcMw(g) || !g->extended);
   assert(graph_valid(scip, g));

   return SCIP_OKAY;
}

/** dual ascent based reductions */
SCIP_RETCODE reduce_da(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const RPDA*           paramsda,           /**< parameters */
   REDSOLLOCAL*          redsollocal,        /**< primal bound info or NULL */
   SCIP_Real*            offsetp,            /**< pointer to store offset */
   int*                  nelims,             /**< pointer to store number of reduced edges */
   SCIP_RANDNUMGEN*      randnumgen          /**< random number generator */
)
{
   EXTPERMA* extpermanent = NULL;
   STPSOLPOOL* pool = NULL;
   SCIP_Real* edgefixingbounds = NULL;
   SCIP_Real* nodefixingbounds = NULL;
   SCIP_Real* nodereplacebounds = NULL;
   SCIP_Real upperbound;
   const SCIP_Bool isRpc = (graph->stp_type == STP_RPCSPG);
   const SCIP_Bool isRpcmw = graph_pc_isRootedPcMw(graph);
   const SCIP_Bool isDirected = graph_typeIsDirected(graph) || (graph->stp_type == STP_DCSTP);
   int* terms;
   int* result;
   int* bestresult;
   SCIP_Bool* pseudoDelNodes;
   int nFixedTerms;
   const int nedges = graph->edges;
   const int nnodes = graph->knots;
   STP_Bool* arcsdeleted;
   SCIP_Bool useExtRed = (paramsda->extredMode != extred_none);
   const SCIP_Bool nodereplacing = paramsda->nodereplacing;
   const SCIP_Bool userec = paramsda->useRec;
   SCIP_Bool havebestsol = FALSE;
   const SCIP_Bool useHops = (paramsda->damode == STP_DAMODE_HOPS);    // todo do that properly

   assert(scip && graph && nelims);
   assert(graph_valid_ancestors(scip, graph) && graph_valid(scip, graph));
   assert(!isRpcmw || !graph->extended);
   assert(!(useExtRed && graph_pc_isMw(graph)));

   if( graph->terms <= 1 )
      return SCIP_OKAY;

#ifdef STP_RPC_FIXEDPROPER
   assert(0 && "check");
   if( isRpcmw )
      return SCIP_OKAY;
#endif

   SCIP_CALL( SCIPallocBufferArray(scip, &pseudoDelNodes, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &result, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &bestresult, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &arcsdeleted, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &edgefixingbounds, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodefixingbounds, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodereplacebounds, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &terms, graph->terms) );

   if( userec )
      SCIP_CALL( solpool_init(scip, &pool, nedges, SOLPOOL_SIZE) );

   if( isRpc )
      reduce_identifyNonLeafTerms(scip, graph);

   if( isDirected )
   {
      for( int e = 0; e < nedges; e++ )
         arcsdeleted[e] = GE(graph->cost[e], FARAWAY);
   }

   upperbound = FARAWAY;

   if( redsollocal )
   {
      reduce_sollocalSetOffset(*offsetp, redsollocal);
      if( isRpcmw )
         SCIP_CALL( reduce_sollocalRebuildTry(scip, graph, redsollocal) );

      upperbound = reduce_sollocalGetUpperBound(redsollocal);
   }

   if( useHops )
   {
      assert(graph->stp_type == STP_DHCSTP);
      upperbound = (SCIP_Real) graph->hoplimit;
   }

   graph_mark(graph);
   collectFixedTerminals(graph, terms, &nFixedTerms);

   if( isRpcmw )
      graph_pc_2trans(scip, graph);

   /* select roots for dual ascent */
   SCIP_CALL( daOrderRoots(scip, graph, terms, nFixedTerms, TRUE, randnumgen) );

   {
      REDCOST* redcostdata;
	   const int nruns = daGetNruns(graph, paramsda, nFixedTerms);
      SCIP_Real cutoffbound = -1.0;
      havebestsol = FALSE;

      SCIP_CALL( daRedcostsInit(scip, graph, paramsda, nruns, &redcostdata) );

      /* main reduction loop */
      for( int run = 0; run < nruns && graph->grad[graph->source] > 0; run++ )
      {
         int ndeletions_run = 0;
         const SCIP_Real damaxdeviation = daGetMaxDeviation(paramsda, randnumgen);
         SCIPdebugMessage("new DA root %d in round %d \n", terms[run], run);

         if( useExtRed && run > 0 )
            redcosts_addLevel(redcostdata);

         redcosts_setRootTop(terms[run], redcostdata);

     //   if( rpc ) {      // int todo; // check for more terminals to be added    }
         if( !useHops && daGuidedIsPromising(graph, run, randnumgen) )
         {
            /* run dual-ascent (and possibly re-root solution stored in 'result') */
            SCIP_CALL( computeDualSolutionGuided(scip, graph, damaxdeviation, redcostdata, result) );
         }
         else
         {
            SCIP_CALL( computeDualSolution(scip, graph, damaxdeviation, redcostdata) );
         }

         if( isDirected )
         {
            if( !useHops )
               SCIP_CALL( computeSteinerTreeRedCostsDirected(scip, graph, redcostdata, result, bestresult, &havebestsol, &upperbound) );
         }
         else
         {
            if( useExtRed || run < 2 )
            {
               const SCIP_Bool useSlackPrune = (run == 1 && paramsda->useSlackPrune);
               SCIP_CALL( computeSteinerTreeRedCosts(scip, graph, redcostdata, useSlackPrune,
                        userec, pool, result, bestresult, &havebestsol, &upperbound) );
            }
            else
            {
               havebestsol = havebestsol && solstp_isUnreduced(scip, graph, bestresult);
            }
         }

         // todo remove
         if( SCIPisFeasGT(scip, redcosts_getDualBoundTop(redcostdata), upperbound) )
         {
            printf("WRONG BOUND: upper=%f lower=%f (round=%d havenewsol=%d)\n", upperbound, redcosts_getDualBoundTop(redcostdata), run, havebestsol);
            return SCIP_ERROR;
         }

         SCIPdebugMessage("upper=%f lower=%f (round=%d havenewsol=%d)\n", upperbound, redcosts_getDualBoundTop(redcostdata), run, havebestsol);

         SCIP_CALL( daRoundInit(scip, upperbound, graph, redcostdata, arcsdeleted, &cutoffbound) );

         updateNodeFixingBounds(nodefixingbounds, graph, redcosts_getRootToNodeDistTop(redcostdata),
               redcosts_getNodeToTermsPathsTop(redcostdata), redcosts_getDualBoundTop(redcostdata), (run == 0));
         updateEdgeFixingBounds(edgefixingbounds, graph, redcosts_getEdgeCostsTop(redcostdata),
               redcosts_getRootToNodeDistTop(redcostdata), redcosts_getNodeToTermsPathsTop(redcostdata), redcosts_getDualBoundTop(redcostdata),
               nedges, (run == 0), TRUE);

         SCIP_CALL( reduceRootedProb(scip, graph, arcsdeleted, redcostdata, bestresult, havebestsol, &ndeletions_run) );
         ndeletions_run += reduceWithNodeFixingBounds(scip, graph, NULL, nodefixingbounds, upperbound);
         havebestsol = havebestsol && solstp_isUnreduced(scip, graph, bestresult);
         ndeletions_run += reduceWithEdgeFixingBounds(scip, graph, NULL, edgefixingbounds, (havebestsol ? bestresult : NULL), upperbound);

         if( useExtRed )
            redcosts_increaseOnDeletedArcsTop(graph, arcsdeleted, redcostdata);

         if( useExtRed && run == nruns - 1 )
         {
            const SCIP_Bool useSd = !graph_pc_isPc(graph);
            SCIP_CALL( extreduce_init(scip, useSd, paramsda->extredMode, graph, redcostdata, &extpermanent) );
            extreduce_extPermaAddRandnumgen(randnumgen, extpermanent);
            havebestsol = havebestsol && solstp_isUnreduced(scip, graph, bestresult);

            extpermanent->solIsValid = havebestsol;
            extpermanent->result = havebestsol ? bestresult : NULL;

            SCIP_CALL( reduceWithEdgeExtReds(scip, upperbound, extpermanent, graph, &ndeletions_run) );
         }
         else if( !isDirected && nodereplacing )
         {
            SCIP_CALL( updateNodeReplaceBounds(scip, redcostdata, graph, nodereplacebounds, upperbound, (run == 0), FALSE));
         }

         daRoundExit(scip, ndeletions_run, graph, nelims);
      } /* root loop */

      if( redsollocal )
      {
         reduce_sollocalSetOffset(*offsetp, redsollocal);
         reduce_sollocalUpdateUpperBound(upperbound, redsollocal);

         if( reduce_sollocalUsesNodesol(redsollocal) )
         {
            havebestsol = havebestsol && solstp_isUnreduced(scip, graph, bestresult);
            if( havebestsol )
            {
               SCIP_Bool isinfeas;
               SCIP_CALL(solstp_rerootInfeas(scip, graph, bestresult, graph->source, &isinfeas));

               if( !isinfeas )
               {
                  SCIP_CALL( reduce_sollocalUpdateNodesol(scip, bestresult, graph, redsollocal) );
               }
            }
         }
      }

      /* do pseudo-elimination? */
      if( !isDirected && !SCIPisZero(scip, cutoffbound) && nodereplacing )
      {
         int nreplacings = 0;
         assert(!graph_pc_isMw(graph));

         markPseudoDeletablesFromBounds(scip, graph, nodereplacebounds, upperbound, pseudoDelNodes);

         if( useExtRed )
         {
            SCIP_CALL( extreduce_pseudoDeleteNodes(scip,
                  pseudoDelNodes, extpermanent, graph, offsetp, &nreplacings) );
         }
         else
         {
            SCIP_CALL( reduce_applyPseudoDeletions(scip, pseudoDelNodes, redcostdata, graph, offsetp, &nreplacings) );
         }
         *nelims += nreplacings;

         if( nreplacings > 0 && userec )
         {
            /* solutions in pool might not be valid anymore */
            solpool_free(scip, &pool);
            SCIP_CALL(solpool_init(scip, &pool, nedges, SOLPOOL_SIZE));
         }

         assert(graph_valid_ancestors(scip, graph));
         graph_mark(graph);
      }

      if( extpermanent )
      {
         extreduce_exit(scip, graph, &extpermanent);
      }

      daRedcostsExit(scip, &redcostdata);
   }

   if( isRpcmw )
      graph_pc_2orgcheck(scip, graph);

   if( userec )
      solpool_free(scip, &pool);

   SCIPfreeBufferArray(scip, &terms);
   SCIPfreeBufferArray(scip, &nodereplacebounds);
   SCIPfreeBufferArray(scip, &nodefixingbounds);
   SCIPfreeBufferArray(scip, &edgefixingbounds);
   SCIPfreeBufferArray(scip, &arcsdeleted);
   SCIPfreeBufferArray(scip, &bestresult);
   SCIPfreeBufferArray(scip, &result);
   SCIPfreeBufferArray(scip, &pseudoDelNodes);

   assert(graph_valid(scip, graph));

   return SCIP_OKAY;
}


/** dual ascent reduction for slack-and-prune heuristic */
SCIP_RETCODE reduce_daSlackPrune(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   int                   minelims,           /**< minimum number of edges to eliminate */
   SCIP_Bool             solgiven,           /**< solution provided? */
   int*                  soledge,            /**< solution edges (in/out) */
   int*                  solnode,            /**< array of nodes of current solution that is not to be destroyed (in/out) */
   int*                  nelims,             /**< pointer to store number of reduced edges */
   SCIP_Real*            upperbound,         /**< pointer to store new upper bound */
   SCIP_Bool*            solImproved,        /**< solution improved? */
   SCIP_Bool*            solReconstructed    /**< solution reconstructed? */
   )
{
   PATH *vnoi;
   SCIP_Real *cost;
   SCIP_Real *costrev;
   SCIP_Real *pathdist;
   int *state;
   int *vbase;
   int *edgearrint2;
   int *pathprededge;
   SCIP_Real obj;
   SCIP_Real tmpcost;
   SCIP_Real lpobjval;
   SCIP_Real objprune;
   SCIP_Real minpathcost;
   SCIP_Bool success;
   SCIP_Bool eliminate;
   int k;
   int e;
   int etmp;
   int head;
   const int nedges = graph_get_nEdges(graph);
   const int nnodes = graph_get_nNodes(graph);
   const int root = graph->source;
   const int* grad = graph->grad;
   const SCIP_Bool rpcmw = graph_pc_isRootedPcMw(graph);
   SCIP_Bool solutionIsRebuilt = FALSE;
   int nfixed = 0;
   int redrounds;
   STP_Bool* marked;

   assert(scip && graph && nelims && solnode && soledge);
   assert(solReconstructed && solImproved);
   assert(!graph_pc_isPcMw(graph) || rpcmw);

   /* 1. step: initialize */

   *solReconstructed = FALSE;
   *solImproved = FALSE;

   if( graph->terms <= 2 )
      goto TERMINATE;

   SCIP_CALL( SCIPallocBufferArray(scip, &cost, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &costrev, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &marked, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &state, 4 * nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &pathdist, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &vbase, 4 * nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &pathprededge, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &edgearrint2, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &vnoi, 4 * nnodes) );

   /* 2. step: - if not provided, compute lower bound and reduced costs
    *          - try to eliminate edges and nodes                        */

   assert(!rpcmw || graph->extended);

   graph_mark(graph);

   if( !solgiven )
   {
      solutionIsRebuilt = TRUE;

      /* try to build MST on solnode nodes */
      for( int i = 0; i < nnodes; i++ )
         graph->mark[i] = (solnode[i] == CONNECT);

      assert(graph->mark[graph->source]);

      for( e = 0; e < nedges; e++ )
         soledge[e] = UNKNOWN;

      graph_path_exec(scip, graph, MST_MODE, root, graph->cost, vnoi);

      for( int i = 0; i < nnodes; i++ )
      {
         e = vnoi[i].edge;
         if( e >= 0 )
         {
            soledge[e] = CONNECT;
         }
         else if( Is_term(graph->term[i]) && i != root )
         {
            solutionIsRebuilt = FALSE;
            break;
         }
      }

      if( solutionIsRebuilt )
      {
         int count;

         do
         {
            count = 0;

            for( int i = nnodes - 1; i >= 0; --i )
            {
               if( (solnode[i] != CONNECT) || Is_term(graph->term[i]) )
                  continue;

               for( e = graph->outbeg[i]; e != EAT_LAST; e = graph->oeat[e] )
                  if( soledge[e] == CONNECT )
                     break;

               if( e == EAT_LAST )
               {
                  /* there has to be exactly one incoming edge */
                  for( e = graph->inpbeg[i]; e != EAT_LAST; e = graph->ieat[e] )
                  {
                     if( soledge[e] == CONNECT )
                     {
                        soledge[e] = UNKNOWN;
                        solnode[i] = UNKNOWN;
                        count++;
                        break;
                     }
                  }
               }
            }
         }
         while( count > 0 );

         solutionIsRebuilt = solstp_isValid(scip, graph, soledge);
      }
   }

   *solReconstructed = solutionIsRebuilt;
   SCIPdebugMessage("solutionIsRebuilt=%d \n", solutionIsRebuilt);

   if( solgiven || solutionIsRebuilt )
   {
      DAPARAMS daparams = { .addcuts = FALSE, .ascendandprune = FALSE, .root = root,
                  .is_pseudoroot = FALSE, .damaxdeviation = -1.0 };
      obj = getSolObj(scip, graph, soledge);

      SCIP_CALL( dualascent_exec(scip, graph, soledge, &daparams, cost, &lpobjval) );
   }
   else
   {
      DAPARAMS daparams = { .addcuts = FALSE, .ascendandprune = FALSE, .root = root,
                  .is_pseudoroot = FALSE, .damaxdeviation = -1.0 };
      obj = FARAWAY;

      SCIP_CALL( dualascent_exec(scip, graph, NULL, &daparams, cost, &lpobjval) );
   }

   assert(dualascent_allTermsReachable(scip, graph, root, cost));

   SCIP_CALL( SCIPStpHeurAscendPruneRun(scip, NULL, graph, cost, edgearrint2, root, &success, FALSE) );
   objprune = getSolObj(scip, graph, edgearrint2);

   assert(success);

   if( success && SCIPisLT(scip, objprune, obj ) )
   {
      *solImproved = TRUE;

      for( int i = 0; i < nnodes; i++ )
         solnode[i] = UNKNOWN;

      for( e = 0; e < nedges; e++ )
      {
         soledge[e] = edgearrint2[e];
         if( soledge[e] == CONNECT )
         {
            solnode[graph->tail[e]] = CONNECT;
            solnode[graph->head[e]] = CONNECT;
         }
      }
   }

   obj = getSolObj(scip, graph, soledge);

   for( e = 0; e < nedges; e++ )
   {
      marked[e] = FALSE;
      costrev[e] = cost[flipedge(e)];
   }

   *upperbound = obj;
   graph_mark(graph);

   /* distance from root to all nodes */
   graph_path_execX(scip, graph, root, cost, pathdist, pathprededge);

   /* no paths should go back to the root */
   for( e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
      costrev[e] = FARAWAY;

   /* build Voronoi diagram */
   graph_get2nextTermPaths(graph, costrev, costrev, vnoi, vbase, state);

#ifndef NDEBUG
   for( k = 0; k < nnodes; k++ )
      if( !Is_term(graph->term[k]) )
         assert(vbase[k + nnodes] != root );
#endif

   /* RPC? If yes, restore original graph */
   if( rpcmw )
   {
      graph_pc_2org(scip, graph);
      graph->mark[root] = FALSE;
   }

   for( e = 0; e < nedges; e++ )
      costrev[e] = -1.0;

   for( redrounds = 0; redrounds < 3; redrounds++ )
   {
      if( redrounds == 0 )
      {
         eliminate = FALSE;
         minpathcost = FARAWAY;
      }
      else if( redrounds == 1 )
      {
         assert(minelims > 0);
         assert(2 * minelims < nedges);

         eliminate = TRUE;
         SCIPsortReal(costrev, nedges);

         /* the required reduced path cost to be surpassed */
         minpathcost = costrev[nedges - 2 * minelims];

         if( SCIPisLE(scip, minpathcost, 0.0) )
            minpathcost = 2 * SCIPepsilon(scip);

         k = nedges - 2 * minelims;

         /* try to first eliminate edges with higher gap */
         for( e = nedges - 1; e > k && e >= 2; e = e - 2 )
         {
            if( SCIPisLE(scip, costrev[e - 2], minpathcost) )
               break;
         }

         if( SCIPisGT(scip, costrev[e], minpathcost) )
            minpathcost = costrev[e];

         if( SCIPisLE(scip, minpathcost, 0.0) )
            minpathcost = 2 * SCIPepsilon(scip);

         for( e = 0; e < nedges; e++ )
            marked[e] = FALSE;
      }
      else
      {
         eliminate = TRUE;

         /* the required reduced path cost to be surpassed */
         minpathcost = costrev[nedges - 2 * minelims];

         if( SCIPisLE(scip, minpathcost, 0.0) )
            minpathcost = 2 * SCIPepsilon(scip);

         for( e = 0; e < nedges; e++ )
            marked[e] = FALSE;
      }

      for( k = 0; k < nnodes; k++ )
      {
         if( grad[k] <= 0 )
            continue;

         if( nfixed > minelims )
            break;

         if( rpcmw && !graph->mark[k] )
         {
            assert(graph_pc_knotIsDummyTerm(graph, k) || k == root);
            continue;
         }

         if( !Is_term(graph->term[k]) && (!eliminate || pathdist[k] + vnoi[k].dist >= minpathcost) && solnode[k] != CONNECT  )
         {
            if( !eliminate )
            {
               tmpcost = pathdist[k] + vnoi[k].dist;

               for( e = graph->outbeg[k]; e != EAT_LAST; e = graph->oeat[e] )
               {
                  const int e2 = flipedge(e);
                  if( SCIPisGT(scip, tmpcost, costrev[e]) )
                     costrev[e] = tmpcost;

                  if( SCIPisGT(scip, tmpcost, costrev[e2]) )
                     costrev[e2] = tmpcost;
               }

               continue;
            }
            nfixed += grad[k];

            assert(!rpcmw || !graph_pc_knotIsDummyTerm(graph, k));
            graph_knot_del(scip, graph, k, TRUE);
         }
         else
         {
            e = graph->outbeg[k];
            while( e != EAT_LAST )
            {
               etmp = graph->oeat[e];
               head = graph->head[e];

               if( rpcmw && !graph->mark[head] )
               {
                  assert(graph_pc_knotIsDummyTerm(graph, head) || head == root);
                  e = etmp;
                  continue;
               }

               if( (soledge[e] == CONNECT) || (soledge[flipedge(e)] == CONNECT) )
               {
                  e = etmp;
                  continue;
               }

               tmpcost = pathdist[k] + cost[e] + vnoi[head].dist;

               if( (!eliminate) || tmpcost >= minpathcost )
               {
                  if( marked[flipedge(e)] )
                  {
                     if( eliminate )
                     {
                        graph_edge_del(scip, graph, e, TRUE);
                        nfixed++;
                     }
                     else
                     {
                        if( SCIPisGT(scip, tmpcost, costrev[e]) )
                           costrev[e] = tmpcost;

                        if( SCIPisGT(scip, tmpcost, costrev[flipedge(e)]) )
                           costrev[flipedge(e)] = tmpcost;
                     }
                  }
                  else
                  {
                     marked[e] = TRUE;
                  }
               }
               e = etmp;
            }
         }
      }

      for( k = 0; k < nnodes; k++ )
      {
         if( nfixed > minelims )
            break;

         if( !graph->mark[k] || Is_term(graph->term[k]) || solnode[k] == CONNECT )
            continue;

         if( grad[k] == 3 )
         {
            tmpcost = pathdist[k] + vnoi[k].dist + vnoi[k + nnodes].dist;

            if( !eliminate || tmpcost >= minpathcost )
            {
               int e2;
               int e3;
               e = graph->outbeg[k];
               assert(graph->oeat[e] != EAT_LAST);
               e2 = graph->oeat[e];
               assert(graph->oeat[e2] != EAT_LAST);
               e3 = graph->oeat[e2];
               assert(graph->oeat[e3] == EAT_LAST);

               if( SCIPisLE(scip, cost[e], 0.0) || SCIPisLE(scip, cost[e2], 0.0) || SCIPisLE(scip, cost[e3], 0.0) )
                  continue;

               if( !eliminate )
               {
                  for( e = graph->outbeg[k]; e != EAT_LAST; e = graph->oeat[e] )
                  {
                     if( SCIPisGT(scip, tmpcost, costrev[e]) )
                        costrev[e] = tmpcost;

                     e2 = flipedge(e);

                     if( SCIPisGT(scip, tmpcost, costrev[e2]) )
                        costrev[e2] = tmpcost;
                  }

                  continue;
               }
               nfixed += 3;
               while( graph->outbeg[k] != EAT_LAST )
                  graph_edge_del(scip, graph, graph->outbeg[k], TRUE);
            }
         }
      }
   }
   SCIPdebugMessage("deleted by da: %d \n", nfixed );

   if( rpcmw )
      graph_pc_2trans(scip, graph);
   assert(graph->mark[root]);

 TERMINATE:
   *nelims = nfixed;
   SCIPfreeBufferArray(scip, &vnoi);
   SCIPfreeBufferArray(scip, &edgearrint2);
   SCIPfreeBufferArray(scip, &pathprededge);
   SCIPfreeBufferArray(scip, &vbase);
   SCIPfreeBufferArray(scip, &pathdist);
   SCIPfreeBufferArray(scip, &state);
   SCIPfreeBufferArray(scip, &marked);
   SCIPfreeBufferArray(scip, &costrev);
   SCIPfreeBufferArray(scip, &cost);

   return SCIP_OKAY;
}


/** dual ascent based reductions for PCSPG and MWCSP */
SCIP_RETCODE reduce_daPcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const RPDA*           paramsda,           /**< parameters */
   REDSOLLOCAL*          redprimal,          /**< primal bound info or NULL */
   PATH*                 vnoi,               /**< Voronoi data structure array */
   SCIP_Real*            pathdist,           /**< distance array for shortest path calculations */
   int*                  vbase,              /**< Voronoi base array */
   int*                  pathedge,           /**< shortest path incoming edge array for shortest path calculations */
   int*                  state,              /**< int 4 * vertices array  */
   STP_Bool*             nodearrchar,        /**< STP_Bool node array for internal computations */
   int*                  nelims,             /**< pointer to store number of reduced edges */
   SCIP_RANDNUMGEN*      randnumgen,         /**< random number generator */
   SCIP_Real             prizesum            /**< sum of positive prizes */
)
{
   STPSOLPOOL* pool = NULL;
   GRAPH* transgraph = NULL;
   SCIP_Real* cost = NULL;
   SCIP_Real* costrev = NULL;
   SCIP_Real* bestcost = NULL;
   SCIP_Real* edgefixingbounds = NULL;
   SCIP_Real* nodefixingbounds = NULL;
   SCIP_Real offset;
   SCIP_Real lpobjval;
   SCIP_Real bestlpobjval;
   SCIP_Real upperbound;
   SCIP_Real minpathcost;
   int* roots = NULL;
   int* result = NULL;
   int* result2 = NULL;
   int* transresult = NULL;
   STP_Bool* marked = NULL;
   int nroots = 0;
   int nfixed = 0;
   int nusedroots;
   const int nnodes = graph_get_nNodes(graph);
   const int nedges = graph_get_nEdges(graph);
   const int extnedges = nedges + 2 * (graph->terms - 1);
   const int root = graph->source;
   SCIP_Bool havenewsol;
   SCIP_Bool userec = paramsda->useRec;
   const SCIP_Bool solbasedda = paramsda->pcmw_solbasedda;
   const SCIP_Bool useDifferentRoots = paramsda->pcmw_useMultRoots;
   const SCIP_Bool markroots = paramsda->pcmw_markroots;
   SCIP_Bool varyroot = useDifferentRoots && userec;
   const SCIP_Real damaxdeviation = paramsda->pcmw_fastDa ? DAMAXDEVIATION_FAST : -1.0;
   DAPARAMS daparams = { .addcuts = FALSE, .ascendandprune = FALSE, .root = root,
               .is_pseudoroot = TRUE, .damaxdeviation = damaxdeviation };

   assert(scip && nelims && nodearrchar);

   if( graph->terms <= 3 )
      return SCIP_OKAY;

   SCIP_CALL( SCIPallocBufferArray(scip, &result, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &transresult, extnedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &marked, extnedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &result2, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cost, extnedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &costrev, extnedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &bestcost, extnedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &edgefixingbounds, extnedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodefixingbounds, nnodes + 1) );

   if( userec )
      SCIP_CALL( solpool_init(scip, &pool, nedges, SOLPOOL_SIZE) );

   if( graph_pc_isPc(graph) )
      reduce_identifyNonLeafTerms(scip, graph);

   /*
    * 1. step: compute lower bound and reduced costs
    */

   graph_pc_2trans(scip, graph);
   offset = 0.0;

   /* transform the problem to a real SAP */
   SCIP_CALL( graph_transPcGetSap(scip, graph, &transgraph, &offset) );

   /* initialize data structures for shortest paths */
   SCIP_CALL( graph_path_init(scip, transgraph) );

   if( paramsda->pcmw_fastDa  )
   {
      SCIP_CALL( dualascent_pathsPcMw(scip, transgraph, cost, &lpobjval, NULL) );

      // todo probably we might try to continue from the reduced cost we have already
      if( !dualascent_allTermsReachable(scip, graph, transgraph->source, cost) )
         SCIP_CALL( dualascent_exec(scip, transgraph, NULL, &daparams, cost, &lpobjval) );
   }
   else
   {
      SCIP_CALL( dualascent_exec(scip, transgraph, NULL, &daparams, cost, &lpobjval) );
   }

   lpobjval += offset;
   bestlpobjval = lpobjval;
   BMScopyMemoryArray(bestcost, cost, extnedges);

   /* compute first primal solution */
   upperbound = FARAWAY;
   havenewsol = FALSE;

   if( redprimal )
   {
      upperbound = reduce_sollocalGetUpperBound(redprimal);
   }

   SCIP_CALL( computeSteinerTreeRedCostsPcMw(scip, graph, NULL, cost, &upperbound, result, result2, pathedge, nodearrchar, &havenewsol) );

   assert(LT(upperbound, FARAWAY));

   /* the required reduced path cost to be surpassed */
   minpathcost = upperbound - lpobjval;

   assert(GE(minpathcost, 0.0));
   SCIPdebugMessage("havenewsol=%d upperbound=%f lpobjval=%f \n", havenewsol, upperbound, lpobjval);

   if( userec)
      SCIPdebugMessage("DA first minpathcost %f \n", minpathcost);

   /* initialize data structures for transgraph */
 //  SCIP_CALL( graph_initHistory(scip, transgraph) );
   computeTransVoronoi(scip, transgraph, vnoi, cost, costrev, pathdist, vbase, pathedge);

   /*
    * 2. step: try to eliminate
    */

   /* restore original graph */
   graph_pc_2org(scip, graph);

   for( int e = 0; e < extnedges; e++ )
      marked[e] = FALSE;

   /* try to reduce the graph */
   assert(daRedCostIsValid(scip, transgraph, cost, state, nodearrchar));

   updateEdgeFixingBounds(edgefixingbounds, graph, cost, pathdist, vnoi, lpobjval, extnedges, TRUE, FALSE);
   updateNodeFixingBounds(nodefixingbounds, graph, pathdist, vnoi, lpobjval, TRUE);

   {
      const SCIP_Bool deleteTransEdges = (solbasedda || varyroot || markroots);
      nfixed += reducePcMw(scip, graph, transgraph, vnoi, cost, pathdist, minpathcost, result, marked, nodearrchar,
            havenewsol, deleteTransEdges);
   }

   /* edges from result might have been deleted! */
   havenewsol = havenewsol && solstp_isUnreduced(scip, graph, result);
   assert(!havenewsol || solstp_isValid(scip, graph, result));

   if( userec )
      SCIPdebugMessage("DA: 1. NFIXED %d \n", nfixed);

   /* rerun dual ascent? */
   if( solbasedda && graph->terms > 2 && SCIPisGT(scip, minpathcost, 0.0) )
   {
      const SCIP_Real oldupperbound = upperbound;

      /* with recombination? */
      if( userec && graph->stp_type != STP_MWCSP )
      {
         // todo is that really useful?
         SCIP_CALL( daPcAddTmSolToPool(scip, graph, result2, pool) );
      }

      /* try to improve both dual and primal bound */
      SCIP_CALL( computePertubedSol(scip, graph, transgraph, pool, vnoi, cost, bestcost, pathdist, pathedge, result, result2,
            transresult, nodearrchar, &upperbound, &lpobjval, &bestlpobjval, &minpathcost, &havenewsol, offset, extnedges) );

      assert(solstp_isValid(scip, graph, result));
      assert(!havenewsol || SCIPisEQ(scip, getSolObj(scip, graph, result), upperbound));

      graph_pc_2orgcheck(scip, graph);

      assert(daRedCostIsValid(scip, transgraph, cost, state, nodearrchar));
      computeTransVoronoi(scip, transgraph, vnoi, cost, costrev, pathdist, vbase, pathedge);
      updateEdgeFixingBounds(edgefixingbounds, graph, cost, pathdist, vnoi, lpobjval, extnedges, FALSE, FALSE);
      updateNodeFixingBounds(nodefixingbounds, graph, pathdist, vnoi, lpobjval, FALSE);

      nfixed += reducePcMw(scip, graph, transgraph, vnoi, cost, pathdist, minpathcost, result, marked, nodearrchar, havenewsol, TRUE);

      nfixed += reducePcMwTryBest(scip, graph, transgraph, vnoi, cost, costrev, bestcost, pathdist, &upperbound,
            &lpobjval, &bestlpobjval, &minpathcost, oldupperbound, result, vbase, state, pathedge, marked, nodearrchar, &havenewsol, extnedges);

      nfixed += reduceWithEdgeFixingBounds(scip, graph, transgraph, edgefixingbounds, NULL, upperbound);
      nfixed += reduceWithNodeFixingBounds(scip, graph, transgraph, nodefixingbounds, upperbound);

      if( userec )
         SCIPdebugMessage("eliminations after sol based run2 with best dual sol %d bestlb %f newlb %f\n", nfixed, bestlpobjval, lpobjval);
   }


   if( reduce_sollocalUsesNodesol(redprimal) )
   {
      havenewsol = havenewsol && solstp_isUnreduced(scip, graph, result);
      if( havenewsol )
      {
         graph_pc_2trans(scip, graph);
         SCIP_CALL( reduce_sollocalUpdateNodesol(scip, result, graph, redprimal) );
         graph_pc_2org(scip, graph);
      }
   }

   if( graph->stp_type == STP_MWCSP && graph->terms < STP_RED_MINBNDTERMS  )
      varyroot = FALSE;

   /* change or mark roots? */
   if( varyroot || markroots )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &roots, graph->terms) );

      SCIP_CALL( daPcFindRoots(scip, graph, transgraph, cost, bestcost, lpobjval, bestlpobjval, upperbound, FALSE, FALSE,
            vnoi, pathedge, vbase, nodearrchar, roots, &nroots));

      /* should prize of terminals be changed? */
      if( nroots > 0 && markroots  )
         daPcMarkRoots(scip, roots, 0, nroots, prizesum, graph, &userec, &pool);

      if( nroots > 0 && varyroot )
         SCIP_CALL( daOrderRoots(scip, graph, roots, nroots, TRUE, randnumgen) );
   }

   if( varyroot )
      nusedroots = MIN(DEFAULT_NMAXROOTS, nroots);
   else
      nusedroots = -1;

   graph_path_exit(scip, transgraph);
   graph_free(scip, &transgraph, TRUE);

   /* loop and change root for dual ascent run */
   for( int run = 0; run < nusedroots; run++  )
   {
      const int tmproot = roots[run];
      int transnnodes;
      int transnedges;

      assert(nroots > 0);
      assert(roots != NULL);
      assert(Is_term(graph->term[tmproot]));

      if( graph->terms <= 2 )
         break;

      assert(!graph_pc_knotIsNonLeafTerm(graph, tmproot));

      SCIP_CALL( graph_transPcGetRsap(scip, graph, &transgraph, roots, nroots, tmproot) );

      assert(graph_valid(scip, transgraph) && STP_SAP == transgraph->stp_type);

      transnnodes = transgraph->knots;
      transnedges = transgraph->edges;

      graph_mark(transgraph);

      /* init data structures for shortest paths and history */
      SCIP_CALL( graph_path_init(scip, transgraph) );

      if( havenewsol && run > 1 )
      {
         BMScopyMemoryArray(transresult, result, graph->edges);
         SCIP_CALL(solstp_reroot(scip, transgraph, transresult, tmproot));
         daparams.root = tmproot;
         daparams.is_pseudoroot = FALSE;
         daparams.damaxdeviation = -1.0;

         SCIP_CALL( dualascent_exec(scip, transgraph, transresult, &daparams, cost, &lpobjval) );
      }
      else
      {
         daparams.root = tmproot;
         daparams.is_pseudoroot = FALSE;
         daparams.damaxdeviation = -1.0;

         SCIP_CALL( dualascent_exec(scip, transgraph, NULL, &daparams, cost, &lpobjval) );
      }

      assert(graph_valid(scip, transgraph));

      for( int e = graph->outbeg[tmproot]; e != EAT_LAST; e = graph->oeat[e] )
      {
         const int k = graph->head[e];
         if( Is_term(graph->term[k]) )
         {
            if( k == root )
               cost[flipedge(e)] = 0.0;
            else
               cost[e] = 0.0;
         }
      }

      for( int k = 0; k < nnodes; k++ )
      {
         if( Is_pseudoTerm(graph->term[k]) && SCIPisGE(scip, graph->prize[k], prizesum) )
         {
            for( int e = graph->outbeg[k]; e != EAT_LAST; e = graph->oeat[e] )
            {
               const int head = graph->head[e];
               if( Is_term(graph->term[head]) && head != root )
                  cost[e] = 0.0;
            }
         }
      }

      havenewsol = FALSE;
      SCIP_CALL( computeSteinerTreeRedCostsPcMw(scip, graph, pool, cost, &upperbound, result, result2, pathedge, nodearrchar, &havenewsol) );

      SCIPdebugMessage("ROOTRUNS upperbound %f \n", upperbound);
      if( pool )
         SCIPdebugMessage("ROOTRUNS best sol in pool %f \n", pool->sols[0]->obj);

      for( int k = 0; k < transnnodes; k++ )
         transgraph->mark[k] = (transgraph->grad[k] > 0);

      /* the required reduced path cost to be surpassed */
      minpathcost = upperbound - lpobjval;

      if( markroots )
      {
         const int oldnroots = nroots;
         SCIP_CALL( daPcFindRoots(scip, graph, transgraph, cost, cost, lpobjval, lpobjval, upperbound, TRUE, TRUE,
               vnoi, pathedge, vbase, nodearrchar, roots, &nroots) );

         /* should prize of terminals be changed? */
         if( nroots > oldnroots  )
            daPcMarkRoots(scip, roots, oldnroots, nroots, prizesum, graph, &userec, &pool);
      }

      SCIPdebugMessage("ROOTRUNS: minpathcost %f \n", minpathcost);
      SCIPdebugMessage("lb: %f ub: %f \n", lpobjval, upperbound);

      /* distance from root to all nodes */
      graph_path_execX(scip, transgraph, tmproot, cost, pathdist, pathedge);

      for( int e = 0; e < transnedges; e++ )
            costrev[e] = cost[flipedge(e)];

      /* no paths should go back to the root */
      for( int e = transgraph->outbeg[tmproot]; e != EAT_LAST; e = transgraph->oeat[e] )
         costrev[e] = FARAWAY;

      /* build Voronoi diagram */
      graph_add1stTermPaths(transgraph, costrev, vnoi, vbase, state);
      graph_add2ndTermPaths(transgraph, costrev, costrev, vnoi, vbase, state);

      /* restore original graph */
      graph_pc_2org(scip, graph);

      assert(graph->mark[tmproot]);
      graph->mark[tmproot] = FALSE;

      /* at first run switch to undirected case */
      if( run == 0 )
         for( int e = 0; e < extnedges; e++ )
            edgefixingbounds[e] = MIN(edgefixingbounds[e], edgefixingbounds[flipedge(e)]);

      updateEdgeFixingBounds(edgefixingbounds, graph, cost, pathdist, vnoi, lpobjval, extnedges, FALSE, TRUE);
      updateNodeFixingBounds(nodefixingbounds, graph, pathdist, vnoi, lpobjval, FALSE);

      for( int e = 0; e < transnedges; e++ )
         marked[e] = FALSE;

      /* try to eliminate vertices and edges */
      nfixed += reducePcMw(scip, graph, transgraph, vnoi, cost, pathdist, minpathcost, result, marked, nodearrchar, havenewsol, TRUE);
      nfixed += reduceWithEdgeFixingBounds(scip, graph, transgraph, edgefixingbounds, NULL, upperbound);
      nfixed += reduceWithNodeFixingBounds(scip, graph, transgraph, nodefixingbounds, upperbound);

      havenewsol = havenewsol && solstp_isUnreduced(scip, graph, result);
      assert(!havenewsol || solstp_isValid(scip, graph, result));
      SCIPdebugMessage("FIXED with changed root %d \n\n", nfixed);

      graph->mark[tmproot] = TRUE;

      transgraph->stp_type = STP_RPCSPG;
      graph_path_exit(scip, transgraph);
      graph_free(scip, &transgraph, TRUE);
   }

   *nelims = nfixed;

   if( pool != NULL )
      solpool_free(scip, &pool);

   SCIPfreeBufferArrayNull(scip, &roots);
   SCIPfreeBufferArray(scip, &nodefixingbounds);
   SCIPfreeBufferArray(scip, &edgefixingbounds);
   SCIPfreeBufferArray(scip, &bestcost);
   SCIPfreeBufferArray(scip, &costrev);
   SCIPfreeBufferArray(scip, &cost);
   SCIPfreeBufferArray(scip, &result2);
   SCIPfreeBufferArray(scip, &marked);
   SCIPfreeBufferArray(scip, &transresult);
   SCIPfreeBufferArray(scip, &result);

   if( redprimal )
   {
      reduce_sollocalUpdateUpperBound(upperbound, redprimal);
   }

   assert(graph_valid(scip, graph));

   return SCIP_OKAY;
}
