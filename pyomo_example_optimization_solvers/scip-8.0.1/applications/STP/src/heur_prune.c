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

/**@file   heur_prune.c
 * @brief  reduction-based primal heuristic for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file implements a reduction based heuristic for Steiner problems. See
 * "SCIP-Jack - A solver for STP and variants with parallelization extensions" (2016) by
 * Gamrath, Koch, Maher, Rehfeldt and Shinano
 *
 * A list of all interface methods can be found in heur_prune.h
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
//#define SCIP_DEBUG
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include "scip/cons_linear.h"
#include "heur_prune.h"
#include "heur_local.h"
#include "graph.h"
#include "reduce.h"
#include "heur_tm.h"
#include "solstp.h"
#include "cons_stp.h"
#include "scip/pub_misc.h"
#include "probdata_stp.h"

#define HEUR_NAME             "prune"
#define HEUR_DESC             "Reduction based heuristic for Steiner problems"
#define HEUR_DISPCHAR         'P'
#define HEUR_PRIORITY         2
#define HEUR_FREQ             -1
#define HEUR_FREQOFS          0
#define HEUR_MAXDEPTH         -1
#define HEUR_TIMING           (SCIP_HEURTIMING_DURINGLPLOOP | SCIP_HEURTIMING_AFTERLPLOOP | SCIP_HEURTIMING_AFTERNODE)
#define HEUR_USESSUBSCIP      FALSE            /**< does the heuristic use a secondary SCIP instance?                  */

#define DEFAULT_PRUNE_MAXFRQ      FALSE       /**< executions of the heuristic at maximum frequency?                  */
#define PRUNE_TMRUNS             100          /**< number of runs in TM heuristic when called by prune heuristic      */
#define PRUNE_TMRUNS_FAST        70           /**< number of runs in TM heuristic when called by prune heuristic      */
#define PRUNE_MINREDELIMS        2            /**< maximum number of eliminations for reduction package when called by prune heuristic */
#define PRUNE_MAXREDROUNDS       6            /**< maximum number of reduction rounds in prune heuristic */
#define MAXNTERMINALS 500
#define MAXNEDGES     10000


/*
 * Data structures
 */

/** primal heuristic data */
struct SCIP_HeurData
{
   int                   bestsolindex;       /**< best solution during the previous run                             */
   int                   ntmruns;            /**< number of runs in TM heuristic                                    */
   int                   nfailures;          /**< number of failures since last successful call                     */
   SCIP_Bool             maxfreq;            /**< should the heuristic be called at maximum frequency?              */
};

/*
 * Local methods
 */



/* set node solution array and get solution value */
static
void setNodeSolArray(
   const GRAPH* g,
   SCIP_Real* RESTRICT uborg,
   int* RESTRICT solnode,
   const int* soledge
   )
{
   SCIP_Real ub = 0.0;
   const int nnodes = graph_get_nNodes(g);
   const int nedges = graph_get_nEdges(g);

   assert(solnode != NULL);
   assert(soledge != NULL);

   for( int k = 0; k < nnodes; k++ )
      solnode[k] = UNKNOWN;

   solnode[g->source] = CONNECT;

   for( int e = 0; e < nedges; e++ )
   {
      if( soledge[e] == CONNECT )
      {
         ub += g->cost[e];
         solnode[g->head[e]] = CONNECT;
      }
   }

#ifndef NDEBUG
   for( int e = 0; e < nedges; e++ )
   {
      if( soledge[e] == CONNECT )
      {
         assert(solnode[g->tail[e]] == CONNECT);
         assert(solnode[g->head[e]] == CONNECT);
      }
   }
#endif

   if( uborg != NULL)
      *uborg = ub;
}

/** computes new solution during execution of prune and updates best global one if possible */
static
SCIP_RETCODE computeNewSols(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   GRAPH*                prunegraph,         /**< pruned graph data structure */
   int*                  nodearrint,         /**< array */
   int*                  solnode,            /**< array for best solution nodes wrt prunegraph */
   int*                  soledge,            /**< array for best solution edges wrt prunegraph */
   int*                  globalsoledge,      /**< array storing best solution wrt g */
   SCIP_Real*            globalobj,          /**< pointer to objective value of best solution wrt g */
   SCIP_Bool             incumbentgiven,     /**< incumbent solution for pruned graph given? */
   SCIP_Bool             beFast,             /**< use fast mode? */
   SCIP_Bool*            success             /**< pointer to store whether a solution could be found */
)
{
   int* const pmark = prunegraph->mark;
   SCIP_Real dummyhop = 0.0;
   int nruns;
   const int nnodes = g->knots;

   assert(graph_valid(scip, prunegraph));
   assert(g->edges == prunegraph->edges);

   /*
    * compute new solution on pruned graph
    */

   nruns = 0;
   for( int k = 0; k < nnodes; k++ )
   {
      pmark[k] = (prunegraph->grad[k] > 0);
      if( pmark[k] )
         nruns++;
   }

   if( beFast )
   {
      nruns = MIN(nruns, PRUNE_TMRUNS);
      nruns = MIN(prunegraph->terms, nruns);
   }
   else
   {
      nruns = MIN(nruns, PRUNE_TMRUNS);
   }
   SCIPStpHeurTMCompStarts(prunegraph, nodearrint, &nruns);

   /* run shortest path heuristic */
   SCIP_CALL( SCIPStpHeurTMRun(scip, pcmode_fromheurdata, prunegraph, nodearrint, NULL, soledge, nruns,
         prunegraph->source, prunegraph->cost, prunegraph->cost, &dummyhop, NULL, success));

   SCIP_CALL( SCIPStpHeurLocalRun(scip, prunegraph, soledge) );

   SCIP_CALL( SCIPStpHeurPruneUpdateSols(scip, g, prunegraph, solnode, soledge,
         globalsoledge, globalobj, incumbentgiven, success) );

   return SCIP_OKAY;
}

/* get reduction bound */
/* todo tune, rather random values at the moment */
static
int getRedBound(
   int nround,
   int nedges
   )
{
   if( nround == 0 )
      return(MAX(nedges / 1000, PRUNE_MINREDELIMS));
   if( nround == 1 )
      return(MAX(nedges / 500, PRUNE_MINREDELIMS));
   return(MAX(nedges / 200, PRUNE_MINREDELIMS));
}


/* todo tune, rather random values at the moment */
static
void setMinMaxElims(
   SCIP*                 scip,
   int*                  minnelims,
   int*                  lminnelims,
   int                   annodes,
   int                   anedges,
   int                   anterms,
   int                   nround,
   int                   maxrounds
   )
{
   int min;
   int totminnelims;
   SCIP_Real factor;

   assert(scip != NULL);
   assert(minnelims != NULL);
   assert(lminnelims != NULL);
   assert(annodes > 0);
   assert(nround >= 0);
   assert(maxrounds >= 1);

   anedges = anedges / 2;

   totminnelims = MAX(PRUNE_MINREDELIMS, (anedges / 20));

   min = (anedges / 10);
   min -= (int) ( ((SCIP_Real) min * anterms) / ( (SCIP_Real) annodes) );
   min = MAX(min, 1);

   factor = (SCIP_Real) anedges / min;
   factor = ((SCIP_Real) nround / (3.0 * maxrounds)) * factor;

   if( SCIPisGT(scip, factor, 1.0) )
      min = (int) ((SCIP_Real) min * factor);

   min = MAX(totminnelims, min);
   min = MIN(min, (anedges - 1));
   min = MAX(min, 1);

   *lminnelims = min / 2;
   *lminnelims = MAX(*lminnelims, 1);

   *minnelims = min;
}

/*
 * Callback methods of primal heuristic
 */

/** copy method for primal heuristic plugins (called when SCIP copies plugins) */
static
SCIP_DECL_HEURCOPY(heurCopyPrune)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(heur != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);

   /* call inclusion method of primal heuristic */
   SCIP_CALL( SCIPStpIncludeHeurPrune(scip) );

   return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static
SCIP_DECL_HEURFREE(heurFreePrune)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;

   assert(heur != NULL);
   assert(scip != NULL);

   /* get heuristic data */
   heurdata = SCIPheurGetData(heur);
   assert(heurdata != NULL);

   /* free heuristic data */
   SCIPfreeMemory(scip, &heurdata);
   SCIPheurSetData(heur, NULL);

   return SCIP_OKAY;
}


/** initialization method of primal heuristic (called after problem was transformed) */
static
SCIP_DECL_HEURINIT(heurInitPrune)
{  /*lint --e{715}*/


   return SCIP_OKAY;
}


/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static
SCIP_DECL_HEURINITSOL(heurInitsolPrune)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;

   assert(heur != NULL);
   assert(scip != NULL);

   /* get heuristic's data */
   heurdata = SCIPheurGetData(heur);

   assert(heurdata != NULL);

   /* initialize data */
   heurdata->ntmruns = PRUNE_TMRUNS;
   heurdata->nfailures = 0;
   heurdata->bestsolindex = -1;

   return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static
SCIP_DECL_HEUREXITSOL(heurExitsolPrune)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** execution method of primal heuristic */
static
SCIP_DECL_HEUREXEC(heurExecPrune)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;
   SCIP_PROBDATA* probdata;
   SCIP_VAR** vars;
   SCIP_SOL* bestsol;                        /* best known solution */
   GRAPH* graph;
   SCIP_Real* xval;
   SCIP_Bool success;
   int e;
   int nvars;
   int nedges;
   int*  soledge;

   assert(heur != NULL);
   assert(scip != NULL);
   assert(result != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);

   /* get heuristic data */
   heurdata = SCIPheurGetData(heur);
   assert(heurdata != NULL);

   /* get problem data */
   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   /* get graph */
   graph = SCIPprobdataGetGraph(probdata);
   assert(graph != NULL);

   nedges = graph->edges;
   *result = SCIP_DIDNOTRUN;

   /* if not STP like variant, return */
   if( graph->stp_type != STP_SPG && graph->stp_type != STP_RSMT && graph->stp_type != STP_OARSMT &&
      graph->stp_type != STP_GSTP )
      return SCIP_OKAY;

   if( (graph->edges > MAXNEDGES) && (graph->terms > MAXNTERMINALS) )
      return SCIP_OKAY;

   /* get best current solution */
   bestsol = SCIPgetBestSol(scip);

   /* no solution available? */
   if( bestsol == NULL )
      return SCIP_OKAY;

   /* has the new solution been found by this very heuristic? */
   if( SCIPsolGetHeur(bestsol) == heur || heurdata->bestsolindex == SCIPsolGetIndex(SCIPgetBestSol(scip)) )
      return SCIP_OKAY;

   xval = SCIPprobdataGetXval(scip, bestsol);

   if( xval == NULL )
      return SCIP_OKAY;

   vars = SCIPprobdataGetVars(scip);
   nvars = SCIPprobdataGetNVars(scip);

   SCIPdebugMessage("START prune heuristic \n");

   assert(vars != NULL);

   /* allocate array to store primal solution */
   SCIP_CALL( SCIPallocBufferArray(scip, &soledge, nedges) );

   for( e = 0; e < nedges; e++ )
   {
      if( SCIPisEQ(scip, xval[e], 1.0) )
         soledge[e] = CONNECT;
      else
         soledge[e] = UNKNOWN;
   }

   /* execute prune heuristic */
   SCIP_CALL( SCIPStpHeurPruneRun(scip, vars, graph, soledge, &success, TRUE, FALSE) );

   /* solution found by prune heuristic? */
   if( success )
   {
      SCIP_Real pobj;
      SCIP_Real* nval;

      SCIPdebugMessage("ADDED valid solution in prune \n");

      assert(solstp_isValid(scip, graph, soledge));
      pobj = 0.0;

      /* allocate memory to store solution */
      SCIP_CALL( SCIPallocBufferArray(scip, &nval, nvars) );

      for( e = 0; e < nedges; e++ )
      {
         if( soledge[e] == CONNECT )
         {
            nval[e] = 1.0;
            pobj += graph->cost[e];
         }
         else
         {
            nval[e] = 0.0;
         }
      }

      SCIPdebugMessage("prune, best: old %f, new %f \n  \n",  SCIPgetSolOrigObj(scip, bestsol) - SCIPprobdataGetOffset(scip), pobj);

      /* try to add new solution to pool */
      SCIP_CALL( SCIPprobdataAddNewSol(scip, nval, heur, &success) );

      /* has solution been added? */
      if( success )
      {
         SCIPdebugMessage("solution added by PRUNE \n  \n");

         *result = SCIP_FOUNDSOL;

         assert(solstp_isValid(scip, graph, soledge));

         /* is the solution the new incumbent? */
         if( SCIPisGT(scip, SCIPgetSolOrigObj(scip, bestsol) - SCIPprobdataGetOffset(scip), pobj) )
         {
            heurdata->nfailures = 0;
         }
         else
         {
            success = FALSE;
         }
      }

      /* solution could not be added or is not best? */
      if( !success )
         heurdata->nfailures++;

      /* free memory */
      SCIPfreeBufferArray(scip, &nval);
   }

   /* store index of incumbent solution */
   heurdata->bestsolindex = SCIPsolGetIndex(SCIPgetBestSol(scip));

   /* free memory */
   SCIPfreeBufferArray(scip, &soledge);

   return SCIP_OKAY;
}


/*
 * primal heuristic specific interface methods
 */

/** updates solutions for pruned graph */
SCIP_RETCODE SCIPStpHeurPruneUpdateSols(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g,                  /**< graph data structure */
   GRAPH*                prunegraph,         /**< pruned graph data structure */
   int*                  solnode,            /**< array for best solution nodes wrt prunegraph */
   int*                  soledge,            /**< array for best solution edges wrt prunegraph */
   int*                  globalsoledge,      /**< array storing best solution wrt g */
   SCIP_Real*            globalobj,          /**< pointer to objective value of best solution wrt g */
   SCIP_Bool             incumbentgiven,     /**< incumbent solution for pruned graph given? */
   SCIP_Bool*            success             /**< pointer to store whether a solution could be found */
   )
{
   SCIP_Real objnew;
   int* RESTRICT edgearrint;
   const int nnodes = g->knots;
   const int nedges = g->edges;
   const int probtype = g->stp_type;
   const SCIP_Bool pcmw = (probtype == STP_MWCSP || probtype == STP_RMWCSP || probtype == STP_RPCSPG || probtype == STP_PCSPG);

   assert(g != NULL);
   assert(scip != NULL);
   assert(soledge != NULL);
   assert(solnode != NULL);

   SCIP_CALL( SCIPallocBufferArray(scip, &edgearrint, nedges) );

   /*
    * compare new solution on pruned graph with (reconstructed) incumbent
    */

   if( incumbentgiven )
   {
      PATH* path;
      SCIP_Real objold;

      SCIP_CALL( SCIPallocBufferArray(scip, &path, nnodes) );

      objnew = solstp_getObjBounded(prunegraph, soledge, 0.0, nedges);

      if( pcmw )
         SCIP_CALL( SCIPStpHeurTMBuildTreePcMw(scip, prunegraph, FALSE, path, prunegraph->cost, &objold, solnode) );
      else
         SCIPStpHeurTMBuildTree(scip, prunegraph, path, prunegraph->cost, &objold, solnode);

      SCIPdebugMessage("objold %f objnew %f \n", objold, objnew);

      /* keep (reconstructed) old solution? */
      if( LT(objold, objnew) )
      {
         for( int e = 0; e < nedges; e++ )
            soledge[e] = UNKNOWN;
         for( int k = 0; k < nnodes; k++ )
         {
            const int e = path[k].edge;

            if( e >= 0 )
               soledge[e] = CONNECT;
         }

         assert(SCIPisEQ(scip, objold, solstp_getObjBounded(prunegraph, soledge, 0.0, nedges)));
      }

      SCIPfreeBufferArray(scip, &path);
   }

   assert(solstp_isValid(scip, prunegraph, soledge));

   setNodeSolArray(prunegraph, NULL, solnode, soledge);

   /*
    * retransform new solution and compare with best global one
    */

   SCIP_CALL( solstp_getOrg(scip, prunegraph, g, soledge, edgearrint) );

   assert(solstp_isValid(scip, g, edgearrint));

   objnew = solstp_getObjBounded(g, edgearrint, 0.0, nedges);

   SCIPdebugMessage("old global obj: %f ... new global obj: %f \n", *globalobj, objnew);

   if( LT(objnew, (*globalobj)) )
   {
      SCIPdebugMessage("new global solution is better \n");
      *globalobj = objnew;

      SCIP_CALL( SCIPStpHeurLocalRun(scip, g, edgearrint) );

      objnew = solstp_getObjBounded(g, edgearrint, 0.0, nedges);

      assert(SCIPisLE(scip, objnew, *globalobj));

      if( objnew < (*globalobj) )
         *globalobj = objnew;

      BMScopyMemoryArray(globalsoledge, edgearrint, nedges);
   }

   assert(*globalobj < FARAWAY);

   graph_mark(prunegraph);

   *success = TRUE;

   SCIPfreeBufferArray(scip, &edgearrint);

   return SCIP_OKAY;
}

/** execute prune heuristic on given graph */
SCIP_RETCODE SCIPStpHeurPruneRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< problem variables or NULL (need to be provided whenever available) */
   GRAPH*                g,                  /**< the graph */
   int*                  soledge,            /**< array to store primal solution (if no solution is provided,
                                                withinitialsol must be set to FALSE) */
   SCIP_Bool*            success,            /**< feasible solution found? */
   const SCIP_Bool       withinitialsol,     /**< solution given? */
   const SCIP_Bool       reducegraph         /**< try to reduce graph initially? */
   )
{
   GRAPH* prunegraph;
   PATH* vnoi;
   PATH* path;
   SCIP_Real globalobj;
   SCIP_Real uborg = 0.0;
   SCIP_Real offset;
   SCIP_Real* cost;
   SCIP_Real* costrev;
   SCIP_Real* nodearrreal;
   const int probtype = g->stp_type;
   const SCIP_Bool pc = (probtype == STP_RPCSPG || probtype == STP_PCSPG);
   const SCIP_Bool mw = (probtype == STP_MWCSP || probtype == STP_RMWCSP);
   const SCIP_Bool pcmw = (pc || mw);
   const int nnodes = g->knots;
   const int nedges = g->edges;
   int annodes;
   int anedges;
   int anterms;
   int reductbound;
   int* heap;
   int* state;
   int* vbase;
   int* solnode;
   int* nodearrint;
   int* edgearrint;
   int* nodearrint2;
   int* globalsoledge;
   STP_Bool* nodearrchar;
   SCIP_Bool solgiven;

   assert(g != NULL);
   assert(scip != NULL);
   assert(soledge != NULL);
   assert(probtype != STP_DHCSTP);
   assert(!(withinitialsol && reducegraph));

   *success = TRUE;

   if( reducegraph )
      solgiven = FALSE;
   else
      solgiven = withinitialsol;

   if( g->terms <= 1)
   {
      for( int e = 0; e < nedges; e++ )
         soledge[e] = UNKNOWN;
      return SCIP_OKAY;
   }

   /* allocate memory */
   SCIP_CALL( SCIPallocBufferArray(scip, &globalsoledge, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solnode, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cost, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &costrev, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodearrchar, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &heap, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &state, 4 * nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodearrreal, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &vbase, 4 * nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodearrint, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &edgearrint, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodearrint2, nnodes + 1) );
   SCIP_CALL( SCIPallocBufferArray(scip, &vnoi, 4 * nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &path, nnodes + 1) );

   if( probtype == STP_RSMT || probtype == STP_OARSMT || probtype == STP_GSTP )
      g->stp_type = STP_SPG;

   /* copy the graph */
   SCIP_CALL( graph_copy(scip, g, &prunegraph) );

   if( vars != NULL )
   {
      prunegraph->norgmodeledges = prunegraph->edges;
      prunegraph->norgmodelknots = prunegraph->knots;
   }

   /* set ancestors of the new graph */
   SCIP_CALL( graph_initHistory(scip, prunegraph) );

   /* set offset (within new graph) to 0.0 */
   offset = 0.0;

   reductbound = getRedBound(0, nedges);

   SCIPdebugMessage("Starting prune... \n");

   assert(!pcmw || g->extended);

   /* problem variables given? */
   if( vars != NULL )
   {
      int nfixededges = 0;

      /* delete fixed edges from the new graph */
      for( int e = 0; e < nedges; e += 2 )
      {
         /* both e and its anti-parallel edge fixed to zero? */
         if( SCIPvarGetUbLocal(vars[e]) < 0.5 && SCIPvarGetUbLocal(vars[e + 1]) < 0.5 )
         {
            if( pcmw )
            {
               if( (!solgiven || (soledge[e] != CONNECT && soledge[e + 1] != CONNECT))
                     && !Is_term(prunegraph->head[e]) && !Is_term(prunegraph->tail[e]) )
               {
                  graph_edge_del(scip, prunegraph, e, TRUE);
                  nfixededges++;
               }
            }
            else
            {
               if( !solgiven || (soledge[e] != CONNECT && soledge[e + 1] != CONNECT) )
               {
                  graph_edge_del(scip, prunegraph, e, TRUE);
                  nfixededges++;
               }
            }
         }
      }
      SCIPdebugMessage("fixed edges in prune: %d \n", nfixededges);

      if( nfixededges >= reductbound )
      {
         graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);
         reductbound = getRedBound(0, anedges);
      }
   }

   SCIP_CALL(graph_path_init(scip, prunegraph));

   /* perform initial reductions? */
   if( reducegraph )
   {
      REDSOL* redsol;
      SCIP_CALL( reduce_solInit(scip, prunegraph, TRUE, &(redsol)) );

      if( pc )
      {
         SCIP_CALL( reduce_redLoopPc(scip, redsol, prunegraph, vnoi, path, nodearrreal, heap, state,
               vbase, nodearrint, edgearrint, nodearrint2, nodearrchar, FALSE, FALSE, FALSE, reductbound, FALSE, TRUE, TRUE) );
      }
      else if( mw )
      {
         SCIP_CALL( reduce_redLoopMw(scip, redsol, prunegraph, vnoi, nodearrreal, state,
               vbase, nodearrint, nodearrchar, FALSE, FALSE, FALSE, reductbound, FALSE, TRUE) );
      }
      else
      {
         RPARAMS parameters = { .dualascent = FALSE, .boundreduce = FALSE, .nodereplacing = TRUE, .reductbound_min = PRUNE_MINREDELIMS,
                                .reductbound = reductbound, .userec = FALSE, .fullreduce = FALSE, .usestrongreds = TRUE };
         BIDECPARAMS decparameters = { .depth = 0, .maxdepth = 2, .newLevelStarted = FALSE };
         REDBASE redbase = { .redparameters = &parameters, .bidecompparams = &decparameters,
                             .solnode = NULL, .redsol = redsol,
                             .vnoi = vnoi, .path = path, .heap = heap,
                             .nodearrreal = nodearrreal,
                             .state = state, .vbase = vbase, .nodearrint = nodearrint,
                             .edgearrint = edgearrint, .nodearrint2 = nodearrint2, .nodearrchar = nodearrchar };

         SCIP_CALL( reduce_redLoopStp(scip, prunegraph, &redbase) );
      }

      offset = reduce_solGetOffset(redsol);
      reduce_solFree(scip, &(redsol));
   }

   /* get number of remaining nodes, edges and terminals */
   graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);

   if( solgiven )
   {
      BMScopyMemoryArray(globalsoledge, soledge, nedges);
      globalobj = solstp_getObjBounded(g, soledge, 0.0, nedges);
      setNodeSolArray(g, &uborg, solnode, soledge);
   }
   else
   {
      globalobj = FARAWAY;
      SCIP_CALL( computeNewSols(scip, g, prunegraph, nodearrint, solnode, soledge, globalsoledge, &globalobj, FALSE, FALSE, success) );

      assert(*success);
   }

   SCIPdebugMessage("initially reduced graph: |V| = %d, |E| = %d, |T| = %d  \n", annodes, anedges, anterms);
   SCIPdebugMessage("entering prune \n\n");

   prunegraph->withInexactReductions = TRUE;
   if( annodes > 0 )
   {
      /* main prune reduction loop */
      for( int i = 0; i < PRUNE_MAXREDROUNDS; i++ )
      {
         REDSOL* redsol;
         int minnelims = 0;
         int brednelims = 0;
         int lminnelims = 0;

         /* get number of remaining edges */
         graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);

         if( anterms <= 2 )
         {
            SCIPdebugMessage("less than two terminals, break !! \n");
            break;
         }

         setMinMaxElims(scip, &minnelims, &lminnelims, annodes, anedges, anterms, i + 1, PRUNE_MAXREDROUNDS);

         if( i > 0 )
         {
            SCIP_CALL( computeNewSols(scip, g, prunegraph, nodearrint, solnode, soledge, globalsoledge,
                 &globalobj, TRUE, TRUE, success) );
         }

         if( pcmw )
            graph_pc_2org(scip, prunegraph);

         SCIP_CALL( reduce_boundPruneHeur(scip, prunegraph, vnoi, cost, nodearrreal, costrev,
               &offset, heap, state, vbase, solnode, soledge, &brednelims, minnelims));

         if( pcmw )
            graph_pc_2trans(scip, prunegraph);

#ifdef SCIP_DEBUG
         graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);
         printf("PRUNE round: %d edges %d  nodes: %d \n", i, anedges / 2, annodes);
         printf("PRUNE round: %d minelims %d  really reduced: %d \n", i, minnelims, brednelims);
#endif

         /* not enough reductions possible? */
         if( brednelims < lminnelims  )
         {
            SCIPdebugMessage("not enough elims in PRUNE; exit %d \n\n", brednelims);
            i = PRUNE_MAXREDROUNDS;
         }


         /* delete all vertices not reachable from the root */
         if( graph_pc_isRootedPcMw(prunegraph) )
            SCIP_CALL(reduce_unconnectedRpcRmw(scip, prunegraph, &offset));
         else
            SCIP_CALL(reduce_unconnected(scip, prunegraph));

         assert(graph_valid(scip, prunegraph));

         /* is the reduced graph still feasible? */
         if( !graph_valid(scip, prunegraph) )
            break;

         /* get number of remaining edges */
         graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);

         reductbound = getRedBound(i + 1, anedges);

         SCIP_CALL( reduce_solInit(scip, prunegraph, TRUE, &(redsol)) );
         SCIP_CALL( reduce_solAddNodesol(prunegraph, solnode, redsol) );

         /* reduce graph */
         if( pc )
         {
            SCIP_CALL( reduce_redLoopPc(scip, redsol, prunegraph, vnoi, path, nodearrreal, heap, state,
                  vbase, nodearrint, edgearrint, nodearrint2, nodearrchar, FALSE, FALSE, FALSE, reductbound, FALSE, TRUE, TRUE) );
         }
         else if( mw )
         {
            SCIP_CALL( reduce_redLoopMw(scip, redsol, prunegraph, vnoi, nodearrreal, state,
                  vbase, nodearrint, nodearrchar, FALSE, FALSE, FALSE, reductbound, FALSE, TRUE) );
         }
         else
         {
            RPARAMS parameters = { .dualascent = FALSE, .boundreduce = FALSE, .nodereplacing = TRUE, .reductbound_min = PRUNE_MINREDELIMS,
                                   .reductbound = reductbound, .userec = FALSE, .fullreduce = FALSE, .usestrongreds = TRUE };
            BIDECPARAMS decparameters = { .depth = 0, .maxdepth = 2, .newLevelStarted = FALSE };
            REDBASE redbase = { .redparameters = &parameters, .bidecompparams = &decparameters,
                                .solnode = NULL, .redsol = redsol,
                                .vnoi = vnoi, .path = path, .heap = heap,
                                .nodearrreal = nodearrreal,
                                .state = state, .vbase = vbase, .nodearrint = nodearrint,
                                .edgearrint = edgearrint, .nodearrint2 = nodearrint2, .nodearrchar = nodearrchar };

            SCIP_CALL( reduce_redLoopStp(scip, prunegraph, &redbase) );
         }

         reduce_solGetNodesol(prunegraph, redsol, solnode);

         offset += reduce_solGetOffset(redsol);
         reduce_solFree(scip, &(redsol));

         /* delete all vertices not reachable from the root */
         if( graph_pc_isRootedPcMw(prunegraph) )
            SCIP_CALL(reduce_unconnectedRpcRmw(scip, prunegraph, &offset));
         else
            SCIP_CALL(reduce_unconnected(scip, prunegraph));

         graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);

         if( anterms <= 2 )
         {
            SCIPdebugMessage("less than two terminals, break! \n");
            SCIP_CALL( computeNewSols(scip, g, prunegraph, nodearrint, solnode, soledge, globalsoledge,
                              &globalobj, TRUE, TRUE, success) );
            break;
         }
      } /* reduction loop */
   } /* main prune loop */

   graph_path_exit(scip, prunegraph);

   *success = TRUE;
   assert(solstp_isValid(scip, g, globalsoledge));

   SCIPdebugMessage("obj of best prune sol: %f \n", solstp_getObjBounded(g, globalsoledge, 0.0, nedges));

   BMScopyMemoryArray(soledge, globalsoledge, nedges);

   /* free memory */
   graph_free(scip, &prunegraph, TRUE);

   SCIPfreeBufferArray(scip, &path);
   SCIPfreeBufferArray(scip, &vnoi);
   SCIPfreeBufferArray(scip, &nodearrint2);
   SCIPfreeBufferArray(scip, &edgearrint);
   SCIPfreeBufferArray(scip, &nodearrint);
   SCIPfreeBufferArray(scip, &vbase);
   SCIPfreeBufferArray(scip, &nodearrreal);
   SCIPfreeBufferArray(scip, &state);
   SCIPfreeBufferArray(scip, &heap);
   SCIPfreeBufferArray(scip, &nodearrchar);
   SCIPfreeBufferArray(scip, &costrev);
   SCIPfreeBufferArray(scip, &cost);
   SCIPfreeBufferArray(scip, &solnode);
   SCIPfreeBufferArray(scip, &globalsoledge);

   if( probtype == STP_RSMT || probtype == STP_OARSMT || probtype == STP_GSTP )
      g->stp_type = probtype;

   return SCIP_OKAY;
}


/** creates the prune primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPStpIncludeHeurPrune(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_HEURDATA* heurdata;
   SCIP_HEUR* heur;

   /* create prune primal heuristic data */
   SCIP_CALL( SCIPallocMemory(scip, &heurdata) );

   /* include primal heuristic */
   SCIP_CALL( SCIPincludeHeurBasic(scip, &heur,
         HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
         HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecPrune, heurdata) );

   assert(heur != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetHeurCopy(scip, heur, heurCopyPrune) );
   SCIP_CALL( SCIPsetHeurFree(scip, heur, heurFreePrune) );
   SCIP_CALL( SCIPsetHeurInit(scip, heur, heurInitPrune) );
   SCIP_CALL( SCIPsetHeurInitsol(scip, heur, heurInitsolPrune) );
   SCIP_CALL( SCIPsetHeurExitsol(scip, heur, heurExitsolPrune) );

   /* add prune primal heuristic parameters */
   SCIP_CALL( SCIPaddBoolParam(scip, "heuristics/"HEUR_NAME"/maxfreq",
         "should the heuristic be executed at maximum frequeny?",
         &heurdata->maxfreq, FALSE, DEFAULT_PRUNE_MAXFRQ, NULL, NULL) );

   return SCIP_OKAY;
}
