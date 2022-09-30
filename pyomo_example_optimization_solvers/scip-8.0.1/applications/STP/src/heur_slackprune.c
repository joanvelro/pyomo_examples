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

/**@file   heur_slackprune.c
 * @brief  dual-ascent and reduction based primal heuristic for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file implements a dual-ascent and reduction based heuristic for Steiner problems. It is based on an approach
 * described in T. Polzin's "Algorithms for the Steiner problem in networks".
 *
 * A list of all interface methods can be found in heur_slackprune.h
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
#include "heur_slackprune.h"
#include "heur_ascendprune.h"
#include "heur_local.h"
#include "heur_prune.h"
#include "graph.h"
#include "reduce.h"
#include "heur_tm.h"
#include "solstp.h"
#include "cons_stp.h"
#include "scip/pub_misc.h"
#include "probdata_stp.h"
#include "prop_stp.h"

#define HEUR_NAME             "slackprune"
#define HEUR_DESC             "Reduction based heuristic for Steiner problems"
#define HEUR_DISPCHAR         'S'
#define HEUR_PRIORITY         1
#define HEUR_FREQ             1
#define HEUR_FREQOFS          0
#define HEUR_MAXDEPTH         -1
#define HEUR_TIMING           (SCIP_HEURTIMING_DURINGLPLOOP | SCIP_HEURTIMING_AFTERLPLOOP | SCIP_HEURTIMING_AFTERNODE)
#define HEUR_USESSUBSCIP      FALSE           /**< does the heuristic use a secondary SCIP instance?                                 */

#define DEFAULT_SLACKPRUNE_MAXFREQ   FALSE       /**< executions of the heuristic at maximum frequency?                             */
#define SLACKPRUNE_MINREDELIMS       2           /**< minimum number of eliminations for reduction package when called by slack-and-prune heuristic */
#define SLACKPRUNE_MAXREDROUNDS      10          /**< maximum number of reduction rounds in slack-prune heuristic */
#define SLACKPRUNE_MINSTALLPROPORTION   0.2      /**< minimum proportion of arcs to be fixed before restarting slack-prune heuristic */
#define SLACKPRUNE_MAXSTALLPROPORTION   0.5       /**< maximum proportion of arcs to be fixed before restarting slack-prune heuristic */
#define SLACKPRUNE_HARDREDRATIO  0.97

#define MAXNTERMINALS   1000
#define MAXNEDGES      20000
#define MAXNEDGES_SPG  10000

#define SLACK_MAXTOTNEDGES 5000

/*
 * Data structures
 */

/** primal heuristic data */
struct SCIP_HeurData
{
   int                   lastnfixededges;    /**< number of fixed edges before the previous run                     */
   int                   bestsolindex;       /**< best solution during the previous run                             */
   int                   nfailures;          /**< number of failures since last successful call                     */
   int                   nexecuted;          /**< number of executions                     */
   SCIP_Bool             maxfreq;            /**< should the heuristic be called at maximum frequency?              */
};

/*
 * Local methods
 */

/* get reduction bound */
static
int getRedBound(
   int nrounds,
   int nedges
   )
{
   if( nrounds == 0)
      return (MAX(nedges / 2000, SLACKPRUNE_MINREDELIMS));
   if( nrounds == 1)
      return (MAX(nedges / 1000, SLACKPRUNE_MINREDELIMS));
   return(MAX(nedges / 500, SLACKPRUNE_MINREDELIMS));
}


static
void setMinMaxElims(
   SCIP*                 scip,
   int*                  minnelims,
   int*                  lminnelims,
   int                   annodes,
   int                   anedges,
   int                   anterms,
   int                   nround,
   int                   maxnrounds
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
   assert(maxnrounds >= 1);

   anedges = anedges / 2;

   totminnelims = MAX(SLACKPRUNE_MINREDELIMS, (anedges / 25));

   min = (int) (anedges * 0.15);
   min -= (int) (((double) min * anterms) / (annodes));
   min = MAX(min, 1);

   factor = (double) anedges / min;
   factor = ((double) nround / (2.5 * maxnrounds)) * factor;

   if( SCIPisGT(scip, factor, 1.0) )
   {
      SCIP_Real tmp = min * factor;
      min = (int) tmp;
   }

   min = MAX(totminnelims, min);

   min = MIN(min, (anedges - 1));
   min = MAX(min, 1);

   *lminnelims = min / 2;
   *lminnelims = MAX(*lminnelims, 1);

   *minnelims = min;

}


/** can the problem class be used? */
static inline
SCIP_Bool probtypeIsValidForSlackPrune(
   const GRAPH*          graph               /**< graph data structure */
)
{
   const int probtype = graph->stp_type;
   return (graph_typeIsSpgLike(graph) || probtype == STP_RMWCSP || probtype == STP_RPCSPG);
}


/** should we abort early? */
static inline
SCIP_Bool abortSlackPruneEarly(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_HEURDATA*        heurdata            /**< heuristic's data */
)
{
	const int nedges = graph->edges;
	SCIP_Bool beAggressive;
	SCIP_Real redratio;

	assert(nedges > 0);
	assert(SCIPprobdataGetNorgEdges(scip) >= nedges);

	if( !probtypeIsValidForSlackPrune(graph) )
	   return TRUE;

	redratio = (SCIP_Real) nedges / (SCIP_Real) SCIPprobdataGetNorgEdges(scip);
	beAggressive = (redratio > SLACKPRUNE_HARDREDRATIO);

	if( !beAggressive && heurdata->nexecuted >= 3 )
	   return TRUE;

	if( !beAggressive && heurdata->nfailures >= 1 )
	   return TRUE;

	if( heurdata->nexecuted >= 1 && SCIPgetNSols(scip) <= 5 )
	   return TRUE;

	if( (graph->edges > MAXNEDGES) && (graph->terms > MAXNTERMINALS) )
	   return TRUE;

	if( graph_typeIsSpgLike(graph) && !beAggressive
       && (graph->edges > MAXNEDGES_SPG) && (graph->terms > MAXNTERMINALS) )
	   return TRUE;

	if( heurdata->bestsolindex == SCIPsolGetIndex(SCIPgetBestSol(scip)) )
	   return TRUE;

	/* after first call? Then only execute once enough edges have been fixed */
	if( !beAggressive && heurdata->nexecuted >= 1 )
	{
	   const SCIP_Real stallproportion = SLACKPRUNE_MINSTALLPROPORTION;

	   if( (SCIPStpNfixedEdges(scip) - heurdata->lastnfixededges) < (int) (stallproportion * nedges) )
	      return TRUE;
	}

   return FALSE;
}


/** does exact reductions */
static
SCIP_RETCODE reduceExact(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                prunegraph,         /**< graph data structure */
   int                   reductbound,        /**< reduction bound */
   SCIP_Bool             fullreduce,         /**< use full reduction techniques? */
   int*                  soledge,            /**< solution edges (in/out) */
   int*                  solnode,            /**< array of nodes of current solution that is not to be destroyed (in/out) */
   SCIP_Real*            offset              /**< objective offset */
   )
{
   REDSOL* redsol;
   PATH* vnoi;
   PATH* path;
   SCIP_Real* nodearrreal;
   int* heap;
   int* state;
   int* vbase;
   int* nodearrint;
   int* edgearrint;
   int* nodearrint2;
   STP_Bool* nodearrchar;
   const int nnodes = graph_get_nNodes(prunegraph);
   const int nedges = graph_get_nEdges(prunegraph);

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

   SCIP_CALL( reduce_solInit(scip, prunegraph, TRUE, &(redsol)) );
   SCIP_CALL( reduce_solAddNodesol(prunegraph, solnode, redsol) );

   if( graph_pc_isPc(prunegraph) )
   {
      SCIP_CALL( reduce_redLoopPc(scip, redsol, prunegraph, vnoi, path, nodearrreal, heap, state,
            vbase, nodearrint, edgearrint, nodearrint2, nodearrchar,
            FALSE, FALSE, FALSE, reductbound, FALSE, TRUE, TRUE) );
   }
   else if( graph_pc_isMw(prunegraph) )
   {
      SCIP_CALL( reduce_redLoopMw(scip, redsol, prunegraph, vnoi, nodearrreal, state,
            vbase, nodearrint, nodearrchar,
            FALSE, FALSE, FALSE, reductbound, FALSE, TRUE) );
   }
   else
   {
      RPARAMS parameters = { .dualascent = FALSE, .boundreduce = FALSE, .nodereplacing = TRUE, .reductbound_min = SLACKPRUNE_MINREDELIMS,
                             .reductbound = reductbound, .userec = FALSE, .fullreduce = fullreduce, .usestrongreds = TRUE };
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

   *offset = reduce_solGetOffset(redsol);
   reduce_solFree(scip, &(redsol));

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

   return SCIP_OKAY;
}

/*
 * Callback methods of primal heuristic
 */

/** copy method for primal heuristic plugins (called when SCIP copies plugins) */
static
SCIP_DECL_HEURCOPY(heurCopySlackPrune)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(heur != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);

   /* call inclusion method of primal heuristic */
   SCIP_CALL( SCIPStpIncludeHeurSlackPrune(scip) );

   return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static
SCIP_DECL_HEURFREE(heurFreeSlackPrune)
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
SCIP_DECL_HEURINIT(heurInitSlackPrune)
{  /*lint --e{715}*/


   return SCIP_OKAY;
}


/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static
SCIP_DECL_HEURINITSOL(heurInitsolSlackPrune)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;

   assert(heur != NULL);
   assert(scip != NULL);

   /* get heuristic's data */
   heurdata = SCIPheurGetData(heur);

   assert(heurdata != NULL);

   /* initialize data */
   heurdata->nexecuted = 0;
   heurdata->nfailures = 0;
   heurdata->bestsolindex = -1;

   /* NOTE: postpones slack-prune */
   heurdata->lastnfixededges = 0;

   return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static
SCIP_DECL_HEUREXITSOL(heurExitsolSlackPrune)
{  /*lint --e{715}*/


   return SCIP_OKAY;
}


/** execution method of primal heuristic */
static
SCIP_DECL_HEUREXEC(heurExecSlackPrune)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;
   SCIP_PROBDATA* probdata;
   SCIP_VAR** vars;
   SCIP_SOL* bestsol;
   GRAPH* graph;
   SCIP_Real* xval;
   SCIP_Bool success;
   int e;
   int nvars;
   int nedges;
   int* soledge;

   assert(heur != NULL);
   assert(scip != NULL);
   assert(result != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);

   heurdata = SCIPheurGetData(heur);
   probdata = SCIPgetProbData(scip);
   graph = SCIPprobdataGetGraph(probdata);

   assert(heurdata && graph);

   *result = SCIP_DIDNOTRUN;

   bestsol = SCIPgetBestSol(scip);

   /* NOTE: might be the case for UG that no feasible solution is available */
   if( bestsol == NULL )
      return SCIP_OKAY;

   if( abortSlackPruneEarly(scip, graph, heurdata) )
	  return SCIP_OKAY;

   xval = SCIPprobdataGetXval(scip, bestsol);
   if( xval == NULL )
      return SCIP_OKAY;

   *result = SCIP_DIDNOTFIND;
   heurdata->nexecuted++;
   heurdata->lastnfixededges = SCIPStpNfixedEdges(scip);

   vars = SCIPprobdataGetVars(scip);
   nvars = SCIPprobdataGetNVars(scip);
   nedges = graph->edges;

   assert(vars != NULL);
   assert(nvars == nedges);

   /* allocate array to store primal solution */
   SCIP_CALL( SCIPallocBufferArray(scip, &soledge, nedges) );

   for( e = 0; e < nedges; e++ )
   {
      if( SCIPisEQ(scip, xval[e], 1.0) )
         soledge[e] = CONNECT;
      else
         soledge[e] = UNKNOWN;
   }

   /* execute actual slackprune heuristic */
   SCIP_CALL( SCIPStpHeurSlackPruneRun(scip, vars, graph, soledge, &success, FALSE, FALSE) );

   /* solution found by slackprune heuristic? */
   if( success )
   {
      SCIP_Real pobj = 0.0;
      SCIP_Real* nval;

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

      SCIPdebugMessage("SP final solution: best: old %f, new %f \n",  SCIPgetSolOrigObj(scip, bestsol), pobj + SCIPprobdataGetOffset(scip));

      /* try to add new solution to pool */
      SCIP_CALL( SCIPprobdataAddNewSol(scip, nval, heur, &success) );

      /* has solution been added? */
      if( success )
      {
         SCIPdebugMessage("better solution added by SLACKPRUNE %f \n", pobj + SCIPprobdataGetOffset(scip));
         *result = SCIP_FOUNDSOL;

         assert(solstp_isValid(scip, graph, soledge));

         /* is the solution NOT the new incumbent? */
         if( SCIPisLE(scip, SCIPgetSolOrigObj(scip, bestsol) - SCIPprobdataGetOffset(scip), pobj) )
         {
             success = FALSE;
         }
      }

      /* free memory */
      SCIPfreeBufferArray(scip, &nval);
   }
   else
   {
      SCIPdebugMessage("slack-prune: solution not valid \n");
   }

   /* solution could not be found, added or is not best? */
   if( !success )
   {
      heurdata->nfailures++;
   }

   /* store index of incumbent solution */
   heurdata->bestsolindex = SCIPsolGetIndex(SCIPgetBestSol(scip));

   /* free memory */
   SCIPfreeBufferArray(scip, &soledge);
   SCIPdebugMessage("leaving slack and prune heuristic \n");

   return SCIP_OKAY;
}


/*
 * primal heuristic specific interface methods
 */

/** execute slack-and-prune heuristic on given graph */
SCIP_RETCODE SCIPStpHeurSlackPruneRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< problem variables or NULL */
   GRAPH*                g,                  /**< graph data structure */
   int*                  soledge,            /**< array to 1. provide and 2. return primal solution */
   SCIP_Bool*            success,            /**< feasible solution found? */
   SCIP_Bool             initialreduce,      /**< try to reduce graph initially? */
   SCIP_Bool             fullreduce          /**< use full reduction techniques? */
   )
{
   GRAPH*    prunegraph;
   SCIP_Real    ubnew;
   SCIP_Real    ubbest;
   SCIP_Real    offsetnew;
   SCIP_Real    globalobj;
   int     i;
   int     k;
   int     e;
   const int nnodes = graph_get_nNodes(g);
   const int nedges = graph_get_nEdges(g);
   int     anterms;
   int     anedges;
   int     annodes;
   int     probtype;
   int     minnelims;
   int     lminnelims;
   int     reductbound;
   int*     solnode;
   int*     globalsoledge;
   const SCIP_Bool isPcMw = graph_pc_isPcMw(g);

   assert(scip != NULL);
   assert(soledge != NULL);
   assert(solstp_isValid(scip, g, soledge));

   probtype = g->stp_type;
   *success = FALSE;

   SCIP_CALL( SCIPallocBufferArray(scip, &solnode, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &globalsoledge, nedges) );
   BMScopyMemoryArray(globalsoledge, soledge, nedges);

   /* mark solution vertices */
   for( k = 0; k < nnodes; k++ )
      solnode[k] = UNKNOWN;

   for( e = 0; e < nedges; e++ )
   {
      if( soledge[e] == CONNECT )
      {
         solnode[g->tail[e]] = CONNECT;
         solnode[g->head[e]] = CONNECT;
      }
   }

   ubbest = solstp_getObjBounded(g, soledge, 0.0, nedges);
   globalobj = ubbest;
   offsetnew = 0.0;    /* set offset (within new graph) to 0.0 */

   if( probtype == STP_RSMT || probtype == STP_OARSMT || probtype == STP_GSTP )
      g->stp_type = STP_SPG;

   SCIP_CALL( graph_copy(scip, g, &prunegraph) );
   SCIP_CALL( graph_initHistory(scip, prunegraph) );
   if( graph_pc_isPcMw(prunegraph) )
      prunegraph->norgmodelknots = prunegraph->knots;
   SCIP_CALL( graph_path_init(scip, prunegraph) );

   reductbound = getRedBound(0, nedges);

   /* variables given? */
   if( vars != NULL )
   {
      int nfixededges = 0;

      /* delete fixed edges from the new graph */
      for( e = 0; e < nedges; e += 2 )
      {
         /* both e and its anti-parallel edge fixed to zero? */
         if( SCIPvarGetUbLocal(vars[e]) < 0.5 && SCIPvarGetUbLocal(vars[e + 1]) < 0.5
            && soledge[e] != CONNECT && soledge[e + 1] != CONNECT )
         {
            if( isPcMw && graph_pc_edgeIsExtended(g, e) )
               continue;
            graph_edge_del(scip, prunegraph, e, TRUE);
            nfixededges++;
         }
      }

      SCIPdebugMessage("fixed edges in slack and prune: %d \n", nfixededges);

      if( nfixededges > reductbound && initialreduce )
      {
         graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);
         reductbound = getRedBound(0, anedges);
      }
   }

   /* perform initial reductions? */
   if( initialreduce )
   {
      SCIP_CALL( reduceExact(scip, prunegraph, reductbound, fullreduce, soledge, solnode, &offsetnew) );
   }

   /* get number of remaining vertices, edges and terminals */
   graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);

   /* main reduction loop */
   for( i = 0; i < SLACKPRUNE_MAXREDROUNDS && anterms > 2; i++ )
   {
      SCIP_Bool solIsImproved;
      SCIP_Bool solIsRebuilt;
      int danelims;
      SCIP_Bool solgiven;

      SCIPdebugMessage("starting round %d \n", i);

      /* update reduction bounds */
      setMinMaxElims(scip, &minnelims, &lminnelims, annodes, anedges, anterms, i + 1, SLACKPRUNE_MAXREDROUNDS);

      solgiven = ((i == 0) && !initialreduce);

      /*  perform heuristic reductions */
      SCIP_CALL( reduce_daSlackPrune(scip, prunegraph, minnelims, solgiven, soledge, solnode, &danelims, &ubnew, &solIsImproved, &solIsRebuilt) );

      /* delete all vertices not reachable from the root */
      SCIP_CALL( reduce_unconnected(scip, prunegraph) );

      assert(graph_valid(scip, prunegraph));

      graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);

      if( anterms <= 2 )
         break;

      SCIPdebugMessage("minelims %d really reduced: %d \n", minnelims, danelims);

      /* not enough reductions possible? */
      if( danelims < lminnelims )
      {
         SCIPdebugMessage("too few elims in DA, break %d < %d\n\n", danelims, lminnelims);
         i = SLACKPRUNE_MAXREDROUNDS;
      }

      /* solution found by ascend and prune? */
      if( solIsImproved )
      {
         assert(solstp_isValid(scip, prunegraph, soledge));
         SCIP_CALL( SCIPStpHeurLocalRun(scip, prunegraph, soledge) );

         SCIP_CALL( SCIPStpHeurPruneUpdateSols(scip, g, prunegraph, solnode, soledge,
               globalsoledge, &globalobj, TRUE, success) );
      }
      else if( solIsRebuilt )
      {
         assert(solstp_isValid(scip, prunegraph, soledge));
         SCIP_CALL( SCIPStpHeurPruneUpdateSols(scip, g, prunegraph, solnode, soledge,
               globalsoledge, &globalobj, FALSE, success) );
      }

      if( solIsImproved || solIsRebuilt )
      {
         const SCIP_Real obj = solstp_getObj(prunegraph, soledge, offsetnew);
         SCIPdebugMessage("old solution: %f new solution %f \n\n", ubbest + SCIPprobdataGetOffset(scip), obj + SCIPprobdataGetOffset(scip));

         if( SCIPisLE(scip, obj, ubbest) )
            ubbest = obj;
      }
      graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);
      reductbound = getRedBound(i, anedges);

      SCIP_CALL( reduceExact(scip, prunegraph, reductbound, fullreduce, soledge, solnode, &offsetnew) );

      graph_get_nVET(prunegraph, &annodes, &anedges, &anterms);
   } /* reduction loop */

   /* NOTE: might still help, even though heuristic was already called */
   SCIP_CALL(SCIPStpHeurLocalRun(scip, g, globalsoledge));

   graph_path_exit(scip, prunegraph);

   *success = solstp_isValid(scip, g, globalsoledge);
   BMScopyMemoryArray(soledge, globalsoledge, nedges);

   assert(*success);

   graph_free(scip, &prunegraph, TRUE);
   SCIPfreeBufferArray(scip, &globalsoledge);
   SCIPfreeBufferArray(scip, &solnode);

   if( probtype == STP_RSMT || probtype == STP_OARSMT || probtype == STP_GSTP )
      g->stp_type = probtype;

   return SCIP_OKAY;
}


/** creates the slackprune primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPStpIncludeHeurSlackPrune(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_HEURDATA* heurdata;
   SCIP_HEUR* heur;

   /* create slackprune primal heuristic data */
   SCIP_CALL( SCIPallocMemory(scip, &heurdata) );

   /* include primal heuristic */
   SCIP_CALL( SCIPincludeHeurBasic(scip, &heur,
         HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
         HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecSlackPrune, heurdata) );

   assert(heur != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetHeurCopy(scip, heur, heurCopySlackPrune) );
   SCIP_CALL( SCIPsetHeurFree(scip, heur, heurFreeSlackPrune) );
   SCIP_CALL( SCIPsetHeurInit(scip, heur, heurInitSlackPrune) );
   SCIP_CALL( SCIPsetHeurInitsol(scip, heur, heurInitsolSlackPrune) );
   SCIP_CALL( SCIPsetHeurExitsol(scip, heur, heurExitsolSlackPrune) );

   /* add slackprune primal heuristic parameters */
   SCIP_CALL( SCIPaddBoolParam(scip, "heuristics/"HEUR_NAME"/maxfreq",
         "should the heuristic be executed at maximum frequeny?",
         &heurdata->maxfreq, FALSE, DEFAULT_SLACKPRUNE_MAXFREQ, NULL, NULL) );


   return SCIP_OKAY;
}
