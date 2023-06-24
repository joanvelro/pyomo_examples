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

/**@file   heur_local.c
 * @brief  Improvement heuristic for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file implements several local heuristics, including vertex insertion, key-path exchange and key-vertex elimination,
 * ("Fast Local Search for Steiner Trees in Graphs" by Uchoa and Werneck). Other heuristics are for PCSTP and MWCSP.
 *
 * A list of all interface methods can be found in heur_local.h.
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
//#define SCIP_DEBUG
#include <assert.h>
#include <string.h>
#include "heur_local.h"
#include "heur_tm.h"
#include "probdata_stp.h"
#include "cons_stp.h"
#include "solstp.h"
#include "stpvector.h"


/* @note if heuristic is running in root node timing is changed there to (SCIP_HEURTIMING_DURINGLPLOOP |
 *       SCIP_HEURTIMING_BEFORENODE), see SCIP_DECL_HEURINITSOL callback
 */

#define HEUR_NAME             "local"
#define HEUR_DESC             "improvement heuristic for STP"
#define HEUR_DISPCHAR         '-'
#define HEUR_PRIORITY         100
#define HEUR_FREQ             1
#define HEUR_FREQOFS          0
#define HEUR_MAXDEPTH         -1
#define HEUR_TIMING           (SCIP_HEURTIMING_BEFORENODE | SCIP_HEURTIMING_DURINGLPLOOP | SCIP_HEURTIMING_AFTERLPLOOP | SCIP_HEURTIMING_AFTERNODE)

#define HEUR_USESSUBSCIP      FALSE  /**< does the heuristic use a secondary SCIP instance? */

#define DEFAULT_DURINGROOT    TRUE
#define DEFAULT_MAXFREQLOC    FALSE
#define DEFAULT_MAXNBESTSOLS  50
#define DEFAULT_NBESTSOLS     15
#define DEFAULT_NBESTSOLS_HARD 50
#define DEFAULT_NELITESOLS    3
#define DEFAULT_MINNBESTSOLS  10
#define DEFAULT_MINNBESTSOLS_HARD  30
#define DEFAULT_RANDSEED      1492          /**< random seed                                                                       */
#define LOCAL_MAXRESTARTS       10
#define LOCAL_MAXRESTARTS_FAST  3


#define GREEDY_MAXRESTARTS  3  /**< Max number of restarts for greedy PC/MW heuristic if improving solution has been found. */
#define GREEDY_EXTENSIONS_MW 6   /**< Number of extensions for greedy MW heuristic. MUST BE HIGHER THAN GREEDY_EXTENSIONS */
#define GREEDY_EXTENSIONS    5  /**< Number of extensions for greedy PC heuristic. */


/*
 * Data structures
 */

/** primal heuristic data */
struct SCIP_HeurData
{
   int                   nfails;             /**< number of fails */
   int                   maxnsols;           /**< maximal number of best solutions to improve */
   int                   nbestsols;          /**< number of best solutions to improve */
   int                   nbestsols_min;      /**< minimum number of best solutions to improve */
   int*                  lastsolindices;     /**< indices of a number of best solutions already tried */
   SCIP_Bool             maxfreq;            /**< should the heuristic be called with maximum frequency? */
   SCIP_Bool             duringroot;         /**< should the heuristic be called during the root node? */
   SCIP_RANDNUMGEN*      randnumgen;         /**< random number generator                                           */
};

/** Voronoi data for local heuristic */
typedef struct Voronoi_data_structures
{
   PATH*                 vnoi_path;           /**< path */
   int*                  vnoi_base;           /**< base*/
   SCIP_Real*            memvdist;           /**< distance */
   int*                  memvbase;           /**< base*/
   int*                  meminedges;         /**< in-edge */
   int*                  vnoi_nodestate;     /**< node state */
   int                   nmems;              /**< number of memorized elements */
   int                   nkpnodes;           /**< number of key path nodes */
} VNOILOC;


/** Connectivity data */
typedef struct connectivity_data
{
   STP_Vectype(int)*     blists_start;       /**< boundary lists starts (on nodes) */
   STP_Vectype(int)*     lvledges_start;     /**< horizontal edges starts (on nodes) */
   PHNODE**              pheap_boundpaths;   /**< boundary paths (on nodes) */
   int*                  pheap_sizes;        /**< size (on nodes) */
   PHNODE*               pheap_elements;     /**< elements, one per edge! */
   int*                  boundaryedges;      /**< current boundary edge */
   UF*                   uf;                 /**< union find */
   int                   nboundaryedges;     /**< number of boundary edges */
} CONN;


/** Key-paths data */
typedef struct keypaths_data_structures
{
   int* const            kpnodes;            /**< key path nodes */
   int* const            kpedges;            /**< key path edges */
   SCIP_Real             kpcost;             /**< cost of key paths */
   int                   nkpnodes;           /**< number of key path nodes */
   int                   nkpedges;           /**< number of key path edges */
   int                   rootpathstart;      /**< start of key path towards root component */
   int                   kptailnode;         /**< needed for single path */
} KPATHS;


/** Solution tree data */
typedef struct solution_tree_data
{
   STP_Bool* const       solNodes;           /**< Steiner tree nodes */
   LCNODE* const         linkcutNodes;       /**< Steiner tree nodes */
   int* const            solEdges;           /**< array indicating whether an arc is part of the solution (CONNECTED/UNKNOWN) */
   STP_Bool* const       nodeIsPinned;       /**< of size nodes */
   STP_Bool* const       nodeIsScanned;      /**< of size nodes */
   int* const            newedges;           /**< marks new edges of the tree */
} SOLTREE;


/** Super graph data */
typedef struct supergraph_data
{
   int* const            supernodes;         /**< super nodes */
   STP_Bool* const       nodeIsLowSupernode; /**< marks the current super-vertices
                                              * (except for the one representing the root-component) */
   PATH*                 mst;                /**< MST */
   SCIP_Real             mstcost;            /**< cost of MST */
   int                   nsupernodes;        /**< number of super nodes */
} SGRAPH;


/** Prize-collecting/maximum-weight connected subgraph data */
typedef struct pcmw_data
{
   SCIP_Real* const      prize_biased;       /**< prize */
   SCIP_Real* const      edgecost_biased;    /**< fully biased edge costs (PC only) */
   SCIP_Real* const      edgecost_org;       /**< original, fully unbiased, edge costs (PC only) */
   STP_Bool* const       prizemark;          /**< marked? */
   int* const            prizemarklist;      /**< list of all marked */
   int                   nprizemarks;        /**< size of list */
   SCIP_Bool             isActive;           /**< actively used (i.e. Pc/Mw graph)? */
   int                   solroot;            /**< root of Steiner tree solution */
} PCMW;


/** local insertion heuristic data */
typedef struct insertion_data
{
   int*                  chainStarts;        /**< pointers to starts of current chains (nInsertions many) */
   int*                  chainEnds;          /**< pointers to ends of current chains (nInsertions many) */
   const SCIP_Real*      edgecosts;          /**< the edge costs (original for PC) */
   int* const            solDegreeNonTerm;   /**< degree of node [v] in current solution; (pseudo) terminals are marked as UNKNOWN */
   int* const            addedEdges;         /**< added edges */
   int* const            cutedgesStart;      /**< cut edges for the chains */
   int* const            cutedgesEnd;        /**< cut edges for the chains */
   STP_Bool* const       solNodes;           /**< solution nodes */
   SCIP_Bool* const      nodeIsBlocked;      /**< is node [v] blocked? */
   int* const            blockedList;        /**< list of currently blocked nodes */
   int                   blockedListSize;    /**< size of list */
   int                   nInsertions;        /**< number of insertions */
   int                   insertionVertex;    /**< vertex to be inserted */
} INSERT;


/*
 * Local methods
 */


/** prune the solution? */
static inline
SCIP_Bool solNeedsPruning(
   SCIP_SOL*             initialsol          /**< SCIP data structure */
)
{
   assert(initialsol);

   if( SCIPsolGetHeur(initialsol) == NULL )
   {
      return TRUE;
   }

   if( strcmp(SCIPheurGetName(SCIPsolGetHeur(initialsol)), "rec") == 0 )
   {
      return FALSE;
   }

   if( strcmp(SCIPheurGetName(SCIPsolGetHeur(initialsol)), "TM") == 0 )
   {
      return FALSE;
   }

   return TRUE;
}

/** prune the solution? */
static inline
SCIP_RETCODE solPrune(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   int*                  results             /**< Steiner tree edges (in/out) */
)
{
   STP_Bool* steinertree;
   const int nnodes = graph->knots;

#ifndef NDEBUG
   const int nedges = graph->edges;
   const SCIP_Real initialobj = solstp_getObjBounded(graph, results, 0.0, nedges);
#endif

   assert(solstp_isValid(scip, graph, results));

   SCIP_CALL( SCIPallocBufferArray(scip, &steinertree, nnodes) );

   solstp_setVertexFromEdge(graph, results, steinertree);

   SCIP_CALL( solstp_prune(scip, graph, results, steinertree) );

   SCIPfreeBufferArray(scip, &steinertree);

#ifndef NDEBUG
   {
       const SCIP_Real initialobj_pruned = solstp_getObjBounded(graph, results, 0.0, nedges);

       assert(SCIPisLE(scip, initialobj_pruned, initialobj));
   }
#endif

   return SCIP_OKAY;
}


/** fills solution array 'results' */
static inline
void solGetStpSol(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_SOL*             initialsol,         /**< SCIP data structure */
   int*                  results             /**< Steiner tree edges (out) */
)
{
   SCIP_Real* xvals = SCIPprobdataGetXval(scip, initialsol);
   const int nedges = graph_get_nEdges(graph);

   assert(results);
   assert(xvals);

   /* set solution array */
   for( int e = 0; e < nedges; e++ )
   {
      if( SCIPisEQ(scip, xvals[e], 1.0) )
      {
         results[e] = CONNECT;
      }
      else
      {
         assert(SCIPisEQ(scip, xvals[e], 0.0));

         results[e] = UNKNOWN;
      }
   }
}


/** initializes */
static
void initSolNumberBounds(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HEURDATA*        heurdata            /**< heuristic's data */
)
{
   assert(heurdata->nbestsols == -1 && heurdata->nbestsols_min == -1);

   if( SCIPprobdataProbIsAdversarial(scip) )
   {
      heurdata->nbestsols = DEFAULT_NBESTSOLS_HARD;
      heurdata->nbestsols_min = DEFAULT_MINNBESTSOLS_HARD;
   }
   else
   {
      heurdata->nbestsols = DEFAULT_NBESTSOLS;
      heurdata->nbestsols_min = DEFAULT_MINNBESTSOLS;
   }
}


/** tries to add solution stored in 'results' to SCIP solution pool */
static inline
SCIP_RETCODE solAddTry(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sols,               /**< SCIP solutions */
   SCIP_HEUR*            heur,               /**< heuristic */
   const GRAPH*          graph,              /**< graph data structure */
   SCIP_Real             initialsol_obj,     /**< objective */
   SCIP_SOL*             initialsol,         /**< SCIP data structure */
   int*                  results,            /**< Steiner tree edges (out) */
   SCIP_RESULT*          result              /**< pointer */
)
{
   SCIP_Real* newsol_vals;
   const int nedges = graph_get_nEdges(graph);
   SCIP_Bool feasible = FALSE;
   SCIP_HEURDATA* heurdata = SCIPheurGetData(heur);

   assert(heurdata);
   assert(*result == SCIP_DIDNOTFIND);

   SCIP_CALL( SCIPallocBufferArray(scip, &newsol_vals, nedges) );

   for( int v = 0; v < nedges; v++ )
      newsol_vals[v] = (results[v] == CONNECT) ? 1.0 : 0.0;

   SCIP_CALL( SCIPStpValidateSol(scip, graph, newsol_vals, FALSE, &feasible) );

   assert(feasible);

   /* is new solution feasible? */
   if( feasible )
   {
      const SCIP_Real newsol_obj = solstp_getObjBounded(graph, results, 0.0, nedges);
      const SCIP_Bool solIsBetter = SCIPisLT(scip, newsol_obj, initialsol_obj);
      const SCIP_Bool solIsBetter_org
         = SCIPisLT(scip, newsol_obj, SCIPgetSolOrigObj(scip, initialsol) - SCIPprobdataGetOffset(scip));

      assert(SCIPisLE(scip, newsol_obj, initialsol_obj));

#ifndef NDEBUG
      {
         SCIP_Real pobj = 0.0;

         for( int v = 0; v < nedges; v++ )
            pobj += graph->cost[v] * newsol_vals[v];

         assert(SCIPisEQ(scip, pobj, newsol_obj));
      }
#endif

      // todo if solIsBetter || solIsBetter_org

      /* has solution been improved? */
      if( solIsBetter || solIsBetter_org )
      {
         SCIP_SOL* const bestsol = sols[0];
         SCIP_Bool success = FALSE;

         SCIP_CALL( SCIPprobdataAddNewSol(scip, newsol_vals, heur, &success) );

         if( success )
         {
            *result = SCIP_FOUNDSOL;

            if( heurdata->nbestsols < heurdata->maxnsols && SCIPisGT(scip, SCIPgetSolOrigObj(scip, bestsol) - SCIPprobdataGetOffset(scip), newsol_obj) )
            {
               heurdata->nfails = 0;
               heurdata->nbestsols++;
            }
            SCIPdebugMessage("success in local: old: %f new: %f \n", (SCIPgetSolOrigObj(scip, bestsol) - SCIPprobdataGetOffset(scip)), newsol_obj);
         }
      }
   }

   SCIPfreeBufferArray(scip, &newsol_vals);

   if( *result != SCIP_FOUNDSOL )
   {
      heurdata->nfails++;
      if( heurdata->nbestsols > heurdata->nbestsols_min && heurdata->nfails > 1 )
         heurdata->nbestsols--;

      SCIPdebugMessage("fail! %d \n", heurdata->nbestsols);
   }

   return SCIP_OKAY;
}

/** can the problem class be used? */
static inline
SCIP_Bool probtypeIsValidForLocal(
   const GRAPH*          graph               /**< graph data structure */
)
{
   const int stp_type = graph->stp_type;
   if( stp_type != STP_SPG && stp_type != STP_RSMT && stp_type != STP_OARSMT && stp_type != STP_PCSPG
      && stp_type != STP_RPCSPG && stp_type != STP_GSTP && stp_type != STP_MWCSP && stp_type != STP_RMWCSP )
      return FALSE;

   return TRUE;
}

/** submethod for local extend */
static
SCIP_RETCODE addToCandidates(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const PATH*           path,               /**< shortest data structure array */
   int                   i,                  /**< node */
   int                   greedyextensions,   /**< greedyextensions */
   int*                  nextensions,        /**< nextensions */
   GNODE*                candidates,         /**< candidates */
   SCIP_PQUEUE*          pqueue              /**< pqueue */
   )
{
   assert(Is_pseudoTerm(graph->term[i]));
   assert(!graph_pc_knotIsFixedTerm(graph, i));

   if( *nextensions < greedyextensions )
   {
      candidates[*nextensions].dist = graph->prize[i] - path[i].dist;
      candidates[*nextensions].number = i;

      SCIP_CALL( SCIPpqueueInsert(pqueue, &(candidates[(*nextensions)++])) );
   }
   else
   {
      /* get candidate vertex of minimum value */
      GNODE* min = (GNODE*) SCIPpqueueFirst(pqueue);
      if( SCIPisLT(scip, min->dist, graph->prize[i] - path[i].dist) )
      {
         min = (GNODE*) SCIPpqueueRemove(pqueue);
         min->dist = graph->prize[i] - path[i].dist;
         min->number = i;
         SCIP_CALL( SCIPpqueueInsert(pqueue, min) );
      }
   }

   return SCIP_OKAY;
}


/** perform local vertex insertion heuristic on given Steiner tree */
static
void markSolTreeNodes(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const int*            solEdges,           /**< Steiner tree edges */
   LCNODE*               linkcutNodes,       /**< Steiner tree nodes */
   STP_Bool*             solNodes            /**< Steiner tree nodes */
   )
{
   const int nnodes = graph->knots;
   const int nedges = graph->edges;

   assert(solstp_isValid(scip, graph, solEdges));

   for( int i = 0; i < nnodes; i++ )
   {
      solNodes[i] = FALSE;
      SCIPlinkcuttreeInitNode(&linkcutNodes[i]);
   }

   /* create a link-cut tree representing the current Steiner tree */
   for( int e = 0; e < nedges; e++ )
   {
      if( solEdges[e] == CONNECT )
         SCIPlinkcuttreeLink(linkcutNodes, graph->head[e], graph->tail[e], flipedge(e));
   }

   /* mark current Steiner tree nodes */
   for( int e = 0; e < nedges; e++ )
   {
      if( solEdges[e] == CONNECT )
      {
         solNodes[graph->tail[e]] = TRUE;
         solNodes[graph->head[e]] = TRUE;
      }
   }

   solNodes[graph->source] = TRUE;

   assert(linkcutNodes[graph->source].edge == -1);
}


/** is given Steiner tree a trivial solution (i.e. contains only one vertex?) */
static
SCIP_Bool solIsTrivialPcMw(
   const GRAPH*          graph,              /**< graph data structure */
   const int*            solEdges            /**< Steiner tree edges */
)
{
   const int root = graph->source;
   SCIP_Bool isTrivial = TRUE;

   assert(graph_pc_isPcMw(graph));
   assert(graph->extended);

   if( graph_pc_isRootedPcMw(graph) )
   {
      for( int e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
      {
         if( solEdges[e] )
         {
            const int head = graph->head[e];
            if( graph_pc_knotIsFixedTerm(graph, head) || !Is_term(graph->term[head]) )
            {
               isTrivial = FALSE;
               break;
            }
         }
      }
   }
   else
   {
      isTrivial = FALSE;
   }


   if( isTrivial )
   {
      SCIPdebugMessage("trivial solution given \n");
   }

   return isTrivial;
}


/** computes lowest common ancestors for all pairs {vbase(v), vbase(u)} such that {u,w} is a boundary edge,
 * first call should be with u := root */
static
SCIP_RETCODE lca(
   SCIP*                 scip,
   const GRAPH*          graph,
   int                   u,
   UF*                   uf,
   STP_Bool*             nodesmark,
   const int*            steineredges,
   STP_Vectype(int)*     lcalists,
   PHNODE**              boundpaths,
   const int*            heapsize,
   const int*            vbase
   )
{
   int* uboundpaths; /* boundary-paths (each one represented by its boundary edge) having node 'u' as an endpoint */
   uf->parent[u] = u;

   for( int oedge = graph->outbeg[u]; oedge != EAT_LAST; oedge = graph->oeat[oedge] )
   {
      const int v = graph->head[oedge];
      if( steineredges[oedge] == CONNECT )
      {
         SCIP_CALL( lca(scip, graph, v, uf, nodesmark, steineredges, lcalists, boundpaths, heapsize, vbase) );
         SCIPStpunionfindUnion(uf, u, v, FALSE);
         uf->parent[SCIPStpunionfindFind(uf, u)] = u;
      }
   }

   nodesmark[u] = TRUE;

   /* iterate through all boundary-paths having one endpoint in the voronoi region of node 'u' */
   SCIP_CALL( SCIPpairheapBuffarr(scip, boundpaths[u], heapsize[u], &uboundpaths) );

   for( int i = 0; i < heapsize[u]; i++ )
   {
      const int oedge = uboundpaths[i];
      const int v = vbase[graph->head[oedge]];
      if( nodesmark[v] )
      {
         const int ancestor = uf->parent[SCIPStpunionfindFind(uf, v)];

         /* if the ancestor of 'u' and 'v' is one of the two, the boundary-edge is already in boundpaths[u] */
         if( ancestor != u && ancestor != v)
         {
            StpVecPushBack(scip, lcalists[ancestor], oedge);
         }
      }
   }

   /* free the boundary-paths array */
   SCIPfreeBufferArrayNull(scip, &uboundpaths);

   return SCIP_OKAY;
}


/** computes lowest common ancestors for all pairs {vbase(v), vbase(u)} such that {u,w} is a boundary edge */
static
SCIP_RETCODE getLowestCommonAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const VNOILOC*        vnoiData,           /**< Voronoi data */
   const SOLTREE*        soltreeData,        /**< solution tree data */
   CONN*                 connectData         /**< data */
)
{
   PHNODE** const boundpaths = connectData->pheap_boundpaths;
   STP_Vectype(int)* lvledges_start = connectData->lvledges_start;
   const int* const solEdges = soltreeData->solEdges;
   int* const pheapsize = connectData->pheap_sizes;
   const int* const vnoibase = vnoiData->vnoi_base;
   STP_Bool* nodesmark;
   const int nnodes = graph->knots;

   assert(SCIPStpunionfindIsClear(scip, connectData->uf));

   SCIP_CALL( SCIPallocBufferArray(scip, &nodesmark, nnodes) );

   for( int i = 0; i < nnodes; ++i )
      nodesmark[i] = FALSE;

   SCIP_CALL( lca(scip, graph, graph->source, connectData->uf, nodesmark, solEdges, lvledges_start, boundpaths, pheapsize, vnoibase) );

   SCIPfreeBufferArray(scip, &nodesmark);

   return SCIP_OKAY;
}


/** recursive method for a DFS ordering of graph 'g' */
static
void dfsorder(
   const GRAPH*          graph,
   const int*            edges,
   int                   node,
   int*                  counter,
   int*                  dfst
   )
{
   int oedge = graph->outbeg[node];

   while( oedge >= 0 )
   {
      if( edges[oedge] >= 0 )
      {
         dfsorder(graph, edges, graph->head[oedge], counter, dfst);
      }
      oedge = graph->oeat[oedge];
   }

   dfst[*counter] = node;
   (*counter)++;
}


/** helper method for pcmwGetNewEdgePrize */
static inline
SCIP_Real getNewPrizeNode(
   const GRAPH*          graph,
   const STP_Bool*       steinertree,
   const SCIP_Real*      prize_biased,
   const int*            graphmark,
   int                   node,
   STP_Bool*             prizemark,
   int*                  prizemarklist,
   int*                  prizemarkcount
   )
{
   SCIP_Real prizesum = 0.0;
   assert(graph_pc_isPcMw(graph));

   if( graphmark[node] && !steinertree[node] && Is_pseudoTerm(graph->term[node]) && !prizemark[node] )
   {
      prizesum += prize_biased[node];
      prizemark[node] = TRUE;
      prizemarklist[(*prizemarkcount)++] = node;
   }

   return prizesum;
}


/** gets root of solution for Pc/Mw or -1 otherwise */
static
int pcmwGetSolRoot(
   const GRAPH*          graph,
   const int*            solEdges
)
{
   int solroot = -1;

   if( graph_pc_isPcMw(graph) && !graph_pc_isRootedPcMw(graph) )
   {
      const int root = graph->source;

      for( int e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
      {
         const int head = graph->head[e];
         if( solEdges[e] == CONNECT && !Is_term(graph->term[head]) )
         {
            assert(graph->cost[e] == 0.0);
            solroot = head;
            break;
         }
      }
      assert(solroot >= 0);
   }

   return solroot;
}


/** get prize not already counted that is associated to edge, not including solution nodes or forbidden ones */
static
SCIP_Real pcmwGetNewEdgePrize(
   const GRAPH*          graph,              /**< graph */
   const STP_Bool*       steinertree,        /**< Steiner tree */
   int                   edge,               /**< the edge */
   PCMW*                 pcmwData            /**< data */
   )
{
   SCIP_Real prizesum = 0.0;

   assert(graph && steinertree && pcmwData);

   if( pcmwData->isActive )
   {
      const int mhead = graph->head[edge];
      const int mtail = graph->tail[edge];
      STP_Bool* const prizemark = pcmwData->prizemark;
      int* const prizemarklist = pcmwData->prizemarklist;

      assert(pcmwData->prize_biased);
      assert(graph_pc_isPcMw(graph));

      prizesum += getNewPrizeNode(graph, steinertree, pcmwData->prize_biased, graph->mark, mhead,
         prizemark, prizemarklist, &(pcmwData->nprizemarks));
      prizesum += getNewPrizeNode(graph, steinertree, pcmwData->prize_biased, graph->mark, mtail,
         prizemark, prizemarklist, &(pcmwData->nprizemarks));
   }

   return prizesum;
}


#ifndef NDEBUG
/** is the data clean? */
static
SCIP_Bool pcmwDataIsClean(
   const GRAPH*                graph,              /**< graph data structure */
   const PCMW*                 pcmwData            /**< data */
)
{
   if( !pcmwData->isActive )
      return TRUE;

   if( pcmwData->nprizemarks != 0 )
   {
      SCIPdebugMessage("nprizemarks != 0 \n");
      return FALSE;
   }

   for( int k = 0; k < graph->knots; k++ )
   {
      if( pcmwData->prizemark[k] )
      {
         SCIPdebugMessage("entry %d is dirty \n", k);
         return FALSE;
      }
   }

   return TRUE;
}
#endif


/** clean the data */
static
void pcmwDataClean(
   const GRAPH*                graph,              /**< graph data structure */
   PCMW*                       pcmwData            /**< data */
)
{
   if( pcmwData->isActive )
   {
      const int* const prizemarklist = pcmwData->prizemarklist;
      STP_Bool* const prizemark = pcmwData->prizemark;
      const int prizemarkcount = pcmwData->nprizemarks;


      for( int pi = 0; pi < prizemarkcount; pi++ )
         prizemark[prizemarklist[pi]] = FALSE;

      pcmwData->nprizemarks = 0;

      assert(pcmwDataIsClean(graph, pcmwData));
   }
}


/** updates for PC/MW: graph->mark and pinned */
static
SCIP_RETCODE pcmwUpdate(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   SOLTREE*              soltreeData,        /**< solution tree data */
   PCMW*                 pcmwData            /**< data */
   )
{
   SCIP_Bool* reachable_nodes;
   STP_Bool* const pinned = soltreeData->nodeIsPinned;
   const int root = graph->source;
   const int nnodes = graph->knots;
   int* const graphmark = graph->mark;
   const int solstartnode = graph_pc_isRootedPcMw(graph)? graph->source : pcmwData->solroot;

   assert(graph->extended);
   assert(graph_pc_isPcMw(graph));
   assert(solstartnode >= 0 && solstartnode < graph->knots);
   assert(soltreeData->solNodes[solstartnode]);

   /* remove unconnected vertices */
   SCIP_CALL( SCIPallocBufferArray(scip, &reachable_nodes, nnodes) );

   SCIP_CALL( graph_trail_costAware(scip, graph, solstartnode, reachable_nodes) );

   for( int k = 0; k < nnodes; k++ )
      graphmark[k] = reachable_nodes[k];

   SCIPfreeBufferArray(scip, &reachable_nodes);

   /* unmark and pin pseudo terminals */
   for( int e = graph->outbeg[root]; e != EAT_LAST; e = graph->oeat[e] )
   {
      const int k = graph->head[e];
      if( Is_term(graph->term[k]) )
      {
         if( !graph_pc_knotIsFixedTerm(graph, k) )
         {
            const int pterm = graph->head[graph->term2edge[k]];

            assert(Is_pseudoTerm(graph->term[pterm]));

            graphmark[k] = FALSE;
            pinned[pterm] = TRUE;
         }
      }
   }

   if( !graph_pc_isRootedPcMw(graph) )
   {
      graphmark[root] = FALSE;
      soltreeData->solNodes[root] = FALSE;
   }

   return SCIP_OKAY;
}


/** checks whether node is crucial, i.e. a terminal or a vertex with degree at least 3 (w.r.t. the Steiner tree) */
static
STP_Bool nodeIsCrucial(
   const GRAPH*          graph,              /**< graph data structure */
   const int*            steineredges,       /**< Steiner edges */
   int                   node                /**< node to check */
)
{
   assert(graph != NULL);
   assert(steineredges != NULL);
   assert(node >= 0 && node < graph->knots);

   if( !Is_term(graph->term[node]) )
   {
      int counter = 0;

      for( int e = graph->outbeg[node]; e != EAT_LAST; e = graph->oeat[e] )
      {
         /* check if the adjacent node is in the Steiner tree */
         if( steineredges[e] == CONNECT || steineredges[flipedge(e)] == CONNECT )
            counter++;
      }

      /* is node of degree smaller 3 in the Steiner tree? */
      if( counter < 3 )
      {
         return FALSE;
      }
   }

   return TRUE;
}


/** gets unbiased edge cost */
static inline
SCIP_Real getEdgeCostUnbiased(
   const GRAPH*          graph,              /**< graph data structure */
   const PCMW*           pcmwData,           /**< data */
   int                   edge                /**< edge */
   )
{
   SCIP_Real cost;

   assert(graph && pcmwData);
   assert(edge >= 0 && edge < graph->edges);

   if( pcmwData->isActive )
   {
      assert(graph->extended);

      if( !graph_pc_isPc(graph) )
      {
         assert(graph->stp_type == STP_MWCSP || graph->stp_type == STP_RMWCSP);

         cost = 0.0;
      }
      else
      {
         assert(pcmwData->edgecost_org);

         cost = pcmwData->edgecost_org[edge];
      }
   }
   else
   {
      cost = graph->cost[edge];
   }

   return cost;
}


/** Get cost of shortest path along boundary edge.
 *  For Pc/Mw: Will consider biased edge costs, but not the actual prizes of proper terminals! */
static
SCIP_Real getBoundaryPathCost(
   const GRAPH*          graph,              /**< graph data structure */
   const VNOILOC*        vnoiData,           /**< data */
   const PCMW*           pcmwData,           /**< data */
   int                   boundaryedge        /**< boundary edge*/
   )
{
   const PATH* const vnoipath = vnoiData->vnoi_path;
   SCIP_Real pathcost;
   const int tail = graph->tail[boundaryedge];
   const int head = graph->head[boundaryedge];

   assert(boundaryedge >= 0);
   assert(vnoiData->vnoi_base[tail] != vnoiData->vnoi_base[head]);
   assert(graph_pc_isPcMw(graph) == pcmwData->isActive);

   pathcost = vnoipath[tail].dist + vnoipath[head].dist;

   if( pcmwData->isActive )
   {
      /* either vnoipath[head] goes into head, or head is the end of the entire boundary path;
       * either way we don't want to subtract the prize! */
      pathcost += getEdgeCostUnbiased(graph, pcmwData, boundaryedge);
   }
   else
   {
      pathcost += graph->cost[boundaryedge];
   }

   assert(pathcost >= 0.0);

   return pathcost;
}


/**  For Pc/Mw: Consider biased edge costs, and the actual prizes of proper terminals! */
static
SCIP_Real getBoundaryPathCostPrized(
   const GRAPH*          graph,              /**< graph data structure */
   const VNOILOC*        vnoiData,           /**< data */
   const SOLTREE*        soltreeData,        /**< solution tree data */
   int                   boundaryedge,       /**< boundary edge */
   PCMW*                 pcmwData            /**< data */
   )
{
   SCIP_Real pathcost;

   assert(boundaryedge >= 0);
   assert(vnoiData->vnoi_base[graph->tail[boundaryedge]] != vnoiData->vnoi_base[graph->head[boundaryedge]]);
   assert(graph_pc_isPcMw(graph) == pcmwData->isActive);

   pathcost = getBoundaryPathCost(graph, vnoiData, pcmwData, boundaryedge);

   if( pcmwData->isActive )
   {
      const STP_Bool* const solNodes = soltreeData->solNodes;
      const PATH* const vnoipath = vnoiData->vnoi_path;
      const int* const vnoibase = vnoiData->vnoi_base;

#ifdef SCIP_DEBUG
      printf("boundary path edge ");
      graph_edge_printInfo(graph, boundaryedge);
#endif
      pathcost -= pcmwGetNewEdgePrize(graph, solNodes, boundaryedge, pcmwData);

      for( int node = graph->tail[boundaryedge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
      {
#ifdef SCIP_DEBUG
         printf("boundary path edge ");
         graph_edge_printInfo(graph, vnoipath[node].edge);
#endif
         pathcost -= pcmwGetNewEdgePrize(graph, solNodes, vnoipath[node].edge, pcmwData);
      }

      for( int node = graph->head[boundaryedge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
      {
#ifdef SCIP_DEBUG
         printf("boundary path edge ");
         graph_edge_printInfo(graph, vnoipath[node].edge);
#endif

         pathcost -= pcmwGetNewEdgePrize(graph, solNodes, vnoipath[node].edge, pcmwData);
      }

      if( pathcost < 0.0 )
         pathcost = 0.0;
   }

   assert(pathcost >= 0.0);

   return pathcost;
}


/** returns edge costs (biased for Pc/Mw) */
static inline
const SCIP_Real* getEdgeCosts(
   const GRAPH*          graph,              /**< graph data structure */
   const PCMW*           pcmwData            /**< data */
)
{
   assert( graph_pc_isPcMw(graph) == pcmwData->isActive);

   return (pcmwData->isActive? pcmwData->edgecost_biased : graph->cost);
}


/** update for key-vertex elimination: Save current boundary edges */
static
SCIP_RETCODE connectivityDataKeyElimUpdate(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const VNOILOC*        vnoiData,           /**< Voronoi data */
   const SGRAPH*         supergraphData,     /**< super-graph*/
   int                   crucnode,           /**< node to eliminate */
   CONN*                 connectData         /**< data */
)
{
   PHNODE** const boundpaths = connectData->pheap_boundpaths;
   STP_Vectype(int)* lvledges_start = connectData->lvledges_start;
   STP_Vectype(int) lvledges_curr = NULL;
   UF* const uf = connectData->uf;
   int* const pheapsize = connectData->pheap_sizes;
   const int* const supernodes = supergraphData->supernodes;
   const int* const vnoibase = vnoiData->vnoi_base;
   const STP_Bool* const isLowSupernode = supergraphData->nodeIsLowSupernode;
   int* const boundaryedges = connectData->boundaryedges;
   const int* const graphmark = graph->mark;
   int nboundaryedges = 0;

   connectData->nboundaryedges = -1;

   /* add vertical boundary-paths between the child components and the root-component (w.r.t. node 'crucnode') */
   for( int k = 0; k < supergraphData->nsupernodes - 1; k++ )
   {
      const int supernode = supernodes[k];
      int edge = UNKNOWN;

      while( boundpaths[supernode] != NULL )
      {
         int node;
         SCIP_Real edgecost;

         SCIP_CALL( SCIPpairheapDeletemin(scip, &edge, &edgecost, &boundpaths[supernode], &pheapsize[supernode]) );

         node = (vnoibase[graph->head[edge]] == UNKNOWN)? UNKNOWN : SCIPStpunionfindFind(uf, vnoibase[graph->head[edge]]);

         /* check whether edge 'edge' represents a boundary-path having an endpoint in the kth-component and in the root-component respectively */
         if( node != UNKNOWN && !isLowSupernode[node] && graphmark[node] )
         {
            boundaryedges[nboundaryedges++] = edge;
            SCIP_CALL( SCIPpairheapInsert(scip, &boundpaths[supernode],
                  connectData->pheap_elements, edge, edgecost, &pheapsize[supernode]) );
            break;
         }
      }
   }

   lvledges_curr = lvledges_start[crucnode];

   /* add horizontal boundary-paths (between the child-components) */
   for( int i = 0; i < StpVecGetSize(lvledges_curr); i++ )
   {
      const int edge = lvledges_curr[i];
      const int basetail = vnoibase[graph->tail[edge]];
      const int basehead = vnoibase[graph->head[edge]];
      const int node = (basehead == UNKNOWN)? UNKNOWN : SCIPStpunionfindFind(uf, basehead);
      const int adjnode = (basetail == UNKNOWN)? UNKNOWN : SCIPStpunionfindFind(uf, basetail);

      /* check whether the current boundary-path connects two child components */
      if( node != UNKNOWN && isLowSupernode[node] && adjnode != UNKNOWN && isLowSupernode[adjnode] )
      {
         assert(graphmark[node] && graphmark[adjnode]);
         boundaryedges[nboundaryedges++] = edge;
      }
   }

   connectData->nboundaryedges = nboundaryedges;

   return SCIP_OKAY;
}


/** initialize */
static
SCIP_RETCODE connectivityDataInit(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const VNOILOC*        vnoiData,           /**< Voronoi data */
   const SOLTREE*        soltreeData,        /**< solution tree data */
   const PCMW*           pcmwData,           /**< data */
   CONN*                 connectData         /**< data */
)
{
   PHNODE** const boundpaths = connectData->pheap_boundpaths;
   STP_Vectype(int)* blists_start = connectData->blists_start;
   STP_Vectype(int)* lvledges_start = connectData->lvledges_start;
   int* const pheapsize = connectData->pheap_sizes;
   const int* const vnoibase = vnoiData->vnoi_base;
   const int* const graphmark = graph->mark;
   const int nnodes = graph->knots;
   const int nedges = graph->edges;

   assert(connectData->nboundaryedges == 0);
   assert(connectData->boundaryedges);

   for( int k = 0; k < nnodes; k++ )
      assert(graph->path_state[k] == CONNECT || !graph->mark[k]);

   for( int k = 0; k < nnodes; ++k )
   {
      if( blists_start[k] )
         StpVecClear(blists_start[k]);
      if( lvledges_start[k] )
         StpVecClear(lvledges_start[k]);
   }

   BMSclearMemoryArray(pheapsize, nnodes);
   BMSclearMemoryArray(boundpaths, nnodes);

   /* link all nodes to their (respective) Voronoi base */
   for( int k = 0; k < nnodes; ++k )
   {
      assert(pheapsize[k] == 0 && boundpaths[k] == NULL);

      if( !graphmark[k] )
         continue;

      StpVecPushBack(scip, blists_start[vnoibase[k]], k);
   }

   /* for each node, store all of its outgoing boundary-edges in a (respective) heap*/
   for( int e = 0; e < nedges; e += 2 )
   {
      if( graph->oeat[e] != EAT_FREE )
      {
         const int tail = graph->tail[e];
         const int head = graph->head[e];

         /* is edge 'e' a boundary-edge? */
         if( vnoibase[tail] != vnoibase[head] && graphmark[tail] && graphmark[head] )
         {
            const SCIP_Real edgecost = getBoundaryPathCost(graph, vnoiData, pcmwData, e);

            assert(vnoibase[tail] != UNKNOWN && vnoibase[head] != UNKNOWN);
            assert(SCIPisGE(scip, edgecost, 0.0));

            /* add the boundary-edge 'e' and its reversed to the corresponding heaps */
            SCIP_CALL( SCIPpairheapInsert(scip, &boundpaths[vnoibase[tail]],
                  connectData->pheap_elements, e, edgecost, &(pheapsize[vnoibase[tail]])) );
            SCIP_CALL( SCIPpairheapInsert(scip, &boundpaths[vnoibase[head]],
                  connectData->pheap_elements, flipedge(e), edgecost, &(pheapsize[vnoibase[head]])) );
         }
      }
   }

   SCIP_CALL( getLowestCommonAncestors(scip, graph, vnoiData, soltreeData, connectData) );

   return SCIP_OKAY;
}


/** get key path above given crucial node */
static
void getKeyPathUpper(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   crucnode,           /**< crucial node to start from */
   const GRAPH*          graph,              /**< graph data structure */
   const SOLTREE*        soltreeData,        /**< solution tree data */
   const PCMW*           pcmwData,           /**< data */
   CONN*                 connectData,        /**< data */
   KPATHS*               keypathsData,       /**< key paths */
   SCIP_Bool*            allGood             /**< all good? */
)
{
   int* const kpnodes = keypathsData->kpnodes;
   const int* const solEdges = soltreeData->solEdges;
   const LCNODE* const linkcutNodes = soltreeData->linkcutNodes;
   const STP_Bool* const solNodes = soltreeData->solNodes;
   const STP_Bool* const pinned = soltreeData->nodeIsPinned;
   PHNODE** const boundpaths = connectData->pheap_boundpaths;
   const SCIP_Real* const edgecosts = getEdgeCosts(graph, pcmwData);
   int* const pheapsize = connectData->pheap_sizes;
   const int* const graphmark = graph->mark;
   int nkpnodes = 0;
   int firstedge2root = -1;
   int kptailnode = -1;
   SCIP_Real kpcost = -FARAWAY;

   assert(*allGood);


   if( Is_term(graph->term[crucnode]) || pinned[crucnode] )
   {
      /* update the data structures; basically merge the whole key path */
      for( int edge = graph->outbeg[crucnode]; edge != EAT_LAST; edge = graph->oeat[edge] )
      {
         int adjnode = graph->head[edge];

         /* check whether edge 'edge' leads to an ancestor of terminal 'crucnode' */
         if( solEdges[edge] == CONNECT && solNodes[adjnode] && graphmark[adjnode] )
         {
            assert( SCIPStpunionfindFind(connectData->uf, adjnode) != crucnode);
            assert(soltreeData->nodeIsScanned[adjnode]);

            SCIPpairheapMeldheaps(scip, &boundpaths[crucnode], &boundpaths[adjnode], &pheapsize[crucnode], &pheapsize[adjnode]);

            /* update the union-find data structure */
            SCIPStpunionfindUnion(connectData->uf, crucnode, adjnode, FALSE);

            /* move along the key-path until its end (i.e. until a crucial node is reached) */
            while( !nodeIsCrucial(graph, solEdges, adjnode) && !pinned[adjnode] )
            {
               int e;
               for( e = graph->outbeg[adjnode]; e != EAT_LAST; e = graph->oeat[e] )
               {
                  if( solEdges[e] == CONNECT )
                     break;

                  assert(UNKNOWN == solEdges[e]);
               }

               /* leaf of the Steiner tree is not a terminal? NOTE: should happen very rarely */
               if( e == EAT_LAST )
               {
                  *allGood = FALSE;
                  return;
               }

               adjnode = graph->head[e];

               if( !solNodes[adjnode] || !graphmark[adjnode] )
                  break;

               assert(soltreeData->nodeIsScanned[adjnode]);
               assert(SCIPStpunionfindFind(connectData->uf, adjnode) != crucnode);

               /* update the union-find data structure */
               SCIPStpunionfindUnion(connectData->uf, crucnode, adjnode, FALSE);

               SCIPpairheapMeldheaps(scip, &boundpaths[crucnode], &boundpaths[adjnode], &pheapsize[crucnode], &pheapsize[adjnode]);
            }
         }
      }
   }

#ifndef NDEBUG
   if( SCIPisGE(scip, graph->cost[linkcutNodes[crucnode].edge], FARAWAY)
      || SCIPisGE(scip, graph->cost[flipedge(linkcutNodes[crucnode].edge)], FARAWAY) )
   {
      assert(graph_pc_isPcMw(graph));
      assert(graph->head[linkcutNodes[crucnode].edge] == graph->source);
   }
#endif

   /* traverse the (unique) key-path containing the parent of the current crucial node 'crucnode' */

   firstedge2root = linkcutNodes[crucnode].edge;
   kptailnode = graph->head[firstedge2root];

   if( pcmwData->isActive )
   {
      /* we don't want to count the prize of 'crucnode' */
      kpcost = getEdgeCostUnbiased(graph, pcmwData, flipedge(firstedge2root));
   }
   else
   {
      kpcost = edgecosts[firstedge2root];
   }

   assert(graph->tail[firstedge2root] == crucnode);
   assert(soltreeData->solEdges[flipedge(firstedge2root)] == CONNECT);

#ifdef SCIP_DEBUG
   printf("key path edge ");
   graph_edge_printInfo(graph, firstedge2root);
#endif

   while( !nodeIsCrucial(graph, solEdges, kptailnode) && !pinned[kptailnode] )
   {
      const int kpedge2root = linkcutNodes[kptailnode].edge;
      kpcost += edgecosts[flipedge(kpedge2root)];

#ifdef SCIP_DEBUG
      printf("key path edge ");
      graph_edge_printInfo(graph, kpedge2root);
#endif

      kpnodes[nkpnodes++] = kptailnode;
      kptailnode = graph->head[kpedge2root];
   }

   keypathsData->kpcost = kpcost;
   keypathsData->kptailnode = kptailnode;
   keypathsData->nkpnodes = nkpnodes;
}


/** unmarks key path nodes */
static
void soltreeUnmarkKpNodes(
   const GRAPH*          graph,              /**< graph data structure */
   const KPATHS*         keypathsData,       /**< key paths */
   SOLTREE*              soltreeData         /**< solution tree data */
)
{
   const int* const kpnodes = keypathsData->kpnodes;
   STP_Bool* const solNodes = soltreeData->solNodes;
   const int nkpnodes = keypathsData->nkpnodes;

   for( int k = 0; k < nkpnodes; k++ )
   {
      const int node = kpnodes[k];

      assert(node >= 0 && node < graph->knots);
      assert(solNodes[node]);

      solNodes[node] = FALSE;
   }
}


/** (re-)marks key path nodes */
static
void soltreeMarkKpNodes(
   const GRAPH*          graph,              /**< graph data structure */
   const KPATHS*         keypathsData,       /**< key paths */
   SOLTREE*              soltreeData         /**< solution tree data */
)
{
   const int* const kpnodes = keypathsData->kpnodes;
   STP_Bool* const solNodes = soltreeData->solNodes;
   const int nkpnodes = keypathsData->nkpnodes;

   for( int k = 0; k < nkpnodes; k++ )
   {
      const int node = kpnodes[k];

      assert(node >= 0 && node < graph->knots);
      assert(!solNodes[node]);

      solNodes[node] = TRUE;
   }
}


/** exchanges key path */
static
SCIP_RETCODE soltreeExchangeKeyPath(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const CONN*           connectData,        /**< data */
   const VNOILOC*        vnoiData,           /**< data */
   const KPATHS*         keypathsData,       /**< key paths */
   const int*            dfstree,            /**< DFS tree */
   const STP_Bool*       scanned,            /**< array to mark which nodes have been scanned */
   int                   dfstree_pos,        /**< current position in dfs tree */
   int                   boundedge_new,      /**< new Voronoi boundary edge */
   SOLTREE*              soltreeData         /**< solution tree data */
)
{
   UF* const uf = connectData->uf;
   PHNODE** const boundpaths = connectData->pheap_boundpaths;
   int* const pheapsize = connectData->pheap_sizes;
   const PATH* const vnoipath = vnoiData->vnoi_path;
   const int* const vnoibase = vnoiData->vnoi_base;
   const int* const kpnodes = keypathsData->kpnodes;
   STP_Bool* const pinned = soltreeData->nodeIsPinned;
   const LCNODE* const linkcutNodes = soltreeData->linkcutNodes;
   int* const solEdges = soltreeData->solEdges;
   STP_Bool* const solNodes = soltreeData->solNodes;
   const int nkpnodes = keypathsData->nkpnodes;
   const int crucnode = dfstree[dfstree_pos];
   int* const graphmark = graph->mark;
   int newpathend = -1;
   int newedge = boundedge_new;
   int node = SCIPStpunionfindFind(uf, vnoibase[graph->head[newedge]]);

   /* remove old keypath */
   assert(solEdges[flipedge(linkcutNodes[crucnode].edge)] != UNKNOWN);

   solEdges[flipedge(linkcutNodes[crucnode].edge)] = UNKNOWN;
   solNodes[crucnode] = FALSE;
   graphmark[crucnode] = FALSE;

   for( int k = 0; k < nkpnodes; k++ )
   {
      const int keypathnode = kpnodes[k];
      assert(solEdges[flipedge(linkcutNodes[keypathnode].edge)] != UNKNOWN);

      solEdges[flipedge(linkcutNodes[keypathnode].edge)] = UNKNOWN;
      solNodes[keypathnode] = FALSE;
      graphmark[keypathnode] = FALSE;
   }

   assert(graphmark[keypathsData->kptailnode]);

   if( node == crucnode )
      newedge = flipedge(newedge);

   for( node = graph->tail[newedge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
   {
      graphmark[node] = FALSE;

      solEdges[flipedge(vnoipath[node].edge)] = CONNECT;
      solEdges[vnoipath[node].edge] = UNKNOWN;
   }

   for( node = graph->head[newedge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
   {
      graphmark[node] = FALSE;

      solEdges[vnoipath[node].edge] = CONNECT;
   }

   solEdges[flipedge(newedge)] = CONNECT;

   newpathend = vnoibase[graph->tail[newedge]];
   assert(node == vnoibase[graph->head[newedge]] );

   /* flip all edges on the ST path between the endnode of the new key-path and the current crucial node */
   assert(SCIPStpunionfindFind(uf, newpathend) == crucnode);

   for( int k = newpathend; k != crucnode; k = graph->head[linkcutNodes[k].edge] )
   {
      assert(graphmark[k]);
      assert( solEdges[flipedge(linkcutNodes[k].edge)] != -1);

      solEdges[flipedge(linkcutNodes[k].edge)] = UNKNOWN;
      solEdges[linkcutNodes[k].edge] = CONNECT;
   }

   for( int k = 0; k < dfstree_pos; k++ )
   {
      if( crucnode == SCIPStpunionfindFind(uf, dfstree[k]) )
      {
         graphmark[dfstree[k]] = FALSE;
         solNodes[dfstree[k]] = FALSE;
      }
   }

   /* update union find */
   if( !Is_term(graph->term[node]) && scanned[node] && !pinned[node] && SCIPStpunionfindFind(uf, node) == node )
   {
      for( int edge = graph->outbeg[node]; edge != EAT_LAST; edge = graph->oeat[edge] )
      {
         int adjnode = graph->head[edge];

         /* check whether edge 'edge' leads to an ancestor of terminal 'node' */
         if( solEdges[edge] == CONNECT && solNodes[adjnode] && graphmark[adjnode] && SCIPStpunionfindFind(uf, adjnode) != node )
         {
            assert(scanned[adjnode]);
            /* meld the heaps */
            SCIPpairheapMeldheaps(scip, &boundpaths[node], &boundpaths[adjnode], &pheapsize[node], &pheapsize[adjnode]);

            /* update the union-find data structure */
            SCIPStpunionfindUnion(uf, node, adjnode, FALSE);

            /* move along the key-path until its end (i.e. until a crucial node is reached) */
            while( !nodeIsCrucial(graph, solEdges, adjnode) && !pinned[adjnode] )
            {
               int e;
               for( e = graph->outbeg[adjnode]; e != EAT_LAST; e = graph->oeat[e] )
               {
                  if( solEdges[e] != -1 )
                     break;
               }

               /* assert that each leaf of the ST is a terminal */
               assert( e != EAT_LAST );
               adjnode = graph->head[e];

               if( !solNodes[adjnode]  )
                  break;

               assert(scanned[adjnode]);
               assert(SCIPStpunionfindFind(uf, adjnode) != node);

               /* update the union-find data structure */
               SCIPStpunionfindUnion(uf, node, adjnode, FALSE);

               /* meld the heaps */
               SCIPpairheapMeldheaps(scip, &boundpaths[node], &boundpaths[adjnode], &pheapsize[node], &pheapsize[adjnode]);
            }
         }
      }

   }
   pinned[node] = TRUE;

   return SCIP_OKAY;
}



/** exchanges key-paths star */
static
SCIP_RETCODE soltreeElimKeyPathsStar(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const CONN*           connectData,        /**< data */
   const VNOILOC*        vnoiData,           /**< data */
   const KPATHS*         keypathsData,       /**< key paths */
   const SGRAPH*         supergraphData,     /**< super-graph */
   const int*            dfstree,            /**< DFS tree */
   const STP_Bool*       scanned,            /**< array to mark which nodes have been scanned */
   int                   dfstree_pos,        /**< current position in dfs tree */
   SOLTREE*              soltreeData         /**< solution tree data */
)
{
   const PATH* mst = supergraphData->mst;
   UF* const uf = connectData->uf;
   const STP_Bool* const isSupernode = supergraphData->nodeIsLowSupernode;
   PHNODE** const boundpaths = connectData->pheap_boundpaths;
   int* const pheapsize = connectData->pheap_sizes;
   const PATH* const vnoipath = vnoiData->vnoi_path;
   const int* const vnoibase = vnoiData->vnoi_base;
   const int* const boundedges = connectData->boundaryedges;
   const int* const kpnodes = keypathsData->kpnodes;
   const int* const kpedges = keypathsData->kpedges;
   STP_Bool* const pinned = soltreeData->nodeIsPinned;
   const LCNODE* const linkcutNodes = soltreeData->linkcutNodes;
   int* const solEdges = soltreeData->solEdges;
   int* const graphmark = graph->mark;
   STP_Bool* const solNodes = soltreeData->solNodes;
   const int nkpnodes = keypathsData->nkpnodes;
   const int nkpedges = keypathsData->nkpedges;
   const int nsupernodes = supergraphData->nsupernodes;

   /* unmark the original edges spanning the supergraph */
   for( int e = 0; e < nkpedges; e++ )
   {
      assert(solEdges[kpedges[e]] == CONNECT);
      solEdges[kpedges[e]] = UNKNOWN;
   }

   /* mark all Steiner tree nodes except for those belonging to the root-component as forbidden */
   for( int k = keypathsData->rootpathstart; k < nkpnodes; k++ )
   {
      graphmark[kpnodes[k]] = FALSE;
      solNodes[kpnodes[k]] = FALSE;
   }

   for( int k = 0; k < dfstree_pos; k++ )
   {
      const int node = SCIPStpunionfindFind(uf, dfstree[k]);
      if( isSupernode[node] || node == dfstree[dfstree_pos] )
      {
         graphmark[dfstree[k]] = FALSE;
         solNodes[dfstree[k]] = FALSE;
      }
   }

   /* add the new edges reconnecting the (super-) components */
   for( int l = 0; l < nsupernodes - 1; l++ )
   {
      const int edge = (mst[l].edge % 2 == 0)? boundedges[mst[l].edge / 2] : flipedge(boundedges[mst[l].edge / 2]);

      /* change the orientation within the target-component if necessary */
      if( !isSupernode[vnoibase[graph->head[edge]]] )
      {
         int node = vnoibase[graph->head[edge]];
         const int nodebase = SCIPStpunionfindFind(uf, node);
         assert(isSupernode[nodebase]);

         while( node != nodebase )
         {
            /* the Steiner tree edge pointing towards the root */
            const int e = linkcutNodes[node].edge;

            assert(solEdges[e] == UNKNOWN && solEdges[flipedge(e)] == CONNECT );

            solEdges[e] = CONNECT;
            solEdges[flipedge(e)] = UNKNOWN;
            node = graph->head[e];
         }
      }

      /* is the vbase of the current boundary-edge tail in the root-component? */
      if( !isSupernode[SCIPStpunionfindFind(uf, vnoibase[graph->tail[edge]])] )
      {
         int node;
         solEdges[edge] = CONNECT;

         for( node = graph->tail[edge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
         {
            graphmark[node] = FALSE;

            if( solEdges[flipedge(vnoipath[node].edge)] == CONNECT )
               solEdges[flipedge(vnoipath[node].edge)] = UNKNOWN;

            solEdges[vnoipath[node].edge] = CONNECT;
         }

         assert(!isSupernode[node] && vnoibase[node] == node);
         assert(graphmark[node]);

         /* is the pinned node its own component identifier? */
         if( !Is_term(graph->term[node]) && scanned[node] && !pinned[node] && SCIPStpunionfindFind(uf, node) == node )
         {
            graphmark[graph->head[edge]] = FALSE;

            for( int oedge = graph->outbeg[node]; oedge != EAT_LAST; oedge = graph->oeat[oedge] )
            {
               int head = graph->head[oedge];

               /* check whether edge 'edge' leads to an ancestor of 'node' */
               if( solEdges[oedge] == CONNECT && graphmark[head] && solNodes[head]  && SCIPStpunionfindFind(uf, head) != node )
               {
                  assert(scanned[head]);

                  SCIPpairheapMeldheaps(scip, &boundpaths[node], &boundpaths[head], &pheapsize[node], &pheapsize[head]);

                  /* update the union-find data structure */
                  SCIPStpunionfindUnion(uf, node, head, FALSE);

                  /* move along the key-path until its end (i.e. until a crucial node is reached) */
                  while( !nodeIsCrucial(graph, solEdges, head) && !pinned[head] )
                  {
                     int e;
                     for( e = graph->outbeg[head]; e != EAT_LAST; e = graph->oeat[e] )
                     {
                        if( solEdges[e] != -1 )
                           break;
                     }

                     /* assert that each leaf of the ST is a terminal */
                     assert( e != EAT_LAST );
                     head = graph->head[e];

                     if( !solNodes[head]  )
                        break;

                     assert(scanned[head]);
                     assert(SCIPStpunionfindFind(uf, head) != node);

                     /* update the union-find data structure */
                     SCIPStpunionfindUnion(uf, node, head, FALSE);

                     SCIPpairheapMeldheaps(scip, &boundpaths[node], &boundpaths[head], &pheapsize[node], &pheapsize[head]);
                  }
               }
            }
         }

         /* mark the start node (lying in the root-component of the Steiner tree) of the current boundary-path as pinned,
          * so that it may not be removed later on */
         pinned[node] = TRUE;

         for( node = graph->head[edge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
         {
            graphmark[node] = FALSE;
            if( solEdges[vnoipath[node].edge] == CONNECT )
               solEdges[vnoipath[node].edge] = UNKNOWN;

            solEdges[flipedge(vnoipath[node].edge)] = CONNECT;
         }
      }
      else /* the vbase of the current boundary-edge tail is NOT in the root-component */
      {
         solEdges[edge] = CONNECT;

         for( int node = graph->tail[edge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
         {
            graphmark[node] = FALSE;
            if( solEdges[vnoipath[node].edge] != CONNECT && solEdges[flipedge(vnoipath[node].edge)] != CONNECT )
            {
               solEdges[vnoipath[node].edge] = CONNECT;
            }
         }

         for( int node = graph->head[edge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
         {
            graphmark[node] = FALSE;

            solEdges[flipedge(vnoipath[node].edge)] = CONNECT;
            solEdges[vnoipath[node].edge] = UNKNOWN;
         }
      }
   }

   for( int k = 0; k < nkpnodes; k++ )
   {
      assert(graphmark[kpnodes[k]] == FALSE);
      assert(solNodes[kpnodes[k]] == FALSE);
   }

   assert(!graphmark[dfstree[dfstree_pos]]);

   return SCIP_OKAY;
}


/** compute cost of alternative key path */
static
SCIP_Real getKeyPathReplaceCost(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const VNOILOC*        vnoiData,           /**< data */
   const SOLTREE*        soltreeData,        /**< solution tree data */
   SCIP_Real             edgecost_old_in,    /**< edge cost of old edge */
   int                   boundedge_old,      /**< Voronoi boundary edge */
   PCMW*                 pcmwData,           /**< data */
   int*                  boundedge_new       /**< new Voronoi boundary edge (in/out) */
)
{
   SCIP_Real edgecost = FARAWAY;
   SCIP_Real edgecost_old = FARAWAY;
   SCIP_Real edgecost_new = FARAWAY;
   int newedge = *boundedge_new;

   assert(pcmwDataIsClean(graph, pcmwData));

   if( boundedge_old != UNKNOWN )
   {
      SCIPdebugMessage("get replace path for old edge: \n");

      edgecost_old = getBoundaryPathCostPrized(graph, vnoiData, soltreeData, boundedge_old, pcmwData);

      assert(SCIPisLE(scip, edgecost_old, edgecost_old_in));
      assert(SCIPisLT(scip, edgecost_old_in, FARAWAY));

      pcmwDataClean(graph, pcmwData);
   }

   if( newedge != UNKNOWN )
   {
      SCIPdebugMessage("get replace path for new edge: \n");

      edgecost_new = getBoundaryPathCostPrized(graph, vnoiData, soltreeData, newedge, pcmwData);

      pcmwDataClean(graph, pcmwData);
   }

   if( SCIPisLT(scip, edgecost_old, edgecost_new) )
   {
      edgecost = edgecost_old;
      *boundedge_new = boundedge_old;
   }
   else
   {
      edgecost = edgecost_new;
      *boundedge_new = newedge;
   }

   assert(SCIPisLT(scip, edgecost, FARAWAY) || (graph_pc_isPcMw(graph) && graph->source == graph->head[*boundedge_new] ) );
   assert(*boundedge_new != UNKNOWN);

   return edgecost;
}


/** restore supergraph data */
static
void supergraphDataRestore(
   const GRAPH*          graph,              /**< graph data structure */
   SGRAPH*               supergraphData      /**< super-graph*/
)
{
   const int* const supernodes = supergraphData->supernodes;
   STP_Bool* const nodeIsLowSupernode = supergraphData->nodeIsLowSupernode;

   SCIPfreeMemoryArray(scip, &(supergraphData->mst));

   /* unmark the descendant supervertices */
   for( int k = 0; k < supergraphData->nsupernodes - 1; k++ )
      nodeIsLowSupernode[supernodes[k]] = FALSE;

#ifndef NDEBUG
   for( int k = 0; k < graph->knots; k++ )
      assert(!nodeIsLowSupernode[k]);
#endif
}

/** compute minimum-spanning tree  */
static
SCIP_RETCODE supergraphComputeMst(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const CONN*           connectData,        /**< data */
   const VNOILOC*        vnoiData,           /**< data */
   const KPATHS*         keypathsData,       /**< key paths */
   int                   crucnode,           /**< node to eliminate */
   SOLTREE*              soltreeData,        /**< solution tree data */
   PCMW*                 pcmwData,           /**< data */
   SGRAPH*               supergraphData      /**< super-graph*/
)
{
   PATH* mst = NULL;
   GRAPH* supergraph = NULL;
   UF* const uf = connectData->uf;
   const int* const supernodes = supergraphData->supernodes;
   int* supernodesid = NULL;
   const SCIP_Real* const edgecosts = getEdgeCosts(graph, pcmwData);
   const STP_Bool* const isLowerSupernode = supergraphData->nodeIsLowSupernode;
   STP_Bool* const solNodes = soltreeData->solNodes;
   int* const newedges = soltreeData->newedges;
   const PATH* const vnoipath = vnoiData->vnoi_path;
   const int* const vnoibase = vnoiData->vnoi_base;
   const int* const boundaryedges = connectData->boundaryedges;
   SCIP_Real mstcost = 0.0;
   const int nboundaryedges = connectData->nboundaryedges;
   const int nnodes = graph->knots;
   const int nsupernodes = supergraphData->nsupernodes;
   /* the (super-) vertex representing the current root-component of the Steiner tree */
   const int superroot = supernodes[nsupernodes - 1];
#ifdef SCIP_DEBUG
   SCIP_Real mstcost_supergraph = 0.0;
   const SCIP_Bool pcmw = graph_pc_isPcMw(graph);
#endif
   assert(nboundaryedges > 0);
   assert(superroot >= 0);
   assert(!supergraphData->mst);

   SCIP_CALL( SCIPallocBufferArray(scip, &supernodesid, nnodes) );
#ifndef NDEBUG
   for( int k = 0; k < nnodes; k++ )
      supernodesid[k] = -1;
#endif

   /* create a supergraph, having the endpoints of the key-paths incident to the current crucial node as (super-) vertices */
   SCIP_CALL( graph_init(scip, &supergraph, nsupernodes, nboundaryedges * 2, 1) );
   supergraph->stp_type = STP_SPG;

   for( int k = 0; k < nsupernodes; k++ )
   {
      supernodesid[supernodes[k]] = k;
      graph_knot_add(supergraph, graph->term[supernodes[k]]);
   }

   if( pcmwData->isActive )
   {
      soltreeUnmarkKpNodes(graph, keypathsData, soltreeData);
   }

   assert(pcmwDataIsClean(graph, pcmwData));

   SCIPdebugMessage("build super-graph: \n");

   /* add edges to the supergraph */
   for( int k = 0; k < nboundaryedges; k++ )
   {
      SCIP_Real edgecost;
      const int edge = boundaryedges[k];
      int node = SCIPStpunionfindFind(uf, vnoibase[graph->tail[edge]]);
      int adjnode = SCIPStpunionfindFind(uf, vnoibase[graph->head[edge]]);

      /* if node 'node' or 'adjnode' belongs to the root-component, take the (temporary) root-component identifier instead */
      node = (isLowerSupernode[node]? node : superroot);
      adjnode = (isLowerSupernode[adjnode]? adjnode : superroot);

      /* compute the cost of the boundary-path pertaining to the boundary-edge 'edge' */
      edgecost = getBoundaryPathCostPrized(graph, vnoiData, soltreeData, edge, pcmwData);
      pcmwDataClean(graph, pcmwData);

      graph_edge_add(scip, supergraph, supernodesid[node], supernodesid[adjnode], edgecost, edgecost);
   }

   /* compute a MST on the supergraph */
   SCIP_CALL( SCIPallocMemoryArray(scip, &(supergraphData->mst), nsupernodes) );
   mst = supergraphData->mst;
   SCIP_CALL( graph_path_init(scip, supergraph) );
   graph_path_exec(scip, supergraph, MST_MODE, nsupernodes - 1, supergraph->cost, mst);

   assert(pcmwDataIsClean(graph, pcmwData));

   /* compute the cost of the MST */

   mstcost = 0.0;

   for( int l = 0; l < nsupernodes - 1; l++ )
   {
      /* compute the edge in the original graph corresponding to the current MST edge */
      const int edge = (mst[l].edge % 2 == 0)? boundaryedges[mst[l].edge / 2] : flipedge(boundaryedges[mst[l].edge / 2]);

      mstcost += getEdgeCostUnbiased(graph, pcmwData, edge);
      mstcost -= pcmwGetNewEdgePrize(graph, solNodes, edge, pcmwData);

#ifdef SCIP_DEBUG
      printf("key vertex mst edge ");
      graph_edge_printInfo(graph, edge);
      if( pcmw ) printf("nodeweights: %f -> %f \n", graph->prize[graph->tail[edge]], graph->prize[graph->head[edge]]);

      mstcost_supergraph += supergraph->cost[mst[l].edge];
#endif

      assert(newedges[edge] != crucnode && newedges[flipedge(edge)] != crucnode);

      /* mark the edge (in the original graph) as visited */
      newedges[edge] = crucnode;

      /* traverse along the boundary-path belonging to the boundary-edge 'edge' */
      for( int node = graph->tail[edge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
      {
         const int e = vnoipath[node].edge;

         /* if edge 'e' and its reversed have not been visited yet */
         if( newedges[e] != crucnode && newedges[flipedge(e)] != crucnode )
         {
            newedges[e] = crucnode;
            mstcost += edgecosts[e];
            mstcost -= pcmwGetNewEdgePrize(graph, solNodes, e, pcmwData);

#ifdef SCIP_DEBUG
            printf("key vertex mst edge ");
            graph_edge_printInfo(graph, e);
            if( pcmw ) printf("nodeweights: %f -> %f \n", graph->prize[graph->tail[edge]], graph->prize[graph->head[edge]]);
#endif
         }
      }

      for( int node = graph->head[edge]; node != vnoibase[node]; node = graph->tail[vnoipath[node].edge] )
      {
         const int e = vnoipath[node].edge;
         const int erev = flipedge(e);

         /* if edge 'e' and its reversed have not been visited yet */
         if( newedges[e] != crucnode && newedges[erev] != crucnode )
         {
            newedges[erev] = crucnode;

            mstcost += edgecosts[e];
            mstcost -= pcmwGetNewEdgePrize(graph, solNodes, e, pcmwData);

#ifdef SCIP_DEBUG
            printf("key vertex mst edge ");
            graph_edge_printInfo(graph, erev);
            if( pcmw ) printf("nodeweights: %f -> %f \n", graph->prize[graph->tail[edge]], graph->prize[graph->head[edge]]);
#endif
         }
      }
   }
#ifdef SCIP_DEBUG
   SCIPdebugMessage("mstcost=%f supergraph mstcost=%f \n", mstcost, mstcost_supergraph);
#endif

   pcmwDataClean(graph, pcmwData);

   supergraphData->mstcost = mstcost;

   if( pcmwData->isActive )
   {
      soltreeMarkKpNodes(graph, keypathsData, soltreeData);
   }

   SCIPfreeBufferArray(scip, &supernodesid);
   graph_path_exit(scip, supergraph);
   graph_free(scip, &supergraph, TRUE);

   return SCIP_OKAY;
}


/** get the key-paths star starting from 'keyvertex' */
static
void getKeyPathsStar(
   int                   keyvertex,          /**< key vertex to start from */
   const GRAPH*          graph,              /**< graph data structure */
   const CONN*           connectData,        /**< data */
   const SOLTREE*        soltreeData,        /**< solution tree data */
   const PCMW*           pcmwData,           /**< data */
   KPATHS*               keypathsData,       /**< key paths */
   SGRAPH*               supergraphData,     /**< super-graph*/
   SCIP_Bool*            success             /**< success? */
)
{
   int* const kpnodes = keypathsData->kpnodes;
   int* const kpedges = keypathsData->kpedges;
   const int* const solEdges = soltreeData->solEdges;
   int* const supernodes = supergraphData->supernodes;
   STP_Bool* isLowSupernode = supergraphData->nodeIsLowSupernode;
   const STP_Bool* const solNodes = soltreeData->solNodes;
   const STP_Bool* const pinned = soltreeData->nodeIsPinned;
   int edgeFromRoot = UNKNOWN;
   int nkpnodes = 0;
   int nkpedges = 0;
   int inedgescount = 0;
   int nsupernodes = 0;
   const SCIP_Real* const edgecosts = getEdgeCosts(graph, pcmwData);

   assert(!pinned[keyvertex] && nodeIsCrucial(graph, solEdges, keyvertex) && !Is_term(graph->term[keyvertex]));

   keypathsData->kpcost = 0.0;
   keypathsData->rootpathstart = -1;
   keypathsData->nkpedges = -1;
   keypathsData->nkpnodes = -1;
   supergraphData->nsupernodes = -1;
   *success = TRUE;

   /* find all key-paths starting in node 'keyvertex' */
   for( int edge = graph->outbeg[keyvertex]; edge != EAT_LAST; edge = graph->oeat[edge] )
   {
      assert(solNodes[graph->tail[edge]]);

      /* check whether the outgoing edge is in the Steiner tree */
      if( (solEdges[edge] == CONNECT && solNodes[graph->head[edge]])
          || solEdges[flipedge(edge)] == CONNECT )
      {
         const int edge_rev = flipedge(edge);

         /* we want to have the edges going into 'keyvertex' to get the prizes right */
         keypathsData->kpcost += edgecosts[edge_rev];
         inedgescount++;

#ifdef SCIP_DEBUG
         printf("key vertex start edge ");
         graph_edge_printInfo(graph, edge_rev);
#endif

         /* does current edge lead to the Steiner tree root? */
         if( solEdges[edge_rev] == CONNECT )
         {
            assert(edgeFromRoot == UNKNOWN);

            edgeFromRoot = edge_rev;
            kpedges[nkpedges++] = edgeFromRoot;

            assert(edge == soltreeData->linkcutNodes[keyvertex].edge);
         }
         else
         {
            int adjnode = graph->head[edge];
            int e = edge;
#ifndef NDEBUG
            const int nkpedges_org = nkpedges;
            assert(solEdges[flipedge(edge)] == UNKNOWN);
#endif
            kpedges[nkpedges++] = e;

            /* move along the key-path until its end (i.e. a crucial or pinned node) is reached */
            while( !pinned[adjnode] && !nodeIsCrucial(graph, solEdges, adjnode) && solNodes[adjnode] )
            {
               /* update the union-find data structure */
               SCIPStpunionfindUnion(connectData->uf, keyvertex, adjnode, FALSE);

               kpnodes[nkpnodes++] = adjnode;

               for( e = graph->outbeg[adjnode]; e != EAT_LAST; e = graph->oeat[e] )
               {
                  if( solEdges[e] == CONNECT )
                  {
                     /* we sum the cost of the edges pointing toward the key vertex */
                     keypathsData->kpcost += edgecosts[flipedge(e)];
                     kpedges[nkpedges++] = e;
#ifdef SCIP_DEBUG
                     printf("key vertex edge ");
                     graph_edge_printInfo(graph, flipedge(e));
#endif
                     break;
                  }
               }

               /* assert that each leaf of the ST is a terminal
                * todo check why this fails, the following should never actually happen  */
               if( e == EAT_LAST )
               {
                  *success = FALSE;
                  goto TERMINATE;
               }

               assert(e != EAT_LAST);
               adjnode = graph->head[e];
            }

            /* does the last node on the path belong to a removed component? */
            if( !solNodes[adjnode] )
            {
               /* assert that e is not the edge incident to the key vertex */
               assert(e != edge && e != edge_rev);
               assert(nkpedges - nkpedges_org >= 2);

               keypathsData->kpcost -= edgecosts[flipedge(e)];
#ifdef SCIP_DEBUG
               printf("key vertex remove edge ");
               graph_edge_printInfo(graph, flipedge(e));
#endif
               nkpedges--;
               adjnode = graph->tail[e];
               if( adjnode != keyvertex )
               {
                  supernodes[nsupernodes++] = adjnode;
                  isLowSupernode[adjnode] = TRUE;
               }
            }
            else
            {
               supernodes[nsupernodes++] = adjnode;
               isLowSupernode[adjnode] = TRUE;
            }

            assert(nkpedges > nkpedges_org);
         } /* edge does not go to root */
      } /* edge is in solution */
   } /* find all (unique) key-paths starting in node 'crucnode' */

   assert(inedgescount >= 0);

   if( inedgescount > 1 && pcmwData->isActive )
   {
      assert(graph_pc_isPcMw(graph));

      if( !graph_pc_isPc(graph) || graph_pc_knotIsNonLeafTerm(graph, keyvertex) )
      {
         /* we have implicitly subtracted the prize of the key vertex several times, need to revert */
         keypathsData->kpcost += (SCIP_Real)(inedgescount - 1) * graph->prize[keyvertex];
      }
      else
      {
         assert(0.0 == graph->prize[keyvertex]);
      }
   }

   /* traverse the key-path leading to the root-component */
   keypathsData->rootpathstart = nkpnodes;
   if( edgeFromRoot != UNKNOWN )
   {
      /* begin with the edge starting in the root-component of key vertex */
      int tail = graph->tail[edgeFromRoot];

      while( !pinned[tail] && !nodeIsCrucial(graph, solEdges, tail) && solNodes[tail] )
      {
         int e;

         kpnodes[nkpnodes++] = tail;

         for( e = graph->inpbeg[tail]; e != EAT_LAST; e = graph->ieat[e] )
         {
            if( solEdges[e] == CONNECT )
            {
               assert(solNodes[graph->tail[e]]);
               keypathsData->kpcost += edgecosts[e];
#ifdef SCIP_DEBUG
               printf("key vertex (root) edge ");
               graph_edge_printInfo(graph, e);
#endif
               kpedges[nkpedges++] = e;
               break;
            }
         }

         assert(e != EAT_LAST);
         tail = graph->tail[e];
      }

      supernodes[nsupernodes++] = tail;
   }

   /* the last of the key-path nodes to be stored is the current key-node */
   kpnodes[nkpnodes++] = keyvertex;

 TERMINATE:

   keypathsData->nkpedges = nkpedges;
   keypathsData->nkpnodes = nkpnodes;
   supergraphData->nsupernodes = nsupernodes;
}


/** preprocessing for Voronoi repair method */
static
void vnoiDataRepairPreprocess(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const KPATHS*         keypathsData,       /**< key paths */
   const CONN*           connectData,        /**< base lists */
   const PCMW*           pcmwData,           /**< data */
   VNOILOC*              vnoiData,           /**< data */
   int*                  nheapelems          /**< to store */
)
{
   STP_Vectype(int)* blists_start = connectData->blists_start;
   PATH* const vnoipath = vnoiData->vnoi_path;
   const int* const kpnodes = keypathsData->kpnodes;
   int* const vnoibase = vnoiData->vnoi_base;
   int* const state = vnoiData->vnoi_nodestate;
   const int* const graphmark = graph->mark;
   const int nkpnodes = keypathsData->nkpnodes;
   const SCIP_Real* const edgecosts = getEdgeCosts(graph, pcmwData);
   int count = 0;

   assert(nheapelems);

   for( int k = 0; k < nkpnodes; k++ )
   {
      STP_Vectype(int) blists_curr = blists_start[kpnodes[k]];
      const int size = StpVecGetSize(blists_curr);

      assert(blists_curr != NULL);

      for( int i = 0; i < size; i++ )
      {
         const int node = blists_curr[i];
         assert(graph_knot_isInRange(graph, node));

         for( int edge_in = graph->inpbeg[node]; edge_in != EAT_LAST; edge_in = graph->ieat[edge_in] )
         {
            const int tail = graph->tail[edge_in];

            /*  is tail node not in C and not forbidden? */
            if( state[tail] == CONNECT && graphmark[tail] && graphmark[vnoibase[tail]] )
            {
               const SCIP_Real newdist = vnoipath[tail].dist + edgecosts[edge_in];

               /* is the new path via tail shorter? */
               if( SCIPisGT(scip, vnoipath[node].dist, newdist) )
               {
                  vnoipath[node].dist = newdist;
                  vnoipath[node].edge = edge_in;
                  vnoibase[node] = vnoibase[tail];
               }
            }
         }

         if( vnoibase[node] != UNKNOWN )
         {
            graph_pathHeapAdd(vnoipath, node, graph->path_heap, state, &count);
         }
      }
   }

   assert(nkpnodes == 0 || count > 0);

   *nheapelems = count;
}

/** restore data */
static
void vnoiDataRestore(
   const CONN*           connectData,        /**< base lists */
   const KPATHS*         keypathsData,       /**< key paths */
   VNOILOC*              vnoiData            /**< data */
)
{
   STP_Vectype(int)* blists_start = connectData->blists_start;
   PATH* vnoipath = vnoiData->vnoi_path;
   int* memvbase = vnoiData->memvbase;
   int* meminedges = vnoiData->meminedges;
   int* vnoibase = vnoiData->vnoi_base;
   const int* kpnodes = keypathsData->kpnodes;
   SCIP_Real* memvdist = vnoiData->memvdist;
   const int nkpnodes = keypathsData->nkpnodes;
   int memcount = 0;

   for( int k = 0; k < nkpnodes; k++ )
   {
      /* restore data of all nodes having the current (internal) key-path node as their voronoi base */
      STP_Vectype(int) blists_curr = blists_start[kpnodes[k]];
      const int size = StpVecGetSize(blists_curr);

      for( int i = 0; i < size; i++ )
      {
         const int node = blists_curr[i];
         vnoibase[node] = memvbase[memcount];
         vnoipath[node].dist = memvdist[memcount];
         vnoipath[node].edge = meminedges[memcount];
         memcount++;
      }
   }

   assert(memcount == vnoiData->nmems);
   assert(vnoiData->nkpnodes == nkpnodes);
}

/** reset data */
static
void vnoiDataReset(
   const CONN*           connectData,        /**< base lists */
   const KPATHS*         keypathsData,       /**< key paths */
   const int*            graphmark,          /**< graph mark */
   VNOILOC*              vnoiData            /**< data */
)
{
   STP_Vectype(int)* blists_start = connectData->blists_start;
   PATH* vnoipath = vnoiData->vnoi_path;
   int* memvbase = vnoiData->memvbase;
   int* meminedges = vnoiData->meminedges;
   int* state = vnoiData->vnoi_nodestate;
   int* vnoibase = vnoiData->vnoi_base;
   const int* kpnodes = keypathsData->kpnodes;
   SCIP_Real* memvdist = vnoiData->memvdist;
   const int nkpnodes = keypathsData->nkpnodes;
   int nresnodes = 0;

   /* reset all nodes (referred to as 'C') whose bases are internal nodes of the current key-paths */
   for( int k = 0; k < nkpnodes; k++ )
   {
      /* reset all nodes having the current (internal) key-path node as their Voronoi base */
      STP_Vectype(int) blists_curr = blists_start[kpnodes[k]];
      const int size = StpVecGetSize(blists_curr);

      for( int i = 0; i < size; i++ )
      {
         const int node = blists_curr[i];

         assert(graphmark[node]);

         /* store data */
         memvbase[nresnodes] = vnoibase[node];
         memvdist[nresnodes] = vnoipath[node].dist;
         meminedges[nresnodes] = vnoipath[node].edge;
         nresnodes++;

         /* reset data */
         vnoibase[node] = UNKNOWN;
         vnoipath[node].dist = FARAWAY;
         vnoipath[node].edge = UNKNOWN;
         state[node] = UNKNOWN;
      }
   }

   vnoiData->nmems = nresnodes;
   vnoiData->nkpnodes = nkpnodes;
}


#ifndef NDEBUG
static
SCIP_Bool solDegIsValid(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const int*            solDegree,          /**< solution degree */
   const LCNODE*         linkcutNodes        /**< Steiner tree nodes */
   )
{
   int* solDegreeNew;
   SCIP_Bool isValid;
   const int nnodes = graph->knots;

   SCIP_CALL_ABORT( SCIPallocBufferArray(scip, &solDegreeNew, nnodes) );

   BMSclearMemoryArray(solDegreeNew, nnodes);

   for( int i = 0; i < nnodes; ++i )
   {
      const int edge = linkcutNodes[i].edge;

      if( edge == -1 )
         continue;

      assert(edge >= 0 && edge < graph->edges);
      assert(graph->tail[edge] == i);

      solDegreeNew[graph->tail[edge]]++;
      solDegreeNew[graph->head[edge]]++;
   }

   isValid = TRUE;

   for( int i = 0; i < nnodes; ++i )
   {
      if( Is_term(graph->term[i]) || Is_pseudoTerm(graph->term[i]) )
      {
         assert(UNKNOWN == solDegree[i]);
         continue;
      }

      if( solDegree[i] != solDegreeNew[i] )
      {
#ifdef SCIP_DEBUG
         graph_knot_printInfo(graph, i);
         SCIPdebugMessage("%d != %d (old/new degree) \n", solDegree[i], solDegreeNew[i]);
#endif
         isValid = FALSE;
         break;
      }
   }

   SCIPfreeBufferArray(scip, &solDegreeNew);

   return isValid;
}


static
SCIP_Bool solNodeIsValid(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   const STP_Bool*       solNodes,           /**< Steiner tree nodes */
   const LCNODE*         linkcutNodes        /**< Steiner tree nodes */
   )
{
   SCIP_Bool isValid;
   STP_Bool* solNodes_new;
   const int nnodes = graph->knots;

   SCIP_CALL_ABORT( SCIPallocBufferArray(scip, &solNodes_new, nnodes) );

   for( int i = 0; i < nnodes; ++i )
      solNodes_new[i] = FALSE;

   for( int i = 0; i < nnodes; ++i )
   {
      const int edge = linkcutNodes[i].edge;

      if( edge >= 0 )
      {
         assert(graph->tail[edge] == i);

         solNodes_new[graph->head[edge]] = TRUE;
         solNodes_new[graph->tail[edge]] = TRUE;
      }
   }

   isValid = TRUE;

   for( int i = 0; i < nnodes; ++i )
   {
      if( solNodes_new[i] != solNodes[i] )
      {
         isValid = FALSE;
         break;
      }
   }

   SCIPfreeBufferArray(scip, &solNodes_new);

   return isValid;
}
#endif


/** reduces solution degree */
static inline
void insertionDecrementSolDegree(
   const GRAPH*          graph,              /**< graph data structure */
   int                   node,               /**< replacement edge */
   INSERT*               insertData          /**< insertion data */
)
{
   int* const solDegreeNonTerm = insertData->solDegreeNonTerm;

   assert(node >= 0);
   assert(insertData->solNodes[node]);

   if( Is_term(graph->term[node]) || Is_pseudoTerm(graph->term[node]) )
   {
      assert(solDegreeNonTerm[node] == UNKNOWN);
   }
   else
   {
      solDegreeNonTerm[node] -= 1;

      assert(solDegreeNonTerm[node] >= 0);
   }
}


/** increases solution degree */
static inline
void insertionIncrementSolDegree(
   const GRAPH*          graph,              /**< graph data structure */
   int                   node,               /**< replacement edge */
   INSERT*               insertData          /**< insertion data */
)
{
   int* const solDegreeNonTerm = insertData->solDegreeNonTerm;

   assert(node >= 0);
   assert(insertData->solNodes[node]);

   if( Is_term(graph->term[node]) || Is_pseudoTerm(graph->term[node]) )
   {
      assert(solDegreeNonTerm[node] == UNKNOWN);
   }
   else
   {
      solDegreeNonTerm[node] += 1;
   }
}

/** subroutine for insertion heuristic */
static
void insertionInit(
   SCIP_HEURDATA*        heurdata,           /**< heuristic data */
   const GRAPH*          graph,              /**< graph data structure */
   const int*            solEdges,           /**< Steiner tree edges */
   int*                  solDegreeNonTerm,   /**< degree */
   SCIP_Bool*            nodeIsBlocked,      /**< blocked */
   int*                  vertices            /**< vertices permuted */
   )
{
   const int nnodes = graph->knots;
   const int nedges = graph->edges;

   assert(solDegreeNonTerm);

   BMSclearMemoryArray(solDegreeNonTerm, nnodes);

   for( int e = 0; e < nedges; e++ )
   {
      if( solEdges[e] == CONNECT )
      {
         solDegreeNonTerm[graph->tail[e]]++;
         solDegreeNonTerm[graph->head[e]]++;
      }
   }

   for( int i = 0; i < nnodes; ++i )
   {
      vertices[i] = i;
      nodeIsBlocked[i] = FALSE;

      if( Is_term(graph->term[i]) || Is_pseudoTerm(graph->term[i]) )
         solDegreeNonTerm[i] = UNKNOWN;
   }

   SCIPrandomPermuteIntArray(heurdata->randnumgen, vertices, 0, nnodes);
}

/** subroutine for insertion heuristic: prepare insertion of new vertex */
static
void insertionInitInsert(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   int                   v_insert,           /**< the vertex to add */
   int                   initialEdge,        /**< first edge from node to tree */
   LCNODE*               linkcutNodes,       /**< Steiner tree nodes */
   INSERT*               insertData,         /**< insertion data (in/out) */
   SCIP_Real*            diff                /**< the initial diff (out) */
   )
{
   const int candHeadInitial = graph->head[initialEdge];
   const int stp_type = graph->stp_type;
   const SCIP_Bool mw = (stp_type == STP_MWCSP || stp_type == STP_RMWCSP);

   assert(graph->tail[initialEdge] == v_insert);
   assert(!Is_term(graph->term[v_insert]));

   SCIPlinkcuttreeLink(linkcutNodes, v_insert, candHeadInitial, initialEdge);

   SCIPdebugMessage("try to insert vertex %d \n", v_insert);

   if( mw )
   {
      assert(!SCIPisPositive(scip, graph->prize[v_insert]));
      *diff = -graph->prize[v_insert];
   }
   else
   {
      const SCIP_Bool pc = graph_pc_isPc(graph);

      *diff = insertData->edgecosts[initialEdge];

      if( pc )
      {
         *diff -= graph->prize[v_insert];
      }
   }

   assert(insertData->solDegreeNonTerm[v_insert] == 0
      || (insertData->solDegreeNonTerm[v_insert] == UNKNOWN && graph_pc_isPcMw(graph)));
   assert(!insertData->nodeIsBlocked[v_insert]);

   insertData->nInsertions = 0;
   insertData->insertionVertex = v_insert;
   insertData->solNodes[v_insert] = TRUE;
   insertData->nodeIsBlocked[v_insert] = TRUE;
   insertionIncrementSolDegree(graph, v_insert, insertData);
   insertionIncrementSolDegree(graph, candHeadInitial, insertData);
}


/** remove (blocked) chains from tree */
static
void insertionFinalizeReplacement(
   const GRAPH*          graph,              /**< graph data structure */
   int                   v_insert,           /**< the vertex to insert */
   LCNODE*               linkcutNodes,       /**< Steiner tree nodes */
   INSERT*               insertData          /**< insertion data */
)
{
   SCIP_Bool* const nodeIsBlocked = insertData->nodeIsBlocked;
   const int* const blockedList = insertData->blockedList;
   STP_Bool* const solNodes = insertData->solNodes;
   const int size = insertData->blockedListSize;
   int* const solDegreeNonTerm = insertData->solDegreeNonTerm;

   for( int i = 0; i < size; i++ )
   {
      const int node = blockedList[i];

      assert(node >= 0 && node < graph->knots);
      assert(nodeIsBlocked[node]);
      assert(solNodes[node]);

      nodeIsBlocked[node] = FALSE;
      solNodes[node] = FALSE;

      if( !(Is_term(graph->term[node]) || Is_pseudoTerm(graph->term[node])) )
      {
         assert(2 == solDegreeNonTerm[node]);

         solDegreeNonTerm[node] = 0;
      }
      else
      {
         assert(UNKNOWN == solDegreeNonTerm[node]);
      }

      SCIPlinkcuttreeInitNode(&linkcutNodes[node]);
   }


   assert(nodeIsBlocked[v_insert]);
   nodeIsBlocked[v_insert] = FALSE;

   for( int k = 0; k < graph->knots; k++ )
      assert(!nodeIsBlocked[k]);

   insertData->blockedListSize = 0;
}


/** remove (blocked) chains from tree */
static inline
void insertionResetBlockedNodes(
   INSERT*               insertData          /**< insertion data */
)
{
   SCIP_Bool* const nodeIsBlocked = insertData->nodeIsBlocked;
   const int* const blockedList = insertData->blockedList;
   const int size = insertData->blockedListSize;

   for( int i = 0; i < size; i++ )
   {
      const int node = blockedList[i];

      assert(node >= 0);
      assert(nodeIsBlocked[node]);
      assert(2 == insertData->solDegreeNonTerm[node]);

      nodeIsBlocked[node] = FALSE;
   }

   assert(nodeIsBlocked[insertData->insertionVertex]);
   nodeIsBlocked[insertData->insertionVertex] = FALSE;
}


/** mark inner chain nodes as blocked */
static inline
void insertionBlockChain(
   const GRAPH*          graph,              /**< graph data structure */
   const LCNODE*         lctree,             /**< tree */
   int                   chainfirst_index,   /**< first chain entry */
   int                   chainlast_index,    /**< last chain entry (inside) */
   INSERT*               insertData          /**< insertion data */
)
{
   if( chainfirst_index != chainlast_index )
   {
      SCIP_Bool* const nodeIsBlocked = insertData->nodeIsBlocked;
      int* const blockedList = insertData->blockedList;

      for( int node = chainfirst_index; node != chainlast_index; node = lctree[node].parent )
      {
         int head;

         assert(node != -1);
         assert(lctree[node].edge >= 0);

         head = graph->head[lctree[node].edge];

         assert(!nodeIsBlocked[head]);

         nodeIsBlocked[head] = TRUE;
         blockedList[insertData->blockedListSize++] = head;
      }
   }
}


/** reinsert (blocked) chains to tree */
static
void insertionRestoreTree(
   const GRAPH*          graph,              /**< graph data structure */
   const int*            insertCands,        /**< insertion candidates */
   int                   lcvertex_insert,    /**< the vertex tested for insertion */
   LCNODE*               linkcutNodes,       /**< Steiner tree nodes */
   INSERT*               insertData          /**< insertion data */
)
{
   int* chainEnds = insertData->chainEnds;
   const int* const addedEdges = insertData->addedEdges;
   const int* const cutedgesStart = insertData->cutedgesStart;
   const int* const cutedgesEnd = insertData->cutedgesEnd;
   const int* const graphTail = graph->tail;
   const int* const graphHead = graph->head;
   const int v = insertData->insertionVertex;

   SCIPlinkcuttreeEvert(linkcutNodes, lcvertex_insert);

   insertionResetBlockedNodes(insertData);

   /* reinsert each chain and remove the  */
   for( int k = insertData->nInsertions - 1; k >= 0; k-- )
   {
      const int cutedgeFirst = cutedgesStart[k];
      const int cutedgeLast = cutedgesEnd[k];
      const int firstNode = graphTail[cutedgeFirst];
      const int secondNode = graphHead[cutedgeFirst];
      const int lastNode = chainEnds[k];

      assert(cutedgeLast >= 0);
      assert(firstNode == insertData->chainStarts[k]);

      /* remove the newly added edge */
      SCIPlinkcuttreeCutNode(&linkcutNodes[graphHead[addedEdges[k]]]);
      insertionDecrementSolDegree(graph, graphHead[addedEdges[k]], insertData);

      /* re-link the tail of the chain */
      SCIPlinkcuttreeEvert(linkcutNodes, firstNode);
      SCIPlinkcuttreeLink(linkcutNodes, firstNode, secondNode, cutedgeFirst);

      /* re-link the head of the chain (if not already done) */
      if( lastNode != firstNode )
         SCIPlinkcuttreeLink(linkcutNodes, lastNode, graphHead[cutedgeLast], cutedgeLast);

      /* reset solution degrees of chain border vertices */
      insertionIncrementSolDegree(graph, graphTail[cutedgeFirst], insertData);
      insertionIncrementSolDegree(graph, graphHead[cutedgeLast], insertData);
   }

   /* finally, cut the edge added first (if it had been cut during the insertion process, it would have been restored above) */
   SCIPlinkcuttreeEvert(linkcutNodes, lcvertex_insert);
   SCIPlinkcuttreeCutNode(&linkcutNodes[graphHead[insertCands[0]]]);
   insertionDecrementSolDegree(graph, graphHead[insertCands[0]], insertData);

   if( Is_term(graph->term[v]) || Is_pseudoTerm(graph->term[v]) )
   {
      assert(graph_pc_isPcMw(graph));
      insertData->solDegreeNonTerm[v] = UNKNOWN;
   }
   else
   {
      insertData->solDegreeNonTerm[v] = 0;
   }

   assert(insertData->solNodes[v]);
   insertData->solNodes[v] = FALSE;

   for( int k = 0; k < graph->knots; k++ )
      assert(!insertData->nodeIsBlocked[k]);

   insertData->blockedListSize = 0;
}


/** replaces chain by a single edge */
static
void insertionReplaceChain(
   const GRAPH*          graph,              /**< graph data structure */
   int                   newedge,            /**< replacement edge */
   LCNODE*               lctree,             /**< tree */
   INSERT*               insertData,         /**< insertion data */
   int                   v_lc,               /**< current vertex */
   int                   headCurr_lc,        /**< head of newedge */
   int                   chainfirst_index,   /**< first chain entry */
   int                   chainlast_index     /**< last chain entry (inside) */
)
{
   LCNODE* chainfirst = &lctree[chainfirst_index];
   LCNODE* chainlast = &lctree[chainlast_index];
   int* const chainStarts = insertData->chainStarts;
   int* const chainEnds = insertData->chainEnds;
   int* const cutedgesStart = insertData->cutedgesStart;
   int* const cutedgesEnd = insertData->cutedgesEnd;
   int* const addedEdges = insertData->addedEdges;
   const int nInsertions = insertData->nInsertions;
   const int newhead = graph->head[newedge];
   const int v_insert = insertData->insertionVertex;

   assert(chainlast->edge >= 0);
   assert(graph->tail[newedge] == v_insert);

   cutedgesStart[nInsertions] = chainfirst->edge;
   cutedgesEnd[nInsertions] = chainlast->edge;
   chainStarts[nInsertions] = chainfirst_index;
   chainEnds[nInsertions] = chainlast_index;
   addedEdges[nInsertions] = newedge;

   insertionIncrementSolDegree(graph, v_insert, insertData);
   insertionIncrementSolDegree(graph, newhead, insertData);

   /* decrease the degree of the chain border vertices */
   insertionDecrementSolDegree(graph, graph->tail[chainfirst->edge], insertData);
   insertionDecrementSolDegree(graph, graph->head[chainlast->edge], insertData);

   SCIPdebugMessage("remove chain %d->%d \n", graph->tail[chainfirst->edge], graph->head[chainlast->edge]);

   insertionBlockChain(graph, lctree, chainfirst_index, chainlast_index, insertData);

   SCIPlinkcuttreeCutNode(chainfirst);
   SCIPlinkcuttreeCutNode(chainlast);

   SCIPlinkcuttreeLink(lctree, v_lc, headCurr_lc, newedge);
   assert(lctree[v_lc].edge == newedge);

   insertData->nInsertions++;
}


/** perform local vertex insertion heuristic on given Steiner tree */
static
void insertionGetCandidateEdges(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HEURDATA*        heurdata,           /**< heuristic data */
   const GRAPH*          graph,              /**< graph data structure */
   const STP_Bool*       solNodes,           /**< Steiner tree nodes */
   int                   vertex,             /**< vertex to be inserted */
   int*                  insertcands,        /**< candidates */
   int*                  ncands              /**< number of candidates */
   )
{
   int insertcount = 0;
   const SCIP_Bool mwpc = graph_pc_isPcMw(graph);
   const SCIP_Bool rpcmw = graph_pc_isRootedPcMw(graph);

   assert(!Is_term(graph->term[vertex]));
   assert(heurdata);

   for( int oedge = graph->outbeg[vertex]; oedge != EAT_LAST; oedge = graph->oeat[oedge] )
   {
      const int head = graph->head[oedge];

      if( !solNodes[head] )
         continue;

      /* skip dummy terminals */
      if( mwpc )
      {
         if( Is_term(graph->term[head]) && !graph_pc_knotIsFixedTerm(graph, head) )
            continue;

         /* note: for unrooted problems the (artificial) source is also a fixed terminal */
         if( !rpcmw && head == graph->source )
            continue;
      }

      assert(SCIPisLT(scip, graph->cost[oedge], FARAWAY));

      insertcands[insertcount++] = oedge;
   }

   if( insertcount >= 3 )
      SCIPrandomPermuteIntArray(heurdata->randnumgen, insertcands, 0, insertcount);

   *ncands = insertcount;
}


/** perform local vertex insertion heuristic on given Steiner tree */
static
SCIP_RETCODE localVertexInsertion(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HEURDATA*        heurdata,           /**< heuristic data or NULL */
   const GRAPH*          graph,              /**< graph data structure */
   STP_Bool*             solNodes,           /**< Steiner tree nodes */
   LCNODE*               linkcutNodes,       /**< Steiner tree nodes */
   int*                  solEdges            /**< array indicating whether an arc is part of the solution (CONNECTED/UNKNOWN) */
   )
{
   int* chainStarts;
   int* chainEnds;
   int* insertCands = NULL;
   int* addedEdges = NULL;
   int* cutedgesStart = NULL;
   int* cutedgesEnd = NULL;
   int* solDegree = NULL;
   int* vertices = NULL;
   int* blockedList = NULL;
   SCIP_Bool* nodeIsBlocked = NULL;

   int newnverts = 0;
   const int nnodes = graph->knots;
   const int nedges = graph->edges;
   const int root = graph->source;
   const int stp_type = graph->stp_type;
   const SCIP_Bool mw = (stp_type == STP_MWCSP || stp_type == STP_RMWCSP);
   const SCIP_Bool pc = graph_pc_isPc(graph);
   const SCIP_Bool mwpc = graph_pc_isPcMw(graph);
   SCIP_Bool solimproved = TRUE;
   SCIP_Real* edgecosts;


#ifndef NDEBUG
   const SCIP_Real initialobj = solstp_getObjBounded(graph, solEdges, 0.0, nedges);
   SCIP_Real diff_total = 0.0;
#endif

   if( stp_type != STP_SPG && stp_type != STP_RSMT && stp_type != STP_OARSMT && stp_type != STP_GSTP && !mwpc )
   {
      SCIPdebugMessage("vertex inclusion does not work for current problem type \n");
      return SCIP_OKAY;
   }

   if( pc )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &edgecosts, nedges) );

      graph_pc_getOrgCosts(scip, graph, edgecosts);
   }
   else
   {
      edgecosts = graph->cost;
   }

   SCIP_CALL( SCIPallocBufferArray(scip, &chainStarts, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &chainEnds, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &blockedList, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodeIsBlocked, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &vertices, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &insertCands, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &addedEdges, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cutedgesStart, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &cutedgesEnd, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solDegree, nnodes) );

   insertionInit(heurdata, graph, solEdges, solDegree, nodeIsBlocked, vertices);

   assert(solDegIsValid(scip, graph, solDegree, linkcutNodes));
   assert(solNodeIsValid(scip, graph, solNodes, linkcutNodes));
   assert(linkcutNodes[graph->source].edge == -1);

   /* main loop */
   while( solimproved )
   {
      INSERT insertData = { .chainStarts = chainStarts, .chainEnds = chainEnds, .edgecosts = edgecosts,
         .solDegreeNonTerm = solDegree, .solNodes = solNodes, .nodeIsBlocked = nodeIsBlocked,
         .cutedgesStart = cutedgesStart, .cutedgesEnd = cutedgesEnd, .addedEdges = addedEdges,
         .blockedList = blockedList, .blockedListSize = 0, .nInsertions = 0, .insertionVertex = -1 };

      solimproved = FALSE;

      for( int i = 0; i < nnodes; i++ )
      {
         SCIP_Real diff;
         int ninsertcands = 0;
         const int v = vertices[i];

         assert(v >= 0 && v < graph->knots);

         if( solNodes[v] || graph->grad[v] <= 1 )
            continue;

         assert(!Is_term(graph->term[v]));

         insertionGetCandidateEdges(scip, heurdata, graph, solNodes, v, insertCands, &ninsertcands);

         /* if there are less than two edges connecting node i and the current tree, continue */
         if( ninsertcands <= 1 )
            continue;

        insertionInitInsert(scip, graph, v, insertCands[0], linkcutNodes, &insertData, &diff);

         /* try to add additional edges between new vertex and tree */
         for( int k = 1; k < ninsertcands; k++ )
         {
            int chainfirst = -1;
            int chainlast = -1;
            const int insertEdgeCurr = insertCands[k];
            const int insertHeadCurr = graph->head[insertEdgeCurr];

            if( nodeIsBlocked[insertHeadCurr] )
            {
               assert(k > 1);

               continue;
            }

            SCIPdebugMessage("insertHeadCurr=%d \n", insertHeadCurr);

            SCIPlinkcuttreeEvert(linkcutNodes, v);

            if( mw )
            {
               const SCIP_Real chainweight = SCIPlinkcuttreeFindMinChainMw(scip, linkcutNodes, graph->prize,
                     graph->head, solDegree, nodeIsBlocked, insertHeadCurr,
                     &chainfirst, &chainlast);

               if( SCIPisLT(scip, chainweight, 0.0) )
               {
                  diff += chainweight;
                  insertionReplaceChain(graph, insertEdgeCurr, linkcutNodes, &insertData, v, insertHeadCurr, chainfirst, chainlast);
               }
            }
            else
            {
               const SCIP_Real chainweight = SCIPlinkcuttreeFindMaxChain(scip, linkcutNodes, edgecosts, pc ? graph->prize : NULL,
                     graph->head, solDegree, nodeIsBlocked, insertHeadCurr,
                     &chainfirst, &chainlast);

               SCIPdebugMessage("chainweight=%f edgecost=%f \n", chainweight, edgecosts[insertEdgeCurr]);

               /* note: comparison needs to be strict to avoid (redundant) removal of current edge */
               if( SCIPisGT(scip, chainweight, edgecosts[insertEdgeCurr]) )
               {
                  diff += edgecosts[insertEdgeCurr];
                  diff -= chainweight;
                  insertionReplaceChain(graph, insertEdgeCurr, linkcutNodes, &insertData, v, insertHeadCurr, chainfirst, chainlast);
               }
            }
         }

         /* is the new tree better? */
         if( SCIPisNegative(scip, diff) )
         {
            SCIPlinkcuttreeEvert(linkcutNodes, root);
            solimproved = TRUE;
            newnverts++;
            assert(solNodes[v]);

            insertionFinalizeReplacement(graph, v, linkcutNodes, &insertData);

            SCIPdebugMessage("Inclusion: ADDED VERTEX %d \n", v);
#ifndef NDEBUG
            diff_total -= diff;
#endif
         }
         else
         {
            /* restore the old tree */
            insertionRestoreTree(graph, insertCands, v, linkcutNodes, &insertData);
         }

         assert(solDegIsValid(scip, graph, solDegree, linkcutNodes));
         assert(solNodeIsValid(scip, graph, solNodes, linkcutNodes));

      } /* for i 0:nnodes */
   }

   /* free buffer memory */
   SCIPfreeBufferArray(scip, &solDegree);
   SCIPfreeBufferArray(scip, &cutedgesEnd);
   SCIPfreeBufferArray(scip, &cutedgesStart);
   SCIPfreeBufferArray(scip, &addedEdges);
   SCIPfreeBufferArray(scip, &insertCands);
   SCIPfreeBufferArray(scip, &vertices);
   SCIPfreeBufferArray(scip, &blockedList);
   SCIPfreeBufferArray(scip, &nodeIsBlocked);
   SCIPfreeBufferArray(scip, &chainEnds);
   SCIPfreeBufferArray(scip, &chainStarts);

   if( pc )
      SCIPfreeBufferArray(scip, &edgecosts);

   for( int e = 0; e < nedges; e++ )
      solEdges[e] = UNKNOWN;

   if( newnverts > 0  )
   {
      SCIP_CALL( solstp_prune(scip, graph, solEdges, solNodes) );

      for( int i = 0; i < nnodes; i++ )
         SCIPlinkcuttreeInitNode(&linkcutNodes[i]);

      /* create a link-cut tree representing the current Steiner tree */
      for( int e = 0; e < nedges; e++ )
      {
         if( solEdges[e] == CONNECT )
         {
            assert(solNodes[graph->tail[e]] && solNodes[graph->head[e]]);

            SCIPlinkcuttreeLink(linkcutNodes, graph->head[e], graph->tail[e], flipedge(e));
         }
      }
      SCIPlinkcuttreeEvert(linkcutNodes, root);
   }
   else
   {
      SCIPlinkcuttreeEvert(linkcutNodes, root);
      for( int i = 0; i < nnodes; i++ )
      {
         if( solNodes[i] && linkcutNodes[i].edge != -1 )
            solEdges[flipedge(linkcutNodes[i].edge)] = CONNECT;
      }
   }

#ifndef NDEBUG
   {
      const SCIP_Real newobj = solstp_getObjBounded(graph, solEdges, 0.0, nedges);
      SCIPdebugMessage("vertex inclusion obj before/after: %f/%f \n", initialobj, newobj);
      assert(SCIPisLE(scip, newobj + diff_total, initialobj));
      assert(SCIPisGE(scip, diff_total, 0.0));
   }
#endif

   assert(solstp_isValid(scip, graph, solEdges));

   return SCIP_OKAY;
}

/** perform local vertex insertion heuristic on given Steiner tree */
static
SCIP_RETCODE localKeyVertexHeuristics(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   SCIP_Bool             useFast,            /**< fast variant? */
   STP_Bool*             solNodes,           /**< Steiner tree nodes */
   LCNODE*               linkcutNodes,       /**< Steiner tree nodes */
   int*                  solEdges,           /**< array indicating whether an arc is part of the solution (CONNECTED/UNKNOWN) */
   SCIP_Bool*            success             /**< solution improved? */
   )
{
   UF uf;  /* union-find */
   STP_Vectype(int)* blists_start = NULL;
   PATH* vnoipath = NULL;
   STP_Vectype(int)* lvledges_start = NULL;  /* horizontal edges */
   PHNODE** boundpaths = NULL;
   PHNODE* pheapelements = NULL;
   SCIP_Real* memvdist = NULL;
   SCIP_Real* edgecostBiased_pc = NULL;
   SCIP_Real* edgecostOrg_pc = NULL;
   SCIP_Real* prize_pc = NULL;
   int* vnoibase = NULL;
   int* kpedges = NULL;
   int* kpnodes = NULL;
   int* dfstree = NULL;
   int* newedges = NULL;
   int* memvbase = NULL;
   int* pheapsize = NULL;
   int* boundedges = NULL;
   int* meminedges = NULL;
   int* supernodes = NULL;
   int* prizemarklist = NULL;
   STP_Bool* pinned = NULL;
   STP_Bool* scanned = NULL;
   STP_Bool* supernodesmark = NULL;
   STP_Bool* prizemark = NULL;
   const int root = graph->source;
   const int nnodes = graph->knots;
   const int nedges = graph->edges;
   const int maxnrestarts = (useFast ? LOCAL_MAXRESTARTS_FAST : LOCAL_MAXRESTARTS);
   const int solroot = pcmwGetSolRoot(graph, solEdges);
   const STP_Bool mwpc = graph_pc_isPcMw(graph);
   SCIP_Bool solimproved = FALSE;

#ifndef NDEBUG
   const SCIP_Real initialobj = solstp_getObjBounded(graph, solEdges, 0.0, graph->edges);
   SCIP_Real objimprovement = 0.0;
#endif

   *success = FALSE;

   assert(mwpc || solroot == -1);

   /* memory needed for both Key-Path Elimination and Exchange */
   SCIP_CALL( SCIPallocBufferArray(scip, &vnoipath, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &vnoibase, nnodes) );

   /* only needed for Key-Path Elimination */
   SCIP_CALL( SCIPallocBufferArray(scip, &newedges, nedges) );
   SCIP_CALL( SCIPallocBufferArray(scip, &lvledges_start, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &boundedges, nedges) );

   /* memory needed for both Key-Path Elimination and Exchange */
   if( mwpc )
   {
      if( graph_pc_isPc(graph) )
      {
         SCIP_CALL(SCIPallocBufferArray(scip, &edgecostOrg_pc, nedges));

         graph_pc_getOrgCosts(scip, graph, edgecostOrg_pc);
      }

      SCIP_CALL(SCIPallocBufferArray(scip, &edgecostBiased_pc, nedges));
      SCIP_CALL(SCIPallocBufferArray(scip, &prize_pc, nnodes));
      SCIP_CALL(SCIPallocBufferArray(scip, &prizemark, nnodes));
      SCIP_CALL(SCIPallocBufferArray(scip, &prizemarklist, nnodes));

      for( int k = 0; k < nnodes; k++ )
         prizemark[k] = FALSE;

      if( graph_pc_isPc(graph) )
      {
         graph_pc_getBiased(graph, edgecostBiased_pc, prize_pc);
      }
      else
      {
         BMScopyMemoryArray(edgecostBiased_pc, graph->cost, graph->edges);
         BMScopyMemoryArray(prize_pc, graph->prize, graph->knots);
      }
   }

   SCIP_CALL( SCIPallocBufferArray(scip, &scanned, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &pheapsize, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &blists_start, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &memvbase, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &memvdist, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &meminedges, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &boundpaths, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &pinned, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &dfstree, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &supernodesmark, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &supernodes, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &kpnodes, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &kpedges, nnodes) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &pheapelements, nedges) );

   for( int k = 0; k < nnodes; k++ )
      graph->mark[k] = (graph->grad[k] > 0);

   graph->mark[root] = TRUE;

   SCIP_CALL( SCIPStpunionfindInit(scip, &uf, nnodes) );

   BMSclearMemoryArray(blists_start, nnodes);
   BMSclearMemoryArray(lvledges_start, nnodes);

   /* main loop */
   for( int nruns = 0, localmoves = 1; nruns < maxnrestarts && localmoves > 0; nruns++ )
   {
      VNOILOC vnoiData = { .vnoi_path = vnoipath, .vnoi_base = vnoibase, .memvdist = memvdist, .memvbase = memvbase,
         .meminedges = meminedges, .vnoi_nodestate = graph->path_state, .nmems = 0, .nkpnodes = -1 };
      KPATHS keypathsData = { .kpnodes = kpnodes, .kpedges = kpedges, .kpcost = 0.0, .nkpnodes = 0, .nkpedges = 0,
         .kptailnode = -1 };
      CONN connectivityData = { .blists_start = blists_start, .pheap_boundpaths = boundpaths, .lvledges_start = lvledges_start,
         .pheap_sizes = pheapsize, .uf = &uf, .boundaryedges = boundedges, .nboundaryedges = 0,
         .pheap_elements = pheapelements };
      SOLTREE soltreeData = { .solNodes = solNodes, .linkcutNodes = linkcutNodes, .solEdges = solEdges, .nodeIsPinned = pinned,
         .nodeIsScanned = scanned, .newedges = newedges };
      SGRAPH supergraphData = { .supernodes = supernodes, .nodeIsLowSupernode = supernodesmark,
         .mst = NULL, .mstcost = 0.0, .nsupernodes = 0 };
      PCMW pcmwData = { .prize_biased = prize_pc, .edgecost_biased = edgecostBiased_pc, .edgecost_org = edgecostOrg_pc,
         .prizemark = prizemark, .prizemarklist = prizemarklist, .nprizemarks = 0, .isActive = graph_pc_isPcMw(graph),
         .solroot = solroot };
      int nstnodes = 0;

      localmoves = 0;

      /* find a DFS order of the ST nodes */
      dfsorder(graph, solEdges, root, &nstnodes, dfstree);

      /* initialize data structures  */
      for( int k = 0; k < nnodes; k++ )
      {
         pinned[k] = FALSE;
         scanned[k] = FALSE;
         supernodesmark[k] = FALSE;
      }

      for( int e = 0; e < nedges; e++ )
         pheapelements[e].element = -1;

      for( int e = 0; e < nedges; e++ )
         newedges[e] = UNKNOWN;

      if( mwpc )
      {
         assert(graph->extended);
         SCIP_CALL( pcmwUpdate(scip, graph, &soltreeData, &pcmwData) );

         /* compute a Voronoi diagram with the Steiner tree nodes as bases */
         graph_voronoi(graph, pcmwData.edgecost_biased, NULL, soltreeData.solNodes, vnoiData.vnoi_base, vnoiData.vnoi_path);
      }
      else
      {
         graph_voronoi(graph, graph->cost, NULL, soltreeData.solNodes, vnoiData.vnoi_base, vnoiData.vnoi_path);
      }

      SCIP_CALL( connectivityDataInit(scip, graph, &vnoiData, &soltreeData, &pcmwData, &connectivityData) );

      /* henceforth, the union-find structure will be used on the Steiner tree */
      assert(uf.nElements == nnodes);
      SCIPStpunionfindClear(scip, &uf);

      /* main loop visiting all nodes of the current Steiner tree in post-order */
      for( int dfstree_pos = 0; dfstree[dfstree_pos] != root; dfstree_pos++ )
      {
         /* current crucial node */
         const int crucnode = dfstree[dfstree_pos];
         int nheapelems = -1;

         assert(!scanned[crucnode]);
         scanned[crucnode] = TRUE;

         SCIPdebugMessage("iteration %d (crucial node: %d) \n", dfstree_pos, crucnode);

         if( solroot == crucnode )
         {
            assert(mwpc && !graph_pc_isRootedPcMw(graph));
            continue;
         }

         /*  has the node been temporarily removed from the Steiner tree? */
         if( !graph->mark[crucnode] )
            continue;

         /* key vertex elimination: */
         /* is node 'crucnode' a removable crucial node? (i.e. not pinned or a terminal) */
         if( !pinned[crucnode] && !Is_term(graph->term[crucnode]) && nodeIsCrucial(graph, solEdges, crucnode) )
         {
            SCIP_Bool allgood;

            getKeyPathsStar(crucnode, graph, &connectivityData, &soltreeData, &pcmwData, &keypathsData, &supergraphData, &allgood);

            /* todo: check this! */
            if( !allgood )
            {
               *success = FALSE;
               localmoves = 0;
               SCIPdebugMessage("terminate key vertex heuristic \n");
               goto TERMINATE;
            }

            assert(keypathsData.nkpnodes != 0); /* if there are no key-path nodes, something has gone wrong */

            /* reset all nodes (referred to as 'C' henceforth) whose bases are internal nodes of the current key-paths */
            vnoiDataReset(&connectivityData, &keypathsData, graph->mark, &vnoiData);

            SCIP_CALL( connectivityDataKeyElimUpdate(scip, graph, &vnoiData, &supergraphData, crucnode, &connectivityData) );

            /* try to connect the nodes of C (directly) to COMP(C), as a preprocessing for graph_voronoiRepairMult */
            vnoiDataRepairPreprocess(scip, graph, &keypathsData, &connectivityData, &pcmwData, &vnoiData, &nheapelems);

            graph_voronoiRepairMult(scip, graph, (pcmwData.isActive? pcmwData.edgecost_biased : graph->cost),
               supernodesmark, &nheapelems, vnoibase, connectivityData.boundaryedges, &(connectivityData.nboundaryedges), &uf, vnoipath);

            SCIP_CALL( supergraphComputeMst(scip, graph, &connectivityData, &vnoiData, &keypathsData,
                  crucnode, &soltreeData, &pcmwData, &supergraphData) );

            assert(crucnode == dfstree[dfstree_pos]);

            SCIPdebugMessage("kpcost=%f  mstcost=%f \n", keypathsData.kpcost, supergraphData.mstcost);

            /* improving solution found? */
            if( SCIPisLT(scip, supergraphData.mstcost, keypathsData.kpcost) )
            {
               localmoves++;
               solimproved = TRUE;

               SCIPdebugMessage("found improving solution in KEY VERTEX ELIMINATION (round: %d) \n \n ", nruns);

               SCIP_CALL( soltreeElimKeyPathsStar(scip, graph, &connectivityData, &vnoiData, &keypathsData, &supergraphData,
                  dfstree, scanned, dfstree_pos, &soltreeData) );

#ifndef NDEBUG
               assert((keypathsData.kpcost - supergraphData.mstcost) >= 0.0);
               objimprovement += (keypathsData.kpcost - supergraphData.mstcost);
#endif
            }
            else /* no improving solution has been found during the move */
            {
               /* meld the heap pertaining to 'crucnode' and all heaps pertaining to descendant key-paths of node 'crucnode' */
               for( int k = 0; k < keypathsData.rootpathstart; k++ )
               {
                  SCIPpairheapMeldheaps(scip, &boundpaths[crucnode], &boundpaths[kpnodes[k]], &pheapsize[crucnode], &pheapsize[kpnodes[k]]);
               }
               for( int k = 0; k < supergraphData.nsupernodes - 1; k++ )
               {
                  SCIPpairheapMeldheaps(scip, &boundpaths[crucnode], &boundpaths[supernodes[k]], &pheapsize[crucnode], &pheapsize[supernodes[k]]);
                  SCIPStpunionfindUnion(&uf, crucnode, supernodes[k], FALSE);
               }
            }

            supergraphDataRestore(graph, &supergraphData);

            /* restore the original Voronoi diagram */
            vnoiDataRestore(&connectivityData, &keypathsData, &vnoiData);
         }  /* end of key vertex elimination */

         /* Key-Path Exchange:
         *  If the crucnode has just been eliminated, skip Key-Path Exchange */
         if( graph->mark[crucnode] )
         {
            SCIP_Real edgecost = FARAWAY;
            int e = UNKNOWN;
            int oldedge = UNKNOWN;
            int newedge = UNKNOWN;
            SCIP_Bool allgood = TRUE;

            assert(graph->mark[crucnode]);

            /* is crucnode not a crucial node and not a pinned vertex? */
            if( (!nodeIsCrucial(graph, solEdges, crucnode) && !pinned[crucnode]) )
               continue;

            /* gets key path from crucnode towards tree root */
            getKeyPathUpper(scip, crucnode, graph, &soltreeData, &pcmwData, &connectivityData, &keypathsData, &allgood);

            if( !allgood )
            {
               *success = FALSE;
               localmoves = 0;
               SCIPdebugMessage("terminate key vertex heuristic \n");
               goto TERMINATE;
            }

#ifndef NDEBUG
            for( int k = 0; k < nnodes; k++ )
               assert(graph->path_state[k] == CONNECT || !graph->mark[k]);
#endif

            /* reset all nodes (henceforth referred to as 'C') whose bases are internal nodes of the current keypath */
            vnoiDataReset(&connectivityData, &keypathsData, graph->mark, &vnoiData);

            while( boundpaths[crucnode] != NULL )
            {
               int base;
               int node;

               SCIP_CALL( SCIPpairheapDeletemin(scip, &e, &edgecost, &boundpaths[crucnode], &(pheapsize[crucnode])) );

               assert(e != UNKNOWN);
               base = vnoibase[graph->head[e]];

               assert(graph->mark[vnoibase[graph->tail[e]]]);
               node = (base == UNKNOWN || !graph->mark[base] )? UNKNOWN : SCIPStpunionfindFind(&uf, base);

               /* does the boundary-path end in the root component? */
               if( node != UNKNOWN && node != crucnode && graph->mark[base] )
               {
                  SCIP_CALL( SCIPpairheapInsert(scip, &boundpaths[crucnode],
                        pheapelements,
                        e, edgecost, &(pheapsize[crucnode])) );
                  break;
               }
            }

            if( boundpaths[crucnode] == NULL )
               oldedge = UNKNOWN;
            else
               oldedge = e;

            /* try to connect the nodes of C (directly) to COMP(C), as a preprocessing for Voronoi-repair */
            vnoiDataRepairPreprocess(scip, graph, &keypathsData, &connectivityData, &pcmwData, &vnoiData, &nheapelems);

            newedge = UNKNOWN;

            /* if there is no key path, nothing has to be repaired */
            if( keypathsData.nkpnodes > 0 )
            {
               graph_voronoiRepair(scip, graph, pcmwData.isActive? pcmwData.edgecost_biased : graph->cost,
                  pcmwData.edgecost_org, &nheapelems, vnoibase, vnoipath, &newedge, crucnode, &uf);
            }
            else
            {
               newedge = linkcutNodes[crucnode].edge;
            }

            edgecost = getKeyPathReplaceCost(scip, graph, &vnoiData, &soltreeData, edgecost, oldedge, &pcmwData, &newedge);

            if( SCIPisLT(scip, edgecost, keypathsData.kpcost) )
            {
               localmoves++;
               solimproved = TRUE;

               SCIPdebugMessage( "ADDING NEW KEY PATH (%f )\n\n", edgecost - keypathsData.kpcost );
#ifndef NDEBUG
               assert((keypathsData.kpcost - edgecost) >= 0.0);
               objimprovement += (keypathsData.kpcost - edgecost);
               assert(crucnode == dfstree[dfstree_pos]);
#endif

               SCIP_CALL( soltreeExchangeKeyPath(scip, graph, &connectivityData, &vnoiData, &keypathsData,
                     dfstree, scanned, dfstree_pos, newedge, &soltreeData) );
            }

            /* restore the original Voronoi diagram */
            vnoiDataRestore(&connectivityData, &keypathsData, &vnoiData);
         }
      }


      /**********************************************************/

   TERMINATE:

      assert(uf.nElements == nnodes);
      SCIPStpunionfindClear(scip, &uf);

      /* free data structures */

      for( int k = nnodes - 1; k >= 0; k-- )
      {
         if( boundpaths[k] )
            SCIPpairheapFree(scip, &boundpaths[k]);
      }

      /* has there been a move during this run? */
      if( localmoves > 0 )
      {
         for( int i = 0; i < nnodes; i++ )
         {
            solNodes[i] = FALSE;
            graph->mark[i] = (graph->grad[i] > 0);
            SCIPlinkcuttreeInitNode(&linkcutNodes[i]);
         }

         graph->mark[root] = TRUE;

         /* create a link-cut tree representing the current Steiner tree */
         for( int e = 0; e < nedges; e++ )
         {
            assert(graph->head[e] == graph->tail[flipedge(e)]);

            /* if edge e is in the tree, so are its incident vertices */
            if( solEdges[e] != -1 )
            {
               assert(CONNECT == solEdges[e]);

               solNodes[graph->tail[e]] = TRUE;
               solNodes[graph->head[e]] = TRUE;
               SCIPlinkcuttreeLink(linkcutNodes, graph->head[e], graph->tail[e], flipedge(e));
            }
         }
         assert( linkcutNodes[root].edge == -1 );
         linkcutNodes[root].edge = -1;
      }
   } /* main loop */

   /* free data structures */
   SCIPStpunionfindFreeMembers(scip, &uf);
   SCIPfreeMemoryArray(scip, &pheapelements);

   for( int k = nnodes - 1; k >= 0; k-- )
   {
      StpVecFree(scip, lvledges_start[k]);
      StpVecFree(scip, blists_start[k]);
   }

   SCIPfreeBufferArray(scip, &kpedges);
   SCIPfreeBufferArray(scip, &kpnodes);
   SCIPfreeBufferArray(scip, &supernodes);
   SCIPfreeBufferArray(scip, &supernodesmark);
   SCIPfreeBufferArray(scip, &dfstree);
   SCIPfreeBufferArray(scip, &pinned);
   SCIPfreeBufferArray(scip, &boundpaths);
   SCIPfreeBufferArray(scip, &meminedges);
   SCIPfreeBufferArray(scip, &memvdist);
   SCIPfreeBufferArray(scip, &memvbase);
   SCIPfreeBufferArray(scip, &blists_start);
   SCIPfreeBufferArray(scip, &pheapsize);
   SCIPfreeBufferArray(scip, &scanned);
   SCIPfreeBufferArrayNull(scip, &prizemarklist);
   SCIPfreeBufferArrayNull(scip, &prizemark);
   SCIPfreeBufferArrayNull(scip, &prize_pc);
   SCIPfreeBufferArrayNull(scip, &edgecostBiased_pc);
   SCIPfreeBufferArrayNull(scip, &edgecostOrg_pc);
   SCIPfreeBufferArray(scip, &boundedges);
   SCIPfreeBufferArray(scip, &lvledges_start);
   SCIPfreeBufferArray(scip, &newedges);
   SCIPfreeBufferArray(scip, &vnoibase);
   SCIPfreeBufferArray(scip, &vnoipath);
   /******/

   if( solimproved )
   {
      SCIP_CALL( solstp_pruneFromEdges(scip, graph, solEdges) );
      *success = TRUE;
   }

#ifndef NDEBUG
   {
      const SCIP_Real newobj = solstp_getObjBounded(graph, solEdges, 0.0, nedges);
      SCIPdebugMessage("key vertex heuristic obj before/after: %f/%f (improvement=%f)\n", initialobj, newobj, objimprovement);
      assert(SCIPisLE(scip, newobj + objimprovement, initialobj));
   }
#endif

   return SCIP_OKAY;
}


/*
 * Callback methods of primal heuristic
 */

/** copy method for primal heuristic plugins (called when SCIP copies plugins) */
static
SCIP_DECL_HEURCOPY(heurCopyLocal)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(heur != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);

   /* call inclusion method of primal heuristic */
   SCIP_CALL( SCIPStpIncludeHeurLocal(scip) );

   return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static
SCIP_DECL_HEURFREE(heurFreeLocal)
{   /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;

   assert(heur != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);
   assert(scip != NULL);

   /* free heuristic data */
   heurdata = SCIPheurGetData(heur);

   assert(heurdata != NULL);

   SCIPfreeRandom(scip, &heurdata->randnumgen);
   SCIPfreeMemory(scip, &heurdata);
   SCIPheurSetData(heur, NULL);

   return SCIP_OKAY;
}

/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static
SCIP_DECL_HEURINITSOL(heurInitsolLocal)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;

   assert(heur != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);
   assert(scip != NULL);

   /* free heuristic data */
   heurdata = SCIPheurGetData(heur);

   heurdata->nfails = 1;
   heurdata->nbestsols = -1;
   heurdata->nbestsols_min = -1;

   SCIP_CALL( SCIPallocMemoryArray(scip, &(heurdata->lastsolindices), heurdata->maxnsols) );

   for( int i = 0; i < heurdata->maxnsols; i++ )
      heurdata->lastsolindices[i] = -1;

   return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static
SCIP_DECL_HEUREXITSOL(heurExitsolLocal)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;

   assert(heur != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);
   assert(scip != NULL);

   /* free heuristic data */
   heurdata = SCIPheurGetData(heur);
   assert(heurdata != NULL);
   assert(heurdata->lastsolindices != NULL);
   SCIPfreeMemoryArray(scip, &(heurdata->lastsolindices));

   return SCIP_OKAY;
}


/** execution method of primal heuristic */
static
SCIP_DECL_HEUREXEC(heurExecLocal)
{  /*lint --e{715}*/
   SCIP_HEURDATA* heurdata;
   SCIP_PROBDATA* probdata;
   GRAPH* graph;                             /* graph structure */
   SCIP_SOL* initialsol;                         /* initial solution */
   SCIP_SOL** sols;                          /* solutions */
   int v;
   int nActiveSols;
   int nsols;                                /* number of all solutions found so far */
   int nedges;
   int* results;
   int* lastsolindices;
   SCIP_Real initialsol_obj;

   assert(heur != NULL);
   assert(scip != NULL);
   assert(strcmp(SCIPheurGetName(heur), HEUR_NAME) == 0);
   assert(result != NULL);

   /* get heuristic data */
   heurdata = SCIPheurGetData(heur);
   assert(heurdata != NULL);
   lastsolindices = heurdata->lastsolindices;
   assert(lastsolindices != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   graph = SCIPprobdataGetGraph(probdata);
   assert(graph != NULL);

   *result = SCIP_DIDNOTRUN;

   /* the local heuristics may not work correctly for several problem variants */
   if( !probtypeIsValidForLocal(graph) )
      return SCIP_OKAY;

   /* no solution available? */
   if( SCIPgetBestSol(scip) == NULL )
      return SCIP_OKAY;

   sols = SCIPgetSols(scip);
   nsols = SCIPgetNSols(scip);
   nedges = graph->edges;

   assert(heurdata->maxnsols >= 0);

   nActiveSols = MIN(heurdata->maxnsols, nsols);

   /* only process each solution once */
   for( v = 0; v < nActiveSols; v++ )
   {
      if( SCIPsolGetIndex(sols[v]) != lastsolindices[v] )
      {
         /* shift all solution indices right of the new solution index */
         for( int i = nActiveSols - 1; i >= v + 1; i-- )
            lastsolindices[i] = lastsolindices[i - 1];
         break;
      }
   }

   /* no new solution available? */
   if( v == nActiveSols )
      return SCIP_OKAY;

   if( heurdata->nbestsols == -1 )
      initSolNumberBounds(scip, heurdata);

   initialsol = sols[v];
   lastsolindices[v] = SCIPsolGetIndex(initialsol);

   /* solution not good enough? */
   if( (v > heurdata->nbestsols && !(heurdata->maxfreq)) )
      return SCIP_OKAY;

   /* has the new solution been found by this very heuristic
    * and not among the elite solutions? (note that there can still be an improvement because heuristic is aborted early) */
   if( SCIPsolGetHeur(initialsol) == heur && v > DEFAULT_NELITESOLS )
      return SCIP_OKAY;

   *result = SCIP_DIDNOTFIND;

   assert(SCIPprobdataGetVars(scip) != NULL);
   assert(SCIPprobdataGetNVars(scip) == nedges);

   /* allocate memory */
   SCIP_CALL( SCIPallocBufferArray(scip, &results, nedges) );

   /* write solution into 'results' array */
   solGetStpSol(scip, graph, initialsol, results);

   if( !solstp_isValid(scip, graph, results) )
   {
      SCIPfreeBufferArray(scip, &results);

      return SCIP_OKAY;
   }

   /* initial pruning necessary? */
   if( solNeedsPruning(initialsol) )
   {
      SCIP_CALL( solPrune(scip, graph, results) );
   }

   initialsol_obj = solstp_getObjBounded(graph, results, 0.0, nedges);

   /* execute local heuristics */
   SCIP_CALL( SCIPStpHeurLocalRun(scip, graph, results) );

#if 0
   if( graph_pc_isPcMw(graph) )
      SCIP_CALL( SCIPStpHeurLocalExtendPcMwImp(scip, graph, results) );
#endif

   /* finally, try to add the solution to the SCIP pool */
   SCIP_CALL( solAddTry(scip, sols, heur, graph, initialsol_obj, initialsol, results, result) );

   SCIPfreeBufferArray(scip, &results);

   return SCIP_OKAY;
}


/** perform local heuristics on a given Steiner tree */
static
SCIP_RETCODE localRun(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   SCIP_Bool             useFast,            /**< fast variant? */
   int*                  solEdges            /**< array indicating whether an arc is part of the solution: CONNECTED/UNKNOWN (in/out) */
   )
{
   SCIP_HEUR* heur;
   SCIP_HEURDATA* heurdata;
   LCNODE* linkcutNodes;
   const int root = graph->source;
   const int nnodes = graph->knots;
   STP_Bool* solNodes;
   const STP_Bool mwpc = graph_pc_isPcMw(graph);
   SCIP_Bool success = FALSE;
#ifndef NDEBUG
   const SCIP_Real initialobj = solstp_getObjBounded(graph, solEdges, 0.0, graph->edges);
#endif

   assert(graph && solEdges);
   assert(graph_valid(scip, graph));

   if( graph->grad[root] == 0 || graph->terms == 1 )
      return SCIP_OKAY;

   if( SCIPisStopped(scip) )
      return SCIP_OKAY;

   if( mwpc )
   {
      assert(graph->extended);

      if( solIsTrivialPcMw(graph, solEdges) )
         return SCIP_OKAY;
   }

   heur = SCIPfindHeur(scip, "local");
   assert(heur != NULL);
   heurdata = SCIPheurGetData(heur);
   assert(heurdata != NULL);

   SCIP_CALL( SCIPallocBufferArray(scip, &linkcutNodes, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solNodes, nnodes) );

   /* now call the actual heuristics */

   if( mwpc )
   {
      SCIP_CALL( SCIPStpHeurLocalExtendPcMw(scip, graph, graph->cost, solEdges, solNodes) );
   }

   markSolTreeNodes(scip, graph, solEdges, linkcutNodes, solNodes);

   SCIP_CALL( localVertexInsertion(scip, heurdata, graph, solNodes, linkcutNodes, solEdges) );

   SCIP_CALL( localKeyVertexHeuristics(scip, graph, useFast, solNodes, linkcutNodes, solEdges, &success) );

   if( success && !useFast )
   {
      markSolTreeNodes(scip, graph, solEdges, linkcutNodes, solNodes);
      SCIP_CALL( localVertexInsertion(scip, heurdata, graph, solNodes, linkcutNodes, solEdges) );
   }

   if( success && mwpc && !useFast )
   {
      SCIP_CALL( SCIPStpHeurLocalExtendPcMw(scip, graph, graph->cost, solEdges, solNodes) );
   }

#ifndef NDEBUG
   {
      const SCIP_Real newobj = solstp_getObjBounded(graph, solEdges, 0.0, graph->edges);
      assert(SCIPisLE(scip, newobj, initialobj));
      assert(solstp_isValid(scip, graph, solEdges));
   }
#endif

   SCIPfreeBufferArray(scip, &solNodes);
   SCIPfreeBufferArray(scip, &linkcutNodes);

   return SCIP_OKAY;
}


/** perform local heuristics on a given Steiner tree */
SCIP_RETCODE SCIPStpHeurLocalRun(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   int*                  solEdges            /**< array indicating whether an arc is part of the solution: CONNECTED/UNKNOWN (in/out) */
   )
{
   SCIP_CALL( localRun(scip, graph, FALSE, solEdges) );
   return SCIP_OKAY;
}


/** perform local heuristics on a given Steiner tree */
SCIP_RETCODE SCIPStpHeurLocalRunFast(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   int*                  solEdges            /**< array indicating whether an arc is part of the solution: CONNECTED/UNKNOWN (in/out) */
   )
{
   SCIP_CALL( localRun(scip, graph, TRUE, solEdges) );
    return SCIP_OKAY;
}


/** Implication based local heuristic for (R)PC and MW */
SCIP_RETCODE SCIPStpHeurLocalExtendPcMwImp(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          graph,              /**< graph data structure */
   int*                  result              /**< array indicating whether an arc is part of the solution (CONNECTED/UNKNOWN) */
)
{
   const int* starts = SCIPStpGetPcImplStarts(scip);
   const int* verts = SCIPStpGetPcImplVerts(scip);

   assert(graph_pc_isPcMw(graph));
   assert(0 && "currently not supported");

   if( starts != NULL )
   {
      const int nnodes = graph->knots;
      STP_Bool* stvertex;
      int nfound = 0;
      int ptermcount = 0;

      assert(graph->extended);
      assert(verts != NULL);

      SCIPallocBufferArray(scip, &stvertex, nnodes);

      solstp_setVertexFromEdge(graph, result, stvertex);

      for( int i = 0; i < nnodes; i++ )
      {
         if( !(Is_term(graph->term[i]) || Is_pseudoTerm(graph->term[i])) )
            continue;

         assert(!graph_pc_knotIsFixedTerm(graph, i));

         ptermcount++;

         if( stvertex[i] )
            continue;

         for( int j = starts[ptermcount - 1]; j < starts[ptermcount]; j++ )
         {
            const int vert = verts[j];
            if( stvertex[vert] )
            {
               /* now connect the vertex */

               graph_knot_printInfo(graph, i);
               nfound++;
               break;
            }
         }
      }

      assert(ptermcount == graph_pc_nNonFixedTerms(graph));

      if( nfound > 0 )
      {
         printf("nfound: %d \n\n\n", nfound);
      }
      else
         printf("none %d \n", 0);

      SCIPfreeBufferArray(scip, &stvertex);
   }

   return SCIP_OKAY;
}

/** Greedy Extension local heuristic for (R)PC and MW */
SCIP_RETCODE SCIPStpHeurLocalExtendPcMw(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   const SCIP_Real*      cost,               /**< edge cost array */
   int*                  stedge,             /**< initialized array to indicate whether an edge is part of the Steiner tree */
   STP_Bool*             stvertex            /**< uninitialized array to indicate whether a vertex is part of the Steiner tree */
   )
{
   GNODE candidates[GREEDY_EXTENSIONS_MW];
   int candidatesup[GREEDY_EXTENSIONS_MW];

   PATH* path;
   PATH* orgpath;
   SCIP_PQUEUE* pqueue;
   SCIP_Real bestsolval;

   int nextensions;
   const int greedyextensions = (graph->stp_type == STP_MWCSP) ? GREEDY_EXTENSIONS_MW : GREEDY_EXTENSIONS;
   const int nedges = graph->edges;
   const int nnodes = graph->knots;
   const int root = graph->source;
   STP_Bool* stvertextmp;
   SCIP_Bool extensions = FALSE;

#ifndef NDEBUG
   SCIP_Real objdiff = 0.0;
   const SCIP_Real initialobj = solstp_getObjBounded(graph, stedge, 0.0, nedges);
#endif

   assert(GREEDY_EXTENSIONS_MW >= greedyextensions);
   assert(scip != NULL);
   assert(graph != NULL);
   assert(stedge != NULL);
   assert(cost != NULL);
   assert(stvertex != NULL);
   assert(graph->extended);

   graph_pc_2transcheck(scip, graph);
   SCIP_CALL( SCIPallocBufferArray(scip, &stvertextmp, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &orgpath, nnodes) );
   SCIP_CALL( SCIPallocBufferArray(scip, &path, nnodes) );

   /* initialize solution vertex array with FALSE */
   BMSclearMemoryArray(stvertex, nnodes);

   stvertex[root] = TRUE;

   for( int j = 0; j < nnodes; j++ )
      path[j].edge = UNKNOWN;

   for( int e = 0; e < nedges; e++ )
   {
      if( stedge[e] == CONNECT )
      {
         path[graph->head[e]].edge = e;
         stvertex[graph->head[e]] = TRUE;
      }
   }

#ifndef NDEBUG
   for( int e = 0; e < nedges; e++ )
      if( stedge[e] == CONNECT )
         assert(stvertex[graph->tail[e]]);
#endif

   graph_path_st_pcmw_extend(scip, graph, cost, FALSE, path, stvertex, &extensions);

   BMScopyMemoryArray(orgpath, path, nnodes);

   /*** compute solution value and save greedyextensions many best unconnected nodes  ***/

   SCIP_CALL( SCIPpqueueCreate(&pqueue, greedyextensions, 2.0, GNODECmpByDist, NULL) );

   assert(orgpath[root].edge == UNKNOWN);

   bestsolval = 0.0;
   nextensions = 0;

   for( int i = 0; i < nnodes; i++ )
   {
      if( graph->grad[i] == 0 || root == i )
         continue;

      if( Is_term(graph->term[i]) && !graph_pc_knotIsFixedTerm(graph, i) )
         continue;

      if( stvertex[i] )
      {
         assert(orgpath[i].edge >= 0);

         bestsolval += graph->cost[orgpath[i].edge];

         if( Is_pseudoTerm(graph->term[i]) )
         {
            bestsolval -= graph->prize[i];
         }
      }
      else if( orgpath[i].edge != UNKNOWN && Is_pseudoTerm(graph->term[i]) )
      {
         SCIP_CALL( addToCandidates(scip, graph, path, i, greedyextensions, &nextensions, candidates, pqueue) );
      }
   }

   for( int restartcount = 0; restartcount < GREEDY_MAXRESTARTS && !graph_pc_isRootedPcMw(graph); restartcount++ )
   {
      int l = 0;
      SCIP_Bool extensionstmp = FALSE;
      int extcount = nextensions;

      /* write extension candidates into array, from max to min */
      while( SCIPpqueueNElems(pqueue) > 0 )
      {
         GNODE* min = (GNODE*) SCIPpqueueRemove(pqueue);
         assert(extcount > 0);
         candidatesup[--extcount] = min->number;
      }
      assert(extcount == 0);

      /* iteratively insert new subpaths and try to improve solution */
      for( ; l < nextensions; l++ )
      {
         const int extensioncand = candidatesup[l];
         if( !stvertex[extensioncand] )
         {
            SCIP_Real newsolval = 0.0;
            int k = extensioncand;

            BMScopyMemoryArray(stvertextmp, stvertex, nnodes);
            BMScopyMemoryArray(path, orgpath, nnodes);

            /* add new extension */
            while( !stvertextmp[k] )
            {
               stvertextmp[k] = TRUE;
               assert(orgpath[k].edge != UNKNOWN);
               k = graph->tail[orgpath[k].edge];
               assert(k != extensioncand);
            }

            graph_path_st_pcmw_extend(scip, graph, cost, TRUE, path, stvertextmp, &extensionstmp);


            for( int j = 0; j < nnodes; j++ )
            {
               if( graph->grad[j] == 0 || root == j )
                  continue;

               if( Is_term(graph->term[j]) && !graph_pc_knotIsFixedTerm(graph, j) )
                  continue;

               if( stvertextmp[j] )
               {
                  assert(path[j].edge >= 0);

                  newsolval += graph->cost[path[j].edge];

                  if( Is_pseudoTerm(graph->term[j]) )
                  {
                     newsolval -= graph->prize[j];
                  }
               }
            }

            /* new solution value better than old one? */
            if( SCIPisLT(scip, newsolval, bestsolval) )
            {
               SCIPdebugMessage("newsolval=%f bestsolval=%f \n", newsolval, bestsolval);

               extensions = TRUE;
               bestsolval = newsolval;

#ifdef SCIP_DEBUG
               for( int k2 = 0; k2 < nnodes; k2++ )
               {
                  if( !stvertex[k2] && stvertextmp[k2] )
                  {
                     printf("added \n");
                     graph_knot_printInfo(graph, k2);
                     graph_edge_printInfo(graph, path[k2].edge);
                  }
               }
#endif

#ifndef NDEBUG
               objdiff += bestsolval - newsolval;
#endif

               BMScopyMemoryArray(stvertex, stvertextmp, nnodes);
               BMScopyMemoryArray(orgpath, path, nnodes);

               /* save greedyextensions many best unconnected nodes  */
               nextensions = 0;

               for( int j = 0; j < nnodes; j++ )
                  if( !stvertex[j] && Is_pseudoTerm(graph->term[j]) && path[j].edge != UNKNOWN )
                     SCIP_CALL( addToCandidates(scip, graph, path, j, greedyextensions, &nextensions, candidates, pqueue) );

               break;
            } /* if new solution value better than old one? */
         } /* if !stvertex[i] */
      } /* for l < nextension */

      /* no more extensions performed? */
      if( l == nextensions )
         break;
   } /* main loop */

   /* have vertices been added? */
   if( extensions )
   {
      assert(graph_pc_isPcMw(graph));

      SCIP_CALL( solstp_prune(scip, graph, stedge, stvertex) );
   }

   assert(solstp_isValid(scip, graph, stedge));

   SCIPpqueueFree(&pqueue);
   SCIPfreeBufferArray(scip, &path);
   SCIPfreeBufferArray(scip, &orgpath);
   SCIPfreeBufferArray(scip, &stvertextmp);

#ifndef NDEBUG
   {
      const SCIP_Real newsolval = solstp_getObjBounded(graph, stedge, 0.0, nedges);
      SCIPdebugMessage("pcmw extend obj: initial=%f, new=%f \n", initialobj, newsolval);
      assert(SCIPisLE(scip, newsolval + objdiff, initialobj));
   }
#endif

   return SCIP_OKAY;
}

/** Greedy Extension local heuristic for (R)PC and MW */
SCIP_RETCODE SCIPStpHeurLocalExtendPcMwOut(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph,              /**< graph data structure */
   int*                  stedge,             /**< initialized array to indicate whether an edge is part of the Steiner tree */
   STP_Bool*             stvertex            /**< uninitialized array to indicate whether a vertex is part of the Steiner tree */
   )
{
   int candidates[GREEDY_EXTENSIONS];
   int ncandidates;
   DHEAP* dheap;
   STP_Bool* stvertextmp;
   SCIP_Real* dist;
   int* pred;
   const int nnodes = graph->knots;
   SCIP_Bool extensions = FALSE;
   int maxnode;
   const SCIP_Bool isexended = graph->extended;

#ifndef NDEBUG
   const int nedges = graph->edges;
   const SCIP_Real initialobj = solstp_getObjBounded(graph, stedge, 0.0, nedges);
#endif

   assert(scip && graph && stedge && stvertex);

   graph_pc_2orgcheck(scip, graph);

   solstp_setVertexFromEdge(graph, stedge, stvertex);

   /* compute candidates for extension */

   maxnode = -1;
   ncandidates = 0;

   for( int k = 0; k < nnodes; k++ )
      if( graph->mark[k] && !stvertex[k] && Is_term(graph->term[k]) && !graph_pc_termIsNonLeafTerm(graph, k) )
      {
         assert(graph->mark[k]);

         if( maxnode == -1 || graph->prize[k] > graph->prize[maxnode] )
            maxnode = k;
      }

   if( maxnode != -1 )
   {
      SCIP_RANDNUMGEN* randnumgen;
      int shift;

      SCIP_CALL( SCIPcreateRandom(scip, &randnumgen, 1, TRUE) );

      SCIP_CALL( SCIPallocBufferArray(scip, &dist, nnodes) );
      SCIP_CALL( SCIPallocBufferArray(scip, &pred, nnodes) );
      SCIP_CALL( SCIPallocBufferArray(scip, &stvertextmp, nnodes) );

      graph_heap_create(scip, nnodes, NULL, NULL, &dheap);
      graph_init_csr(scip, graph);

      shift = SCIPrandomGetInt(randnumgen, 0, nnodes - 1);
      ncandidates = 1;
      candidates[0] = maxnode;

      for( int k = 0; k < nnodes && ncandidates < GREEDY_EXTENSIONS; k++ )
      {
         const int node = (k + shift) % nnodes;
         if( graph->mark[k] && !stvertex[node] && Is_term(graph->term[node])
            && !graph_pc_termIsNonLeafTerm(graph, node) && node != maxnode )
         {
            assert(graph->mark[node]);
            candidates[ncandidates++] = node;
         }
      }

      SCIPfreeRandom(scip, &randnumgen);
   }

   /* main loop */
   for( int k = 0; k < ncandidates; k++ )
   {
      const int cand = candidates[k];
      SCIP_Bool success = FALSE;

      if( stvertex[cand] )
      {
         assert(k > 0);
         continue;
      }

      graph_path_st_pcmw_extendOut(scip, graph, cand, stvertex, dist, pred, stvertextmp, dheap, &success);

      if( success )
         extensions = TRUE;
   }

   /* have vertices been added? */
   if( extensions )
   {
      graph_pc_2trans(scip, graph);

      assert(graph_pc_isPcMw(graph));

      SCIP_CALL( solstp_prune(scip, graph, stedge, stvertex) );
   }

   if( maxnode != -1 )
   {
      graph_heap_free(scip, TRUE, TRUE, &dheap);
      graph_free_csr(scip, graph);

      SCIPfreeBufferArray(scip, &stvertextmp);
      SCIPfreeBufferArray(scip, &pred);
      SCIPfreeBufferArray(scip, &dist);
   }

#ifndef NDEBUG
   assert(SCIPisLE(scip, solstp_getObjBounded(graph, stedge, 0.0, nedges), initialobj));
#endif

   if( isexended && !graph->extended )
      graph_pc_2trans(scip, graph);

   if( !isexended && graph->extended )
      graph_pc_2org(scip, graph);

   return SCIP_OKAY;
}


/** creates the local primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPStpIncludeHeurLocal(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_HEURDATA* heurdata;
   SCIP_HEUR* heur;

   /* create Local primal heuristic data */
   SCIP_CALL( SCIPallocMemory(scip, &heurdata) );

   /* include primal heuristic */
   SCIP_CALL( SCIPincludeHeurBasic(scip, &heur,
         HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
         HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecLocal, heurdata) );

   assert(heur != NULL);

   /* set non-NULL pointers to callback methods */
   SCIP_CALL( SCIPsetHeurCopy(scip, heur, heurCopyLocal) );
   SCIP_CALL( SCIPsetHeurFree(scip, heur, heurFreeLocal) );
   SCIP_CALL( SCIPsetHeurInitsol(scip, heur, heurInitsolLocal) );
   SCIP_CALL( SCIPsetHeurExitsol(scip, heur, heurExitsolLocal) );

   /* add local primal heuristic parameters */
   SCIP_CALL( SCIPaddBoolParam(scip, "stp/duringroot",
         "should the heuristic be called during the root node?",
         &heurdata->duringroot, FALSE, DEFAULT_DURINGROOT, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip, "heuristics/"HEUR_NAME"/maxfreq",
         "should the heuristic be executed at maximum frequeny?",
         &heurdata->maxfreq, FALSE, DEFAULT_MAXFREQLOC, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip, "heuristics/"HEUR_NAME"/maxnsols",
         "maximum number of best solutions to improve",
         &heurdata->maxnsols, FALSE, DEFAULT_MAXNBESTSOLS, DEFAULT_NBESTSOLS_HARD, 100, NULL, NULL) );

   SCIP_CALL( SCIPcreateRandom(scip, &heurdata->randnumgen, DEFAULT_RANDSEED, TRUE) );

   return SCIP_OKAY;
}
