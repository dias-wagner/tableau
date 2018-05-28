/*
 * formula.h
 *
 * Class declarations for formulas.
 *
 */

#ifndef __FORMULA_H__
#define __FORMULA_H__

#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// Encapsulates a formula.
class Formula
{
 public:
  // Types of operators.
  enum opType {AND, ANDN, OR, ORN, IMPLIES, NOT, ATOM};

  // Constructor for opType = ANDN or ORN.
  Formula(opType t, vector<Formula *>& vfml);
  // Constructor for opType = AND, OR or IMPLIES.
  Formula(opType t, Formula *l, Formula *r);
  // Constructor for opType = NOT.
  Formula(opType t, Formula *r);
  // Constructor for opType = ATOM.
  Formula(const string& a);

  // Copy constructor.
  Formula(const Formula& rhs);

  // Destructor. Destroys all subformulas.
  ~Formula();

  // Returns a string representation of the formula.
  string toString() const;

  // Returns the size of the formula (atom ocurrences + operator ocurrences)
  unsigned int size(bool count_atoms = true) const;

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

  // If operator == ANDN or ORN, then the elements of vector fmls must
  // be not NULL, and the string atom and the pointers left and right
  // have no significance.
  // If operator == AND, OR or IMPLIES, then the pointers left and
  // right must not be NULL, and the string atom and the vector fmls
  // have no significance.
  // If operator == NOT, then the pointer right must not be NULL and
  // the pointer left, the string atom and the vector fmls have no
  // significance.
  // If operator == ATOM, then the string atom must not be empty and
  // the pointers left and right and the vector fmls have no
  // significance.

  // Type of operator.
  opType op;

  // Atom.
  string atom;

  // Left formula.
  Formula *left;

  // Right formula.
  Formula *right;

  // Vector of formulas for operators ANDN and ORN.
  vector<Formula *> fmls;
};


// Utility functions

// Parse a formula from a string. The formula (and its subformulas
// except atoms) must be enclosed in parenthesis. Returns a newly
// allocated pointer to the formula represented by the string or a
// null pointer if the string is not a valid formula.
Formula *parse(const string& s);

#endif
