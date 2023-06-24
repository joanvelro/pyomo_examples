/*
  Copyright (c) 2003-2021 Tommi Junttila
  Released under the GNU Lesser General Public License version 3.

  This file is part of bliss.

  bliss is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, version 3 of the License.

  bliss is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with bliss.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <new>
#include <set>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <cstdio>
#include <cassert>
#include <climits>

#include "bliss/defs.hh"
#include "bliss/timer.hh"
#include "bliss/graph.hh"
#include "bliss/partition.hh"
#include "bliss/utils.hh"

namespace bliss {


/*-------------------------------------------------------------------------
 *
 * Routines for undirected graphs
 *
 *-------------------------------------------------------------------------*/

Graph::Vertex::Vertex()
{
  color = 0;
}


Graph::Vertex::~Vertex()
{
  ;
}


void
Graph::Vertex::add_edge(const unsigned int other_vertex)
{
  edges.push_back(other_vertex);
}


void
Graph::Vertex::remove_duplicate_edges(std::vector<bool>& tmp)
{
  remove_duplicates(edges, tmp);
}


/**
 * Sort the edges leaving the vertex according to
 * the vertex number of the other edge end.
 * Time complexity: O(e log(e)), where e is the number of edges
 * leaving the vertex.
 */
void
Graph::Vertex::sort_edges()
{
  std::sort(edges.begin(), edges.end());
}



/*-------------------------------------------------------------------------
 *
 * Constructor and destructor for undirected graphs
 *
 *-------------------------------------------------------------------------*/


Graph::Graph(const unsigned int nof_vertices)
{
  vertices.resize(nof_vertices);
  sh = shs_flm;
}


Graph::~Graph()
{
  ;
}


unsigned int
Graph::add_vertex(const unsigned int color)
{
  const unsigned int vertex_num = vertices.size();
  vertices.resize(vertex_num + 1);
  vertices.back().color = color;
  return vertex_num;
}


void
Graph::add_edge(const unsigned int vertex1, const unsigned int vertex2)
{
  //fprintf(stderr, "(%u,%u) ", vertex1, vertex2);
  if(vertex1 >= get_nof_vertices() or vertex2 >= get_nof_vertices())
    throw std::out_of_range("out of bounds vertex number");
  vertices[vertex1].add_edge(vertex2);
  vertices[vertex2].add_edge(vertex1);
}


unsigned int
Graph::get_color(const unsigned int vertex) const
{
  return vertices[vertex].color;
}

void
Graph::change_color(const unsigned int vertex, const unsigned int color)
{
  if(vertex >= get_nof_vertices())
    throw std::out_of_range("out of bounds vertex number");
  vertices[vertex].color = color;
}


Graph*
Graph::copy() const {
  const unsigned int n = get_nof_vertices();
  Graph* result = new Graph(n);
  // Color the vertices in the copy
  for(unsigned int vi = 0; vi < n; vi++)
    result->change_color(vi, get_color(vi));
  // Add the edges in the copy
  for(unsigned int vi = 0; vi < n; vi++) {
    for(unsigned int neighbour: vertices[vi].edges)
      if(neighbour >= vi)
        result->add_edge(vi, neighbour);
  }
  return result;
}


/*-------------------------------------------------------------------------
 *
 * Read graph in the DIMACS format.
 * Returns 0 if an error occurred.
 *
 *-------------------------------------------------------------------------*/

Graph*
Graph::read_dimacs(FILE* const fp, FILE* const errstr)
{
  Graph *g = 0;
  unsigned int nof_vertices;
  unsigned int nof_edges;
  unsigned int line_num = 1;
  int c;

  const bool verbose = false;
  FILE* verbstr;
  if(verbose)
      verbstr = stdout;

  /* Read comments and the problem definition line */
  while(1)
    {
      c = getc(fp);
      if(c == 'c')
        {
          /* A comment, ignore the rest of the line */
          while((c = getc(fp)) != '\n')
            {
              if(c == EOF)
                {
                  if(errstr)
                    fprintf(errstr,
                        "error in line %u: not in DIMACS format\n",
                        line_num);
                  goto error_exit;
                }
            }
          line_num++;
          continue;
        }
      if(c == 'p')
        {
          /* The problem definition line */
          if(fscanf(fp, " edge %u %u\n", &nof_vertices, &nof_edges) != 2)
            {
              if(errstr)
                fprintf(errstr, "error in line %u: not in DIMACS format\n",
                    line_num);
              goto error_exit;
            }
          line_num++;
          break;
        }
      if(errstr)
        fprintf(errstr, "error in line %u: not in DIMACS format\n", line_num);
      goto error_exit;
    }

  if(nof_vertices <= 0)
    {
      if(errstr)
        fprintf(errstr, "error: no vertices\n");
      goto error_exit;
    }
  if(verbose)
    {
      fprintf(verbstr, "Instance has %d vertices and %d edges\n",
          nof_vertices, nof_edges);
      fflush(verbstr);
    }

  g = new Graph(nof_vertices);

  //
  // Read vertex colors
  //
  if(verbose)
    {
      fprintf(verbstr, "Reading vertex colors...\n");
      fflush(verbstr);
    }
  while(1)
    {
      c = getc(fp);
      if(c != 'n')
        {
          ungetc(c, fp);
          break;
        }
      ungetc(c, fp);
      unsigned int vertex;
      unsigned int color;
      if(fscanf(fp, "n %u %u\n", &vertex, &color) != 2)
        {
          if(errstr)
            fprintf(errstr, "error in line %u: not in DIMACS format\n",
                line_num);
          goto error_exit;
        }
      if(!((vertex >= 1) && (vertex <= nof_vertices)))
        {
          if(errstr)
            fprintf(errstr,
                "error in line %u: vertex %u not in range [1,...,%u]\n",
                line_num, vertex, nof_vertices);
          goto error_exit;
        }
      line_num++;
      g->change_color(vertex - 1, color);
    }
  if(verbose)
    {
      fprintf(verbstr, "Done\n");
      fflush(verbstr);
    }

  //
  // Read edges
  //
  if(verbose)
    {
      fprintf(verbstr, "Reading edges...\n");
      fflush(verbstr);
    }
  for(unsigned i = 0; i < nof_edges; i++)
    {
      unsigned int from, to;
      if(fscanf(fp, "e %u %u\n", &from, &to) != 2)
        {
          if(errstr)
            fprintf(errstr, "error in line %u: not in DIMACS format\n",
                line_num);
          goto error_exit;
        }
      if(!((from >= 1) && (from <= nof_vertices)))
        {
          if(errstr)
            fprintf(errstr,
                "error in line %u: vertex %u not in range [1,...,%u]\n",
                line_num, from, nof_vertices);
          goto error_exit;
        }
      if(!((to >= 1) && (to <= nof_vertices)))
        {
          if(errstr)
            fprintf(errstr,
                "error in line %u: vertex %u not in range [1,...,%u]\n",
                line_num, to, nof_vertices);
          goto error_exit;
        }
      line_num++;
      g->add_edge(from-1, to-1);
    }
  if(verbose)
    {
      fprintf(verbstr, "Done\n");
      fflush(verbstr);
    }

  return g;

 error_exit:
  if(g)
    delete g;
  return 0;

}


void
Graph::write_dimacs(FILE* const fp)
{
  remove_duplicate_edges();
  sort_edges();

  /* First count the total number of edges */
  unsigned int nof_edges = 0;
  for(unsigned int vi = 0; vi < get_nof_vertices(); vi++)
    {
      for(const unsigned int neighbour: vertices[vi].edges)
        if(vi <= neighbour)
          nof_edges++;
    }

  /* Output the "header" line */
  fprintf(fp, "p edge %u %u\n", get_nof_vertices(), nof_edges);

  /* Print the color of each vertex */
  for(unsigned int vi = 0; vi < get_nof_vertices(); vi++)
    {
      Vertex &v = vertices[vi];
      fprintf(fp, "n %u %u\n", vi+1, v.color);
      /*
      if(v.color != 0)
        {
          fprintf(fp, "n %u %u\n", i+1, v.color);
        }
      */
    }

  /* Print the edges */
  for(unsigned int vi = 0; vi < get_nof_vertices(); vi++)
    {
      for(const unsigned int neighbour: vertices[vi].edges)
        if(vi <= neighbour)
          fprintf(fp, "e %u %u\n", vi+1, neighbour+1);
    }
}



void
Graph::sort_edges()
{
  for(Vertex& v: vertices)
    v.sort_edges();
}


int
Graph::cmp(Graph& other)
{
  /* Compare the numbers of vertices */
  if(get_nof_vertices() < other.get_nof_vertices())
    return -1;
  if(get_nof_vertices() > other.get_nof_vertices())
    return 1;
  /* Compare vertex colors */
  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      if(vertices[i].color < other.vertices[i].color)
        return -1;
      if(vertices[i].color > other.vertices[i].color)
        return 1;
    }
  /* Compare vertex degrees */
  remove_duplicate_edges();
  other.remove_duplicate_edges();
  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      if(vertices[i].nof_edges() < other.vertices[i].nof_edges())
        return -1;
      if(vertices[i].nof_edges() > other.vertices[i].nof_edges())
        return 1;
    }
  /* Compare edges */
  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      Vertex &v1 = vertices[i];
      Vertex &v2 = other.vertices[i];
      v1.sort_edges();
      v2.sort_edges();
      std::vector<unsigned int>::const_iterator ei1 = v1.edges.begin();
      std::vector<unsigned int>::const_iterator ei2 = v2.edges.begin();
      while(ei1 != v1.edges.end())
        {
          if(*ei1 < *ei2)
            return -1;
          if(*ei1 > *ei2)
            return 1;
          ei1++;
          ei2++;
        }
    }
  return 0;
}


Graph*
Graph::permute(const std::vector<unsigned int>& perm) const
{
#if defined(BLISS_CONSISTENCY_CHECKS)
#endif

  Graph* const g = new Graph(get_nof_vertices());
  for(size_t i = 0; i < get_nof_vertices(); i++)
    {
      const Vertex& v = vertices[i];
      Vertex& permuted_v = g->vertices[perm[i]];
      permuted_v.color = v.color;
      for(std::vector<unsigned int>::const_iterator ei = v.edges.begin();
          ei != v.edges.end();
          ei++)
        {
          const unsigned int dest_v = *ei;
          permuted_v.add_edge(perm[dest_v]);
        }
      permuted_v.sort_edges();
    }
  return g;
}

Graph*
Graph::permute(const unsigned int* perm) const
{
#if defined(BLISS_CONSISTENCY_CHECKS)
  if(!is_permutation(get_nof_vertices(), perm))
    throw std::logic_error(__FILE__ ":" __LINE__ ": a permutation was expected");
#endif

  Graph* const g = new Graph(get_nof_vertices());
  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      const Vertex& v = vertices[i];
      Vertex& permuted_v = g->vertices[perm[i]];
      permuted_v.color = v.color;
      for(std::vector<unsigned int>::const_iterator ei = v.edges.begin();
          ei != v.edges.end();
          ei++)
        {
          const unsigned int dest_v = *ei;
          permuted_v.add_edge(perm[dest_v]);
        }
      permuted_v.sort_edges();
    }
  return g;
}





/*-------------------------------------------------------------------------
 *
 * Print graph in graphviz format
 *
 *-------------------------------------------------------------------------*/


void
Graph::write_dot(const char* const filename)
{
  FILE *fp = fopen(filename, "w");
  if(fp)
    {
      write_dot(fp);
      fclose(fp);
    }
}

void
Graph::write_dot(FILE* const fp)
{
  remove_duplicate_edges();

  fprintf(fp, "graph g {\n");

  unsigned int vnum = 0;
  for(std::vector<Vertex>::iterator vi = vertices.begin();
      vi != vertices.end();
      vi++, vnum++)
    {
      Vertex& v = *vi;
      fprintf(fp, "v%u [label=\"%u:%u\"];\n", vnum, vnum, v.color);
      for(std::vector<unsigned int>::const_iterator ei = v.edges.begin();
          ei != v.edges.end();
          ei++)
        {
          const unsigned int vnum2 = *ei;
          if(vnum2 > vnum)
            fprintf(fp, "v%u -- v%u\n", vnum, vnum2);
        }
    }

  fprintf(fp, "}\n");
}









/*-------------------------------------------------------------------------
 *
 * Get a hash value for the graph.
 *
 *-------------------------------------------------------------------------*/

unsigned int
Graph::get_hash()
{
  remove_duplicate_edges();
  sort_edges();

  UintSeqHash h;

  h.update(get_nof_vertices());

  /* Hash the color of each vertex */
  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      h.update(vertices[i].color);
    }

  /* Hash the edges */
  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      Vertex &v = vertices[i];
      for(std::vector<unsigned int>::const_iterator ei = v.edges.begin();
          ei != v.edges.end();
          ei++)
        {
          const unsigned int dest_i = *ei;
          if(dest_i < i)
            continue;
          h.update(i);
          h.update(dest_i);
        }
    }

  return h.get_value();
}





void
Graph::remove_duplicate_edges()
{
  std::vector<bool> tmp(vertices.size(), false);

  for(Vertex& v: vertices) {
#if defined(BLISS_EXPENSIVE_CONSISTENCY_CHECKS)
    for(unsigned int i = 0; i < tmp.size(); i++)
      assert(tmp[i] == false);
#endif
    v.remove_duplicate_edges(tmp);
  }
}





/*-------------------------------------------------------------------------
 *
 * Partition independent invariants
 *
 *-------------------------------------------------------------------------*/

/*
 * Return the color of the vertex.
 * Time complexity: O(1)
 */
unsigned int
Graph::vertex_color_invariant(const Graph* const g, const unsigned int v)
{
  return g->vertices[v].color;
}

/*
 * Return the degree of the vertex.
 * Time complexity: O(1)
 */
unsigned int
Graph::degree_invariant(const Graph* const g, const unsigned int v)
{
  return g->vertices[v].nof_edges();
}

/*
 * Return 1 if the vertex v has a self-loop, 0 otherwise
 * Time complexity: O(E_v), where E_v is the number of edges leaving v
 */
unsigned int
Graph::selfloop_invariant(const Graph* const g, const unsigned int v)
{
  const Vertex& vertex = g->vertices[v];
  for(std::vector<unsigned int>::const_iterator ei = vertex.edges.begin();
      ei != vertex.edges.end();
      ei++)
    {
      if(*ei == v)
        return 1;
    }
  return 0;
}



/*-------------------------------------------------------------------------
 *
 * Refine the partition p according to a partition independent invariant
 *
 *-------------------------------------------------------------------------*/

bool
Graph::refine_according_to_invariant(unsigned int (*inv)(const Graph* const g,
      const unsigned int v))
{
  bool refined = false;

  for(Partition::Cell* cell = p.first_nonsingleton_cell; cell; )
    {

      Partition::Cell* const next_cell = cell->next_nonsingleton;

      const unsigned int* ep = p.elements + cell->first;
      for(unsigned int i = cell->length; i > 0; i--, ep++)
        {
          const unsigned int ival = inv(this, *ep);
          p.invariant_values[*ep] = ival;
          if(ival > cell->max_ival)
            {
              cell->max_ival = ival;
              cell->max_ival_count = 1;
            }
          else if(ival == cell->max_ival)
            {
              cell->max_ival_count++;
            }
        }
      Partition::Cell* const last_new_cell = p.zplit_cell(cell, true);
      refined |= (last_new_cell != cell);
      cell = next_cell;
    }

  return refined;
}












/*-------------------------------------------------------------------------
 *
 * Split the neighbourhood of a cell according to the equitable invariant
 *
 *-------------------------------------------------------------------------*/

bool
Graph::split_neighbourhood_of_cell(Partition::Cell* const cell)
{


  const bool was_equal_to_first = refine_equal_to_first;

  if(compute_eqref_hash)
    {
      eqref_hash.update(cell->first);
      eqref_hash.update(cell->length);
    }

  const unsigned int* ep = p.elements + cell->first;
  for(unsigned int i = cell->length; i > 0; i--)
    {
      const Vertex& v = vertices[*ep++];

      for(const unsigned int neighbour: v.edges)
        {
          Partition::Cell * const neighbour_cell = p.get_cell(neighbour);
          if(neighbour_cell->is_unit())
            continue;
          const unsigned int ival = ++p.invariant_values[neighbour];
          if(ival > neighbour_cell->max_ival)
            {
              neighbour_cell->max_ival = ival;
              neighbour_cell->max_ival_count = 1;
              if(ival == 1) {
                neighbour_heap.insert(neighbour_cell->first);
              }
            }
          else if(ival == neighbour_cell->max_ival) {
            neighbour_cell->max_ival_count++;
          }
        }
    }

  while(not neighbour_heap.is_empty())
    {
      const unsigned int start = neighbour_heap.remove();
      Partition::Cell * const neighbour_cell = p.get_cell(p.elements[start]);

      if(compute_eqref_hash)
        {
          eqref_hash.update(neighbour_cell->first);
          eqref_hash.update(neighbour_cell->length);
          eqref_hash.update(neighbour_cell->max_ival);
          eqref_hash.update(neighbour_cell->max_ival_count);
        }


      Partition::Cell* const last_new_cell = p.zplit_cell(neighbour_cell, true);

      /* Update certificate and hash if needed */
      const Partition::Cell* c = neighbour_cell;
      while(true)
        {
          if(in_search)
            {
              /* Build certificate */
              cert_add_redundant(CERT_SPLIT, c->first, c->length);
              /* No need to continue? */
              if(refine_compare_certificate and
                  (refine_equal_to_first == false) and
                  (refine_cmp_to_best < 0))
                goto worse_exit;
            }
          if(compute_eqref_hash)
            {
              eqref_hash.update(c->first);
              eqref_hash.update(c->length);
            }
          if(c == last_new_cell)
            break;
          c = c->next;
        }
    }

  if(refine_compare_certificate and
     (refine_equal_to_first == false) and
     (refine_cmp_to_best < 0))
    return true;

  return false;

 worse_exit:
  /* Clear neighbour heap */
  UintSeqHash rest;
  while(!neighbour_heap.is_empty())
    {
      const unsigned int start = neighbour_heap.remove();
      Partition::Cell * const neighbour_cell = p.get_cell(p.elements[start]);
      if(opt_use_failure_recording and was_equal_to_first)
        {
          rest.update(neighbour_cell->first);
          rest.update(neighbour_cell->length);
          rest.update(neighbour_cell->max_ival);
          rest.update(neighbour_cell->max_ival_count);
        }
      neighbour_cell->max_ival = 0;
      neighbour_cell->max_ival_count = 0;
     p.clear_ivs(neighbour_cell);
    }
  if(opt_use_failure_recording and was_equal_to_first)
    {
      for(unsigned int i = p.splitting_queue.size(); i > 0; i--)
        {
          Partition::Cell* const cell2 = p.splitting_queue.pop_front();
          rest.update(cell2->first);
          rest.update(cell2->length);
          p.splitting_queue.push_back(cell2);
        }
      rest.update(failure_recording_fp_deviation);
      failure_recording_fp_deviation = rest.get_value();
    }

  return true;
}



bool
Graph::split_neighbourhood_of_unit_cell(Partition::Cell* const unit_cell)
{


  const bool was_equal_to_first = refine_equal_to_first;

  if(compute_eqref_hash)
    {
      eqref_hash.update(0x87654321);
      eqref_hash.update(unit_cell->first);
      eqref_hash.update(1);
    }

  const Vertex& v = vertices[p.elements[unit_cell->first]];

  for(const unsigned int neighbour: v.edges)
    {
      Partition::Cell * const neighbour_cell = p.get_cell(neighbour);

      if(neighbour_cell->is_unit()) {
          if(in_search) {
              /* Remember neighbour in order to generate certificate */
              neighbour_heap.insert(neighbour_cell->first);
            }
          continue;
        }
      if(neighbour_cell->max_ival_count == 0)
        {
          neighbour_heap.insert(neighbour_cell->first);
        }
      neighbour_cell->max_ival_count++;

      unsigned int * const swap_position =
        p.elements + neighbour_cell->first + neighbour_cell->length -
        neighbour_cell->max_ival_count;
      *p.in_pos[neighbour] = *swap_position;
      p.in_pos[*swap_position] = p.in_pos[neighbour];
      *swap_position = neighbour;
      p.in_pos[neighbour] = swap_position;
    }

  while(!neighbour_heap.is_empty())
    {
      const unsigned int start = neighbour_heap.remove();
      Partition::Cell* neighbour_cell = p.get_cell(p.elements[start]);

#if defined(BLISS_CONSISTENCY_CHECKS)
      if(neighbour_cell->is_unit()) {
      } else {
      }
#endif

      if(compute_eqref_hash)
        {
          eqref_hash.update(neighbour_cell->first);
          eqref_hash.update(neighbour_cell->length);
          eqref_hash.update(neighbour_cell->max_ival_count);
        }

      if(neighbour_cell->length > 1 and
          neighbour_cell->max_ival_count != neighbour_cell->length)
        {
          Partition::Cell * const new_cell =
            p.aux_split_in_two(neighbour_cell,
                neighbour_cell->length -
                neighbour_cell->max_ival_count);
          unsigned int *ep = p.elements + new_cell->first;
          unsigned int * const lp = p.elements+new_cell->first+new_cell->length;
          while(ep < lp)
            {
              p.element_to_cell_map[*ep] = new_cell;
              ep++;
            }
          neighbour_cell->max_ival_count = 0;


          if(compute_eqref_hash)
            {
              /* Update hash */
              eqref_hash.update(neighbour_cell->first);
              eqref_hash.update(neighbour_cell->length);
              eqref_hash.update(0);
              eqref_hash.update(new_cell->first);
              eqref_hash.update(new_cell->length);
              eqref_hash.update(1);
            }

          /* Add cells in splitting_queue */
          if(neighbour_cell->is_in_splitting_queue()) {
            /* Both cells must be included in splitting_queue in order
               to ensure refinement into equitable partition */
            p.splitting_queue_add(new_cell);
            } else {
              Partition::Cell *min_cell, *max_cell;
              if(neighbour_cell->length <= new_cell->length) {
                  min_cell = neighbour_cell;
                  max_cell = new_cell;
                } else {
                  min_cell = new_cell;
                  max_cell = neighbour_cell;
                }
              /* Put the smaller cell in splitting_queue */
              p.splitting_queue_add(min_cell);
              if(max_cell->is_unit()) {
                  /* Put the "larger" cell also in splitting_queue */
                  p.splitting_queue_add(max_cell);
                }
            }
          /* Update pointer for certificate generation */
          neighbour_cell = new_cell;
        }
      else
      {
        /* neighbour_cell->length == 1 ||
           neighbour_cell->max_ival_count == neighbour_cell->length */
        neighbour_cell->max_ival_count = 0;
      }

      /*
       * Build certificate if required
       */
      if(in_search)
        {
          for(unsigned int i = neighbour_cell->first,
              j = neighbour_cell->length;
              j > 0;
              j--, i++)
            {
              /* Build certificate */
              cert_add(CERT_EDGE, unit_cell->first, i);
              /* No need to continue? */
              if(refine_compare_certificate and
                  (refine_equal_to_first == false) and
                  (refine_cmp_to_best < 0))
                goto worse_exit;
            }
        } /* if(in_search) */
    } /* while(!neighbour_heap.is_empty()) */

  if(refine_compare_certificate and
      (refine_equal_to_first == false) and
      (refine_cmp_to_best < 0))
    return true;

  return false;

 worse_exit:
  /* Clear neighbour heap */
  UintSeqHash rest;
  while(!neighbour_heap.is_empty())
    {
      const unsigned int start = neighbour_heap.remove();
      Partition::Cell * const neighbour_cell = p.get_cell(p.elements[start]);
      if(opt_use_failure_recording and was_equal_to_first)
        {
          rest.update(neighbour_cell->first);
          rest.update(neighbour_cell->length);
          rest.update(neighbour_cell->max_ival_count);
        }
      neighbour_cell->max_ival_count = 0;
    }
  if(opt_use_failure_recording and was_equal_to_first)
    {
      rest.update(failure_recording_fp_deviation);
      failure_recording_fp_deviation = rest.get_value();
    }
  return true;
}









/*-------------------------------------------------------------------------
 *
 * Check whether the current partition p is equitable.
 * Performance: very slow, use only for debugging purposes.
 *
 *-------------------------------------------------------------------------*/

bool Graph::is_equitable() const
{
  const unsigned int N = get_nof_vertices();
  if(N == 0)
    return true;

  std::vector<unsigned int> first_count = std::vector<unsigned int>(N, 0);
  std::vector<unsigned int> other_count = std::vector<unsigned int>(N, 0);

  for(Partition::Cell *cell = p.first_cell; cell; cell = cell->next)
    {
      if(cell->is_unit())
        continue;

      unsigned int *ep = p.elements + cell->first;
      const Vertex &first_vertex = vertices[*ep++];

      /* Count how many edges lead from the first vertex to
       * the neighbouring cells */
      for(std::vector<unsigned int>::const_iterator ei =
          first_vertex.edges.begin();
          ei != first_vertex.edges.end();
          ei++)
        {
          first_count[p.get_cell(*ei)->first]++;
        }

      /* Count and compare to the edges of the other vertices */
      for(unsigned int i = cell->length; i > 1; i--)
        {
          const Vertex &vertex = vertices[*ep++];
          for(std::vector<unsigned int>::const_iterator ei =
              vertex.edges.begin();
              ei != vertex.edges.end();
              ei++)
            {
              other_count[p.get_cell(*ei)->first]++;
            }
          for(Partition::Cell *cell2 = p.first_cell;
              cell2;
              cell2 = cell2->next)
            {
              if(first_count[cell2->first] != other_count[cell2->first])
                {
                  /* Not equitable */
                  return false;
                }
              other_count[cell2->first] = 0;
            }
        }
      /* Reset first_count */
      for(unsigned int i = 0; i < N; i++)
        first_count[i] = 0;
    }
  return true;
}





/*-------------------------------------------------------------------------
 *
 * Build the initial equitable partition
 *
 *-------------------------------------------------------------------------*/

void Graph::make_initial_equitable_partition()
{
  refine_according_to_invariant(&vertex_color_invariant);
  p.splitting_queue_clear();
  //p.print_signature(stderr); fprintf(stderr, "\n");

  refine_according_to_invariant(&selfloop_invariant);
  p.splitting_queue_clear();
  //p.print_signature(stderr); fprintf(stderr, "\n");

  refine_according_to_invariant(&degree_invariant);
  p.splitting_queue_clear();
  //p.print_signature(stderr); fprintf(stderr, "\n");

  refine_to_equitable();
  //p.print_signature(stderr); fprintf(stderr, "\n");


}







/*-------------------------------------------------------------------------
 *
 * Find the next cell to be splitted
 *
 *-------------------------------------------------------------------------*/


Partition::Cell*
Graph::find_next_cell_to_be_splitted(Partition::Cell* cell)
{
  switch(sh) {
  case shs_f:   return sh_first();
  case shs_fs:  return sh_first_smallest();
  case shs_fl:  return sh_first_largest();
  case shs_fm:  return sh_first_max_neighbours();
  case shs_fsm: return sh_first_smallest_max_neighbours();
  case shs_flm: return sh_first_largest_max_neighbours();
  default:
    throw std::logic_error("unknown splitting heuristics");
    return 0;
  }
}

/** \internal
 * A splitting heuristic.
 * Returns the first nonsingleton cell in the current partition.
 */
Partition::Cell*
Graph::sh_first()
{
  Partition::Cell* best_cell = 0;
  for(Partition::Cell* cell = p.first_nonsingleton_cell;
      cell;
      cell = cell->next_nonsingleton)
    {
      if(opt_use_comprec and p.cr_get_level(cell->first) != cr_level)
        continue;
      best_cell = cell;
      break;
    }
  return best_cell;
}

/** \internal
 * A splitting heuristic.
 * Returns the first smallest nonsingleton cell in the current partition.
 */
Partition::Cell*
Graph::sh_first_smallest()
{
  Partition::Cell* best_cell = 0;
  unsigned int best_size = UINT_MAX;
  for(Partition::Cell* cell = p.first_nonsingleton_cell;
      cell;
      cell = cell->next_nonsingleton)
    {
      if(opt_use_comprec and p.cr_get_level(cell->first) != cr_level)
        continue;
      if(cell->length < best_size)
        {
          best_size = cell->length;
          best_cell = cell;
        }
    }
  return best_cell;
}

/** \internal
 * A splitting heuristic.
 * Returns the first largest nonsingleton cell in the current partition.
 */
Partition::Cell*
Graph::sh_first_largest()
{
  Partition::Cell* best_cell = 0;
  unsigned int best_size = 0;
  for(Partition::Cell* cell = p.first_nonsingleton_cell;
      cell;
      cell = cell->next_nonsingleton)
    {
      if(opt_use_comprec and p.cr_get_level(cell->first) != cr_level)
        continue;
      if(cell->length > best_size)
        {
          best_size = cell->length;
          best_cell = cell;
        }
    }
  return best_cell;
}

/** \internal
 * A splitting heuristic.
 * Returns the first nonsingleton cell with max number of neighbouring
 *   nonsingleton cells.
 * Assumes that the partition p is equitable.
 * Assumes that the max_ival fields of the cells are all 0.
 */
Partition::Cell*
Graph::sh_first_max_neighbours()
{
  Partition::Cell* best_cell = 0;
  int best_value = -1;
  _neighbour_cells.clear();
  for(Partition::Cell* cell = p.first_nonsingleton_cell;
      cell;
      cell = cell->next_nonsingleton)
    {
      if(opt_use_comprec and p.cr_get_level(cell->first) != cr_level)
        continue;
      const Vertex& v = vertices[p.elements[cell->first]];
      for(const unsigned int neighbour: v.edges)
        {
          Partition::Cell * const neighbour_cell = p.get_cell(neighbour);
          if(neighbour_cell->is_unit())
            continue;
          neighbour_cell->max_ival++;
          if(neighbour_cell->max_ival == 1)
            _neighbour_cells.push_back(neighbour_cell);
        }
      int value = 0;
      while(not _neighbour_cells.empty())
        {
          Partition::Cell* const neighbour_cell = _neighbour_cells.back();
          _neighbour_cells.pop_back();
          if(neighbour_cell->max_ival != neighbour_cell->length)
            value++;
          neighbour_cell->max_ival = 0;
        }
      if(value > best_value)
        {
          best_value = value;
          best_cell = cell;
        }
    }
  return best_cell;
}

/** \internal
 * A splitting heuristic.
 * Returns the first smallest nonsingleton cell with max number of neighbouring
 * nonsingleton cells.
 * Assumes that the partition p is equitable.
 * Assumes that the max_ival fields of the cells are all 0.
 */
Partition::Cell*
Graph::sh_first_smallest_max_neighbours()
{
  Partition::Cell* best_cell = 0;
  int best_value = -1;
  unsigned int best_size = UINT_MAX;
  _neighbour_cells.clear();
  for(Partition::Cell* cell = p.first_nonsingleton_cell;
      cell;
      cell = cell->next_nonsingleton)
    {

      if(opt_use_comprec and p.cr_get_level(cell->first) != cr_level)
        continue;

      const Vertex& v = vertices[p.elements[cell->first]];
      for(const unsigned int neighbour: v.edges)
        {
          Partition::Cell* const neighbour_cell = p.get_cell(neighbour);
          if(neighbour_cell->is_unit())
            continue;
          neighbour_cell->max_ival++;
          if(neighbour_cell->max_ival == 1)
            _neighbour_cells.push_back(neighbour_cell);
        }
      int value = 0;
      while(not _neighbour_cells.empty())
        {
          Partition::Cell* const neighbour_cell = _neighbour_cells.back();
          _neighbour_cells.pop_back();
          if(neighbour_cell->max_ival != neighbour_cell->length)
            value++;
          neighbour_cell->max_ival = 0;
        }
      if((value > best_value) or
          (value == best_value and cell->length < best_size))
        {
          best_value = value;
          best_size = cell->length;
          best_cell = cell;
        }
    }
  return best_cell;
}

/** \internal
 * A splitting heuristic.
 * Returns the first largest nonsingleton cell with max number of neighbouring
 * nonsingleton cells.
 * Assumes that the partition p is equitable.
 * Assumes that the max_ival fields of the cells are all 0.
 */
  Partition::Cell*
Graph::sh_first_largest_max_neighbours()
{
  Partition::Cell* best_cell = 0;
  int best_value = -1;
  unsigned int best_size = 0;
  _neighbour_cells.clear();
  for(Partition::Cell* cell = p.first_nonsingleton_cell;
      cell;
      cell = cell->next_nonsingleton)
    {

      if(opt_use_comprec and p.cr_get_level(cell->first) != cr_level)
        continue;
      const Vertex& v = vertices[p.elements[cell->first]];
      std::vector<unsigned int>::const_iterator ei = v.edges.begin();
      for(unsigned int j = v.nof_edges(); j > 0; j--)
        {
          Partition::Cell* const neighbour_cell = p.get_cell(*ei++);
          if(neighbour_cell->is_unit())
            continue;
          neighbour_cell->max_ival++;
          if(neighbour_cell->max_ival == 1)
            _neighbour_cells.push_back(neighbour_cell);
        }
      int value = 0;
      while(not _neighbour_cells.empty())
        {
          Partition::Cell* const neighbour_cell = _neighbour_cells.back();
          _neighbour_cells.pop_back();
          if(neighbour_cell->max_ival != neighbour_cell->length)
            value++;
          neighbour_cell->max_ival = 0;
        }
      if((value > best_value) or
          (value == best_value and cell->length > best_size))
        {
          best_value = value;
          best_size = cell->length;
          best_cell = cell;
        }
    }
  return best_cell;
}




















/*-------------------------------------------------------------------------
 *
 * Initialize the certificate size and memory
 *
 *-------------------------------------------------------------------------*/

void
Graph::initialize_certificate()
{
  certificate_index = 0;
  certificate_current_path.clear();
  certificate_first_path.clear();
  certificate_best_path.clear();
}





/*-------------------------------------------------------------------------
 *
 * Check whether perm is an automorphism.
 * Slow, mainly for debugging and validation purposes.
 *
 *-------------------------------------------------------------------------*/

bool
Graph::is_automorphism(unsigned int* const perm) const
{
  std::set<unsigned int, std::less<unsigned int> > edges1;
  std::set<unsigned int, std::less<unsigned int> > edges2;

#if defined(BLISS_CONSISTENCY_CHECKS)
  if(!is_permutation(get_nof_vertices(), perm))
    throw std::logic_error(__FILE__ ":" __LINE__ ": a permutation was expected");
#endif

  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      const Vertex& v1 = vertices[i];
      edges1.clear();
      for(std::vector<unsigned int>::const_iterator ei = v1.edges.cbegin();
          ei != v1.edges.cend();
          ei++)
        edges1.insert(perm[*ei]);

      const Vertex& v2 = vertices[perm[i]];
      edges2.clear();
      for(std::vector<unsigned int>::const_iterator ei = v2.edges.cbegin();
          ei != v2.edges.cend();
          ei++)
        edges2.insert(*ei);

      if(!(edges1 == edges2))
        return false;
    }

  return true;
}




bool
Graph::is_automorphism(const std::vector<unsigned int>& perm) const
{

  if(!(perm.size() == get_nof_vertices() and is_permutation(perm)))
    return false;

  std::set<unsigned int, std::less<unsigned int> > edges1;
  std::set<unsigned int, std::less<unsigned int> > edges2;

  for(unsigned int i = 0; i < get_nof_vertices(); i++)
    {
      const Vertex& v1 = vertices[i];
      edges1.clear();
      for(std::vector<unsigned int>::const_iterator ei = v1.edges.begin();
          ei != v1.edges.end();
          ei++)
        edges1.insert(perm[*ei]);

      const Vertex& v2 = vertices[perm[i]];
      edges2.clear();
      for(std::vector<unsigned int>::const_iterator ei = v2.edges.begin();
          ei != v2.edges.end();
          ei++)
        edges2.insert(*ei);

      if(!(edges1 == edges2))
        return false;
    }

  return true;
}







bool
Graph::nucr_find_first_component(const unsigned int level)
{

  cr_component.clear();
  cr_component_elements = 0;

  /* Find first non-discrete cell in the component level */
  Partition::Cell* first_cell = p.first_nonsingleton_cell;
  while(first_cell)
    {
      if(p.cr_get_level(first_cell->first) == level)
        break;
      first_cell = first_cell->next_nonsingleton;
    }

  /* The component is discrete, return false */
  if(!first_cell)
    return false;

  std::vector<Partition::Cell*> component;
  first_cell->max_ival = 1;
  component.push_back(first_cell);

  for(unsigned int i = 0; i < component.size(); i++)
    {
      Partition::Cell* const cell = component[i];

      const Vertex& v = vertices[p.elements[cell->first]];
      for(unsigned int neighbour: v.edges)
        {
          Partition::Cell* const neighbour_cell = p.get_cell(neighbour);

          // Skip unit neighbours
          if(neighbour_cell->is_unit())
            continue;
          // Already marked to be in the same component?
          if(neighbour_cell->max_ival == 1)
            continue;
          // Is the neighbour at the same component recursion level?
          if(p.cr_get_level(neighbour_cell->first) != level)
            continue;

          if(neighbour_cell->max_ival_count == 0)
            neighbour_heap.insert(neighbour_cell->first);
          neighbour_cell->max_ival_count++;
        }
      while(!neighbour_heap.is_empty())
        {
          const unsigned int start = neighbour_heap.remove();
          Partition::Cell* const neighbour_cell =
            p.get_cell(p.elements[start]);

          /* Skip saturated neighbour cells */
          if(neighbour_cell->max_ival_count == neighbour_cell->length)
            {
              neighbour_cell->max_ival_count = 0;
              continue;
            }
          neighbour_cell->max_ival_count = 0;
          neighbour_cell->max_ival = 1;
          component.push_back(neighbour_cell);
        }
    }

  for(Partition::Cell* const cell: component) {
    cell->max_ival = 0;
    cr_component.push_back(cell->first);
    cr_component_elements += cell->length;
  }
  if(verbstr and verbose_level > 2) {
    fprintf(verbstr, "NU-component with %lu cells and %u vertices\n",
        (long unsigned)cr_component.size(), cr_component_elements);
    fflush(verbstr);
  }

  return true;
}




bool
Graph::nucr_find_first_component(const unsigned int level,
    std::vector<unsigned int>& component,
    unsigned int& component_elements,
    Partition::Cell*& sh_return)
{
  component.clear();
  component_elements = 0;
  sh_return = 0;
  unsigned int sh_first  = 0;
  unsigned int sh_size   = 0;
  unsigned int sh_nuconn = 0;

  /* Find first non-discrete cell in the component level */
  Partition::Cell* first_cell = p.first_nonsingleton_cell;
  while(first_cell)
    {
      if(p.cr_get_level(first_cell->first) == level)
        break;
      first_cell = first_cell->next_nonsingleton;
    }

  if(!first_cell)
    {
      /* The component is discrete, return false */
      return false;
    }

  std::vector<Partition::Cell*> comp;
  _neighbour_cells.clear();

  first_cell->max_ival = 1;
  comp.push_back(first_cell);

  for(unsigned int i = 0; i < comp.size(); i++)
    {
      Partition::Cell* const cell = comp[i];

      const Vertex& v = vertices[p.elements[cell->first]];
      for(unsigned int neighbour: v.edges)
        {
          Partition::Cell* const neighbour_cell = p.get_cell(neighbour);

          // Skip unit neighbours
          if(neighbour_cell->is_unit())
            continue;
          // Is the neighbour at the same component recursion level?
          //if(p.cr_get_level(neighbour_cell->first) != level)
          //  continue;
          if(neighbour_cell->max_ival_count == 0)
            _neighbour_cells.push_back(neighbour_cell);
          neighbour_cell->max_ival_count++;
        }
      unsigned int nuconn = 1;
      while(not _neighbour_cells.empty())
        {
          Partition::Cell* const neighbour_cell = _neighbour_cells.back();
          _neighbour_cells.pop_back();

          /* Skip saturated neighbour cells */
          if(neighbour_cell->max_ival_count == neighbour_cell->length)
            {
              neighbour_cell->max_ival_count = 0;
              continue;
            }
          nuconn++;
          neighbour_cell->max_ival_count = 0;
          if(neighbour_cell->max_ival == 0) {
              comp.push_back(neighbour_cell);
              neighbour_cell->max_ival = 1;
            }
        }

      switch(sh) {
      case shs_f:
        if(sh_return == 0 or
              cell->first <= sh_first) {
            sh_return = cell;
            sh_first = cell->first;
          }
        break;
      case shs_fs:
        if(sh_return == 0 or
              cell->length < sh_size or
              (cell->length == sh_size and cell->first <= sh_first)) {
            sh_return = cell;
            sh_first = cell->first;
            sh_size = cell->length;
          }
        break;
      case shs_fl:
        if(sh_return == 0 or
            cell->length > sh_size or
            (cell->length == sh_size and cell->first <= sh_first)) {
          sh_return = cell;
          sh_first = cell->first;
          sh_size = cell->length;
        }
        break;
      case shs_fm:
        if(sh_return == 0 or
              nuconn > sh_nuconn or
              (nuconn == sh_nuconn and cell->first <= sh_first)) {
            sh_return = cell;
            sh_first = cell->first;
            sh_nuconn = nuconn;
          }
        break;
      case shs_fsm:
        if(sh_return == 0 or
            nuconn > sh_nuconn or
            (nuconn == sh_nuconn and
             (cell->length < sh_size or
              (cell->length == sh_size and cell->first <= sh_first)))) {
            sh_return = cell;
            sh_first = cell->first;
            sh_size = cell->length;
            sh_nuconn = nuconn;
          }
        break;
      case shs_flm:
        if(sh_return == 0 or
            nuconn > sh_nuconn or
            (nuconn == sh_nuconn and
             (cell->length > sh_size or
              (cell->length == sh_size and cell->first <= sh_first)))) {
            sh_return = cell;
            sh_first = cell->first;
            sh_size = cell->length;
            sh_nuconn = nuconn;
          }
        break;
      default:
        throw std::logic_error("unknown splitting heuristics");
        return 0;
      }
    }
  assert(sh_return);

  for(unsigned int i = 0; i < comp.size(); i++)
    {
      Partition::Cell* const cell = comp[i];
      cell->max_ival = 0;
      component.push_back(cell->first);
      component_elements += cell->length;
    }

  if(verbstr and verbose_level > 2) {
    fprintf(verbstr, "NU-component with %lu cells and %u vertices\n",
        (long unsigned)component.size(), component_elements);
    fflush(verbstr);
  }

  return true;
}




} // namespace bliss
