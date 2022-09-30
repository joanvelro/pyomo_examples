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

/**@file   cons_stp.c
 * @brief  Constraint handler for Steiner problems
 * @author Gerald Gamrath
 * @author Daniel Rehfeldt
 * @author Michael Winkler
 *
 * This file checks solutions for feasibility and separates violated model constraints. For more details see \ref STP_CONS page.
 *
 * @page STP_CONS Separating violated constraints
 *
 * In this file a constraint handler checking solutions for feasibility and separating violated model constraints is implemented.
 * The separation problem for the cut inequalities described in \ref STP_PROBLEM can be solved by a max-flow algorithm in
 * polynomial time.  Regarding the variable values of a given LP solution as capacities on the edges, one can check for each
 * \f$ t \in T \setminus \{r\} \f$, with \f$ r \f$ being the root, whether the minimal \f$ (r, t) \f$-cut is less than one. In this case,
 * a violated cut inequality has been found, otherwise none exists. In order to calculate such a minimal cut an adaptation of Hao
 * and Orlin's preflow-push algorithm (see A Faster Algorithm for Finding the Minimum Cut in a Directed Graph) is used. Furthermore, the file implements a dual ascent heuristic, based on a concept described
 * in "A dual ascent approach for Steiner tree problems on a directed graph." by R. Wong.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include <string.h>

#include "cons_stp.h"
#include "probdata_stp.h"
#include "graph.h"
#include "portab.h"
#include "branch_stp.h"
#include "prop_stp.h"
#include "sepaspecial.h"
#include "mincut.h"
#include "scip/scip.h"
#include "scip/misc.h"
#include "scip/cons_linear.h"
#include <time.h>
#if 0
#ifdef WITH_UG
#define ADDCUTSTOPOOL 1
#else
#define ADDCUTSTOPOOL 0
#endif
#endif

#define ADDCUTSTOPOOL 0
#define STP_SEPASPECIAL_USECLIQUE FALSE
#define STP_SEPASPECIAL_USECVTIMP FALSE

/**@name Constraint handler properties
 *
 * @{
 */

#define CONSHDLR_NAME          "stp"
#define CONSHDLR_DESC          "steiner tree constraint handler"
#define CONSHDLR_SEPAPRIORITY   9999999 /**< priority of the constraint handler for separation */
#define CONSHDLR_ENFOPRIORITY         0 /**< priority of the constraint handler for constraint enforcing */
#define CONSHDLR_CHECKPRIORITY  9999999 /**< priority of the constraint handler for checking feasibility */
#define CONSHDLR_SEPAFREQ             1 /**< frequency for separating cuts; zero means to separate only in the root node */
#define CONSHDLR_PROPFREQ             0 /**< frequency for propagating domains; zero means only preprocessing propagation */
#define CONSHDLR_EAGERFREQ            1 /**< frequency for using all instead of only the useful constraints in separation,
                                         *   propagation and enforcement, -1 for no eager evaluations, 0 for first only */
#define CONSHDLR_DELAYSEPA        FALSE /**< should separation method be delayed, if other separators found cuts? */
#define CONSHDLR_DELAYPROP        FALSE /**< should propagation method be delayed, if other propagators found reductions? */
#define CONSHDLR_NEEDSCONS         TRUE /**< should the constraint handler be skipped, if no constraints are available? */

#define DEFAULT_MAXROUNDS            20 /**< maximal number of separation rounds per node (-1: unlimited) */
#define DEFAULT_MAXROUNDSROOT        -1 /**< maximal number of separation rounds in the root node (-1: unlimited) */
#define DEFAULT_MAXSEPACUTS     INT_MAX /**< maximal number of cuts separated per separation round */
#define DEFAULT_MAXSEPACUTSROOT INT_MAX /**< maximal number of cuts separated per separation round in the root node */

#define CONSHDLR_PROP_TIMING       SCIP_PROPTIMING_BEFORELP

#define DEFAULT_BACKCUT        FALSE /**< Try Back-Cuts FALSE*/
#define DEFAULT_CREEPFLOW      TRUE  /**< Use Creep-Flow */
#define DEFAULT_DISJUNCTCUT    FALSE /**< Only disjunct Cuts FALSE */
#define DEFAULT_NESTEDCUT      FALSE /**< Try Nested-Cuts FALSE*/
#define DEFAULT_FLOWSEP        TRUE  /**< Try Flow-Cuts */
#define DEFAULT_INFLOWSEP       TRUE  /**< Try in-flow Cuts */
#define DEFAULT_INFLOWTERMSEP   TRUE  /**< Try terminal in-flow Cuts */
#define DEFAULT_OUTFLOWSEP      TRUE
#define DEFAULT_BALANCEFLOWSEP  TRUE


/**@} */

/*
 * Data structures
 */

/** @brief Constraint data for  \ref cons_stp.c "Stp" constraints */
struct SCIP_ConsData
{
   GRAPH*                graph;              /**< graph data structure */
};


/** @brief Constraint handler data for \ref cons_stp.c "Stp" constraint handler */
struct SCIP_ConshdlrData
{
   SCIP_RANDNUMGEN*      randnumgen;         /**< random number generator                                           */
   PACLIQUES*            pacliques;          /**< pseudo ancestor cliques */
   PCIMPLICATION*        pcimplications;     /**< prize-collecting implications */
   VTIMPLICATION*        vtimplications;     /**< vertex-terminal implications */
   SCIP_Bool             backcut;            /**< should backcuts be applied? */
   SCIP_Bool             creepflow;          /**< should creepflow cuts be applied? */
   SCIP_Bool             disjunctcut;        /**< should disjunction cuts be applied? */
   SCIP_Bool             nestedcut;          /**< should nested cuts be applied? */
   SCIP_Bool             flowsep;            /**< should flow separation be applied? */
   SCIP_Bool             inflowsep;          /**< should unit in-flow separation be applied? */
   SCIP_Bool             intermflowsep;      /**< should unit terminal in-flow separation be applied? */
   SCIP_Bool             outflowsep;         /**< should single-edge out-flow separation be applied? */
   SCIP_Bool             balanceflowsep;     /**< should flow-balance separation be applied? */
   int                   maxrounds;          /**< maximal number of separation rounds per node (-1: unlimited) */
   int                   maxroundsroot;      /**< maximal number of separation rounds in the root node (-1: unlimited) */
   int                   maxsepacuts;        /**< maximal number of cuts separated per separation round */
   int                   maxsepacutsroot;    /**< maximal number of cuts separated per separation round in the root node */
};



/**@name Local methods
 *
 * @{
 */



#if 0
/** separate degree-2 cuts */
static
SCIP_RETCODE sep_deg2(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        conshdlr,           /**< constraint handler */
   SCIP_CONSHDLRDATA*    conshdlrdata,       /**< constraint handler data */
   SCIP_CONSDATA*        consdata,           /**< constraint data */
   int                   maxcuts,            /**< maximal number of cuts */
   int*                  ncuts               /**< pointer to store number of cuts */
   )
{
   GRAPH* g;
   SCIP_VAR** vars;
   SCIP_ROW* row = NULL;
   SCIP_Real* xval;
   int cutscount = 0;
   int nnodes;
   const SCIP_Bool* deg2bounded = SCIPStpPropGet2BoundedArr(scip);

   assert(scip != NULL);
   assert(conshdlr != NULL);
   assert(conshdlrdata != NULL);
   assert(deg2bounded != NULL);

   vars = SCIPprobdataGetVars(scip);
   g = consdata->graph;
   assert(g != NULL);

   xval = SCIPprobdataGetXval(scip, NULL);
   assert(xval != NULL);

   nnodes = g->knots;

   for( int i = 0; i < nnodes; i++ )
   {
      double inoutsum;

      if( Is_term(g->term[i]) )
         continue;

      if( !deg2bounded[i] )
         continue;

      inoutsum = 0.0;

      for( int e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
      {
         inoutsum += xval[e] + xval[flipedge_Uint(e)];
         assert(flipedge_Uint(e) == (unsigned) flipedge(e));
      }

      if( SCIPisFeasGT(scip, inoutsum, 2.0) )
      {
         SCIP_Bool infeasible;

         SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "deg2", -SCIPinfinity(scip), 2.0, FALSE, FALSE, TRUE));

         SCIP_CALL(SCIPcacheRowExtensions(scip, row));

         for( int e = g->outbeg[i]; e != EAT_LAST; e = g->oeat[e] )
         {
            SCIP_CALL(SCIPaddVarToRow(scip, row, vars[e], 1.0));
            SCIP_CALL(SCIPaddVarToRow(scip, row, vars[flipedge_Uint(e)], 1.0));
            assert(flipedge_Uint(e) == (unsigned) flipedge(e));
         }

         SCIP_CALL(SCIPflushRowExtensions(scip, row));

         SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));

#if ADDCUTSTOPOOL
         /* add cut to pool */
         if( !infeasible )
         SCIP_CALL( SCIPaddPoolCut(scip, row) );
#endif

         cutscount++;

         SCIP_CALL(SCIPreleaseRow(scip, &row));

         if( *ncuts + cutscount >= maxcuts )
            break;
      }
   }

   printf("Deg2 Separator: %d Inequalities added\n", cutscount);
   *ncuts += cutscount;

   return SCIP_OKAY;
}
#endif



/** separate in-flow cuts:
 *  input of a non-terminal vertex has to be <= 1.0 */
static
SCIP_RETCODE sep_flowIn(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        conshdlr,           /**< constraint handler */
   const SCIP_Real*      nodes_inflow,       /**< incoming flow per node */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      xval,               /**< LP-solution values */
   int                   vertex,             /**< vertex */
   SCIP_VAR**            vars,               /**< variables */
   int*                  cutcount            /**< counts cuts */
)
{
   assert(xval && cutcount && vars);
   assert(!Is_term(g->term[vertex]));

#ifndef NDEBUG
   {
      SCIP_Real sum = 0.0;

      for( int k = g->inpbeg[vertex]; k != EAT_LAST; k = g->ieat[k] )
         sum += xval[k];

      assert(SCIPisEQ(scip, sum, nodes_inflow[vertex]));
   }
#endif

   if( SCIPisFeasGT(scip, nodes_inflow[vertex], 1.0) )
   {
      SCIP_ROW* row = NULL;
      SCIP_Bool infeasible;

      SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "inflow", -SCIPinfinity(scip), 1.0, FALSE, FALSE, TRUE));
      SCIP_CALL(SCIPcacheRowExtensions(scip, row));

      for( int k = g->inpbeg[vertex]; k != EAT_LAST; k = g->ieat[k] )
         SCIP_CALL(SCIPaddVarToRow(scip, row, vars[k], 1.0));

      SCIP_CALL(SCIPflushRowExtensions(scip, row));
      SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));
      assert(!infeasible);

#if ADDCUTSTOPOOL
      /* if at root node, add cut to pool */
      if( !infeasible )
         SCIP_CALL( SCIPaddPoolCut(scip, row) );
#endif

      (*cutcount)++;

      SCIP_CALL(SCIPreleaseRow(scip, &row));
   }

   return SCIP_OKAY;
}


/** separate terminal in-flow cuts
 *  at terminal input sum == 1
 *  basically a cut (starcut) */
static
SCIP_RETCODE sep_flowTermIn(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        conshdlr,           /**< constraint handler */
   const SCIP_Real*      nodes_inflow,       /**< incoming flow per node */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      xval,               /**< LP-solution values */
   const int*            termorg,            /**< original terminals or NULL */
   int                   vertex,             /**< vertex */
   SCIP_VAR**            vars,               /**< variables */
   int*                  cutcount            /**< counts cuts */
)
{
#ifndef NDEBUG
   SCIP_Real sum = 0.0;

   for( int k = g->inpbeg[vertex]; k != EAT_LAST; k = g->ieat[k] )
      sum += xval[k];

   assert(SCIPisEQ(scip, sum, nodes_inflow[vertex]));
#endif

   assert(Is_term(g->term[vertex]));

   if( !SCIPisFeasEQ(scip, nodes_inflow[vertex], 1.0) )
   {
      SCIP_ROW* row = NULL;
      SCIP_Bool infeasible;
      const SCIP_Bool isLocal = (termorg != NULL && termorg[vertex] != g->term[vertex]);

      SCIP_CALL( SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "term", 1.0, 1.0, isLocal, FALSE, TRUE) );

      SCIP_CALL(SCIPcacheRowExtensions(scip, row));

      for( int k = g->inpbeg[vertex]; k != EAT_LAST; k = g->ieat[k] )
         SCIP_CALL(SCIPaddVarToRow(scip, row, vars[k], 1.0));

      SCIP_CALL(SCIPflushRowExtensions(scip, row));
      SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));

      if( infeasible )
      {
         assert(isLocal);
      }
      else
      {
         (*cutcount)++;
      }

#if ADDCUTSTOPOOL
      /* add cut to pool */
      if( !infeasible )
         SCIP_CALL( SCIPaddPoolCut(scip, row) );
#endif


      SCIP_CALL(SCIPreleaseRow(scip, &row));
   }

   return SCIP_OKAY;
}


/** separate flow-balance constraints
 *  incoming flow <= outgoing flow */
static
SCIP_RETCODE sep_flowBalance(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        conshdlr,           /**< constraint handler */
   const SCIP_Real*      nodes_inflow,       /**< incoming flow per node */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      xval,               /**< LP-solution values */
   int                   vertex,             /**< vertex */
   SCIP_VAR**            vars,               /**< variables */
   int*                  cutcount            /**< counts cuts */
)
{
   SCIP_ROW* row = NULL;
   SCIP_Real sum = -nodes_inflow[vertex];

   assert(!Is_term(g->term[vertex]));

   for( int k = g->outbeg[vertex]; k != EAT_LAST; k = g->oeat[k] )
      sum += xval[k];

#ifndef NDEBUG
   {
      SCIP_Real sum_debug = 0.0;
      for( int k = g->inpbeg[vertex]; k != EAT_LAST; k = g->ieat[k] )
         sum_debug -= xval[k];

      for( int k = g->outbeg[vertex]; k != EAT_LAST; k = g->oeat[k] )
         sum_debug += xval[k];

      assert(SCIPisEQ(scip, sum_debug, sum));
   }
#endif

   if( SCIPisFeasNegative(scip, sum) )
   {
      SCIP_Bool infeasible;

      SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "flowbalance", 0.0, (g->terms == 2) ? 0.0 : SCIPinfinity(scip), FALSE, FALSE, TRUE));
      SCIP_CALL(SCIPcacheRowExtensions(scip, row));

      for( int k = g->inpbeg[vertex]; k != EAT_LAST; k = g->ieat[k] )
         SCIP_CALL(SCIPaddVarToRow(scip, row, vars[k], -1.0));

      for( int k = g->outbeg[vertex]; k != EAT_LAST; k = g->oeat[k] )
         SCIP_CALL(SCIPaddVarToRow(scip, row, vars[k], 1.0));

      SCIP_CALL(SCIPflushRowExtensions(scip, row));
      SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));
      assert(!infeasible);

#if ADDCUTSTOPOOL
      /* if at root node, add cut to pool */
      if( !infeasible )
         SCIP_CALL( SCIPaddPoolCut(scip, row) );
#endif

      (*cutcount)++;

      SCIP_CALL(SCIPreleaseRow(scip, &row));
   }

   return SCIP_OKAY;
}


/** separate
 * the value of each outgoing edge needs to be smaller than the sum of the in-going edges */
static
SCIP_RETCODE sep_flowEdgeOut(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        conshdlr,           /**< constraint handler */
   const SCIP_Real*      nodes_inflow,       /**< incoming flow per node */
   const GRAPH*          g,                  /**< graph data structure */
   const SCIP_Real*      xval,               /**< LP-solution values */
   int                   vertex,             /**< vertex */
   SCIP_VAR**            vars,               /**< variables */
   int*                  cutcount            /**< counts cuts */
)
{
   const int i = vertex;

   for( int ijedge = g->outbeg[i]; ijedge != EAT_LAST; ijedge = g->oeat[ijedge] )
   {
      const int j = g->head[ijedge];
      const SCIP_Real sum = nodes_inflow[i] - xval[ijedge] - xval[flipedge(ijedge)];

#ifndef NDEBUG
      SCIP_Real sum_debug = -xval[ijedge];

      for( int e = g->inpbeg[i]; e != EAT_LAST; e = g->ieat[e] )
         if( g->tail[e] != j )
            sum_debug += xval[e];

      assert(SCIPisEQ(scip, sum, sum_debug));
#endif

      if( SCIPisFeasNegative(scip, sum) )
      {
         SCIP_Bool infeasible;
         SCIP_ROW* row = NULL;

         SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "edgeflow", 0.0, SCIPinfinity(scip), FALSE, FALSE, TRUE));
         SCIP_CALL(SCIPcacheRowExtensions(scip, row));

         SCIP_CALL(SCIPaddVarToRow(scip, row, vars[ijedge], -1.0));

         for( int e = g->inpbeg[i]; e != EAT_LAST; e = g->ieat[e] )
         {
            if( g->tail[e] != j )
               SCIP_CALL(SCIPaddVarToRow(scip, row, vars[e], 1.0));
         }

         SCIP_CALL(SCIPflushRowExtensions(scip, row));
         SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));
         assert(!infeasible);

#if ADDCUTSTOPOOL
         /* add cut to pool */
         if( !infeasible )
            SCIP_CALL( SCIPaddPoolCut(scip, row) );
#endif

         (*cutcount)++;
         SCIP_CALL(SCIPreleaseRow(scip, &row));
      }
   }

   return SCIP_OKAY;
}


/** separate flow-cuts */
static
SCIP_RETCODE sep_flow(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        conshdlr,           /**< constraint handler */
   SCIP_CONSHDLRDATA*    conshdlrdata,       /**< constraint handler data */
   SCIP_CONSDATA*        consdata,           /**< constraint data */
   const int*            termorg,            /**< original terminals or NULL */
   int                   maxcuts,            /**< maximal number of cuts */
   int*                  ncuts               /**< pointer to store number of cuts */
   )
{
   const GRAPH* const g = consdata->graph;
   SCIP_VAR** vars = SCIPprobdataGetVars(scip);
   const SCIP_Real* xval =SCIPprobdataGetXval(scip, NULL);
   SCIP_Real* nodes_inflow;
   int count = 0;
   const int nnodes = graph_get_nNodes(g);
   const int maxcuts_local = maxcuts - *ncuts;
   const SCIP_Bool flowsep = conshdlrdata->flowsep;
   const SCIP_Bool inflowsep = conshdlrdata->inflowsep;
   const SCIP_Bool intermflowsep = conshdlrdata->intermflowsep;
   const SCIP_Bool outflowsep = conshdlrdata->outflowsep;
   const SCIP_Bool balanceflowsep = conshdlrdata->balanceflowsep;

   assert(conshdlr && xval && ncuts && vars);

   SCIP_CALL( SCIPallocBufferArray(scip, &nodes_inflow, nnodes) );

   for( int i = 0; i < nnodes; i++ )
   {
      SCIP_Real sum = 0;

      for( int e = g->inpbeg[i]; e >= 0; e = g->ieat[e] )
         sum += xval[e];

      nodes_inflow[i] = sum;
   }

   for( int i = 0; i < nnodes; i++ )
   {
      if( i == g->source )
         continue;

      if( intermflowsep && Is_term(g->term[i]) )
      {
         SCIP_CALL( sep_flowTermIn(scip, conshdlr, nodes_inflow, g, xval, termorg, i, vars, &count) );

         if( count >= maxcuts_local )
            break;
      }

      /* flow cuts disabled? */
      if( !flowsep )
         continue;

      if( outflowsep )
      {
         SCIP_CALL( sep_flowEdgeOut(scip, conshdlr, nodes_inflow, g, xval, i, vars, &count) );

         if( count >= maxcuts_local )
            break;
      }

      /* from here on consider only non terminals */
      if( Is_term(g->term[i]) )
         continue;

      if( inflowsep )
      {
         SCIP_CALL( sep_flowIn(scip, conshdlr, nodes_inflow, g, xval, i, vars, &count) );

         if( count >= maxcuts_local )
            break;
      }

      if( balanceflowsep )
      {
         SCIP_CALL( sep_flowBalance(scip, conshdlr, nodes_inflow, g, xval, i, vars, &count) );

         if( count >= maxcuts_local )
            break;
      }
   }

   SCIPdebugMessage("Flow Separator: %d Inequalities added\n", count);

   *ncuts += count;

   SCIPfreeBufferArray(scip, &nodes_inflow);

   return SCIP_OKAY;
}


/**@} */


/**@name Callback methods
 *
 * @{
 */

/** copy method for constraint handler plugins (called when SCIP copies plugins) */
static
SCIP_DECL_CONSHDLRCOPY(conshdlrCopyStp)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(conshdlr != NULL);
   assert(strcmp(SCIPconshdlrGetName(conshdlr), CONSHDLR_NAME) == 0);

   /* call inclusion method of constraint handler */
   SCIP_CALL( SCIPincludeConshdlrStp(scip) );

   *valid = TRUE;

   return SCIP_OKAY;
}

/** destructor of constraint handler to free constraint handler data (called when SCIP is exiting) */
static
SCIP_DECL_CONSFREE(consFreeStp)
{  /*lint --e{715}*/
   SCIP_CONSHDLRDATA* conshdlrdata;

   assert(scip != NULL);
   assert(conshdlr != NULL);
   assert(strcmp(SCIPconshdlrGetName(conshdlr), CONSHDLR_NAME) == 0);

   /* free constraint handler data */
   conshdlrdata = SCIPconshdlrGetData(conshdlr);
   assert(conshdlrdata != NULL);

   SCIPfreeRandom(scip, &conshdlrdata->randnumgen);

   SCIPfreeMemory(scip, &conshdlrdata);

   SCIPconshdlrSetData(conshdlr, NULL);

   return SCIP_OKAY;
}

/** solving process initialization method of constraint handler (called when branch and bound process is about to begin) */
static
SCIP_DECL_CONSINITSOL(consInitsolStp)
{  /*lint --e{715}*/
#ifdef WITH_UG
   SCIPStpConshdlrSetGraph(scip);
#endif
   return SCIP_OKAY;
}

static
SCIP_DECL_CONSEXITSOL(consExitsolStp)
{  /*lint --e{715}*/
   SCIP_CONSHDLRDATA* conshdlrdata;

   conshdlrdata = SCIPconshdlrGetData(conshdlr);
   assert(conshdlrdata);

   if( conshdlrdata->pcimplications )
      sepaspecial_pcimplicationsFree(scip, &(conshdlrdata->pcimplications));

   if( conshdlrdata->pacliques )
      sepaspecial_pacliquesFree(scip, &(conshdlrdata->pacliques));

   if( conshdlrdata->vtimplications )
      sepaspecial_vtimplicationsFree(scip, &(conshdlrdata->vtimplications));

   return SCIP_OKAY;
}

/** frees specific constraint data */
static
SCIP_DECL_CONSDELETE(consDeleteStp)
{  /*lint --e{715}*/
   assert(conshdlr != NULL);
   assert(strcmp(SCIPconshdlrGetName(conshdlr), CONSHDLR_NAME) == 0);
   assert(consdata != NULL);
   assert(*consdata != NULL);

   SCIPfreeBlockMemory(scip, consdata);

   return SCIP_OKAY;
}

/** transforms constraint data into data belonging to the transformed problem */
static
SCIP_DECL_CONSTRANS(consTransStp)
{  /*lint --e{715}*/
   SCIP_CONSDATA* sourcedata;
   SCIP_CONSDATA* targetdata;

   assert(conshdlr != NULL);
   assert(strcmp(SCIPconshdlrGetName(conshdlr), CONSHDLR_NAME) == 0);
   assert(SCIPgetStage(scip) == SCIP_STAGE_TRANSFORMING);
   assert(sourcecons != NULL);
   assert(targetcons != NULL);

   sourcedata = SCIPconsGetData(sourcecons);
   assert(sourcedata != NULL);

   /* create constraint data for target constraint */
   SCIP_CALL( SCIPallocBlockMemory(scip, &targetdata) );

   targetdata->graph = sourcedata->graph;

   /* create target constraint */
   SCIP_CALL( SCIPcreateCons(scip, targetcons, SCIPconsGetName(sourcecons), conshdlr, targetdata,
         SCIPconsIsInitial(sourcecons), SCIPconsIsSeparated(sourcecons), SCIPconsIsEnforced(sourcecons),
         SCIPconsIsChecked(sourcecons), SCIPconsIsPropagated(sourcecons),
         SCIPconsIsLocal(sourcecons), SCIPconsIsModifiable(sourcecons),
         SCIPconsIsDynamic(sourcecons), SCIPconsIsRemovable(sourcecons), SCIPconsIsStickingAtNode(sourcecons)) );

   return SCIP_OKAY;
}

/** LP initialization method of constraint handler (called before the initial LP relaxation at a node is solved) */
static
SCIP_DECL_CONSINITLP(consInitlpStp)
{  /*lint --e{715}*/
#if 0
   SCIP_PROBDATA* probdata;
   GRAPH* graph;

   SCIP_Real lpobjval;

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   graph = SCIPprobdataGetGraph(probdata);
   assert(graph != NULL);

   SCIP_CALL( SCIPdualAscentPcStp(scip, graph, NULL, &lpobjval, TRUE, 1) );
#endif

   return SCIP_OKAY;
}

/** separation method of constraint handler for LP solutions */
static
SCIP_DECL_CONSSEPALP(consSepalpStp)
{  /*lint --e{715}*/
   SCIP_CONSHDLRDATA* conshdlrdata;
   SCIP_CONSDATA* consdata;
   GRAPH* g;
   int* termorg = NULL;
   int* nodestatenew = NULL;
   int maxcuts;
   int ncuts = 0;
   const SCIP_Bool atrootnode = (SCIPnodeGetDepth(SCIPgetCurrentNode(scip)) == 0);
   SCIP_Bool chgterms = FALSE;
#ifndef NDEBUG
   int nterms;
#endif

   *result = SCIP_DIDNOTRUN;

   conshdlrdata = SCIPconshdlrGetData(conshdlr);
   assert(conshdlrdata != NULL);

   maxcuts = atrootnode ? conshdlrdata->maxsepacutsroot : conshdlrdata->maxsepacuts;

   assert(nconss == 1);
   consdata = SCIPconsGetData(conss[0]);

   assert(consdata != NULL);

   g = consdata->graph;
   assert(g != NULL);

#ifndef NDEBUG
   nterms = g->terms;
#endif

   /* vertex branching possible? */
   if( SCIPStpBranchruleProbIsCompatible(g) )
   {
      chgterms = (!atrootnode && SCIPStpBranchruleIsActive(scip));

      if( chgterms )
      {
         SCIPdebugMessage("adapting branched on vertices for separation \n");
      }
   }

   if( graph_pc_isPcMw(g) && g->stp_type != STP_BRMWCSP )
   {
      if( conshdlrdata->pcimplications == NULL )
      {
         graph_pc_2org(scip, g);
         SCIP_CALL( sepaspecial_pcimplicationsInit(scip, g, &(conshdlrdata->pcimplications)) );
         graph_pc_2trans(scip, g);
      }

      SCIP_CALL( sepaspecial_pcimplicationsSeparate(scip, conshdlr, conshdlrdata->pcimplications, maxcuts, &ncuts) );
   }

#if STP_SEPASPECIAL_USECLIQUE
   if( conshdlrdata->pacliques == NULL )
   {
      SCIP_CALL( sepaspecial_pacliquesInit(scip, g, &(conshdlrdata->pacliques)) );
   }

   SCIP_CALL( sepaspecial_pacliquesSeparate(scip, conshdlr, conshdlrdata->pacliques, maxcuts, &ncuts) );
#endif


#if STP_SEPASPECIAL_USECVTIMP
   if( conshdlrdata->vtimplications == NULL )
   {
      SCIP_CALL( sepaspecial_vtimplicationsInit(scip, g, &(conshdlrdata->vtimplications)) );
   }

   SCIP_CALL( sepaspecial_vtimplicationsSeparate(scip, conshdlr, conshdlrdata->vtimplications, maxcuts, &ncuts) );
#endif

   /* change graph according to branch-and-bound terminal changes  */
   if( chgterms )
   {
      SCIP_Bool conflict = FALSE;
      const int nnodes = g->knots;

      SCIP_CALL(SCIPallocBufferArray(scip, &nodestatenew, nnodes));
      SCIP_CALL(SCIPallocBufferArray(scip, &termorg, nnodes));
      BMScopyMemoryArray(termorg, g->term, nnodes);

      SCIPStpBranchruleInitNodeState(g, nodestatenew);
      SCIP_CALL( SCIPStpBranchruleGetVertexChgs(scip, nodestatenew, &conflict) );

      assert(!conflict);

      for( int k = 0; k < nnodes; k++ )
      {
         if( nodestatenew[k] == BRANCH_STP_VERTEX_TERM && !Is_term(g->term[k]) )
            graph_knot_chg(g, k, STP_TERM);
      }
   }

   SCIP_CALL( sep_flow(scip, conshdlr, conshdlrdata, consdata, termorg, maxcuts, &ncuts) );

   /* NOTE: for 2-terminal problems no cuts are necessary if flows are given */
   if( !conshdlrdata->flowsep || g->terms != 2 )
   {
      SCIP_CALL( mincut_separateLp(scip, conshdlr, NULL, termorg, consdata->graph, maxcuts, &ncuts) );
   }

   if( ncuts > 0 )
      *result = SCIP_SEPARATED;

   /* restore graph */
   if( chgterms )
   {
      const int nnodes = g->knots;

      for( int k = 0; k < nnodes; k++ )
      {
         if( g->term[k] != termorg[k] )
            graph_knot_chg(g, k, termorg[k]);
      }
   }

#ifndef NDEBUG
   assert(g->terms == nterms);
#endif

   SCIPfreeBufferArrayNull(scip, &termorg);
   SCIPfreeBufferArrayNull(scip, &nodestatenew);

   return SCIP_OKAY;
}


/** constraint enforcing method of constraint handler for LP solutions */
static
SCIP_DECL_CONSENFOLP(consEnfolpStp)
{  /*lint --e{715}*/
   SCIP_Bool feasible;
   SCIP_CONSDATA* consdata;

   // todo
   assert(nconss == 1);

   for( int i = 0; i < nconss; i++ )
   {
      consdata = SCIPconsGetData(conss[i]);

      SCIP_CALL( SCIPStpValidateSol(scip, consdata->graph, SCIPprobdataGetXval(scip, NULL), FALSE, &feasible) );

      if( !feasible )
      {
         *result = SCIP_INFEASIBLE;
         return SCIP_OKAY;
      }
   }

   *result = SCIP_FEASIBLE;

   return SCIP_OKAY;
}

/** constraint enforcing method of constraint handler for pseudo solutions */
static
SCIP_DECL_CONSENFOPS(consEnfopsStp)
{  /*lint --e{715}*/
   SCIP_Bool feasible;

   // todo
   assert(nconss == 1);

   for( int i = 0; i < nconss; i++ )
   {
      const SCIP_CONSDATA* consdata = SCIPconsGetData(conss[i]);

      SCIP_CALL( SCIPStpValidateSol(scip, consdata->graph, SCIPprobdataGetXval(scip, NULL), FALSE, &feasible) );

      if( !feasible )
      {
         *result = SCIP_INFEASIBLE;
         return SCIP_OKAY;
      }
   }
   *result = SCIP_FEASIBLE;

   return SCIP_OKAY;
}

/** feasibility check method of constraint handler for integral solutions */
static
SCIP_DECL_CONSCHECK(consCheckStp)
{ /*lint --e{715}*/
   const GRAPH* g = SCIPprobdataGetGraph2(scip);
   SCIP_Bool feasible;

   assert(g != NULL);

   SCIP_CALL(SCIPStpValidateSol(scip, g, SCIPprobdataGetXval(scip, sol), FALSE, &feasible));

   if( !feasible )
   {
      *result = SCIP_INFEASIBLE;
      return SCIP_OKAY;
   }

   *result = SCIP_FEASIBLE;

   return SCIP_OKAY;
}

/** domain propagation method of constraint handler */
static
SCIP_DECL_CONSPROP(consPropStp)
{  /*lint --e{715}*/
   SCIP_PROBDATA* probdata;
   GRAPH* graph;

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   graph = SCIPprobdataGetGraph(probdata);
   assert(graph != NULL);

   /* for degree constrained model, check whether problem is infeasible */
   if( graph->stp_type == STP_DCSTP )
   {
      int k;
      int nnodes;
      int degsum;
      int* maxdegs;

      nnodes = graph->knots;
      maxdegs = graph->maxdeg;

      assert(maxdegs != NULL);

      degsum = 0;
      for( k = 0; k < nnodes; k++ )
      {
         if( Is_term(graph->term[k]) )
         {
            assert(maxdegs[k] > 0);
            degsum += maxdegs[k] - 1;
         }
         else
         {
            assert(maxdegs[k] >= 0);
            degsum += MAX(maxdegs[k] - 2, 0);
         }
      }

      if( degsum < graph->terms - 2 )
         *result = SCIP_CUTOFF;
      else
	 *result = SCIP_DIDNOTFIND;
   }
   return SCIP_OKAY;
}

/** variable rounding lock method of constraint handler */
static
SCIP_DECL_CONSLOCK(consLockStp)
{  /*lint --e{715}*/
   SCIP_VAR** vars;
   int nvars;
   int v;

   assert(scip != NULL);
   assert(cons != NULL);

   vars = SCIPprobdataGetVars(scip);
   nvars = SCIPprobdataGetNVars(scip);

   for( v = 0; v < nvars; ++v )
   {
      SCIP_CALL( SCIPaddVarLocksType(scip, vars[v], locktype, nlockspos, nlocksneg) );
   }

   return SCIP_OKAY;
}

/** constraint copying method of constraint handler */
static
SCIP_DECL_CONSCOPY(consCopyStp)
{  /*lint --e{715}*/
   const char* consname;
   SCIP_PROBDATA* probdata;
   GRAPH* graph;

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   graph = SCIPprobdataGetGraph(probdata);
   assert(graph != NULL);

   consname = SCIPconsGetName(sourcecons);

   /* creates and captures a and constraint */
   SCIP_CALL( SCIPcreateConsStp(scip, cons, consname, graph) );

   *valid = TRUE;

   return SCIP_OKAY;
}


/**@} */

/**@name Interface methods
 *
 * @{
 */

/** creates the handler for stp constraints and includes it in SCIP */
SCIP_RETCODE SCIPincludeConshdlrStp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CONSHDLRDATA* conshdlrdata;
   SCIP_CONSHDLR* conshdlr;

   /* create stp constraint handler data */
   SCIP_CALL( SCIPallocMemory(scip, &conshdlrdata) );

   conshdlr = NULL;
   /* include constraint handler */
   SCIP_CALL( SCIPincludeConshdlrBasic(scip, &conshdlr, CONSHDLR_NAME, CONSHDLR_DESC,
         CONSHDLR_ENFOPRIORITY, CONSHDLR_CHECKPRIORITY, CONSHDLR_EAGERFREQ, CONSHDLR_NEEDSCONS,
         consEnfolpStp, consEnfopsStp, consCheckStp, consLockStp,
         conshdlrdata) );
   assert(conshdlr != NULL);

   SCIP_CALL( SCIPsetConshdlrCopy(scip, conshdlr, conshdlrCopyStp, consCopyStp) );
   SCIP_CALL( SCIPsetConshdlrDelete(scip, conshdlr, consDeleteStp) );
   SCIP_CALL( SCIPsetConshdlrTrans(scip, conshdlr, consTransStp) );
   SCIP_CALL( SCIPsetConshdlrInitsol(scip, conshdlr, consInitsolStp) );
   SCIP_CALL( SCIPsetConshdlrExitsol(scip, conshdlr, consExitsolStp) );

   SCIP_CALL( SCIPsetConshdlrInitlp(scip, conshdlr, consInitlpStp) );
   SCIP_CALL( SCIPsetConshdlrProp(scip, conshdlr, consPropStp, CONSHDLR_PROPFREQ, CONSHDLR_DELAYPROP,
         CONSHDLR_PROP_TIMING) );
   SCIP_CALL( SCIPsetConshdlrSepa(scip, conshdlr, consSepalpStp, NULL, CONSHDLR_SEPAFREQ,
         CONSHDLR_SEPAPRIORITY, CONSHDLR_DELAYSEPA) );
   SCIP_CALL( SCIPsetConshdlrFree(scip, conshdlr, consFreeStp) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/backcut", "Try Back-Cuts",
         &conshdlrdata->backcut, TRUE, DEFAULT_BACKCUT, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/creepflow", "Use Creep-Flow",
         &conshdlrdata->creepflow, TRUE, DEFAULT_CREEPFLOW, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/disjunctcut", "Only disjunct Cuts",
         &conshdlrdata->disjunctcut, TRUE, DEFAULT_DISJUNCTCUT, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/nestedcut", "Try Nested-Cuts",
         &conshdlrdata->nestedcut, TRUE, DEFAULT_NESTEDCUT, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/flowsep", "Try Flow-Cuts",
         &conshdlrdata->flowsep, TRUE, DEFAULT_FLOWSEP, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/inflowsep", "Try Unit Inflow-Cuts",
         &conshdlrdata->inflowsep, TRUE, DEFAULT_INFLOWSEP, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/intermflowsep", "Try terminal Unit Inflow-Cuts",
         &conshdlrdata->intermflowsep, TRUE, DEFAULT_INFLOWTERMSEP, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/outflowsep", "Try single edge Outflow-Cuts",
         &conshdlrdata->outflowsep, TRUE, DEFAULT_OUTFLOWSEP, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "constraints/stp/balanceflowsep", "Try Flow-balance Cuts",
         &conshdlrdata->balanceflowsep, TRUE, DEFAULT_BALANCEFLOWSEP, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip, "constraints/"CONSHDLR_NAME"/maxrounds",
         "maximal number of separation rounds per node (-1: unlimited)",
         &conshdlrdata->maxrounds, FALSE, DEFAULT_MAXROUNDS, -1, INT_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip, "constraints/"CONSHDLR_NAME"/maxroundsroot",
         "maximal number of separation rounds per node in the root node (-1: unlimited)",
         &conshdlrdata->maxroundsroot, FALSE, DEFAULT_MAXROUNDSROOT, -1, INT_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip, "constraints/"CONSHDLR_NAME"/maxsepacuts",
         "maximal number of cuts separated per separation round",
         &conshdlrdata->maxsepacuts, FALSE, DEFAULT_MAXSEPACUTS, 0, INT_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip, "constraints/"CONSHDLR_NAME"/maxsepacutsroot",
         "maximal number of cuts separated per separation round in the root node",
         &conshdlrdata->maxsepacutsroot, FALSE, DEFAULT_MAXSEPACUTSROOT, 0, INT_MAX, NULL, NULL) );

   conshdlrdata->pacliques = NULL;
   conshdlrdata->pcimplications = NULL;
   conshdlrdata->vtimplications = NULL;

   SCIP_CALL( SCIPcreateRandom(scip, &conshdlrdata->randnumgen, 1, TRUE) );

   return SCIP_OKAY;
}

/** creates and captures a stp constraint */
SCIP_RETCODE SCIPcreateConsStp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS**           cons,               /**< pointer to hold the created constraint */
   const char*           name,               /**< name of constraint */
   GRAPH*                graph               /**< graph data structure */
   )
{
   SCIP_CONSHDLR* conshdlr;
   SCIP_CONSDATA* consdata;

   /* find the stp constraint handler */
   conshdlr = SCIPfindConshdlr(scip, CONSHDLR_NAME);
   if( conshdlr == NULL )
   {
      SCIPerrorMessage("stp constraint handler not found\n");
      return SCIP_PLUGINNOTFOUND;
   }

   SCIP_CALL( SCIPallocBlockMemory(scip, &consdata) );

   consdata->graph = graph;

   /* create constraint */
   SCIP_CALL( SCIPcreateCons(scip, cons, name, conshdlr, consdata, FALSE, TRUE, TRUE, TRUE, TRUE,
         FALSE, FALSE, FALSE, FALSE, FALSE) );

   return SCIP_OKAY;
}

/** add cut corresponding to contraction */
SCIP_RETCODE SCIPStpAddContractionCut(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             edge,               /**< edge */
   SCIP_VAR*             revedge,            /**< reversed edge */
   SCIP_Bool             localcut            /**< add local cut? */
)
{
   SCIP_ROW* row = NULL;
   SCIP_CONSHDLR* conshdlr;
   SCIP_Bool infeasible;

   if( SCIPvarGetLbLocal(edge) > 0.5 || SCIPvarGetUbLocal(edge) < 0.5 || SCIPvarGetLbLocal(revedge) > 0.5 || SCIPvarGetUbLocal(revedge) < 0.5 )
   {
      printf("cannot add contraction cut \n");
      return SCIP_OKAY;
   }

   conshdlr = SCIPfindConshdlr(scip, "stp");
   assert(conshdlr != NULL);
   assert(SCIPconshdlrGetNConss(conshdlr) > 0);

   SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "contraction", 1.0, SCIPinfinity(scip), localcut, FALSE, TRUE));
   SCIP_CALL(SCIPcacheRowExtensions(scip, row));

   SCIP_CALL(SCIPaddVarToRow(scip, row, edge, 1.0));
   SCIP_CALL(SCIPaddVarToRow(scip, row, revedge, 1.0));

   SCIP_CALL(SCIPflushRowExtensions(scip, row));

   SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));

#if ADDCUTSTOPOOL
   /* add cut to pool */
   if( !infeasible )
   SCIP_CALL( SCIPaddPoolCut(scip, row) );
#endif

   SCIP_CALL(SCIPreleaseRow(scip, &row));

   return SCIP_OKAY;
}

/** sets graph */
void SCIPStpConshdlrSetGraph(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CONSDATA* consdata;
   SCIP_CONSHDLR* conshdlr;

   conshdlr = SCIPfindConshdlr(scip, "stp");
   assert(conshdlr != NULL);
#ifdef WITH_UG
   if( SCIPconshdlrGetNConss(conshdlr) == 0 )
      return;
#endif

   assert(SCIPconshdlrGetNConss(conshdlr) > 0);
   consdata = SCIPconsGetData(SCIPconshdlrGetConss(conshdlr)[0]);
   assert(consdata != NULL);
   consdata->graph = SCIPprobdataGetGraph2(scip);
   assert(consdata->graph != NULL);
}


/** returns implications start array */
const int* SCIPStpGetPcImplStarts(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CONSHDLR* conshdlr = NULL;
   SCIP_CONSHDLRDATA* conshdlrdata;

   conshdlr = SCIPfindConshdlr(scip, "stp");
   conshdlrdata = SCIPconshdlrGetData(conshdlr);
   assert(conshdlrdata);

   if( !(conshdlrdata->pcimplications) )
      return NULL;

   return sepaspecial_pcimplicationsGetStarts(conshdlrdata->pcimplications);
}


/** returns number implications starts */
int SCIPStpGetPcImplNstarts(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CONSHDLR* conshdlr = NULL;
   SCIP_CONSHDLRDATA* conshdlrdata;

   conshdlr = SCIPfindConshdlr(scip, "stp");
   conshdlrdata = SCIPconshdlrGetData(conshdlr);
   assert(conshdlrdata);

   if( !(conshdlrdata->pcimplications) )
      return 0;

   return sepaspecial_pcimplicationsGetNstarts(conshdlrdata->pcimplications);
}


/** returns implications vertices array */
const int* SCIPStpGetPcImplVerts(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CONSHDLR* conshdlr = NULL;
   SCIP_CONSHDLRDATA* conshdlrdata;

   conshdlr = SCIPfindConshdlr(scip, "stp");
   conshdlrdata = SCIPconshdlrGetData(conshdlr);
   assert(conshdlrdata);

   if( !(conshdlrdata->pcimplications) )
      return NULL;

   return sepaspecial_pcimplicationsGetVerts(conshdlrdata->pcimplications);
}


/**@} */
