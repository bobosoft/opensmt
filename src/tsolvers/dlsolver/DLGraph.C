/*********************************************************************
Author: Edgar Pek <edgar.pek@lu.unisi.ch>
      , Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

OpenSMT -- Copyright (C) 2008, Roberto Bruttomesso

OpenSMT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenSMT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenSMT. If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
#include "DLGraph.h"
#include "DLSolver.h"

//
// Destructor
//
template< class T > DLGraph<T>::~DLGraph( )
{
  typename Enode2Vertex::iterator i2v;
  for ( i2v = vertexMap.begin(); i2v != vertexMap.end(); ++i2v )
    delete i2v->second;

  typename Enode2Edge::iterator i2e;
  for ( i2e = edgeMap.begin(); i2e != edgeMap.end(); ++i2e ){
    delete (i2e->second).pos; delete (i2e->second).neg;
  }
}

//
// We assume that each atom has been
// rewritten (by DLCanonizer)
// into one of the following forms:
//
// x <= y
// x - y <= c
//
template <class T> DLComplEdges<T> DLGraph<T>::getDLEdge(Enode *e)
{
  if (edgeMap.find(e) == edgeMap.end())
  {
    assert( !e->hasPolarity( ) );
    Enode * lhs = e->get1st( );
    Enode * rhs = e->get2nd( );
    const bool lhs_v_c = lhs->isVar( ) || lhs->isConstant( ) || ( lhs->isUminus( ) && lhs->get1st( )->isConstant( ) );
    const bool rhs_v_c = rhs->isVar( ) || rhs->isConstant( ) || ( rhs->isUminus( ) && rhs->get1st( )->isConstant( ) );

    Enode * x = NULL;
    Enode * y = NULL;

    if ( lhs_v_c && rhs_v_c )
    {
      if ( lhs->isVar( ) && rhs->isVar( ) )
      {
	tmp_edge_weight = 0;
	x = lhs;
	y = rhs;
      }
      else if ( lhs->isVar( ) )
      {
	assert( rhs->isConstant( ) || ( rhs->isUminus( ) && rhs->get1st( )->isConstant( ) ) );
	tmp_edge_weight = rhs->isConstant( )
	                ? rhs->getCar( )->getValue( )
#if FAST_RATIONALS
	                : Real(-rhs->get1st( )->getCar( )->getValue( ));
#else
	                : -1 * rhs->get1st( )->getCar( )->getValue( );
#endif
	x = lhs;
      }
      else
      {
	assert( lhs->isConstant( ) || ( lhs->isUminus( ) && lhs->get1st( )->isConstant( ) ) );
	tmp_edge_weight = lhs->isConstant( )
	                ? lhs->getCar( )->getValue( )
#if FAST_RATIONALS
	                : Real(-(rhs->get1st( )->getCar( )->getValue( )));
#else
	                : -1 * rhs->get1st( )->getCar( )->getValue( );
#endif
	y = rhs;
      }
    }
    else
    {
      Enode * d = e->get1st( )->isMinus( ) ? e->get1st( ) : e->get2nd( );
      Enode * c = e->get1st( )->isMinus( ) ? e->get2nd( ) : e->get1st( );

      assert( c->isConstant( ) || ( c->isUminus( ) && c->get1st( )->isConstant( ) ) );

#if FAST_RATIONALS
      tmp_edge_weight = c->isConstant( )
                      ? c->getCar( )->getValue( )
	              : Real(-c->get1st( )->getCar( )->getValue( ));
      tmp_edge_weight = e->get1st( )->isMinus( )
                      ? tmp_edge_weight
                      : Real(-tmp_edge_weight);
#else
      tmp_edge_weight = c->isConstant( )
                      ? c->getCar( )->getValue( )
                      : -1 * c->get1st( )->getCar( )->getValue( );
      tmp_edge_weight = e->get1st( )->isMinus( )
                      ? tmp_edge_weight
                      : -1 * tmp_edge_weight;
#endif
      x = e->get1st( )->isMinus( ) ? d->get1st( ) : d->get2nd( );
      y = e->get1st( )->isMinus( ) ? d->get2nd( ) : d->get1st( );
    }

#if RESCALE_IN_DL
    T posWeight = getPosWeight( posWeight ) * ( config.logic == QF_RDL ? egraph.getRescale( posWeight ) : 1 );
#else
    T posWeight = getPosWeight( posWeight );
#endif
#if FAST_RATIONALS
    T negWeight = -posWeight -1;
#else
    T negWeight = -1 * posWeight -1;
#endif

    DLVertex<T> *u = getDLVertex(x);
    DLVertex<T> *v = getDLVertex(y);
    DLEdge<T> *pos = new DLEdge<T>(e, 2*edgeMap.size( ), u, v, posWeight);
    DLEdge<T> *neg = new DLEdge<T>(e, 2*edgeMap.size( ) + 1, v, u, negWeight);
    DLComplEdges<T> edges = DLComplEdges<T>( pos, neg );
    edgeMap.insert( pair< Enode *, DLComplEdges<T> > (e, edges) );
    return edges;
  }
  else
    return edgeMap.find(e)->second;
}

template<class T> void DLGraph<T>::insertStatic(Enode *c)
{
  DLEdge<T> *pos = getDLEdge( c ).pos;
  DLEdge<T> *neg = getDLEdge( c ).neg;

  Vcnt = vertexMap.size();
  sAdj.resize( Vcnt );

  dAdj.resize( Vcnt ); dAdjInc.resize( Vcnt );
  hAdj.resize( Vcnt ); hAdjInc.resize( Vcnt );
  iAdj.resize( Vcnt );
  pq_dx_it.resize( Vcnt );
  pq_dy_it.resize( Vcnt );

  sAdj[ pos->u->id ].push_back( pos );
  sAdj[ neg->u->id ].push_back( neg );
  sEdges.push_back( pos );
  sEdges.push_back( neg );
  Ecnt = Ecnt + 2;
  assert( sEdges.size( ) == Ecnt );

  // maintaining the set of inactive edges
  if ( config.dlconfig.theory_propagation > 0 )
    insertInactive( c );
}

template< class T > void DLGraph<T>::deleteActive( Enode * c )
{
  assert ( c->hasPolarity( ) );
  assert ( edgeMap.find( c ) != edgeMap.end( ) );
  DLComplEdges<T> edges = edgeMap.find( c )->second;

  DLEdge<T> *e = c->getPolarity( ) == l_True ? edges.pos : edges.neg;
  DLEdge<T> *d = dAdj[ e->u->id ].back( );
  assert ( d == e );
  dAdj[ e->u->id ].pop_back( );
  dEdges.back( );
  dEdges.pop_back( );
  assert( e->v->id < (int) dAdjInc.size( ) );
  DLEdge<T> *i = dAdjInc[ e->v->id ].back( );
  assert ( i == d );
  dAdjInc[ e->v->id ].pop_back( );
  after_backtrack = true;
  updateDynDegree( e );

  if ( config.dlconfig.theory_propagation > 0 )
  {
    insertInactive( c );
  }
}

//
// TODO: Call "inactive" functions only if the
// call may trigger some deduction. If deduction
// is disabled, or in case we know a priori
// that the call will be unsat (TODO: this is the case
// for getting reasons) do not update "inactive"
// data structures
//
template< class T> void DLGraph<T>::insertInactive( Enode * e )
{
  assert ( edgeMap.find( e ) != edgeMap.end( ) );
  DLComplEdges<T> edges = edgeMap.find( e )->second;
  DLEdge<T> * pos = edges.pos;
  hAdj   [ pos->u->id ].push_back( pos );
  hAdjInc[ pos->v->id ].push_back( pos );
  updateHDegree( pos );

  DLEdge<T> * neg = edges.neg;
  hAdj   [ neg->u->id ].push_back( neg );
  hAdjInc[ neg->v->id ].push_back( neg );
  updateHDegree( neg );
}

template< class T >void DLGraph<T>::insertImplied( Enode * c )
{
  assert( config.dlconfig.theory_propagation > 0 );
  deleteInactive( c );
}

template < class T >DLEdge<T> * DLGraph<T>::insertDynamic( Enode * c, bool reason )
{
  assert( c->hasPolarity( ) );
  assert( edgeMap.find( c ) != edgeMap.end( ) );

  DLComplEdges<T> edges = edgeMap.find( c )->second;
  DLEdge<T> *e = c->getPolarity ( ) == l_True ? edges.pos : edges.neg;
  assert( e );

  dAdj[ e->u->id ].push_back( e );
  dEdges.push_back( e );

  assert( e->v->id < (int) dAdjInc.size( ) );
  dAdjInc[ e->v->id ].push_back( e );

  updateDynDegree( e );
  max_dyn_edges = dEdges.size( ) >  max_dyn_edges ? dEdges.size( ) : max_dyn_edges;

  if ( config.dlconfig.theory_propagation > 0 )
    deleteInactive( c );

  return e;
}

template< class T > void DLGraph<T>::deleteInactive( Enode * e )
{

  assert ( edgeMap.find( e ) != edgeMap.end( ) );
  DLComplEdges<T> edges = edgeMap.find( e )->second;
  DLEdge<T> * pos = edges.pos;
  DLEdge<T> * neg;
  neg = edges.neg;

  // delete inserted edge from the set of inactive edges
  assert( pos->u->id < (int) hAdj.size( ) );
  deleteFromAdjList( hAdj   [ pos->u->id ], pos );
  assert( pos->v->id < (int) hAdjInc.size( ) );
  deleteFromAdjList( hAdjInc[ pos->v->id ], pos );
  updateHDegree( pos );

  assert( neg->u->id < (int) hAdj.size( ) );
  deleteFromAdjList( hAdj   [ neg->u->id ], neg );
  assert( neg->v->id < (int) hAdjInc.size( ) );
  deleteFromAdjList( hAdjInc[ neg->v->id ], neg );
  updateHDegree( neg );
  assert ( find( hEdges.begin( ), hEdges.end( ), pos ) == hEdges.end( ) );
  assert ( find( hEdges.begin( ), hEdges.end( ), neg ) == hEdges.end( ) );
}

// dfs implementation
template< class T > bool DLGraph<T>::dfsVisit( DLEdge<T> * e )
{
  cerr << endl << "[dfsVisit] edge " << e << endl;
  assert( dfs_stack.empty( ) );
  ///bool found_cycle = false;

  initDfsVisited( );
  initDfsFinished( );

  //cycle_edges.resize( Vcnt );
  dfs_stack.push_back( e->u );

  while ( !dfs_stack.empty( ) )
  {
    DLVertex<T> * u = dfs_stack.back( );
    cerr << "popping vertex " << u->e << " from stack." << endl;
    dfs_stack.pop_back( );
    setDfsVisited( u );
    AdjList & adjList = dAdj[ u->id ];
    typename AdjList::iterator it;
    for ( it = adjList.begin( ); it != adjList.end( ); ++ it )
    {
      //cerr << "checking edge " << *it << endl;
      DLVertex<T> * v = (*it)->v;
      conflict_edges[ v->id ] = *it;
      if ( ! isDfsVisited( v ) )
      {
	dfs_stack.push_back( v );
      }
      else if ( isDfsFinished( v ) )
      {
	//cerr << "edge " << *it << " is a back edge." << endl;
	//cerr << "u = " << (*it)->u->e << endl;
	//cerr << "v = " << (*it)->v->e << endl;
	negCycleVertex = (*it)->v; // TODO: check this
	cerr << "[dfsVisit] neg Cycle vertex " << negCycleVertex->e << endl;
	//found_cycle = true;
	//cerr << "[dfsVisit] found cycle " << endl;
	dfs_stack.clear( );
	doneDfsVisited( );
	doneDfsFinished( );
	return false;
      }
    }
    setDfsFinished( u );
  }

  doneDfsVisited( );
  doneDfsFinished( );

  return true;

/*
  if ( found_cycle )
  {
    DLVertex<T> * s = e->u;
    ///cerr << "Cycle: " << endl;
    do
    {
      DLEdge<T> * edge = conflict_edges[ s->id ];
      s = edge->u;
      //cerr << edge << endl;
    }
    while( s != e->u );
    return false;
  }
  else
    return true;
   */

}
// check for a neg cycle by dfs
template< class T > bool DLGraph<T>::checkNegCycleDFS( Enode *c, bool reason )
{
  DLEdge<T> *e = insertDynamic( c, reason );
  if ( e == NULL )
    return true;

  assert( changed_vertices.empty( ) );

  conflict_edges.resize( Vcnt ); // move the initialization!

  DLVertex<T> *u = e->u; DLVertex<T> *v = e->v;
  // gamma(v) = pi(u) + d - pi(v)
  v->gamma = u->pi  + e->wt - v->pi;

  if (v->gamma < 0)
  {
    dfs_stack.push_back(v);
    conflict_edges[v->id] = e; // TODO check this
  }
  initGamma( ); initPiPrime( );
  while ( !dfs_stack.empty( ) )
  {
    DLVertex<T> * s = dfs_stack.back( );
    dfs_stack.pop_back( );
    // pi'(s) = pi(s) + gamma(s)
    if ( ! isPiPrime( s ) )
    {
      s->old_pi = s->pi;
      changed_vertices.push_back( s );
    }
    s->pi = s->pi + s->gamma;
    updatePiPrime( s );
    // gamma(s) = 0
    s->gamma = 0;
    readGamma( s );
    AdjList & adjList = dAdj[s->id];
    for ( typename AdjList::iterator it = adjList.begin( ); it != adjList.end( ); ++it )
    {
      DLVertex<T> *t = (*it)->v;
      // if pi'(t) = pi(t) then
      if ( !isPiPrime( t ) )
      {
	if ( ! isGammaRead( t ) )
	{
	  t->gamma = 0;
	  readGamma( t );
	}
	const T value = s->pi + (*it)->wt - t->pi;

	if ( t->id == u->id )
	{ // t = u (t is the source vertex)
	  assert( u == t );
	  if ( value < t->gamma )
	  {
	    negCycleVertex = u; // TODO: check this
	    conflict_edges[t->id] = *it;
	    // restore the old_pi
	    for ( typename vector< DLVertex<T> * >::iterator it = changed_vertices.begin( ); it != changed_vertices.end( ); ++ it )
	      (*it)->pi = (*it)->old_pi;

	    changed_vertices.clear( );
	    dfs_stack.clear( );
	    doneGamma( ); donePiPrime( );
	    return false;
	  }
	}
	else
	{
	  // pq.decrease_key(t)
	  // if ( s->pi + (*it)->wt - t->pi  < t->gamma )
	  if ( value < t->gamma )
	  {
	    // t->gamma == 0 implies that t is not on the heap
	    if ( t->gamma == 0 )
	    {
	      // t->gamma = s->pi + (*it)->wt - t->pi;
	      t->gamma = value;
	      dfs_stack.push_back ( t );
	      conflict_edges[t->id] = *it;
	    }
	    else
	    {
	      assert( t->gamma < 0 );
	      // find t in the vector dfs_stack O(n)
	      typename vector< DLVertex<T> *>::iterator v_it;
	      for (v_it = dfs_stack.begin( ); v_it != dfs_stack.end( ); ++v_it)
	      {
		if( (*v_it) == t ) // update t->gamma in the vector
		{
		  (*v_it)->gamma = value;
		  break;
		}
	      }
	    }
	    conflict_edges[t->id] = *it;
	  }
	}
      }
    }
  }
  doneGamma( ); donePiPrime( );
  changed_vertices.clear( );

  return true;
}
//
// Check for a negative cycle in a constraint graph
//
template< class T > bool DLGraph<T>::checkNegCycle( Enode *c, bool reason )
{
#if PRINT_CLUSTERS
  computeNeighb( );

  ofstream out( "clusters.dot" );
  out << "Graph dump {" << endl;

  for ( size_t i = 0 ; i < enode_to_neighb.size( ) ; i ++ )
  {
    if ( enode_to_neighb[ i ].size( ) == 0 )
      continue;
    if ( enode_to_neighb[ i ].size( ) >= 3 )
    {
      cerr << id_to_enode[ i ] << " >= 3 " << endl;
      continue;
    }

    cerr << id_to_enode[ i ] << " <= 3 " << endl;

    for ( set< Enode * >::iterator it = enode_to_neighb[ i ].begin( )
	; it != enode_to_neighb[ i ].end( )
	; it ++ )
    {
      out << id_to_enode[ i ] << " -- " << *it << ";" << endl;
    }
  }

  out << endl << "}" << endl;

  exit( 1 ) ;
#endif

  assert( changed_vertices.empty( ) );

  DLEdge<T> *e = insertDynamic( c, reason );
  if ( e == NULL )
    return true;


  conflict_edges.resize( Vcnt ); // move the initialization!

  // run dfs when dealing with partial order
  //cerr << dfsVisit( e ) << endl;
  //bool ret_dfs =  dfsVisit( e );
  //cerr << "ret dfs " << ret_dfs << endl;
  //return ret_dfs;

  DLVertex<T> *u = e->u; DLVertex<T> *v = e->v;
  // gamma(v) = pi(u) + d - pi(v)
  v->gamma = u->pi  + e->wt - v->pi;

  if (v->gamma < 0)
  {
    vertex_heap.push_back(v); push_heap(vertex_heap.begin(), vertex_heap.end(), typename  DLVertex<T>::gammaGreaterThan() );
    assert( is_heap( vertex_heap.begin(), vertex_heap.end(),  typename DLVertex<T>::gammaGreaterThan() ) );
    conflict_edges[v->id] = e; // TODO check this
  }
  initGamma( ); initPiPrime( );
  while ( !vertex_heap.empty( ) )
  {
    assert( is_heap( vertex_heap.begin(), vertex_heap.end(), typename DLVertex<T>::gammaGreaterThan() ) );

    DLVertex<T> * s = vertex_heap.front( );
    pop_heap( vertex_heap.begin( ), vertex_heap.end( ), typename DLVertex<T>::gammaGreaterThan( ) );
    vertex_heap.pop_back( );
    assert( is_heap( vertex_heap.begin( ), vertex_heap.end( ), typename DLVertex<T>::gammaGreaterThan( ) ) );
    // pi'(s) = pi(s) + gamma(s)
    if ( ! isPiPrime( s ) )
    {
      s->old_pi = s->pi;
      changed_vertices.push_back( s );
    }
    s->pi = s->pi + s->gamma;
    updatePiPrime( s );
    // gamma(s) = 0
    s->gamma = 0;
    readGamma( s );
    AdjList & adjList = dAdj[s->id];
    for ( typename AdjList::iterator it = adjList.begin( ); it != adjList.end( ); ++it )
    {
      DLVertex<T> *t = (*it)->v;
      // if pi'(t) = pi(t) then
      if ( !isPiPrime( t ) )
      {
	if ( ! isGammaRead( t ) )
	{
	  t->gamma = 0;
	  readGamma( t );
	}
	const T value = s->pi + (*it)->wt - t->pi;

	if ( t->id == u->id )
	{ // t = u (t is the source vertex)
	  assert( u == t );
	  if ( value < t->gamma )
	  {
	    negCycleVertex = u; // TODO: check this
	    conflict_edges[t->id] = *it;
	    // restore the old_pi
	    for ( typename vector< DLVertex<T> * >::iterator it = changed_vertices.begin( ); it != changed_vertices.end( ); ++ it )
	      (*it)->pi = (*it)->old_pi;

	    changed_vertices.clear( );
	    vertex_heap.clear( );
	    doneGamma( ); donePiPrime( );
	    return false;
	  }
	}
	else
	{
	  // pq.decrease_key(t)
	  // if ( s->pi + (*it)->wt - t->pi  < t->gamma )
	  if ( value < t->gamma )
	  {
	    // t->gamma == 0 implies that t is not on the heap
	    if ( t->gamma == 0 )
	    {
	      // t->gamma = s->pi + (*it)->wt - t->pi;
	      t->gamma = value;
	      vertex_heap.push_back ( t );
	      conflict_edges[t->id] = *it;
	    }
	    else
	    {
	      assert( t->gamma < 0 );
	      // find t in the vector vertex_heap O(n)
	      typename vector< DLVertex<T> *>::iterator v_it;
	      for (v_it = vertex_heap.begin( ); v_it != vertex_heap.end( ); ++v_it)
	      {
		if( (*v_it) == t ) // update t->gamma in the vector
		{
		  (*v_it)->gamma = value;
		  break;
		}
	      }
	    }
	    conflict_edges[t->id] = *it;
	  }
	}
      }
    }
    make_heap( vertex_heap.begin(), vertex_heap.end() , typename DLVertex<T>::gammaGreaterThan() );
  }
  doneGamma( ); donePiPrime( );
  changed_vertices.clear( );

  return true;
}

//
// Find edges with the larger weight than the shortest path between
// the edge endpoints
//
template< class T> void DLGraph<T>::findHeavyEdges( Enode * c )
{
  assert( c->hasPolarity( ) );

  //DLComplEdges edges = edgeMap.find(c)->second;
  DLComplEdges<T> edges = getDLEdge( c );
  DLEdge<T> *e = c->getPolarity ( ) == l_True ? edges.pos : edges.neg;

  // TODO: move this in the one-time called init procedure
  if ( 0 == LAZY_GENERATION )
  {
    if ( Vcnt > (unsigned) bSPT.size() ) bSPT.resize( Vcnt );
    if ( Vcnt > (unsigned) fSPT.size() ) fSPT.resize( Vcnt );
    if ( Ecnt > (unsigned) shortest_paths.size( ) ) shortest_paths.resize( Ecnt );
  }

  // check if there is a parallel edge of smaller weight - if yes: return
  if ( isParallelAndHeavy( e ) )
    return;

  if ( 0 == LAZY_GENERATION )
  {
    updateSPT( e, DL_sssp_forward);
    updateSPT( e, DL_sssp_backward);
  }

  initRwt( );

  initDxRel( );
  total_in_deg_dx_rel  = 0;
  dx_relevant_vertices.clear( );
  e->v->setRelevancy( DL_sssp_forward, true ); updateDxRel( e->v );
  //max_dist_from_src = 0;
  findSSSP( e->u, DL_sssp_forward );

  initDyRel( );
  total_out_deg_dy_rel = 0;
  dy_relevant_vertices.clear( );
  e->u->setRelevancy( DL_sssp_backward, true ); updateDyRel( e->u );
  findSSSP( e->v, DL_sssp_backward );

  doneRwt( );
  iterateInactive( e );

  // clear the shortest path tree
  clearSPTs( );
  doneDxRel( ); doneDyRel( );
}

template< class T> void DLGraph<T>::iterateInactive( DLEdge<T> * e )
{
  if ( total_out_deg_dy_rel < total_in_deg_dx_rel )
  {
    for ( typename vector< DLVertex<T> * >::iterator it = dy_relevant_vertices.begin( ); it != dy_relevant_vertices.end( ); ++ it )
    {
      assert( isDyRelValid( *it ) && (*it)->dy_relevant );
      AdjList & adj_list = hAdj[ (*it)->id ];
      for ( typename AdjList::iterator aIt = adj_list.begin( ); aIt != adj_list.end( ); ++ aIt )
      {
	if ( (*aIt)->c->hasPolarity( ) || (*aIt)->c->isDeduced( ) )
	  continue;
	const bool v_is_relevant = isDxRelValid( (*aIt)->v ) && (*aIt)->v->dx_relevant;
	if ( v_is_relevant )
	{
	  const T rpath_wt = (*it)->dy + (*aIt)->v->dx - e->rwt;
	  addIfHeavy( rpath_wt, *aIt, e );
	}
      }
    }
  }
  else
  {
    for ( typename vector< DLVertex<T> *>::iterator it = dx_relevant_vertices.begin( ); it != dx_relevant_vertices.end( ); ++ it )
    {
      assert( isDxRelValid( *it ) );
      assert( (*it)->dx_relevant  );
      assert( (unsigned)(*it)->id < hAdjInc.size( ) );
      AdjList & adj_list = hAdjInc[ (*it)->id ];
      for ( typename AdjList::iterator aIt = adj_list.begin( ); aIt != adj_list.end( ); ++ aIt )
      {
	if ( (*aIt)->c->hasPolarity( ) || (*aIt)->c->isDeduced( ) )
	  continue;
	const bool u_is_relevant = isDyRelValid( (*aIt)->u ) && (*aIt)->u->dy_relevant;
	if ( u_is_relevant )
	{
	  const T rpath_wt = (*aIt)->u->dy + (*it)->dx - e->rwt;
	  addIfHeavy( rpath_wt, *aIt, e );
	}
      }
    }
  }
}

//
// Shortest path computation
// if   direction = DL_sssp_forward then forwardSSSP   ("to x")
// else                                  backwardSSSP  ("from x")
//
template< class T > void DLGraph<T>::findSSSP( DLVertex<T> * x, DL_sssp_direction direction )
{
  unsigned no_relevant = 0;

  initDist( ); initFinalDist( );  // initialize a new token for dist

  ( direction == DL_sssp_forward ) ? assert( pq_dx.empty( ) ) : assert( pq_dy.empty( ) ) ;

  x->setDist( direction, 0 );	  // x is the source vertex
  readDist( x );

  x->setDistFrom( direction, 0 ); // to track depth of the shortest path tree

  // handle delta relevancy
  x->setRelevancy( direction, false );
  if ( direction == DL_sssp_forward ) updateDxRel( x ); else updateDyRel( x );

  pushPBheap( direction, x );
  while ( !emptyPBheap( direction ) )
  {
    DLVertex<T> * u = topPBheap( direction );
    popPBheap( direction );
    finalDist( u );
    if ( u->getRelevancy( direction ) == true )
    {
      insertRelevantVertices( u, direction );
      -- no_relevant;
    }

    // handle delta relevancy
    const bool  valid_rel_u = ( direction == DL_sssp_forward) ? isDxRelValid( u ) : isDyRelValid( u );
    const bool  rel_u  =  (valid_rel_u) ? u->getRelevancy( direction ) : false;
    if ( direction == DL_sssp_forward ) updateDxRel( u ); else updateDyRel( u );

    // iterate through the adjacency list
    AdjList & adj_list = (direction == DL_sssp_forward) ? dAdj[ u->id ] : dAdjInc[ u->id ];
    if ( adj_list.size( ) > max_adj_list_size ) max_adj_list_size = adj_list.size( );
    unsigned no_of_adj_edges = 0;
    for( typename AdjList::iterator it  = adj_list.begin( ); it != adj_list.end  ( ); ++it )
    {
      ++ no_of_adj_edges;
      DLVertex<T> * v = (direction == DL_sssp_forward) ? (*it)->v : (*it)->u;
      // check if v's distance is final
      if ( isDistFinal( v ) )
	continue;
      // IMPORTANT: if v has the final distance then the reduced weight for the
      // corresponding edge will not be updated. So, the
      // backward and forward graphs can have different edge weights.

      //  make sure that rwt is valid after this point
      if ( ! isRwtValid( *it ) )
      {
	(*it)->rwt = (*it)->u->pi + (*it)->wt - (*it)->v->pi;
	assert( (*it)->rwt >= 0 );  // INVARIANT: rwt(e) >= 0
	updateRwt( *it );
      }
      assert( isRwtValid( *it ) ); // INVARIANT: rwt is not stale

      // find new distance
      const T dist = u->getDist( direction ) + (*it)->rwt;
      assert ( dist >= 0 );
      if ( ! isDistRead( v ) )
      { // initial distance is +infinity, so just assign computed distance

	v->setDist( direction, dist ); // set the shortest path distance
	if ( 0 == LAZY_GENERATION )
	  updateSPT( *it, direction ); // update the shortest path tree

	// handle delta relevancy
	const bool  valid_rel_v = ( direction == DL_sssp_forward ) ? isDxRelValid( v ) : isDyRelValid( v );
	if ( ! valid_rel_v )
	{
	  v->setRelevancy( direction, rel_u  ); // propagate relevancy
	  if ( direction == DL_sssp_forward ) updateDxRel( v ); else updateDyRel( v );
	}
	// v has a valid relevancy here

	// INVARIANT: v is NOT on the heap
	( direction == DL_sssp_forward ) ?  assert( find( pq_dx.begin( ), pq_dx.end( ), v) == pq_dx.end( ) )
	  :  assert( find( pq_dy.begin( ), pq_dy.end( ), v) == pq_dy.end( ) );

	// PUSH ON THE VECTOR: push v on the heap
	pushPBheap( direction, v );
	if ( v->getRelevancy( direction ) == true)
	{
	  ++ no_relevant;
	  v->setDistFrom( direction, u->getDistFrom( direction ) + 1 );
	}
      }
      else
      {
	( direction == DL_sssp_forward ) ? assert( find( pq_dx.begin( ), pq_dx.end( ), v) != pq_dx.end( ) )
	  : assert( find( pq_dy.begin( ), pq_dy.end( ), v) != pq_dy.end( ) );

	if ( v->getDist( direction ) > dist )
	{
	  v->setDist( direction, dist );

	  if ( v->getRelevancy( direction ) == false  && rel_u == true )
	    ++ no_relevant;
	  else if ( v->getRelevancy( direction ) == true && rel_u == false )
	    -- no_relevant;

	  v->setRelevancy( direction, rel_u  ); // propagate relevancy
	  if ( direction == DL_sssp_forward ) updateDxRel( v ); else updateDyRel( v );

	  modifyPBheap( direction, v );
	  if ( 0 == LAZY_GENERATION )
	    updateSPT( *it, direction );
	  if ( v->getRelevancy( direction ) == true)
	  {
	    v->setDistFrom( direction, u->getDistFrom( direction ) + 1 );
	  }

	}
      }
      readDist( v ); // we computed the distance
    }

    if ( no_relevant <= 0)
      break;
  }
  doneDist( ); doneFinalDist( ); // done with the dist computation
  clearPBheap( direction );
}

//
// Update shortest path trees: backward / forward
//
// TODO: refactor - change to update shortest path tree
//
template <class T> void DLGraph< T >::updateSPT( DLEdge<T> * e, DL_sssp_direction go )
{
  if ( go == DL_sssp_forward )
  {
    if ( e->v->dist_from_src > max_dist_from_src ) max_dist_from_src = e->v->dist_from_src;
    fSPT[ e->v->id ] = e;
  }
  else
  {
    if ( e->u->dist_from_dst > max_dist_from_dst ) max_dist_from_dst = e->u->dist_from_dst;
    bSPT[ e->u->id ] = e;
  }
}
//
// Find shortest path for an edge in the SPT
//
// added for lazy_eager schema
//
template <class T> bool DLGraph<T>::findShortestPath( DLEdge<T> * e )
{
  assert( e->id < (int) shortest_paths.size( ) );
  // DIRTY TRICK: reasons should be unique
  // --> the problem is that the edges in the inactive edges are not unique
  if( !shortest_paths[ e->id ].empty( ) )
    return false;

  DLVertex<T> *x = e->r->u;
  DLVertex<T> *y = e->r->v;

  DLEdge<T> *spt_edge = bSPT[ e->u->id ];
  shortest_paths[ e->id ].push_back( spt_edge );
  while ( spt_edge->u != x )
  {
    spt_edge = bSPT[ spt_edge->v->id ];
    assert( spt_edge );
    shortest_paths[ e->id ].push_back( spt_edge );
  }
  assert( shortest_paths[ e->id ].back( ) == e->r );

  spt_edge = fSPT[ e->v->id ];
  if ( spt_edge->u != x )
  {
    list< DLEdge<T> * > backward_path;
    backward_path.push_front( spt_edge );
    while( spt_edge->u != y )
    {
      spt_edge = fSPT[ spt_edge->u->id ];
      assert( spt_edge );
      backward_path.push_front( spt_edge );
    }

    while( ! backward_path.empty( ) )
    {
      shortest_paths[ e->id ].push_back( backward_path.front( ) );
      backward_path.pop_front( );
    }
  }
  return true;
}

//
// Print adjacency list
//
template< class T> void DLGraph<T>::printAdj(vector<AdjList> & adj)
{
  typename vector<AdjList>::iterator it;
  int i = 0;
  for(it = adj.begin(); it != adj.end(); ++it, ++i)
  {
    cerr << "Vertex " << i << " ====> ";
    printAdjList(*it);
    cerr << " " << endl;
  }
}

template< class T> void DLGraph<T>::printAdjList(AdjList & adjList)
{
  typename AdjList::iterator it;
  for (it = adjList.begin(); it != adjList.end(); ++it)
    cerr << *it << "  ";
}

template< class T> void DLGraph<T>::printDynGraphAsDotty( const char * filename, DLEdge<T> *e )
{
  ofstream out( filename );
  out << "DiGraph dump {" << endl;

  for ( typename vector< DLVertex<T> * >::iterator it = vertices.begin( )
      ; it != vertices.end( )
      ; ++ it )
  {
    AdjList & adjList = dAdj[(*it)->id];
    typename AdjList::iterator jt;
    for (jt = adjList.begin(); jt != adjList.end(); ++jt)
    {
      if ( (*jt) == e )
	printPlainEdge( out, *jt, "[color=red];" );
      else
	printPlainEdge( out, *jt );

    }
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}


template< class T > void DLGraph<T>::printSSSPAsDotty( const char * filename, DLVertex<T> * u , DL_sssp_direction direction )
{
  ofstream out( filename );
  out << "DiGraph dump {" << endl;
  out << "\"" << u->e << " | " << u->getDist( direction ) << "\"" << " [color=red];" << endl;

  for ( typename vector< DLVertex<T> * >::iterator it = vertices.begin( )
      ; it != vertices.end( )
      ; ++ it )
  {
    AdjList & adjList = dAdj[(*it)->id];
    typename AdjList::iterator jt;
    for (jt = adjList.begin(); jt != adjList.end(); ++jt)
      printSSSPEdge( out, *jt, direction );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}

template< class T> void DLGraph<T>::printInactiveAsDotty( const char * filename )
{
  ofstream out ( filename );
  out << "DiGraph dump { " << endl;
  typename vector< DLEdge<T> * >::iterator it;
  for ( it = hEdges.begin( ); it != hEdges.end( ); ++ it)
  {
    const bool u_is_relevant = isDyRelValid( (*it)->u ) && (*it)->u->dy_relevant;
    const bool v_is_relevant = isDxRelValid( (*it)->v ) && (*it)->v->dx_relevant;
    string attrib = (u_is_relevant && v_is_relevant) ? " [color=red]; " : " ;";

    printDistEdge( out, *it, attrib );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;

}

template< class T > void DLGraph<T>::printDeducedAsDotty( const char * filename )
{
  ofstream out( filename );
  out << "DiGraph dump {" << endl;

  for ( typename vector< DLEdge<T> * >::iterator it = heavy_edges.begin( )
      ; it != heavy_edges.end( ); ++ it)
  {
    string attrib = " [color=green]; ";
    printDistEdge( out, *it, attrib );
  }

  for ( typename vector< DLVertex<T> * >::iterator it = vertices.begin( )
      ; it != vertices.end( )
      ; ++ it )
  {
    AdjList & adjList = dAdj[(*it)->id];
    typename AdjList::iterator jt;
    for (jt = adjList.begin(); jt != adjList.end(); ++jt)
      printDistEdge( out, *jt );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}

template< class T> void DLGraph<T>::printShortestPath( DLEdge<T> * e, const char * filename )
{
  assert( e );
  DLPath & shortest_path = getShortestPath( e );
  ofstream out( filename );
  out << "DiGraph sp {" << endl;

  printDistEdge( out, e, "[color=red];" );

  for (typename DLPath::iterator it = shortest_path.begin( ); it != shortest_path.end( ); ++ it )
  {
    printDistEdge( out, *it );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}

template <class T > void DLGraph<T>::printDLPath( DLPath path, const char * filename )
{
  //assert( path );
  ofstream out( filename );

  out << "DiGraph sp {" << endl;

  for ( typename DLPath::iterator it = path.begin( ); it != path.end( ); ++ it )
  {
    printDistEdge( out, *it );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}