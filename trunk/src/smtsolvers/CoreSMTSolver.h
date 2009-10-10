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

/****************************************************************************************[Solver.C]
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef MINISATSMTSOLVER_H
#define MINISATSMTSOLVER_H

#define CACHE_POLARITY     0
#define LAZY_COMMUNICATION 1

#include "SMTSolver.h"

#include <cstdio>

#include "Vec.h"
#include "Heap.h"
#include "Alg.h"

#include "SolverTypes.h"
// Added
#include "LA.h"

//=================================================================================================
// Solver -- the main class:

class CoreSMTSolver : public SMTSolver 
{
public:

    // Constructor/Destructor:
    //
    CoreSMTSolver( Egraph &, SMTConfig & );
    ~CoreSMTSolver();

#if NEW_SIMPLIFICATIONS
    vector< LAExpression * > top_level_eqs;
    bool                     doing_t_simp;
#endif

    // Problem specification:
    //
    Var     newVar    (bool polarity = true, bool dvar = true); // Add a new variable with parameters specifying variable mode.
    bool    addClause (vec<Lit>& ps);                           // Add a clause to the solver. NOTE! 'ps' may be shrunk by this method!

    // Solving:
    //
    bool    simplify     ();                        // Removes already satisfied clauses.
    bool    solve        (const vec<Lit>& assumps); // Search for a model that respects a given set of assumptions.
    bool    solve        ();                        // Search without assumptions.
    bool    okay         () const;                  // FALSE means solver is in a conflicting state

    // Variable mode:
    // 
    void    setPolarity    (Var v, bool b); // Declare which polarity the decision heuristic should use for a variable. Requires mode 'polarity_user'.
    void    setDecisionVar (Var v, bool b); // Declare if a variable should be eligible for selection in the decision heuristic.

    // Read state:
    //
    lbool   value      (Var x) const;       // The current value of a variable.
    lbool   value      (Lit p) const;       // The current value of a literal.
    lbool   modelValue (Lit p) const;       // The value of a literal in the last model. The last call to solve must have been satisfiable.
    int     nAssigns   ()      const;       // The current number of assigned literals.
    int     nClauses   ()      const;       // The current number of original clauses.
    int     nLearnts   ()      const;       // The current number of learnt clauses.
    int     nVars      ()      const;       // The current number of variables.

//=================================================================================================
// Added Code

    void pushBacktrackPoint ( );
    void popBacktrackPoint  ( );
    void reset              ( );

    inline void restoreOK   ( )       { ok = true; }
    inline bool isOK        ( ) const { return ok; }

// Added Code
//=================================================================================================

    // Extra results: (read-only member variable)
    //
    vec<lbool> model;             // If problem is satisfiable, this vector contains the model (if any).
    vec<Lit>   conflict;          // If problem is unsatisfiable (possibly under assumptions),
                                  // this vector represent the final conflict clause expressed in the assumptions.

    // Mode of operation:
    //
    double    var_decay;          // Inverse of the variable activity decay factor.                                            (default 1 / 0.95)
    double    clause_decay;       // Inverse of the clause activity decay factor.                                              (1 / 0.999)
    double    random_var_freq;    // The frequency with which the decision heuristic tries to choose a random variable.        (default 0.02)
    int       restart_first;      // The initial restart limit.                                                                (default 100)
    double    restart_inc;        // The factor with which the restart limit is multiplied in each restart.                    (default 1.5)
    double    learntsize_factor;  // The intitial limit for learnt clauses is a factor of the original clauses.                (default 1 / 3)
    double    learntsize_inc;     // The limit for learnt clauses is multiplied with this factor each restart.                 (default 1.1)
    bool      expensive_ccmin;    // Controls conflict clause minimization.                                                    (default TRUE)
    int       polarity_mode;      // Controls which polarity the decision heuristic chooses. See enum below for allowed modes. (default polarity_false)
    int       verbosity;          // Verbosity level. 0=silent, 1=some progress report                                         (default 0)

    enum { polarity_true = 0, polarity_false = 1, polarity_user = 2, polarity_rnd = 3 };

    // Statistics: (read-only member variable)
    //
    uint64_t starts, decisions, rnd_decisions, propagations, conflicts;
    uint64_t clauses_literals, learnts_literals, max_literals, tot_literals;

protected:

    // Helper structures:
    //
    struct VarOrderLt {
        const vec<double>&  activity;
        bool operator () (Var x, Var y) const { return activity[x] > activity[y]; }
        VarOrderLt(const vec<double>&  act) : activity(act) { }
    };

    friend class VarFilter;
    struct VarFilter {
        const CoreSMTSolver& s;
        VarFilter(const CoreSMTSolver& _s) : s(_s) {}
        bool operator()(Var v) const { return toLbool(s.assigns[v]) == l_Undef && s.decision_var[v]; }
    };

//=================================================================================================
// Added Code

    //
    // Data structures required for communicating conflicts and deductions
    // incrementality, backtrackability ...
    //
    enum oper_t { NEWVAR, NEWUNIT, NEWCLAUSE };

    vector< size_t >               undo_stack_size;     // Keep track of stack_oper size
    vector< int >                  undo_trail_size;     // Keep track of trail size
    vector< oper_t >		   undo_stack_oper;     // Keep track of operations
    vector< void * >		   undo_stack_elem;     // Keep track of aux info

// Added Code
//=================================================================================================

    // Solver state:
    //
    bool                ok;               // If FALSE, the constraints are already unsatisfiable. No part of the solver state may be used!
    vec<Clause*>        clauses;          // List of problem clauses.
    vec<Clause*>        learnts;          // List of learnt clauses.
    double              cla_inc;          // Amount to bump next clause with.
    vec<double>         activity;         // A heuristic measurement of the activity of a variable.
    double              var_inc;          // Amount to bump next variable with.
    vec<vec<Clause*> >  watches;          // 'watches[lit]' is a list of constraints watching 'lit' (will go there if literal becomes true).
    vec<char>           assigns;          // The current assignments (lbool:s stored as char:s).
    vec<char>           polarity;         // The preferred polarity of each variable.
    vec<char>           decision_var;     // Declares if a variable is eligible for selection in the decision heuristic.
    vec<Lit>            trail;            // Assignment stack; stores all assigments made in the order they were made.
    vec<int>            trail_lim;        // Separator indices for different decision levels in 'trail'.
    vec<Clause*>        reason;           // 'reason[var]' is the clause that implied the variables current value, or 'NULL' if none.
    vec<int>            level;            // 'level[var]' contains the level at which the assignment was made.
    int                 qhead;            // Head of queue (as index into the trail -- no more explicit propagation queue in MiniSat).
    int                 simpDB_assigns;   // Number of top-level assignments since last execution of 'simplify()'.
    int64_t             simpDB_props;     // Remaining number of propagations that must be made before next execution of 'simplify()'.
    vec<Lit>            assumptions;      // Current set of assumptions provided to solve by the user.
    Heap<VarOrderLt>    order_heap;       // A priority queue of variables ordered with respect to the variable activity.
    double              random_seed;      // Used by the random variable selection.
    double              progress_estimate;// Set by 'search()'.
    bool                remove_satisfied; // Indicates whether possibly inefficient linear scan for satisfied clauses should be performed in 'simplify'.

#if CACHE_POLARITY
    vec<char>           prev_polarity;    // The previous polarity of each variable.
#endif

    // Temporaries (to reduce allocation overhead). Each variable is prefixed by the method in which it is
    // used, exept 'seen' wich is used in several places.
    //
    vec<char>           seen;
    vec<Lit>            analyze_stack;
    vec<Lit>            analyze_toclear;
    vec<Lit>            add_tmp;

    // Main internal methods:
    //
    void     insertVarOrder   (Var x);                                                 // Insert a variable in the decision order priority queue.
    Lit      pickBranchLit    (int polarity_mode, double random_var_freq);             // Return the next decision variable.
    void     newDecisionLevel ();                                                      // Begins a new decision level.
    void     uncheckedEnqueue (Lit p, Clause* from = NULL);                            // Enqueue a literal. Assumes value of literal is undefined.
    bool     enqueue          (Lit p, Clause* from = NULL);                            // Test if fact 'p' contradicts current state, enqueue otherwise.
    Clause*  propagate        ();                                                      // Perform unit propagation. Returns possibly conflicting clause.
    void     cancelUntil      (int level);                                             // Backtrack until a certain level.
    void     analyze          (Clause* confl, vec<Lit>& out_learnt, int& out_btlevel); // (bt = backtrack)
    void     analyzeFinal     (Lit p, vec<Lit>& out_conflict);                         // COULD THIS BE IMPLEMENTED BY THE ORDINARIY "analyze" BY SOME REASONABLE GENERALIZATION?
    bool     litRedundant     (Lit p, uint32_t abstract_levels);                       // (helper method for 'analyze()')
    lbool    search           (int nof_conflicts, int nof_learnts);                    // Search for a given number of conflicts.
    void     reduceDB         ();                                                      // Reduce the set of learnt clauses.
    void     removeSatisfied  (vec<Clause*>& cs);                                      // Shrink 'cs' to contain only non-satisfied clauses.

    // Maintaining Variable/Clause activity:
    //
    void     varDecayActivity ();                      // Decay all variables with the specified factor. Implemented by increasing the 'bump' value instead.
    void     varBumpActivity  (Var v);                 // Increase a variable with the current 'bump' value.

    // Added Line
    void     boolVarDecActivity( );                    // Decrease boolean atoms activity
    void     claDecayActivity ();                      // Decay all clauses with the specified factor. Implemented by increasing the 'bump' value instead.
    void     claBumpActivity  (Clause& c);             // Increase a clause with the current 'bump' value.

    // Operations on clauses:
    //
    void     attachClause     (Clause& c);             // Attach a clause to watcher lists.
    void     detachClause     (Clause& c);             // Detach a clause to watcher lists.
    void     removeClause     (Clause& c);             // Detach and free a clause.
    bool     locked           (const Clause& c) const; // Returns TRUE if a clause is a reason for some implication in the current state.
    bool     satisfied        (const Clause& c) const; // Returns TRUE if a clause is satisfied in the current state.

    // Misc:
    //
    int      decisionLevel    ()      const; // Gives the current decisionlevel.
    uint32_t abstractLevel    (Var x) const; // Used to represent an abstraction of sets of decision levels.
    double   progressEstimate ()      const; // DELETE THIS ?? IT'S NOT VERY USEFUL ...

    // Debug:
    void     printLit         (Lit l);
    template<class C>
    void     printClause      (const C& c);
    // Added Line
    template<class C>
    void     printSMTClause   ( ostream & os, const C& c );
    void     verifyModel      ();
    void     checkLiteralCount();

    // Static helpers:
    //

    // Returns a random float 0 <= x < 1. Seed must never be 0.
    static inline double drand(double& seed) {
        seed *= 1389796;
        int q = (int)(seed / 2147483647);
        seed -= (double)q * 2147483647;
        return seed / 2147483647; }

    // Returns a random integer 0 <= x < size. Seed must never be 0.
    static inline int irand(double& seed, int size) {
        return (int)(drand(seed) * size); }


//=================================================================================================
// Added Code

public:

    bool  addSMTClause      ( vector< Enode * > & ); // Adds clause to SAT solver
    lbool smtSolve          ( );                     // Solve
#ifndef SMTCOMP
    void   printModel       ( ostream & );           // Prints model
#endif

protected:

#ifdef STATISTICS
    void   printStatistics     ( ostream & );   // Prints statistics
#endif
    void   printTrail          ( );             // Prints the trail (debugging)
    int    checkTheory         ( bool );        // Checks consistency in theory
    int    deduceTheory        ( );             // Perform theory-deductions
    void   cancelUntilVar      ( Var );         // Backtrack until a certain variable
    void   cancelUntilVarTempInit ( Var );      // Backtrack until a certain variable
    void   cancelUntilVarTempDone ( );          // Backtrack until a certain variable
    int    restartNextLimit    ( int );         // Next conflict limit for restart

    Clause *	       fake_clause;             // Fake clause for unprovided reasons
    vec< Clause * >    cleanup;                 // For cleaning up 
    bool	       first_model_found;       // True if we found a first boolean model
    double	       skip_step;               // Steps to skip in calling tsolvers
    long               skipped_calls;           // Calls skipped so far
    long               learnt_t_lemmata;        // T-Lemmata stored during search
    long               perm_learnt_t_lemmata;   // T-Lemmata stored during search
    unsigned           luby_i;                  // Keep track of luby index
    unsigned           luby_k;                  // Keep track of luby k
    vector< unsigned > luby_previous;           // Previously computed luby numbers
    bool               cuvti;                   // For cancelUntilVarTemp
    vec<Lit>           lit_to_restore;          // For cancelUntilVarTemp
    vec<char>          val_to_restore;          // For cancelUntilVarTemp
    //
    // TODO: move more data in STATISTICS
    //
#ifdef STATISTICS
    double             preproc_time;
    double             tsolvers_time;
    unsigned           elim_tvars;
    unsigned           total_tvars;
#endif

// Added Code
//=================================================================================================
};


//=================================================================================================
// Implementation of inline methods:


inline void CoreSMTSolver::insertVarOrder(Var x) {
    if (!order_heap.inHeap(x) && decision_var[x]) order_heap.insert(x); }

inline void CoreSMTSolver::varDecayActivity() { var_inc *= var_decay; }
inline void CoreSMTSolver::varBumpActivity(Var v) {
    if ( (activity[v] += var_inc) > 1e100 ) {
        // Rescale:
        for (int i = 0; i < nVars(); i++)
            activity[i] *= 1e-100;
        var_inc *= 1e-100; }

    // Update order_heap with respect to new activity:
    if (order_heap.inHeap(v))
        order_heap.decrease(v); }

//=================================================================================================
// Added Code

inline void CoreSMTSolver::boolVarDecActivity( ) 
{
#if 1
  for (int i = 2; i < nVars(); i++)
  {
    Enode * e = theory_handler->varToEnode( i );
#if 1
    if ( !e->isVar( ) && !first_model_found )
    {
      activity[i] += e->getWeightInc( ) * var_inc;
      // Update order_heap with respect to new activity:
      if (order_heap.inHeap(i))
	order_heap.decrease(i); 
    }
#else
    if ( e->isVar( ) && !first_model_found )
    {
      activity[i] += var_inc;
      // Update order_heap with respect to new activity:
      if (order_heap.inHeap(i))
	order_heap.decrease(i); 
    }
#endif
  }
#endif
}
	
// Added Code
//=================================================================================================

inline void CoreSMTSolver::claDecayActivity() { cla_inc *= clause_decay; }
inline void CoreSMTSolver::claBumpActivity (Clause& c) {
        if ( (c.activity() += cla_inc) > 1e20 ) {
            // Rescale:
            for (int i = 0; i < learnts.size(); i++)
                learnts[i]->activity() *= 1e-20;
            cla_inc *= 1e-20; } }

inline bool     CoreSMTSolver::enqueue         (Lit p, Clause* from)   { return value(p) != l_Undef ? value(p) != l_False : (uncheckedEnqueue(p, from), true); }
inline bool     CoreSMTSolver::locked          (const Clause& c) const { return reason[var(c[0])] == &c && value(c[0]) == l_True; }
inline void     CoreSMTSolver::newDecisionLevel()                      { trail_lim.push(trail.size()); }

inline int      CoreSMTSolver::decisionLevel ()      const   { return trail_lim.size(); }
inline uint32_t CoreSMTSolver::abstractLevel (Var x) const   { return 1 << (level[x] & 31); }
inline lbool    CoreSMTSolver::value         (Var x) const   { return toLbool(assigns[x]); }
inline lbool    CoreSMTSolver::value         (Lit p) const   { return toLbool(assigns[var(p)]) ^ sign(p); }
inline lbool    CoreSMTSolver::modelValue    (Lit p) const   { return model[var(p)] ^ sign(p); }
inline int      CoreSMTSolver::nAssigns      ()      const   { return trail.size(); }
inline int      CoreSMTSolver::nClauses      ()      const   { return clauses.size(); }
inline int      CoreSMTSolver::nLearnts      ()      const   { return learnts.size(); }
inline int      CoreSMTSolver::nVars         ()      const   { return assigns.size(); }
inline void     CoreSMTSolver::setPolarity   (Var v, bool b) { polarity    [v] = (char)b; }
inline void     CoreSMTSolver::setDecisionVar(Var v, bool b) { decision_var[v] = (char)b; if (b) { insertVarOrder(v); } }
inline bool     CoreSMTSolver::solve         ()              { vec<Lit> tmp; return solve(tmp); }
inline bool     CoreSMTSolver::okay          ()      const   { return ok; }



//=================================================================================================
// Debug + etc:


#define reportf(format, args...) ( fflush(stdout), fprintf(stderr, format, ## args), fflush(stderr) )

static inline void logLit(FILE* f, Lit l)
{
    fprintf(f, "%sx%d", sign(l) ? "~" : "", var(l)+1);
}

static inline void logLits(FILE* f, const vec<Lit>& ls)
{
    fprintf(f, "[ ");
    if (ls.size() > 0){
        logLit(f, ls[0]);
        for (int i = 1; i < ls.size(); i++){
            fprintf(f, ", ");
            logLit(f, ls[i]);
        }
    }
    fprintf(f, "] ");
}

static inline const char* showBool(bool b) { return b ? "true" : "false"; }


// Just like 'assert()' but expression will be evaluated in the release version as well.
static inline void check(bool expr) { (void)expr; assert(expr); }


inline void CoreSMTSolver::printLit(Lit l)
{
    reportf("%s%d:%c:%d", sign(l) ? "-" : " ", var(l)+1, value(l) == l_True ? '1' : (value(l) == l_False ? '0' : 'X'), level[var(l)]);
}


template<class C>
inline void CoreSMTSolver::printClause(const C& c)
{
    for (int i = 0; i < c.size(); i++){
        printLit(c[i]);
        fprintf(stderr, " ");
    }
}

//=================================================================================================
// Added code

template<class C>
inline void CoreSMTSolver::printSMTClause( ostream & os, const C& c )
{
  for (int i = 0; i < c.size(); i++)
  {
    Var v = var(c[i]);
    if ( v <= 1 ) continue;
    Enode * e = theory_handler->varToEnode( v );
    os << (sign(c[i])?"(not ":" ") << e << (sign(c[i])?") ":" ");
  }
}

// Added code
//=================================================================================================

//=================================================================================================

#endif
