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

/**@file   graph_history.c
 * @brief  includes graph ancestor methods for Steiner problems
 * @author Daniel Rehfeldt
 *
 * Graph ancestor methods for Steiner problems. Usually used to reconstruct a graph after reductions
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

/*lint -esym(766,stdlib.h) -esym(766,malloc.h)         */
/*lint -esym(766,string.h) */
#include <assert.h>
#include "graph.h"
#include "portab.h"


/** fixed graph components, typedef FIXED */
struct fixed_graph_components
{
   IDX*                  fixedges;           /**< fixed edges */
   int*                  fixpseudonodes;     /**< fixed psuedo eliminated nodes  */
   int                   nfixnodes;          /**< number of fixed nodes  */
   SCIP_Bool             movedFrom;          /**< ancestor information moved away? */
};

/** blocked pseudo ancestors */
typedef struct blocked_pseudo_ancestor
{
   int**                 blocks;             /**< blocks of ancestors (of size  nblocks) */
   int*                  sizes;              /**< current number of ancestors (of size  nblocks)  */
   int*                  capacities;         /**< current capacities of ancestors (of size nblocks) */
   int                   nblocks;            /**< number of ancestor blocks */
} BLOCKANS;

/** node ancestors resulting from pseudo-elimination, typedef PSEUDOANS */
struct pseudo_ancestors
{
   BLOCKANS*             ans_halfedges;      /**< (half) edge ancestors  */
   BLOCKANS*             ans_nodes;          /**< (pc/mw) node ancestors  */
   int                   nnodes;             /**< number of nodes */
   int                   halfnedges;         /**< half number of edges */
   int                   nAllPseudoancestors;/**< number of pseudo-ancestors */
};


/** get next power of 2 number; assumes number >= 1 and number < UINT_MAX */
static inline
int getNextPow2(int number)
{
   uint32_t n = (uint32_t) number;
   assert(number >= 1);

   n--;
   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;
   n++;
   SCIPdebugMessage("pow2 %d from number %d \n", n, number);

   assert(n < INT_MAX );
   assert((int) n >= number && (int) n <= 2 * number);
   assert(number >= 1);

   return (int) n;
}


/** mark ancestors of given edge */
static
SCIP_Bool ancestorsMarkConflict(
   const GRAPH*          graph,              /**< graph data structure */
   int                   edge,               /**< edge to use */
   int*                  ancestormark        /**< ancestor mark array */
   )
{
   assert(edge >= 0);

   for( IDX* curr = graph->ancestors[edge]; curr != NULL;  curr = curr->parent )
   {
      const unsigned idx = ((unsigned) curr->index) / 2;

      assert(curr->index >= 0 && idx < (unsigned) (MAX(graph->edges, graph->orgedges) / 2));

      if( ancestormark[idx] )
         return TRUE;

      ancestormark[idx] = 1;
   }

   return FALSE;
}


/** unmark ancestors of given edge */
static
void ancestorsUnmarkConflict(
   const GRAPH*          graph,              /**< graph data structure */
   int                   edge,               /**< edge to use */
   int*                  ancestormark        /**< ancestor mark array */
   )
{
   assert(edge >= 0);

   for( IDX* curr = graph->ancestors[edge]; curr != NULL; curr = curr->parent )
   {
      assert(curr->index >= 0 && curr->index / 2 < (MAX(graph->edges, graph->orgedges) / 2));
      ancestormark[((unsigned) curr->index) / 2] = 0;
   }
}


/** initialize */
static
SCIP_RETCODE blockedAncestors_init(
   int                   nblocks,            /**< number of ancestor blocks */
   BLOCKANS**            blockedans          /**< blocked pseudo-ancestors */
)
{
   int** blocks;
   int* sizes;
   int* capacities;

   assert(nblocks > 0);

   SCIP_CALL( SCIPallocMemory(scip, blockedans) );

   (*blockedans)->nblocks = nblocks;

   SCIP_CALL( SCIPallocMemoryArray(scip, &(blocks), nblocks) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &(sizes), nblocks) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &(capacities), nblocks) );

   for( int k = 0; k < nblocks; k++ )
   {
      blocks[k] = NULL;
      sizes[k] = 0;
      capacities[k] = 0;
   }

   (*blockedans)->blocks = blocks;
   (*blockedans)->sizes = sizes;
   (*blockedans)->capacities = capacities;

   return SCIP_OKAY;
}


/** free */
static inline
void blockedAncestors_freeBlock(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   block_id,           /**< id for which to free pseudo ancestors */
   BLOCKANS*             blockedans         /**< blocked pseudo-ancestors */
)
{
   const int capacity = blockedans->capacities[block_id];

   assert(scip && blockedans && blockedans->sizes && blockedans->capacities);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(capacity >= 0);

   if( capacity > 0 )
   {
      assert(blockedans->blocks[block_id]);
      SCIPfreeBlockMemoryArray(scip, &(blockedans->blocks[block_id]), capacity);

      blockedans->sizes[block_id] = 0;
      blockedans->capacities[block_id] = 0;
   }

   assert(blockedans->sizes[block_id] == 0);
   assert(blockedans->capacities[block_id] == 0);
   assert(blockedans->blocks[block_id] == NULL);
}


/** free */
static
void blockedAncestors_free(
   SCIP*                 scip,               /**< SCIP data structure */
   BLOCKANS**            blockedans          /**< blocked pseudo-ancestors */
)
{
   BLOCKANS* const ancestors = (*blockedans);
   int* const * blocks = ancestors->blocks;
   const int* sizes = ancestors->sizes;
   const int* capacities = ancestors->capacities;
   const int nblocks = ancestors->nblocks;

   assert(blockedans && ancestors);
   assert(nblocks >= 1);
   assert(capacities && sizes && blocks);

   for( int e = nblocks - 1; e >= 0; --e )
      blockedAncestors_freeBlock(scip, e, ancestors);

   SCIPfreeMemoryArray(scip, &sizes);
   SCIPfreeMemoryArray(scip, &capacities);
   SCIPfreeMemoryArray(scip, &blocks);

   SCIPfreeMemoryArray(scip, blockedans);
}


/** hash ancestors of given entry */
static inline
void blockedAncestors_hash(
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int                   block_id,           /**< entry for which to reallocate */
   int*                  hasharr             /**< hash array for pseudo ancestors */
)
{
   const int nancestors = blockedans->sizes[block_id];
   const int* const ancestorsblock = blockedans->blocks[block_id];

   assert(blockedans && hasharr);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(nancestors >= 0);

   for( int k = 0; k < nancestors; k++ )
   {
      const int a = ancestorsblock[k];
      assert(a >= 0 && hasharr[a] == 0);

      hasharr[a] = 1;
   }
}


/** unhash nAncestorsToClean many ancestors  */
static inline
void blockedAncestors_unhashPartial(
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int                   block_id,           /**< entry for which to reallocate */
   int                   nAncestorsToClean,  /**< number of (first) ancestors to use for clean, or -1 to clean all */
   int*                  hasharr             /**< hash array for pseudo ancestors */
)
{
   const int* const ancestorsblock = blockedans->blocks[block_id];
   const int n = (nAncestorsToClean >= 0) ? nAncestorsToClean : blockedans->sizes[block_id];

   assert(blockedans && hasharr);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(nAncestorsToClean >= 0 || nAncestorsToClean == -1);

   for( int k = 0; k < n; k++ )
   {
      const int a = ancestorsblock[k];
      assert(a >= 0);
      assert(hasharr[a] == 1);

      hasharr[a] = 0;
   }
}


/** hash ancestors of given entry with possible conflicts */
static inline
void blockedAncestors_hashDirty(
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int                   block_id,           /**< entry for which to reallocate */
   SCIP_Bool             revertIfConflict,   /**< break on conflict? */
   SCIP_Bool*            conflict,           /**< conflict? */
   int*                  hasharr             /**< hash array for pseudo ancestors */
)
{
   int k;
   const int nancestors = blockedans->sizes[block_id];
   const int* const ancestorsblock = blockedans->blocks[block_id];

   assert(blockedans && hasharr);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(nancestors >= 0);

   *conflict = FALSE;

   for( k = 0; k < nancestors; k++ )
   {
      const int a = ancestorsblock[k];
      assert(a >= 0 );
      assert(hasharr[a] == 0 || hasharr[a] == 1);

      if( hasharr[a] == 1 )
      {
         *conflict = TRUE;

         if( revertIfConflict )
            break;
      }

      hasharr[a] = 1;
   }

   /* do we need to clean up? */
   if( *conflict && revertIfConflict )
   {
      assert(k < nancestors);

      for( int k2 = 0; k2 < k; k2++ )
      {
         const int a = ancestorsblock[k2];
         assert(a >= 0 );
         assert(hasharr[a] == 1);

         hasharr[a] = 0;
      }
   }
}


/** unhash ancestors of given entry with possible conflicts */
static inline
void blockedAncestors_unhashDirty(
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int                   block_id,           /**< entry for which to reallocate */
   int*                  hasharr             /**< hash array for pseudo ancestors */
)
{
   const int nancestors = blockedans->sizes[block_id];
   const int* const ancestorsblock = blockedans->blocks[block_id];

   assert(blockedans && hasharr);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(nancestors >= 0);

   for( int k = 0; k < nancestors; k++ )
   {
      const int a = ancestorsblock[k];
      assert(a >= 0 );
      assert(hasharr[a] == 0 || hasharr[a] == 1);

      hasharr[a] = 0;
   }
}


/** ancestor already hashed? */
static inline
SCIP_Bool blockedAncestors_hashIsHit(
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int                   ancestor,           /**< ancestor to check */
   const int*            hasharr             /**< hash array for pseudo ancestors*/
)
{
   assert(blockedans && hasharr);
   assert(hasharr[ancestor] == 0 || hasharr[ancestor] == 1);

   return (hasharr[ancestor] == 1);
}


/** any ancestors of given entry are already hashed? */
static inline
SCIP_Bool blockedAncestors_hashIsHitBlock(
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int                   block_id,           /**< entry for which to check for conflicts */
   const int*            hasharr             /**< hash array for pseudo ancestors */
)
{
   const int nancestors = blockedans->sizes[block_id];
   const int* const ancestorsblock = blockedans->blocks[block_id];

   assert(blockedans && hasharr);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(nancestors >= 0);

   for( int k = 0; k < nancestors; k++ )
   {
      const int a = ancestorsblock[k];
      assert(a >= 0 );
      assert(hasharr[a] == 0 || hasharr[a] == 1);

      if( 1 == hasharr[a] )
         return TRUE;
   }

   return FALSE;
}


/** reallocate ancestor array of given entry */
static inline
SCIP_RETCODE blockedAncestors_realloc(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   block_id,           /**< entry for which to reallocate */
   int                   capacity_new,       /**< the new capacity */
   BLOCKANS*             blockedans          /**< blocked pseudo-ancestors */
)
{
   const int capacity_old = blockedans->capacities[block_id];

   assert(scip && blockedans);
   assert(block_id >= 0 && block_id < blockedans->nblocks);
   assert(capacity_old >= 0 && capacity_new > capacity_old);
   assert(capacity_new >= 1);

   if( capacity_old == 0 )
   {
      assert(blockedans->blocks[block_id] == NULL);
      assert(blockedans->sizes[block_id] == 0);

      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &(blockedans->blocks[block_id]), capacity_new) );
   }
   else
   {
      assert(blockedans->blocks[block_id] != NULL);
      assert(capacity_old >= 1);

      SCIP_CALL( SCIPreallocBlockMemoryArray(scip, &(blockedans->blocks[block_id]), capacity_old, capacity_new) );
   }

   blockedans->capacities[block_id] = capacity_new;

   return SCIP_OKAY;
}


/** pseudo ancestors of given block without conflicts? */
static inline
SCIP_Bool blockedAncestors_blockIsValid(
   int                   block_id,           /**< entry for which to reallocate */
   const BLOCKANS*       blockedans,         /**< blocked pseudo-ancestors */
   int*                  hasharr             /**< clean hash array for pseudo ancestors */
)
{
   SCIP_Bool conflict;

   assert(blockedans);
   assert(block_id >= 0 && block_id < blockedans->nblocks);

   blockedAncestors_hashDirty(blockedans, block_id, FALSE, &conflict, hasharr);
   blockedAncestors_unhashDirty(blockedans, block_id, hasharr);

   return !conflict;
}


/** appends copy of pseudo ancestors of source to target */
static
SCIP_RETCODE blockedAncestors_appendArray(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   block_target,       /**< target block */
   const int*            ancestors_source,   /**< ancestors to append */
   int                   size_source,        /**< number of ancestors to append */
   SCIP_Bool             breakOnConflict,    /**< break on conflict */
   int                   hasharr_size,       /**< size for hash array */
   BLOCKANS*             blockedans_target,  /**< blocked pseudo-ancestors */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   int* ancestors_target;
   int* hasharr;
   int position_target;
   const int size_target_org = blockedans_target->sizes[block_target];
   const int size_targetPlusSource = size_target_org + size_source;

   assert(size_source > 0);
   assert(ancestors_source && blockedans_target);

   SCIP_CALL( SCIPallocCleanBufferArray(scip, &hasharr, hasharr_size) );

   /* need to realloc target ancestor array? */
   if( size_targetPlusSource > blockedans_target->capacities[block_target] )
   {
      const int capacity_up = getNextPow2(size_targetPlusSource);
      SCIP_CALL( blockedAncestors_realloc(scip, block_target, capacity_up, blockedans_target) );
   }

   ancestors_target = blockedans_target->blocks[block_target];

   /* mark ancestors of target */
   blockedAncestors_hash(blockedans_target, block_target, hasharr);

   position_target = size_target_org;

   /* add source to target */
   for( int e = 0; e < size_source; e++ )
   {
      const int ancestor = ancestors_source[e];
      assert(ancestor < hasharr_size);

      if( blockedAncestors_hashIsHit(blockedans_target, ancestor, hasharr) )
      {
         *conflict = TRUE;

         if( breakOnConflict )
            break;

         continue;
      }

      assert(position_target < blockedans_target->capacities[block_target]);

      ancestors_target[position_target++] = ancestor;
   }

   assert(position_target <= size_targetPlusSource);
   assert(position_target >= blockedans_target->sizes[block_target]);

   blockedans_target->sizes[block_target] = position_target;

   /* unmark ancestors of target */
   blockedAncestors_unhashPartial(blockedans_target, block_target, size_target_org, hasharr);

   SCIPfreeCleanBufferArray(scip, &hasharr);

   return SCIP_OKAY;
}


/** appends copy of pseudo ancestors of source to target */
static
SCIP_RETCODE blockedAncestors_appendCopy(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   block_target,       /**< target block */
   const BLOCKANS*       blockedans_source,  /**< blocked pseudo-ancestors */
   int                   block_source,       /**< source block */
   SCIP_Bool             breakOnConflict,    /**< break on conflict */
   int                   hasharr_size,       /**< size of hash array */
   BLOCKANS*             blockedans_target,  /**< blocked pseudo-ancestors */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   const int size_source = blockedans_source->sizes[block_source];

   assert(scip && blockedans_source && blockedans_target && conflict);
   assert(hasharr_size >= 1 && size_source >= 0);
   assert(block_target >= 0 && block_target < blockedans_target->nblocks);
   assert(block_source >= 0 && block_source < blockedans_source->nblocks);

   *conflict = FALSE;

   /* anything to append? */
   if( size_source > 0 )
   {
      const int* const ancestors_source = blockedans_source->blocks[block_source];

      SCIP_CALL( blockedAncestors_appendArray(scip, block_target, ancestors_source, size_source, breakOnConflict, hasharr_size, blockedans_target, conflict) );
   }

   return SCIP_OKAY;
}


/** adds pseudo ancestor to ancestor list of given block */
static
SCIP_RETCODE blockedAncestors_addAncestor(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   block_target,       /**< block for which to add pseudo ancestor */
   int                   ancestor,           /**< (index of) pseudo ancestor */
   int                   hasharr_size,       /**< size of hash array */
   BLOCKANS*             blockedans          /**< blocked pseudo-ancestors */
)
{
   int* const sizes = blockedans->sizes;
   int* const capacities = blockedans->capacities;

   assert(scip && blockedans);
   assert(block_target >= 0 && block_target < blockedans->nblocks);
   assert(ancestor < hasharr_size);

   /* need to reallocate? */
   if( sizes[block_target] == capacities[block_target] )
   {
      const int capacity_up = getNextPow2(sizes[block_target] + 1);
      SCIP_CALL( blockedAncestors_realloc(scip, block_target, capacity_up, blockedans) );
   }

   assert(sizes[block_target] < capacities[block_target]);

   blockedans->blocks[block_target][sizes[block_target]++] = ancestor;

#ifndef NDEBUG
   {
      int* hasharr;
      assert(hasharr_size >= 1);
      SCIP_CALL_ABORT( SCIPallocCleanBufferArray(scip, &hasharr, hasharr_size) );
      assert(blockedAncestors_blockIsValid(block_target, blockedans, hasharr));
      SCIPfreeCleanBufferArray(scip, &hasharr);
   }
#endif

   return SCIP_OKAY;
}


/** valid pseudo-ancestors (no conflicts)? */
static
SCIP_Bool blockedAncestors_isValid(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   hasharr_size,       /**< size of hash array */
   const BLOCKANS*       blockedans          /**< blocked pseudo-ancestors */
)
{
   int* hasharr;
   SCIP_Bool isValid = TRUE;

   assert(scip && blockedans);
   assert(hasharr_size >= 1);

   /* check whether sizes/capacities are correct */
   for( int e = 0; e < blockedans->nblocks && isValid; e++ )
   {
      if( blockedans->sizes[e] > blockedans->capacities[e] )
         isValid = FALSE;

      if( blockedans->sizes[e] > 0 && blockedans->blocks[e] == NULL )
         isValid = FALSE;

      if( blockedans->capacities[e] > 0 && blockedans->blocks[e] == NULL )
         isValid = FALSE;
   }

   SCIP_CALL_ABORT( SCIPallocCleanBufferArray(scip, &hasharr, hasharr_size) );

   /* check whether there are conflict within the ancestor blocks */
   for( int e = 0; e < blockedans->nblocks && isValid; e++ )
   {
      if( blockedans->blocks[e] != NULL && !blockedAncestors_blockIsValid(e, blockedans, hasharr) )
         isValid = FALSE;
   }

   SCIPfreeCleanBufferArray(scip, &hasharr);

   return isValid;
}


/** checks */
static
SCIP_Bool fixedPseudoAncestorsAreValid(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g                   /**< the graph */
   )
{
   int *hasharr;
   const int *fixednodes = graph_getFixpseudonodes(scip, g);
   const int arrsize = graph_pseudoAncestorsGetHashArraySize(g->pseudoancestors);
   const int nfixednodes = graph_getNfixpseudonodes(g);
   SCIP_Bool isValid = TRUE;

   assert(arrsize >= nfixednodes);
   SCIP_CALL_ABORT( SCIPallocClearMemoryArray(scip, &hasharr, arrsize) );

   for( int k = 0; k < nfixednodes; k++ )
   {
      const int node = fixednodes[k];
      assert(node >= 0 && node < arrsize);

      if( hasharr[node] != 0 )
      {
         SCIPerrorMessage("fail for ancestor node %d (hashed twice) \n", node);
         isValid = FALSE;
         break;
      }

      hasharr[node] = 1;
   }

   SCIPfreeMemoryArray(scip, &hasharr);

   return isValid;
}

/** initializes singleton edge ancestors */
SCIP_RETCODE graph_singletonAncestors_init(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< the graph */
   int                   edge,               /**< edge to initialize from */
   SINGLETONANS*         singletonans        /**< singleton edge ancestors */
)
{
   SCIP_Bool conflict = FALSE;

   assert(scip && g && singletonans);
   assert(edge >= 0 && edge < g->edges);

   singletonans->npseudoancestors = graph_edge_nPseudoAncestors(g, edge);
   singletonans->edge = edge;
   singletonans->ancestors = NULL;
   singletonans->revancestors = NULL;

   if( singletonans->npseudoancestors > 0 )
   {
      const int* const pseudoancestors = graph_edge_getPseudoAncestors(g, edge);
      SCIP_CALL( SCIPallocMemoryArray(scip, &(singletonans->pseudoancestors), singletonans->npseudoancestors) );
      BMScopyMemoryArray(singletonans->pseudoancestors, pseudoancestors, singletonans->npseudoancestors);
   }
   else
   {
      assert(singletonans->npseudoancestors == 0);
      singletonans->pseudoancestors = NULL;
   }

   if( !g->ancestors[edge] || !g->ancestors[flipedge(edge)] )
   {
      assert(graph_typeIsUndirected(g));
      SCIP_CALL( SCIPintListNodeAppendCopy(scip, &(singletonans->ancestors), graph_edge_getAncestors(g, edge), &conflict) );
      assert(!conflict);
   }
   else
   {
      assert(!graph_typeIsUndirected(g));
      SCIP_CALL( SCIPintListNodeAppendCopy(scip, &(singletonans->ancestors), g->ancestors[edge], &conflict) );
      assert(!conflict);

      SCIP_CALL( SCIPintListNodeAppendCopy(scip, &(singletonans->revancestors), g->ancestors[flipedge(edge)], &conflict) );
      assert(!conflict);
   }

   return SCIP_OKAY;
}


/** initializes singleton edge ancestors */
void graph_singletonAncestors_freeMembers(
   SCIP*                 scip,               /**< SCIP data structure */
   SINGLETONANS*         singletonans        /**< singleton edge ancestors */
)
{
   assert(scip && singletonans);
   assert(singletonans->ancestors);
   assert(singletonans->pseudoancestors || singletonans->npseudoancestors == 0);
   assert(singletonans->npseudoancestors >= 0);

   if( singletonans->npseudoancestors > 0 )
      SCIPfreeMemoryArray(scip, &(singletonans->pseudoancestors));

   SCIPintListNodeFree(scip, &(singletonans->ancestors));
   SCIPintListNodeFree(scip, &(singletonans->revancestors));
}


/** valid ancestors (no conflicts)? */
SCIP_Bool graph_valid_ancestors(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g                   /**< the graph */
)
{
   int* edgemark;
   const int nedges = g->edges;
   const int nancestors = MAX(g->edges, g->orgedges) / 2;
   const SCIP_Bool pcmw = graph_pc_isPcMw(g);
   SCIP_Bool isValid = TRUE;

   assert(scip != NULL && g != NULL);
   assert(g->ancestors != NULL);

   SCIP_CALL_ABORT( SCIPallocBufferArray(scip, &edgemark, nancestors) );

   for( int e = 0; e < nancestors; e++ )
      edgemark[e] = FALSE;

   for( int e = 0; e < nedges; e += 2 )
   {
      SCIP_Bool conflict;

      if( g->oeat[e] == EAT_FREE )
         continue;

      if( pcmw && !SCIPisEQ(scip, g->cost[e], g->cost[e + 1]) )
         continue;

      if( !g->ancestors[e] )
      {
         isValid = FALSE;
         break;
      }

      if( graph_typeIsUndirected(g) && g->ancestors[e + 1] )
      {
         isValid = FALSE;
         break;
      }

      conflict = ancestorsMarkConflict(g, e, edgemark);
      ancestorsUnmarkConflict(g, e, edgemark);

      if( conflict )
      {
         isValid = FALSE;
         break;
      }

      conflict = ancestorsMarkConflict(g, e + 1, edgemark);
      ancestorsUnmarkConflict(g, e + 1, edgemark);

      if( conflict )
      {
         isValid = FALSE;
         break;
      }

   }

   SCIPfreeBufferArray(scip, &edgemark);

   if( isValid && !g->withInexactReductions )
      isValid = fixedPseudoAncestorsAreValid(scip, g);

   return isValid;
}


/** hash ancestors of given edge */
void graph_pseudoAncestors_hashEdge(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   edge,               /**< edge for which to hash */
   int*                  hasharr             /**< hash array */
)
{
   const int halfedge = edge / 2;

   assert(pseudoancestors && hasharr);
   assert(halfedge >= 0 && halfedge < pseudoancestors->halfnedges);

   blockedAncestors_hash(pseudoancestors->ans_halfedges, halfedge, hasharr);
}


/** hash ancestors of given node */
void graph_pseudoAncestors_hashNode(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   node,               /**< node for which to hash */
   int*                  hasharr             /**< hash array */
)
{
   assert(pseudoancestors && hasharr);
   assert(node >= 0 && node < pseudoancestors->nnodes);

   blockedAncestors_hash(pseudoancestors->ans_nodes, node, hasharr);
}


/** unhash ancestors of given edge */
void graph_pseudoAncestors_unhashEdge(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   edge,               /**< edge for which to unhash */
   int*                  hasharr             /**< hash array */
)
{
   const int halfedge = edge / 2;

   assert(pseudoancestors && hasharr);
   assert(halfedge >= 0 && halfedge < pseudoancestors->halfnedges);

   blockedAncestors_unhashPartial(pseudoancestors->ans_halfedges, halfedge, -1, hasharr);
}


/** hash ancestors of given node */
void graph_pseudoAncestors_unhashNode(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   node,               /**< node for which to unhash */
   int*                  hasharr             /**< hash array */
)
{
   assert(pseudoancestors && hasharr);
   assert(node >= 0 && node < pseudoancestors->nnodes);

   blockedAncestors_unhashPartial(pseudoancestors->ans_nodes, node, -1, hasharr);
}


/** hash ancestors of given edge with possible conflicts */
void graph_pseudoAncestors_hashEdgeDirty(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   edge,               /**< edge for which to hash */
   SCIP_Bool             revertIfConflict,   /**< revert on conflict? */
   SCIP_Bool*            conflict,           /**< conflict? */
   int*                  hasharr             /**< hash array */
)
{
   const int halfedge = edge / 2;

   assert(pseudoancestors && hasharr);
   assert(halfedge >= 0 && halfedge < pseudoancestors->halfnedges);

   blockedAncestors_hashDirty(pseudoancestors->ans_halfedges, halfedge, revertIfConflict, conflict, hasharr);
}


/** hash ancestors of given node with possible conflicts */
void graph_pseudoAncestors_hashNodeDirty(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   node,               /**< node for which to hash */
   SCIP_Bool             revertIfConflict,   /**< revert on conflict? */
   SCIP_Bool*            conflict,           /**< conflict? */
   int*                  hasharr             /**< hash array */
)
{
   assert(pseudoancestors && hasharr);
   assert(node >= 0 && node < pseudoancestors->nnodes);

   blockedAncestors_hashDirty(pseudoancestors->ans_nodes, node, revertIfConflict, conflict, hasharr);
}


/** unhash ancestors of given edge with possible conflicts */
void graph_pseudoAncestors_unhashEdgeDirty(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   edge,               /**< edge for which to unhash */
   int*                  hasharr             /**< hash array */
)
{
   const int halfedge = edge / 2;

   assert(pseudoancestors && hasharr);
   assert(halfedge >= 0 && halfedge < pseudoancestors->halfnedges);

   blockedAncestors_unhashDirty(pseudoancestors->ans_halfedges, halfedge, hasharr);
}


/** hash ancestors of given node with possible conflicts */
void graph_pseudoAncestors_unhashNodeDirty(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   node,               /**< node for which to unhash */
   int*                  hasharr             /**< hash array */
)
{
   assert(pseudoancestors && hasharr);
   assert(node >= 0 && node < pseudoancestors->nnodes);

   blockedAncestors_unhashDirty(pseudoancestors->ans_nodes, node, hasharr);
}


/** any ancestors of given edge already hashed? */
SCIP_Bool graph_pseudoAncestors_edgeIsHashed(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   edge,               /**< edge for which to check */
   const int*            hasharr             /**< hash array */
)
{
   const int block_id = edge / 2;

   assert(pseudoancestors && hasharr);

   return blockedAncestors_hashIsHitBlock(pseudoancestors->ans_halfedges, block_id, hasharr);
}

/** any ancestors of given node already hashed? */
SCIP_Bool graph_pseudoAncestors_nodeIsHashed(
   const PSEUDOANS*      pseudoancestors,    /**< pseudo-ancestors */
   int                   node,               /**< node for which to check */
   const int*            hasharr             /**< hash array */
)
{
   assert(pseudoancestors && hasharr);

   return blockedAncestors_hashIsHitBlock(pseudoancestors->ans_nodes, node, hasharr);
}


/** initializes edge ancestors */
SCIP_RETCODE graph_initAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
)
{
   const int nedges = graph_get_nEdges(g);
   IDX** ancestors = g->ancestors;

   assert(ancestors);

   if( graph_typeIsUndirected(g) )
   {
      for( int e = 0; e < nedges; e += 2 )
      {
         SCIP_CALL( SCIPallocBlockMemory(scip, &(ancestors[e])) ); /*lint !e866*/
         ancestors[e + 1] = NULL;
         ancestors[e]->index = e;
         ancestors[e]->parent = NULL;
      }
   }
   else
   {
   for( int e = 0; e < nedges; e++ )
   {
      SCIP_CALL( SCIPallocBlockMemory(scip, &(ancestors[e])) ); /*lint !e866*/
      ancestors[e]->index = e;
      ancestors[e]->parent = NULL;
   }
   }

   return SCIP_OKAY;
}


/** any ancestors of given edges in conflict? */
SCIP_Bool graph_pseudoAncestors_edgesInConflict(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< the graph */
   const int*            edges,              /**< edges to check */
   int                   nedges              /**< number of edges to check */
)
{
   const PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   int* hasharr;
   int i;
   SCIP_Bool conflict = FALSE;

   assert(g && edges);

   SCIP_CALL_ABORT( SCIPallocCleanBufferArray(scip, &hasharr, graph_pseudoAncestorsGetHashArraySize(g->pseudoancestors)) );

    for( i = 0; i < nedges; i++ )
    {
       graph_pseudoAncestors_hashEdgeDirty(pseudoancestors, edges[i], TRUE, &conflict, hasharr);

       if( conflict )
          break;
    }

    for( int j = 0; j < i; j++ )
       graph_pseudoAncestors_unhashEdge(pseudoancestors, edges[j], hasharr);

#ifndef NDEBUG
    for( int j = 0; j < g->knots; j++ )
       assert(hasharr[0] == 0);
#endif

    SCIPfreeCleanBufferArray(scip, &hasharr);

    return conflict;
}


/** initializes pseudo ancestors */
SCIP_RETCODE graph_initPseudoAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
)
{
   assert(scip && g);
   assert(g->pseudoancestors == NULL);

   SCIP_CALL( graph_initPseudoAncestorsSized(scip, g->edges, g) );

   return SCIP_OKAY;
}


/** initializes pseudo ancestors with given size */
SCIP_RETCODE graph_initPseudoAncestorsSized(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nedges,             /**< number of edges to use */
   GRAPH*                g                   /**< the graph */
)
{
   PSEUDOANS* pseudoancestors;

   assert(scip && g);
   assert(g->pseudoancestors == NULL);
   assert(nedges > 0);
   assert(nedges % 2 == 0);

   SCIP_CALL( SCIPallocMemory(scip, &pseudoancestors) );

   g->pseudoancestors = pseudoancestors;

   pseudoancestors->nnodes = g->knots;
   pseudoancestors->nAllPseudoancestors = 0;
   pseudoancestors->halfnedges = nedges / 2;

   SCIP_CALL( blockedAncestors_init(pseudoancestors->halfnedges, &(pseudoancestors->ans_halfedges)) );

   if( graph_pc_isPcMw(g) )
   {
      SCIP_CALL( blockedAncestors_init(g->knots, &(pseudoancestors->ans_nodes)) );
   }
   else
   {
      pseudoancestors->ans_nodes = NULL;
   }

   return SCIP_OKAY;
}


/** frees pseudo ancestors */
void graph_freePseudoAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
   )
{
   PSEUDOANS* pseudoancestors;

   assert(scip && g && g->pseudoancestors);
   assert(g->pseudoancestors->nAllPseudoancestors >= 0);

   pseudoancestors = g->pseudoancestors;

   blockedAncestors_free(scip, &(pseudoancestors->ans_halfedges));

   if( pseudoancestors->ans_nodes )
      blockedAncestors_free(scip, &(pseudoancestors->ans_nodes));

   assert(pseudoancestors->ans_halfedges == NULL);
   assert(pseudoancestors->ans_nodes == NULL);

   SCIPfreeMemoryArray(scip, &(g->pseudoancestors));

   assert(g->pseudoancestors == NULL);
}


/** frees pseudo ancestor block for given edge */
void graph_edge_delPseudoAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_free,          /**< edge for which to free pseudo ancestors */
   GRAPH*                g                   /**< the graph */
)
{
   const int block_id = edge_free / 2;

   assert(scip && g && g->pseudoancestors && g->pseudoancestors->ans_halfedges);
   assert(block_id >= 0 && block_id < g->pseudoancestors->halfnedges);

   blockedAncestors_freeBlock(scip, block_id, g->pseudoancestors->ans_halfedges);
}


/** frees pseudo ancestor block for given node */
void graph_knot_delPseudoAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node_free,          /**< node for which to free pseudo ancestors */
   GRAPH*                g                   /**< the graph */
)
{
   assert(scip && g && g->pseudoancestors && g->pseudoancestors->ans_nodes);
   assert(graph_pc_isPcMw(g));
   assert(node_free >= 0 && node_free < g->pseudoancestors->nnodes);

   blockedAncestors_freeBlock(scip, node_free, g->pseudoancestors->ans_nodes);
}


/** returns number of pseudo ancestors for given edge */
int graph_edge_nPseudoAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   edge          /**< edge for which to return number of pseudo ancestors */
   )
{
   const int halfedge = edge / 2;

   assert(g && g->pseudoancestors && g->pseudoancestors->ans_halfedges);
   assert(halfedge >= 0 && halfedge < g->pseudoancestors->halfnedges);

   return g->pseudoancestors->ans_halfedges->sizes[halfedge];
}


/** returns number of pseudo ancestors for given node */
int graph_knot_nPseudoAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   node          /**< node for which to return number of pseudo ancestors */
   )
{
   assert(g && g->pseudoancestors && g->pseudoancestors->ans_nodes);
   assert(node >= 0 && node < g->pseudoancestors->nnodes);

   return g->pseudoancestors->ans_nodes->sizes[node];
}


/** returns pseudo ancestors for given edge */
const int* graph_edge_getPseudoAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   edge          /**< edge for which to return pseudo ancestors */
   )
{
   const int halfedge = edge / 2;

   assert(g && g->pseudoancestors && g->pseudoancestors->ans_halfedges);
   assert(halfedge >= 0 && halfedge < g->pseudoancestors->halfnedges);

   return g->pseudoancestors->ans_halfedges->blocks[halfedge];
}


/** returns pseudo ancestors for given edge */
IDX* graph_edge_getAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   edge          /**< edge for which to return ancestors */
   )
{
   int e = edge;

   assert(g);
   assert(graph_edge_isInRange(g, edge));
   assert(!graph_typeIsUndirected(g) || (g->ancestors[edge] == NULL) == (edge % 2 == 1));

   if( !g->ancestors[edge] )
   {
      assert(graph_typeIsUndirected(g));
      e--;
      assert(e == flipedge(edge));
   }
   assert(g->ancestors[e]);

   return g->ancestors[e];
}


/** prints pseudo ancestors for given node */
void graph_knot_printPseudoAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   node          /**< node for which to return pseudo ancestors */
   )
{
   int* ancestors;
   int nancestors;

   assert(g && g->pseudoancestors && g->pseudoancestors->ans_nodes);
   assert(node >= 0 && node < g->pseudoancestors->nnodes);

   ancestors = g->pseudoancestors->ans_nodes->blocks[node];
   nancestors = g->pseudoancestors->ans_nodes->sizes[node];

   printf("node %d \n", node);

   for( int i = 0; i < nancestors; i++ )
      printf("...ancestor: %d \n", ancestors[i]);
}


/** prints pseudo ancestors for given edge */
void graph_edge_printPseudoAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   edge          /**< edge for which to return pseudo ancestors */
   )
{
   const int halfedge = edge / 2;
   int* ancestors;
   int nancestors;

   assert(g && g->pseudoancestors && g->pseudoancestors->ans_halfedges);
   assert(halfedge >= 0 && halfedge < g->pseudoancestors->halfnedges);

   ancestors = g->pseudoancestors->ans_halfedges->blocks[halfedge];
   nancestors = g->pseudoancestors->ans_halfedges->sizes[halfedge];

   for( int i = 0; i < nancestors; i++ )
      printf("...ancestor: %d \n", ancestors[i]);
}


/** returns pseudo ancestors for given node */
const int* graph_knot_getPseudoAncestors(
   const GRAPH*          g,            /**< the graph */
   int                   node          /**< node for which to return pseudo ancestors */
   )
{
   assert(g && g->pseudoancestors && g->pseudoancestors->ans_nodes);
   assert(node >= 0 && node < g->pseudoancestors->nnodes);

   return g->pseudoancestors->ans_nodes->blocks[node];
}


/** returns number of pseudo-ancestors */
int graph_getNpseudoAncestors(
   const GRAPH*          g                /**< the graph */
   )
{
   assert(g && g->pseudoancestors);

   return g->pseudoancestors->nAllPseudoancestors;
}


/** adds new pseudo ancestor and provides index */
void graph_addPseudoAncestor(
   GRAPH*                g,                /**< the graph (in/out)  */
   int*                  pseudoancestor    /**< gives the new pseudo ancestor index (out) */
   )
{
   assert(pseudoancestor && g && g->pseudoancestors);
   assert(g->pseudoancestors->nAllPseudoancestors >= 0);
   g->pseudoancestors->nAllPseudoancestors++;

   *pseudoancestor = (g->pseudoancestors->nAllPseudoancestors - 1);
}


/** adds n new pseudo ancestor */
void graph_addPseudoAncestors(
   int                   nadd,             /**< number of ancestors to add */
   GRAPH*                g                 /**< the graph (in/out)  */
   )
{
   assert(g && g->pseudoancestors);
   assert(nadd > 0);

   assert(g->pseudoancestors->nAllPseudoancestors >= 0);
   g->pseudoancestors->nAllPseudoancestors += nadd;
}


/** returns minimum required number for hash array */
int graph_pseudoAncestorsGetHashArraySize(
   const PSEUDOANS*      pseudoancestors     /**< pseudo-ancestors */
   )
{
   assert(pseudoancestors);
   assert(pseudoancestors->nAllPseudoancestors >= 0);
   assert(getNextPow2(pseudoancestors->nAllPseudoancestors + 1) > pseudoancestors->nAllPseudoancestors);

   return getNextPow2(pseudoancestors->nAllPseudoancestors + 1);
}


/** appends copy of pseudo ancestors of edge_source to edge_target */
SCIP_RETCODE graph_pseudoAncestors_appendCopyEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_target,        /**< edge target */
   int                   edge_source,        /**< edge source */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);
   const int target = edge_target / 2;
   const int source = edge_source / 2;

   assert(scip && g && pseudoancestors && conflict);

   SCIP_CALL( blockedAncestors_appendCopy(scip, target, pseudoancestors->ans_halfedges, source, revertIfConflict,
         hasharr_size, pseudoancestors->ans_halfedges, conflict) );

   return SCIP_OKAY;
}


/** appends copy of pseudo ancestors of node source to node target */
SCIP_RETCODE graph_pseudoAncestors_appendCopyNode(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node_target,        /**< node target */
   int                   node_source,        /**< node source */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);

   assert(scip && g && pseudoancestors && conflict);

   SCIP_CALL( blockedAncestors_appendCopy(scip, node_target,  pseudoancestors->ans_nodes, node_source, revertIfConflict,
         hasharr_size, pseudoancestors->ans_nodes, conflict) );

   return SCIP_OKAY;
}


/** appends copy of pseudo ancestors of node source to edge target */
SCIP_RETCODE graph_pseudoAncestors_appendCopyNodeToEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_target,        /**< edge target */
   int                   node_source,        /**< node source */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);
   const int target = edge_target / 2;

   assert(scip && g && pseudoancestors && conflict);

   SCIP_CALL( blockedAncestors_appendCopy(scip, target,  pseudoancestors->ans_nodes, node_source, revertIfConflict,
         hasharr_size, pseudoancestors->ans_halfedges, conflict) );

   return SCIP_OKAY;
}


/** appends copy of pseudo ancestors of edge source to node target */
SCIP_RETCODE graph_pseudoAncestors_appendCopyEdgeToNode(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node_target,        /**< node target */
   int                   edge_source,        /**< edge source */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);
   const int source = edge_source / 2;

   assert(scip && g && pseudoancestors && conflict);

   SCIP_CALL( blockedAncestors_appendCopy(scip, node_target,  pseudoancestors->ans_halfedges, source, revertIfConflict,
         hasharr_size, pseudoancestors->ans_nodes, conflict) );

   return SCIP_OKAY;
}


/** appends given of pseudo ancestors to edge target */
SCIP_RETCODE graph_pseudoAncestors_appendCopySingToEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_target,        /**< edge target */
   const SINGLETONANS*   source,             /**< source */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   assert(scip && g && conflict && source);

   *conflict = FALSE;

   if( source->npseudoancestors > 0 )
   {
      const PSEUDOANS* const pseudoancestors = g->pseudoancestors;
      const int* const ancestors = source->pseudoancestors;
      const int target = edge_target / 2;
      const int nancestors = source->npseudoancestors;
      const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);

      assert(pseudoancestors);

      SCIP_CALL( blockedAncestors_appendArray(scip, target, ancestors, nancestors, revertIfConflict, hasharr_size, pseudoancestors->ans_halfedges, conflict) );
   }

   return SCIP_OKAY;
}



/** appends copy of pseudo ancestors in array form to edge target */
SCIP_RETCODE graph_pseudoAncestors_appendCopyArrayToEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_target,        /**< edge target */
   const int*            ancestors,          /**< pseudo ancestors array */
   int                   nancestors,         /**< number of ancestors */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   assert(scip && g && conflict);
   assert(nancestors >= 0);

   *conflict = FALSE;

   if( nancestors > 0 )
   {
      const PSEUDOANS* const pseudoancestors = g->pseudoancestors;
      const int target = edge_target / 2;
      const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);
      const SCIP_Bool revertIfConflict = FALSE;

      assert(ancestors);
      assert(pseudoancestors);

      SCIP_CALL( blockedAncestors_appendArray(scip, target, ancestors, nancestors, revertIfConflict, hasharr_size, pseudoancestors->ans_halfedges, conflict) );
   }

   return SCIP_OKAY;
}

/** appends pseudo ancestors of edge_source to edge_target, ancestors for edge_source are deleted */
SCIP_RETCODE graph_pseudoAncestors_appendMoveEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_target,        /**< target edge */
   int                   edge_source,        /**< source edge  */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   SCIP_CALL( graph_pseudoAncestors_appendCopyEdge(scip, edge_target, edge_source, revertIfConflict, g, conflict) );
   graph_edge_delPseudoAncestors(scip, edge_source, g);

   return SCIP_OKAY;
}


/** appends pseudo ancestors of node source to node target, ancestors for source are deleted */
SCIP_RETCODE graph_pseudoAncestors_appendMoveNode(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node_target,        /**< target node */
   int                   node_source,        /**< source node */
   SCIP_Bool             revertIfConflict,   /**< break on conflict */
   GRAPH*                g,                  /**< the graph */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   SCIP_CALL( graph_pseudoAncestors_appendCopyNode(scip, node_target, node_source, revertIfConflict, g, conflict) );
   graph_knot_delPseudoAncestors(scip, node_source, g);

   return SCIP_OKAY;
}


/** adds pseudo ancestor to ancestor list of given edge */
SCIP_RETCODE graph_pseudoAncestors_addToEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge_target,        /**< edge for which to add pseudo ancestor */
   int                   ancestor,           /**< (index of) pseudo ancestor */
   GRAPH*                g                   /**< the graph */
)
{
   PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);
   const int target = edge_target / 2;

   assert(scip && g && pseudoancestors);
   assert(target >= 0 && target < g->pseudoancestors->halfnedges);
   assert(ancestor >= 0 && ancestor < g->pseudoancestors->nAllPseudoancestors);

   SCIP_CALL( blockedAncestors_addAncestor(scip, target, ancestor, hasharr_size, pseudoancestors->ans_halfedges) );

   return SCIP_OKAY;
}


/** adds pseudo ancestor to ancestor list of given node */
SCIP_RETCODE graph_pseudoAncestors_addToNode(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node_target,        /**< node for which to add pseudo ancestor */
   int                   ancestor,           /**< (index of) pseudo ancestor */
   GRAPH*                g                   /**< the graph */
)
{
   PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);

   assert(scip && g && pseudoancestors);
   assert(graph_pc_isPcMw(g));
   assert(pseudoancestors->ans_nodes);
   assert(node_target >= 0 && node_target < g->pseudoancestors->nnodes);
   assert(ancestor >= 0 && ancestor < g->pseudoancestors->nAllPseudoancestors);
   assert(!graph_pc_knotIsDummyTerm(g, node_target));

   SCIP_CALL( blockedAncestors_addAncestor(scip, node_target, ancestor, hasharr_size, pseudoancestors->ans_nodes) );

   return SCIP_OKAY;
}


/** valid pseudo-ancestors (no conflicts)? */
SCIP_Bool graph_valid_pseudoAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g                   /**< the graph */
)
{
   const PSEUDOANS* const pseudoancestors = g->pseudoancestors;
   const int hasharr_size = graph_pseudoAncestorsGetHashArraySize(pseudoancestors);
   SCIP_Bool isValid;

   assert(scip && g && pseudoancestors);

   isValid = blockedAncestors_isValid(scip, hasharr_size, pseudoancestors->ans_halfedges);

   if( !isValid )
      return FALSE;

   if( graph_pc_isPcMw(g) )
   {
      isValid = blockedAncestors_isValid(scip, hasharr_size, pseudoancestors->ans_nodes);

      if( !isValid )
         return FALSE;
   }

   return TRUE;
}

#if 0
/** check whether conflict for one edge */
SCIP_RETCODE graph_checkConflict1_pseudoAncestors(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g,                  /**< the graph */
   int                   edge1,              /**< first edge */
   SCIP_Bool*            conflict            /**< conflict? */
)
{
   const int block_id1 = edge1 / 2;


   assert(scip && g && g->pseudoancestors);
   assert(block_id1 >= 0 && block_id1 < g->pseudoancestors->halfnedges);


   return SCIP_OKAY;
}
#endif


/** initializes fixed */
SCIP_RETCODE graph_init_fixed(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
)
{
   FIXED* fixedcomponents;

   assert(scip && g);
   assert(g->fixedcomponents == NULL);

   SCIP_CALL( SCIPallocMemory(scip, &(g->fixedcomponents)) );

   fixedcomponents = g->fixedcomponents;

   fixedcomponents->fixedges = NULL;
   fixedcomponents->fixpseudonodes = NULL;
   fixedcomponents->nfixnodes = 0;
   fixedcomponents->movedFrom = FALSE;

   return SCIP_OKAY;
}


/** frees fixed */
void graph_free_fixed(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
   )
{
   IDX* curr;
   FIXED* fixedcomponents = g->fixedcomponents;

   assert(scip && g);
   assert(fixedcomponents != NULL);
   assert(fixedcomponents->nfixnodes >= 0);

   if( fixedcomponents->nfixnodes > 0 )
      SCIPfreeBlockMemoryArray(scip, &(fixedcomponents->fixpseudonodes), fixedcomponents->nfixnodes);

   curr = fixedcomponents->fixedges;
   while( curr != NULL )
   {
      fixedcomponents->fixedges = curr->parent;
      SCIPfreeBlockMemory(scip, &(curr));

      curr = fixedcomponents->fixedges;
   }

   SCIPfreeMemory(scip, &(g->fixedcomponents));
}


/** frees fixed edges */
void graph_free_fixedEdgesOnly(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                g                   /**< the graph */
   )
{
   IDX* curr;
   FIXED* fixedcomponents = g->fixedcomponents;

   assert(scip && g);
   assert(fixedcomponents != NULL);
   assert(fixedcomponents->nfixnodes >= 0);

   curr = fixedcomponents->fixedges;
   while( curr != NULL )
   {
      fixedcomponents->fixedges = curr->parent;
      SCIPfreeBlockMemory(scip, &(curr));

      curr = fixedcomponents->fixedges;

      assert(0 && "todo should not happen");

   }
}


/** adds new fixed components */
SCIP_RETCODE graph_fixed_add(
   SCIP*                 scip,               /**< SCIP data structure */
   IDX*                  edges,              /**< edge to add or NULL */
   const int*            pseudonodes,        /**< nodes to add */
   int                   npseudonodes,       /**< number of nodes to add  */
   GRAPH*                g                   /**< the graph */
)
{
   FIXED* fixedcomponents;

   assert(scip && g);
   assert(npseudonodes >= 0);

   if( g->fixedcomponents == NULL )
      SCIP_CALL( graph_init_fixed(scip, g) );

   fixedcomponents = g->fixedcomponents;
   assert(!fixedcomponents->movedFrom);

   if( edges )
   {
      SCIP_Bool conflict = FALSE;

      SCIP_CALL( SCIPintListNodeAppendCopy(scip, &(fixedcomponents->fixedges), edges, &conflict) );
      assert(!conflict);
   }

   if( npseudonodes > 0 )
   {
      int nfixnnodes = fixedcomponents->nfixnodes;
      const int nfixnnodes_new = nfixnnodes + npseudonodes;

      assert(pseudonodes);

      if( nfixnnodes == 0 )
      {
         assert(!fixedcomponents->fixpseudonodes);
         assert(nfixnnodes_new == npseudonodes);

         SCIP_CALL( SCIPallocBlockMemoryArray(scip, &(fixedcomponents->fixpseudonodes), nfixnnodes_new) );
      }
      else
      {
         assert(fixedcomponents->fixpseudonodes);

         SCIP_CALL( SCIPreallocBlockMemoryArray(scip, &(fixedcomponents->fixpseudonodes), nfixnnodes, nfixnnodes_new) );
      }

      for( int i = 0; i < npseudonodes; i++ )
         fixedcomponents->fixpseudonodes[nfixnnodes++] = pseudonodes[i];

      assert(nfixnnodes == nfixnnodes_new);
      fixedcomponents->nfixnodes = nfixnnodes_new;

      assert(g->withInexactReductions || fixedPseudoAncestorsAreValid(scip, g));
   }

   return SCIP_OKAY;
}


/** adds ancestors from given edges */
SCIP_RETCODE graph_fixed_addEdge(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   edge,               /**< edge */
   GRAPH*                g                   /**< the graph */
)
{
   assert(scip && g);
   assert(edge >= 0 && edge < g->edges);
   assert(g->ancestors);

   SCIP_CALL( graph_fixed_add(scip, graph_edge_getAncestors(g, edge), graph_edge_getPseudoAncestors(g, edge),
         graph_edge_nPseudoAncestors(g, edge), g) );

   return SCIP_OKAY;
}


/** adds ancestors from given edges */
SCIP_RETCODE graph_fixed_addNodePc(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node,               /**< node */
   GRAPH*                g                   /**< the graph */
)
{
   assert(scip && g);
   assert(node >= 0 && node < g->knots);
   assert(graph_pc_isPcMw(g));
   assert(!graph_pc_knotIsDummyTerm(g, node)); /* todo really? */
   assert(g->ancestors && g->pcancestors);

   SCIP_CALL( graph_fixed_add(scip, g->pcancestors[node], graph_knot_getPseudoAncestors(g, node),
         graph_knot_nPseudoAncestors(g, node), g) );

   return SCIP_OKAY;
}


/** moves ancestors from given edges */
SCIP_RETCODE graph_fixed_moveNodePc(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   node,               /**< node */
   GRAPH*                g                   /**< the graph */
)
{
   assert(scip && g);
   assert(node >= 0 && node < g->knots);
   assert(graph_pc_isPcMw(g));
   assert(g->pcancestors);

   SCIP_CALL( graph_fixed_addNodePc(scip, node, g) );

   if( g->pcancestors[node] )
      SCIPintListNodeFree(scip, &(g->pcancestors[node]));

   graph_knot_delPseudoAncestors(scip, node, g);

   return SCIP_OKAY;
}


/** resets */
void graph_fixed_resetMoved(
   GRAPH*                g                   /**< the graph */
)
{
   assert(g);

   if( g->fixedcomponents != NULL )
   {
      assert(g->fixedcomponents->movedFrom);
      g->fixedcomponents->movedFrom = FALSE;
   }
}


/** initializes */
SCIP_RETCODE graph_initContractTracing(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPH*                graph               /**< graph */
   )
{
   int* trace;
   const int nnodes = graph_get_nNodes(graph);

   assert(scip);
   assert(!graph->contracttrace);

   SCIP_CALL( SCIPallocMemoryArray(scip, &(trace), nnodes) );

   for( int i = 0; i < nnodes; i++ )
      trace[i] = -1;

   graph->contracttrace = trace;

   return SCIP_OKAY;
}


/** has the node a contract trace? */
SCIP_Bool graph_knot_hasContractTrace(
   int                   node,               /**< node to trace back from */
   const GRAPH*          graph               /**< graph */
   )
{
   assert(graph && graph->contracttrace);
   assert(graph_knot_isInRange(graph, node));

   return (graph->contracttrace[node] != -1);
}


/** traces contraction back; returns traced node */
int graph_contractTrace(
   int                   node,               /**< node to trace back from */
   const GRAPH*          graph               /**< graph */
   )
{
   const int* const trace = graph->contracttrace;
   int newnode;

   assert(graph && trace);
   assert(graph_knot_isInRange(graph, node));
   assert(graph->grad[node] == 0);
   assert(trace[node] != -1);

   for( newnode = node; trace[newnode] != -1; newnode = trace[newnode]  )
   {
      assert(graph_knot_isInRange(graph, newnode));
   }
   assert(graph_knot_isInRange(graph, newnode));

   return newnode;
}

/** copies and potentially moves fixed components */
SCIP_RETCODE graph_copyFixed(
   SCIP*                 scip,                /**< SCIP data structure */
   GRAPH*                g,                   /**< the original graph */
   SCIP_Bool             moveFixedEdges,      /**< move fixed edges? If false, nothing is done! */
   GRAPH*                g_copy               /**< the copied graph */
)
{
   FIXED* fixed_copy;
   FIXED* fixed_org;

   assert(scip && g && g_copy);
   assert(g_copy->fixedcomponents == NULL);

   if( g->fixedcomponents == NULL )
   {
      return SCIP_OKAY;
   }

   SCIP_CALL( graph_init_fixed(scip, g_copy) );
   fixed_copy = g_copy->fixedcomponents;
   fixed_org = g->fixedcomponents;

   assert(fixed_copy && fixed_org);

   if( moveFixedEdges  )
   {
      assert(!fixed_org->movedFrom);

      fixed_copy->fixedges = fixed_org->fixedges;
      fixed_org->fixedges = NULL;
      fixed_org->movedFrom = TRUE;
   }

   fixed_copy->nfixnodes = fixed_org->nfixnodes;
   if( fixed_copy->nfixnodes > 0 )
   {
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &(fixed_copy->fixpseudonodes), fixed_copy->nfixnodes) );
      BMScopyMemoryArray(fixed_copy->fixpseudonodes, fixed_org->fixpseudonodes, fixed_copy->nfixnodes);
   }
   else
   {
      fixed_copy->fixpseudonodes = NULL;
   }

   return SCIP_OKAY;
}


/** gets fixed edges */
IDX* graph_getFixedges(
   const GRAPH*          g                   /**< the graph */
)
{
   assert(g);

   if( !g->fixedcomponents )
      return NULL;

   return g->fixedcomponents->fixedges;
}

/** gets fixed pseudo eliminated nodes */
const int* graph_getFixpseudonodes(
   SCIP*                 scip,               /**< SCIP data structure */
   const GRAPH*          g                   /**< the graph */
)
{
   assert(g && scip);

   if( !g->fixedcomponents )
      return NULL;

   return g->fixedcomponents->fixpseudonodes;
}

/** gets number of fixed pseudo eliminated nodes */
int graph_getNfixpseudonodes(
   const GRAPH*          g                   /**< the graph */
)
{
   assert(g);

   if( !g->fixedcomponents )
      return 0;

   return g->fixedcomponents->nfixnodes;
}
