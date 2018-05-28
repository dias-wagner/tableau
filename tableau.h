/*****************************************************************************
 * tableau.h
 *
 * Class declarations for the generic tableau.
 *****************************************************************************/

#ifndef __TABLEAU_H__
#define __TABLEAU_H__

#include <string>
#include <vector>

#include "formula.h"


//////////////////////////////////////////////////////////////////////////////
// Encapsulates a signed formula.
//////////////////////////////////////////////////////////////////////////////

class SignedFormula
{
 public:
  enum Sign {S_F, S_T};
  SignedFormula(Sign s, Formula *fml);

  // Formula types.
  enum fmlType {ALPHA, BETA, LITERAL};

  // Returns the type of the formula.
  fmlType type() const;

  // Returns a string representation of the formula.
  string toString() const;

  // Returns the value of the formula according to a valuation. Returns:
  // -1: undefined
  //  0: false
  //  1: true
  int value(map<string, int>& valuation) const;

  // Returns the polarity of the specified atom in the formula. Returns:
  // -1: no ocurrences of this atom on the formula
  //  0: negative polarity
  //  1: positive polarity
  //  2: ocurrences of both polarities
  int polarity(const string& atom) const;  

  // Counts the number of ocurrences of atoms in the formula that are
  // described in valuation.
  unsigned int atomsIn(const map<string, int>& valuation) const;

  // Counts the number of ocurrences of atoms in the formula that are
  // not described in valuation.
  unsigned int atomsOut(const map<string, int>& valuation) const;

  // Counts the number of ocurrences of atoms in the formula that are
  // not described in the atom set.
  unsigned int atomsOut(const set<string>& atomset) const;

  // Returns the distance between the formula and the set of atoms in
  // the valuation. This distance is the minimum distance between an
  // atom occurring in the formula and an atom described in the
  // valuation.
  double distanceFrom(const map<string, int>& valuation,
		      const map<string, int>& atom_dist) const;

  Sign sign;
  Formula *formula;
  fmlType ty;
};


//////////////////////////////////////////////////////////////////////////////
// A rule is a pointer to a function returning bool and having the
// input and output formulas as parameters.
//////////////////////////////////////////////////////////////////////////////

typedef bool (*Rule)(const vector<SignedFormula *>&, vector<SignedFormula *>&);


//////////////////////////////////////////////////////////////////////////////
// Encapsulates a generic tableau strategy.
//////////////////////////////////////////////////////////////////////////////

class TableauStrategy
{
 public:
  TableauStrategy();
  virtual ~TableauStrategy();
  
  // initializes the object with the items of a tableau. Returns true
  // if the tableau is already close.  This method also uses the
  // Floyd-Warshall algorithm to calculate the minimum distance
  // between each pair of atoms.
  bool init(const string& tableau_id,
	    vector<SignedFormula *> *items,
	    vector<SignedFormula *> *alphas,
	    vector<SignedFormula *> *betas,
	    vector<SignedFormula *> *lits);

  // classifies the formulas in the vector items. Puts them into the
  // corresponding vector: alphas, betas or lits. Returns true if the
  // tableau is closed.
  bool classify(unsigned int& index);

  // choose the next alpha formula to be analysed. Returns the index
  // of the formula in the vector alphas.
  virtual unsigned int chooseAlpha();
  // choose the next beta formula to be analysed.
  virtual unsigned int chooseBeta();
  // chooses the next rule to be applied (-1=none; 0=alpha; 1=beta).
  virtual int nextRule() = 0;
  
 protected:
  // Initializes M and atom2node, which will be used in the F-W algorithm.
  void init_floyd_warshall(Formula *fml, int nt, int nf,
			   int **M, map<string, set<int> >& atom2node,
			   int parent, int *next_node, int level);

  // the id of the tableau associated to this strategy object.
  string id;

  vector<SignedFormula *> *_items;
  
  vector<SignedFormula *> *_alphas;
  vector<SignedFormula *> *_betas;
  vector<SignedFormula *> *_lits;

  // if _mlits[str] = 1, we have F str
  // if _mlits[str] = 2, we have T str
  // if _mlits[str] = 3, we have both
  map<string, int> _mlits;

  // Map of distances between connective nodes, calculated by the
  // Floyd-Warshall algorithm in the first call to the init()
  // method. For a pair of atoms a and b, the key is "a,b".
  map<string, int> _atom_dist;

  // Maximum atom distance.
  int _max_atom_dist;

  // indicates if the F-W algorithm was already run in this object.
  bool _fw_done;
};


//////////////////////////////////////////////////////////////////////////////
// Encapsulates a generic tableau.
//////////////////////////////////////////////////////////////////////////////

class Tableau
{
 public:
  Tableau(const string& id, SignedFormula *fml,
	  Tableau *parent = NULL);
  Tableau(const string& id, const vector<SignedFormula *>& fmls,
	  Tableau *parent = NULL);
  virtual ~Tableau();

  // Sets the strategy object.
  virtual void setStrategy(TableauStrategy *strategy);

  // String representation of the tableau.
  virtual string toString(int level=0) const;

  // Tries to close the tableau (returns true if successful).
  virtual bool close() = 0;

  // Returns the total number of nodes of the tableau (including children).
  unsigned int countNodes();

  // Returns the total number of formulae of the tableau (including children).
  unsigned int countFormulae();

 protected:
  // Applies the index'th rule of the tableau. Returns true if successful.
  bool applyRule(unsigned int index,
		 const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out);

  // Create a child tableau
  virtual void createChild(const string& id, SignedFormula *fml) = 0;

  // Formulae of the tableau.
  vector<SignedFormula *> _items;
  // Child tableaux.
  vector<Tableau *> _children;
  // Parent tableau node.
  Tableau *_parent;
  // Rules of the tableau.
  vector<Rule> _rules;

  // Id of the tableau.
  string _id;

 private:
  // Strategy of the tableau.
  TableauStrategy *_strategy;
};

#endif
