/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@gmail.com>

OpenSMT -- Copyright (C) 2009, Roberto Bruttomesso

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
*********************************************************************/

#ifndef ENODE_H
#define ENODE_H

#include "EnodeTypes.h"
#include "Otl.h"

class Enode
{
public:
  
  //
  // Constructor for Enil
  //
  Enode  ( );                                          
  //
  // Constructor for symbols
  //
  Enode ( const enodeid_t       // id 
	, const char *          // name/value
	, const etype_t         // enode type
	, const unsigned        // data type
	, vector< unsigned > &  // arity
	);
  // 
  // Constructor for numbers
  //
  Enode ( const enodeid_t       // id 
	, const char *          // name/value
	, const etype_t         // enode type
	, const unsigned        // data type
	);
  //
  // Constructor for terms and lists
  //
  Enode ( const enodeid_t       // id
	, Enode *               // car
	, Enode *               // cdr
        );
  //
  // Constructor for defs
  //
  Enode ( const enodeid_t       // id
	, Enode *	        // def
	);		
  //
  // Destructor
  //
  ~Enode ( );
  //
  // Check if a node is Enil
  //
  inline bool isEnil            ( ) const { return id == ENODE_ID_ENIL; }
  inline bool isList            ( ) const { return (properties & ETYPE_MASK) == ETYPE_LIST; }
  inline bool isTerm            ( ) const { return (properties & ETYPE_MASK) == ETYPE_TERM; }
  inline bool isSymb            ( ) const { return (properties & ETYPE_MASK) == ETYPE_SYMB; }
  inline bool isNumb            ( ) const { return (properties & ETYPE_MASK) == ETYPE_NUMB; }
  inline bool isDef             ( ) const { return (properties & ETYPE_MASK) == ETYPE_DEF; }

  inline void setEtype          ( const etype_t t ) 
  { 
    assert( t == ETYPE_SYMB 
	 || t == ETYPE_NUMB 
	 || t == ETYPE_LIST 
	 || t == ETYPE_TERM 
	 || t == ETYPE_DEF );
    properties |= t;
  }

  inline void setArity          ( const unsigned a ) { assert( a <= ARITY_N ); properties |= (a << ARITY_SHIFT); }
  inline void setDtype          ( const unsigned t ) { properties |= t; }
  //
  // Check if a term node represents a certain symbol
  //
  inline bool isPlus            ( ) const { return hasSymbolId( ENODE_ID_PLUS	     ); }
  inline bool isMinus           ( ) const { return hasSymbolId( ENODE_ID_MINUS	     ); }
  inline bool isUminus          ( ) const { return hasSymbolId( ENODE_ID_UMINUS	     ); }
  inline bool isTimes           ( ) const { return hasSymbolId( ENODE_ID_TIMES	     ); }
  inline bool isDiv             ( ) const { return hasSymbolId( ENODE_ID_DIV	     ); }
  inline bool isEq              ( ) const { return hasSymbolId( ENODE_ID_EQ	     ); }
  inline bool isNeq             ( ) const { return hasSymbolId( ENODE_ID_NEQ	     ); }
  inline bool isLeq             ( ) const { return hasSymbolId( ENODE_ID_LEQ	     ); }
  inline bool isGeq             ( ) const { return hasSymbolId( ENODE_ID_GEQ	     ); }
  inline bool isLt              ( ) const { return hasSymbolId( ENODE_ID_LT	     ); }
  inline bool isGt              ( ) const { return hasSymbolId( ENODE_ID_GT	     ); }
  inline bool isBvslt           ( ) const { return hasSymbolId( ENODE_ID_BVSLT       ); }
  inline bool isBvsgt           ( ) const { return hasSymbolId( ENODE_ID_BVSGT       ); }
  inline bool isBvsle           ( ) const { return hasSymbolId( ENODE_ID_BVSLE       ); }
  inline bool isBvsge           ( ) const { return hasSymbolId( ENODE_ID_BVSGE       ); }
  inline bool isBvult           ( ) const { return hasSymbolId( ENODE_ID_BVULT       ); }
  inline bool isBvugt           ( ) const { return hasSymbolId( ENODE_ID_BVUGT       ); }
  inline bool isBvule           ( ) const { return hasSymbolId( ENODE_ID_BVULE       ); }
  inline bool isBvuge           ( ) const { return hasSymbolId( ENODE_ID_BVUGE       ); }
  inline bool isConcat          ( ) const { return hasSymbolId( ENODE_ID_CONCAT      ); }
  inline bool isCbe             ( ) const { return hasSymbolId( ENODE_ID_CBE         ); }
  inline bool isBvand           ( ) const { return hasSymbolId( ENODE_ID_BVAND       ); }
  inline bool isBvor            ( ) const { return hasSymbolId( ENODE_ID_BVOR        ); }
  inline bool isBvxor           ( ) const { return hasSymbolId( ENODE_ID_BVXOR       ); }
  inline bool isBvnot           ( ) const { return hasSymbolId( ENODE_ID_BVNOT       ); }
  inline bool isBvadd           ( ) const { return hasSymbolId( ENODE_ID_BVADD       ); }
  inline bool isBvsub           ( ) const { return hasSymbolId( ENODE_ID_BVSUB       ); }
  inline bool isBvmul           ( ) const { return hasSymbolId( ENODE_ID_BVMUL       ); }
  inline bool isBvneg           ( ) const { return hasSymbolId( ENODE_ID_BVNEG       ); }
  inline bool isBvlshr          ( ) const { return hasSymbolId( ENODE_ID_BVLSHR      ); }
  inline bool isBvashr          ( ) const { return hasSymbolId( ENODE_ID_BVASHR      ); }
  inline bool isBvshl           ( ) const { return hasSymbolId( ENODE_ID_BVSHL       ); }
  inline bool isBvsrem          ( ) const { return hasSymbolId( ENODE_ID_BVSREM      ); }
  inline bool isBvurem          ( ) const { return hasSymbolId( ENODE_ID_BVUREM      ); }
  inline bool isBvsdiv          ( ) const { return hasSymbolId( ENODE_ID_BVSDIV      ); }
  inline bool isBvudiv          ( ) const { return hasSymbolId( ENODE_ID_BVUDIV      ); }
  inline bool isSignExtend      ( ) const { int i; return sscanf( car->getName( ), "sign_extend[%d]", &i ) == 1; }
  bool        isSignExtend      ( int * );
  inline bool isZeroExtend      ( ) const { return hasSymbolId( ENODE_ID_ZERO_EXTEND ); }
  inline bool isImplies         ( ) const { return hasSymbolId( ENODE_ID_IMPLIES     ); }
  inline bool isAnd             ( ) const { return hasSymbolId( ENODE_ID_AND         ); }
  inline bool isOr              ( ) const { return hasSymbolId( ENODE_ID_OR          ); }
  inline bool isNot             ( ) const { return hasSymbolId( ENODE_ID_NOT         ); }
  inline bool isIff             ( ) const { return hasSymbolId( ENODE_ID_IFF         ); }
  inline bool isXor             ( ) const { return hasSymbolId( ENODE_ID_XOR         ); }
  inline bool isTrue            ( ) const { return hasSymbolId( ENODE_ID_TRUE        ); }
  inline bool isFalse           ( ) const { return hasSymbolId( ENODE_ID_FALSE       ); }
  inline bool isIte             ( ) const { return hasSymbolId( ENODE_ID_ITE         ); }
  inline bool isIfthenelse      ( ) const { return hasSymbolId( ENODE_ID_IFTHENELSE  ); }
  inline bool isDistinct        ( ) const { return hasSymbolId( ENODE_ID_DISTINCT    ); }
  inline bool isBoolcast        ( ) const { return hasSymbolId( ENODE_ID_BOOLCAST    ); }
  inline bool isWord1cast       ( ) const { return hasSymbolId( ENODE_ID_WORD1CAST   ); }
  inline bool isUp              ( )       { return car->id > ENODE_ID_LAST && isAtom( ); }
  inline bool isUf              ( )       { return car->id > ENODE_ID_LAST && !isAtom( ); }

  bool        isExtract         ( int * msb, int * lsb );
  inline bool isExtract         ( ) { assert( isTerm( ) ); assert( car->symb_data ); return car->symb_data->lsb != -1; }
  inline void setExtract        ( const int ilsb ) { assert( isSymb( ) ); assert( symb_data ); symb_data->lsb = ilsb; }
  inline void getExtract        ( int * msb, int * lsb ) { assert( isSymb( ) ); assert( symb_data ); *lsb = symb_data->lsb; *msb = getWidth( ) -1 + *lsb; }

  bool        isVar             ( ); // True if it is a variable
  bool        isConstant        ( ); // True if it is a constant
  bool        isLit             ( ); // True if it is a literal
  bool        isAtom            ( ); // True if it is an atom
  bool        isTLit            ( ); // True if it is a theory literal
  bool        isTAtom           ( ); // True if it is a theory atom
  bool        isBooleanOperator ( ); // True if it is a boolean operator

  inline bool isDTypeBool       ( ) { assert( isTerm( ) || isSymb( ) || isNumb( ) ); return (properties & DTYPE_MASK) == DTYPE_BOOL; }
  inline bool isDTypeU          ( ) { assert( isTerm( ) || isSymb( ) || isNumb( ) ); return (properties & DTYPE_MASK) == DTYPE_U; }
  inline bool isDTypeReal       ( ) { assert( isTerm( ) || isSymb( ) || isNumb( ) ); return (properties & DTYPE_MASK) == DTYPE_REAL; }
  inline bool isDTypeInt        ( ) { assert( isTerm( ) || isSymb( ) || isNumb( ) ); return (properties & DTYPE_MASK) == DTYPE_INT; }
  inline bool isDTypeBitVec     ( ) { assert( isTerm( ) || isSymb( ) || isNumb( ) ); return (properties & DTYPE_MASK) == DTYPE_BITVEC; }

  inline bool hasCongData       ( ) { return cong_data != NULL; }

  void        allocCongData     ( ); 

  //
  // Getty and Setty methods
  //
  inline enodeid_t            getId     ( ) const { return id; }
  inline unsigned             getArity  ( ) const { return ((properties & ARITY_MASK) >> ARITY_SHIFT); }
  inline unsigned             getDType  ( ) const { return (properties & DTYPE_MASK) | (properties & WIDTH_MASK); }
  inline vector< unsigned > & getSort   ( )       { assert( isSymb( ) ); assert( symb_data ); return symb_data->args_sorts; }
  inline char *  getName                ( ) const { assert( isSymb( ) || isNumb( ) ); assert( symb_data ); return symb_data->name; }
  inline Enode * getCar                 ( ) const { return car; }
  inline Enode * getCdr                 ( ) const { return cdr; }
  inline Enode * getDef                 ( ) const { assert( isDef( ) ); assert( car ); return car; }            
                                        
  inline Enode * getNext                ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->next; }
  inline int     getSize                ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->size; }
  inline Enode * getParent              ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->parent; }
  inline Enode * getSameCar             ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->same_car; }
  inline Enode * getSameCdr             ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->same_cdr; }
  inline int     getParentSize          ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->parent_size; }
  inline Enode * getCgPtr               ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->cg_ptr; }
  inline Elist * getForbid              ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->forbid; }
  inline dist_t  getDistClasses         ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->dist_classes; }

  const Real &   getValue               ( ) const;
  Enode *        getRoot                ( ) const;
  enodeid_t      getCid                 ( ) const;
  Enode *        getConstant            ( ) const;
                                        
  inline Enode * getExpParent           ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_parent; }
  inline Enode * getExpRoot             ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_root; }
  inline int     getExpClassSize        ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_class_size; }
  inline Enode * getExpHighestNode      ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_highest_node; }
  inline Reason * getExpReason          ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_reason; }
  inline int     getExpTimeStamp        ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_time_stamp; }
                                        
  inline lbool   getPolarity            ( ) const { assert( isTerm( ) && atom_data ); return atom_data->polarity; }
  inline bool	 hasPolarity            ( ) const { assert( isTerm( ) && atom_data ); return atom_data->has_polarity; }
  inline lbool   getDeduced             ( ) const { assert( isTerm( ) && atom_data ); return atom_data->deduced; }
  inline bool    isDeduced              ( ) const { assert( isTerm( ) && atom_data ); return atom_data->is_deduced; }
  inline lbool   getDecPolarity         ( )       { assert( isAtom( ) && atom_data ); return atom_data->dec_polarity; }
  inline int     getWeightInc           ( )       { assert( isAtom( ) && atom_data ); return atom_data->weight_inc; }
  inline int     getDedIndex            ( ) const { assert( isTerm( ) && atom_data ); return atom_data->ded_index; }
  inline int     getDistIndex           ( ) const { assert( isTerm( ) && atom_data ); return atom_data->dist_index; }

  inline Enode * getCb                  ( ) const            { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->cb; }
  inline Enode * getRef                 ( ) const            { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->root; }
  inline int     getWidth               ( ) const            { assert( isTerm( ) || isSymb( ) || isNumb( ) ); return (properties & WIDTH_MASK); }
  inline void    setWidth               ( const uint32_t w ) 
  { 
    assert( isTerm( ) ); 
    assert( w < MAX_WIDTH ); 
    // Reset width
    properties &= ~WIDTH_MASK;
    properties |= w; 
    assert( getWidth( ) == static_cast<int>( w ) ); 
  }
  inline void    setValue               ( const Real & v )   { assert( cong_data->term_data->value ); *(cong_data->term_data->value) = v; }            
  inline void    setRoot                ( Enode * e )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->root = e; }
  inline void    setCid                 ( const enodeid_t c ){ assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->cid = c; }               
  inline void    setDef                 ( Enode * e )        { assert( e ); assert( isDef( ) ); car = e; }            
  inline void    setNext                ( Enode * e )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->next = e; }
  inline void    setSize                ( const int s )      { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->size = s; }
  inline void    setParent              ( Enode * e )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->parent = e; }          
  inline void    setSameCar             ( Enode * e )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->same_car = e; }     
  inline void    setSameCdr             ( Enode * e )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->same_cdr = e; }       
  inline void    setParentSize          ( const int s )      { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->parent_size = s; }       
  inline void    setCgPtr               ( Enode * e )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->cg_ptr = e; }            
  inline void    setForbid              ( Elist * l )        { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->forbid = l; }            
  inline void    setDistClasses         ( const dist_t & d ) { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->dist_classes = d; }            
                                                                 
  inline void    setConstant            ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); assert( e == NULL || e->isConstant( ) ); cong_data->term_data->constant = e; }                  
  inline void    setExpParent           ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_parent = e; }             
  inline void    setExpRoot             ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_root = e; }                   
  inline void    setExpClassSize        ( const int s )          { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_class_size = s; }            
  inline void    setExpHighestNode      ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_highest_node = e; }                
  inline void    setExpReason           ( Reason * r )           { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_reason = r; }                    
  inline void    setExpTimeStamp        ( const int t )          { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_time_stamp = t; }             
  inline void    setPolarity            ( const lbool p )        { assert( isTerm( ) && atom_data ); assert( !atom_data->has_polarity ); atom_data->polarity = p; atom_data->has_polarity = true; } 
  inline void    resetPolarity          ( )			 { assert( isTerm( ) && atom_data ); assert( atom_data->has_polarity ); atom_data->has_polarity = false; } 
  inline void    setDeduced             ( const lbool d, int i ) { assert( isTerm( ) && atom_data ); assert( !atom_data->is_deduced ); atom_data->deduced = d; atom_data->ded_index = i; atom_data->is_deduced = true; }                     
  inline void    setDeduced             ( const bool s, int i )  { setDeduced( s ? l_False : l_True, i ); }
  inline void    resetDeduced           ( )			 { assert( isTerm( ) && atom_data ); assert( atom_data->is_deduced ); atom_data->is_deduced = false; }                     
  inline void    setDecPolarity         ( const lbool s )        { assert( isAtom( ) && atom_data ); atom_data->dec_polarity = s; }
  inline void    setWeightInc           ( const int w )          { assert( isAtom( ) && atom_data ); atom_data->weight_inc = w; }
  inline void    setDistIndex           ( const int d )	         { assert( isTerm( ) && atom_data ); atom_data->dist_index = d; }                
                                                                 
  inline void    setCb                  ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->cb = e; }                            
  // 
  // Congruence closure related routines
  //
  void                 addParent            ( Enode * );   // Adds a parent to this node. Useful for congruence closure
  void                 removeParent         ( Enode * );   // Remove a parent of this node
  //
  // Shortcuts for retrieving a term's arguments
  //
  inline Enode *       get1st               ( ) const;     // Get first argument in constant time
  inline Enode *       get2nd               ( ) const;     // Get second argument in constant time 
  inline Enode *       get3rd               ( ) const;     // Get third argument in constant time

  bool                 addToCongruence      ( ) const;
  unsigned             sizeInMem            ( ) const;

  void                 print	            ( ostream & ); // Prints the enode
  void                 printSig	            ( ostream & ); // Prints the enode signature

#ifdef BUILD_64
  inline const enodeid_pair_t    getSig    ( ) const { return encode( car->getRoot( )->getCid( ), cdr->getRoot( )->getCid( ) ); }
#else
  inline const Pair( enodeid_t ) getSig    ( ) const { return make_pair( car->getRoot( )->getCid( ), cdr->getRoot( )->getCid( ) ); }
#endif
  inline enodeid_t               getSigCar ( ) const { return car->getRoot( )->getCid( ); }
  inline enodeid_t               getSigCdr ( ) const { return cdr->getRoot( )->getCid( ); }

  inline friend ostream &  operator<<( ostream & os, Enode * e )    { assert( e ); e->print( os ); return os; }

  struct idLessThan
  {
    inline bool operator( )( Enode * x, Enode * y ) 
    {   
      return (x->getCar( )->getId( ) <  y->getCar( )->getId( )) 
	  || (x->getCar( )->getId( ) == y->getCar( )->getId( ) && x->getCdr( )->getId( ) < y->getCdr( )->getId( ) ); 
    }   
  };  

  struct cidLessThan
  {
    inline bool operator( )( Enode * x, Enode * y ) 
    {   
      if ( x == y ) return false;
      if ( x->isEnil( ) ) return true;
      if ( y->isEnil( ) ) return false;
      return (x->getCar( )->getCid( ) <  y->getCar( )->getCid( )) 
	  || (x->getCar( )->getCid( ) == y->getCar( )->getCid( ) && x->getCdr( )->getCid( ) < y->getCdr( )->getCid( ) ); 
    }   
  };  

private:
  //
  // Standard informations for terms
  //
  const enodeid_t   id;         // Node unique identifier
  uint32_t          properties; // Contains all the properties of this node (see EnodeTypes.h for bitfields definition)
  Enode *           car;        // For car / defs
  Enode *           cdr;        // For cdr
  union {
  CongData *        cong_data;  // For terms and lists that go in the congruence
  SymbData *        symb_data;  // For symbols/numbers
  };                
  AtomData *        atom_data;  // For atom terms only
                    
  inline bool       hasSymbolId    ( const enodeid_t id ) const { assert( isTerm( ) ); return car->getId( ) == id; }
};

inline const Real & Enode::getValue ( ) const 
{ 
  assert( isTerm( ) || isNumb( ) );
  if ( isTerm( ) )
  {
    assert( isTerm( ) );
    assert( cong_data );
    assert( cong_data->term_data );
    assert( cong_data->term_data->value );
    return *(cong_data->term_data->value); 
  }
  
  assert( symb_data );
  return *(symb_data->value);
}

inline Enode * Enode::getRoot ( ) const 
{ 
  assert( !isDef( ) ); 
  if ( (isTerm( ) || isList( )) && cong_data != NULL ) 
    return cong_data->root;
  return const_cast<Enode *>(this);
}

inline enodeid_t Enode::getCid ( ) const 
{ 
  assert( !isDef( ) );
  if ( (isTerm( ) || isList( )) && cong_data )
    return cong_data->cid;
  return id;
}

inline Enode * Enode::getConstant ( ) const 
{ 
  assert( isTerm( ) || isList( ) ); 
  assert( cong_data ); 
  if ( isTerm( ) ) 
  {
    assert( cong_data );
    assert( cong_data->term_data );
    return cong_data->term_data->constant;
  }

  return NULL;
}

//
// enode is a literal if it is 
// an atom or a negated atom
//
inline bool Enode::isLit( )
{
  if ( !isTerm( ) ) return false;
  if ( isAtom( ) ) return true;          
  // if ( car->getArity( ) != ENODE_ARITY_1 ) return false;
  if ( getArity( ) != 1 ) return false;
  Enode * arg = get1st( );
  return isNot( ) && arg->isAtom( );   
}
//
// enode is an atom if it has boolean type and
// it is not a boolean operator. Constants true
// and false are considered atoms
//
inline bool Enode::isAtom( )
{
  if ( !isTerm( ) ) return false;
  if ( !isDTypeBool( ) ) return false; 
  if ( isBooleanOperator( ) ) return false; 

  return true;
}
//
// enode is a tatom if it has boolean type and
// it is not a boolean operator nor a boolean variable
// nor constants true and false.
//
inline bool Enode::isTAtom( )
{
  if ( !isTerm( ) ) return false;
  if ( !isAtom( ) ) return false;
  if ( isVar( ) )   return false;
  if ( isTrue( ) )  return false;
  if ( isFalse( ) ) return false;
  return true;
}
//
// enode is a literal if it is 
// an atom or a negated atom
//
inline bool Enode::isTLit( )
{
  if ( !isTerm( ) ) return false;
  if ( isTAtom( ) ) return true;          
  if ( getArity( ) != 1 ) return false;
  Enode * arg = get1st( );
  return isNot( ) && arg->isTAtom( );   
}

inline bool Enode::isVar( )
{
  if ( car->getId( )    <= ENODE_ID_LAST ) return false;     // If the code is predefined, is not a variable
  if ( isConstant( ) ) return false;                         // If it's a constant is not a variable
  if ( !isTerm( ) ) return false;		             // If is not a term is not a variable
  if ( getArity( ) != 0 ) return false;
  return car->isSymb( );	                             // Final check 
}

inline bool Enode::isConstant( )
{
  if ( !isTerm( ) ) return false;		         // Check is a term
  return isTrue( ) || isFalse( ) || car->isNumb( );      // Only numbers, true, false are constant
}

inline bool Enode::isBooleanOperator( )
{
  return isAnd( ) || isOr( )  || isNot( ) 
      || isIff( ) || isXor( ) || isImplies( ) 
      || isIfthenelse( );
}

inline Enode * Enode::get1st ( ) const
{
  assert( isTerm( ) );
  assert( getArity( ) > 0 );
  return getCdr( )->getCar( );
}

inline Enode * Enode::get2nd ( ) const
{
  assert( isTerm( ) );
  assert( getArity( ) > 1 );
  return getCdr( )->getCdr( )->getCar( );
}

inline Enode * Enode::get3rd ( ) const
{
  assert( isTerm( ) );
  assert( getArity( ) > 2 );
  return getCdr( )->getCdr( )->getCdr( )->getCar( );
}

inline bool Enode::isExtract ( int * omsb, int * olsb ) 
{ 
  assert( isTerm( ) );
  if ( !isExtract( ) ) return false;
  assert( isDTypeBitVec( ) );
  *olsb = car->symb_data->lsb;
  *omsb = *olsb + getWidth( ) - 1; 
  return true;
}

inline bool Enode::isSignExtend ( int * i ) 
{ 
  assert( isTerm( ) );
  return sscanf( car->getName( ), "sign_extend[%d]", i ) == 1; 
}

inline unsigned Enode::sizeInMem( ) const 
{
  unsigned size = sizeof( Enode );
  if ( isSymb( ) )
  {
    assert( symb_data );
    assert( symb_data->name );
    size += sizeof( SymbData ) + strlen( symb_data->name );
  }
  if ( isNumb( ) )
  {
    assert( symb_data );
    assert( symb_data->name );
    assert( symb_data->value );
    size += sizeof( SymbData ) + strlen( symb_data->name ) + sizeof( Real );
  }
  if ( atom_data ) size += sizeof( AtomData );
  if ( cong_data ) 
  {
    size += sizeof( CongData );
    if ( cong_data->term_data ) size += sizeof( TermData );
  }
  return size;
}

inline void Enode::allocCongData( ) 
{ 
  assert( isTerm( ) || isList( ) );
  assert( cong_data == NULL ); 
  cong_data = new CongData( id, const_cast<Enode *>(this) ); 
  if ( isTerm( ) )
    cong_data->term_data = new TermData( const_cast<Enode *>(this) );
}

#endif
