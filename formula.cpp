/*
 * formula.cpp
 *
 * Definitions for formulas.
 *
 */

#include <cassert>
#include <cctype>
#include <stack>

#include "formula.h"


using namespace std;

// Members of class Formula.

Formula::Formula(Formula::opType t, vector<Formula *>& vfml)
{
  assert(t == ANDN || t == ORN);
  assert(vfml.size() >= 2);

  op = t;
  left = right = NULL;
  atom = "";
  fmls = vfml;
}

Formula::Formula(Formula::opType t, Formula *l, Formula *r)
{
  assert(t != NOT && t != ATOM);

  op = t;
  left = l;
  right = r;
  atom = "";
}

Formula::Formula(Formula::opType t, Formula *r)
{
  assert (t == NOT);

  op = t;
  left = NULL;
  right = r;
  atom = "";
}

Formula::Formula(const string& a)
{
  op = ATOM;
  left = right = NULL;
  atom = a;
}

Formula::Formula(const Formula& rhs)
{
  if (&rhs == this)
    return;
  
  op = rhs.op;
  if (rhs.left)
    left = new Formula(*(rhs.left));
  else
    left = NULL;
  if (rhs.right)
    right = new Formula(*(rhs.right));
  else
    right = NULL;

  for (unsigned int i = 0; i < rhs.fmls.size(); i++)
    fmls.push_back(new Formula(*(rhs.fmls[i])));

  atom = rhs.atom;
}

Formula::~Formula()
{
  if (left)
    delete left;
  if (right)
    delete right;
  for (unsigned int i = 0; i < fmls.size(); i++)
    delete fmls[i];
}

string Formula::toString() const
{
  string s;
  switch (op) {
  case ATOM:
    s = atom;
    break;
  case NOT:
    s = "(!" + right->toString() + ")";
    break;
  case OR:
    s = "(" + left->toString() + "|" + right->toString() + ")";
    break;
  case AND:
    s =  "(" + left->toString() + "&" + right->toString() + ")";
    break;
  case IMPLIES:
    s = "(" + left->toString() + "->" + right->toString() + ")";
    break;
  case ANDN:
    s = "(" + fmls[0]->toString();
    for (unsigned int i = 1; i < fmls.size(); i++)
      s += "&" + fmls[i]->toString();
    s += ")";
    break;
  case ORN:
    s = "(" + fmls[0]->toString();
    for (unsigned int i = 1; i < fmls.size(); i++)
      s += "|" + fmls[i]->toString();
    s += ")";
    break;
  }
  return s;
}

unsigned int Formula::size(bool count_atoms) const
{
  switch (op) {
  case ATOM:
    if (count_atoms)
      return 1;
    else
      return 0;
  case NOT:
    return 1 + right->size(count_atoms);
  case OR: case AND: case IMPLIES:
    return 1 + left->size(count_atoms) + right->size(count_atoms);
  case ORN: case ANDN:
    {
      unsigned int sz = 1;
      for (unsigned int i = 0; i < fmls.size(); i++)
	sz += fmls[i]->size(count_atoms);
      return sz;
    }
  }

  return 0;
}

int Formula::value(map<string, int>& valuation) const
{
  switch (op) {
  case ATOM:
    {
      if (valuation.find(atom) == valuation.end())
	return -1;
      else
	return valuation[atom];
    }
    break;
  case NOT:
    {
      int valr = right->value(valuation);
      if (valr == -1)
	return -1;
      else
	return (valr ? 0 : 1);
    }
    break;
  case OR:
    {
      int vl, vr;
      vl = left->value(valuation);
      vr = right->value(valuation);
      if (vl == 1 || vr == 1) return 1;
      if (vl == 0 && vr == 0) return 0;
      return -1;
    }
    break;
  case AND:
    {
      int vl, vr;
      vl = left->value(valuation);
      vr = right->value(valuation);
      if (vl == 0 || vr == 0) return 0;
      if (vl == 1 && vr == 1) return 1;
      return -1;
    }
    break;
  case IMPLIES:
    {
      int vl, vr;
      vl = left->value(valuation);
      vr = right->value(valuation);
      if (vl == 0 || vr == 1) return 1;
      if (vl == 1 && vr == 0) return 0;
      return -1;
    }
    break;
  case ORN:
    {
      unsigned int j, cu = 0, ct = 0, cf = 0;
      for (j = 0; j < fmls.size(); j++) {
	int vj = fmls[j]->value(valuation);
	if (vj == -1) cu++;
	else if (vj == 0) cf++;
	else ct++;
      }
      if (ct > 0) return 1;
      if (cf == fmls.size()) return 0;
      return -1;
    }
  case ANDN:
    {
      unsigned int j, cu = 0, ct = 0, cf = 0;
      for (j = 0; j < fmls.size(); j++) {
	int vj = fmls[j]->value(valuation);
	if (vj == -1) cu++;
	else if (vj == 0) cf++;
	else ct++;
      }
      if (cf > 0) return 0;
      if (ct == fmls.size()) return 1;
      return -1;
    }
  }
  return -1;
}

int Formula::polarity(const string& str) const
{
  switch (op) {
  case ATOM:
    {
      if (str == atom)
	return 1;
      else
	return -1;
    }
    break;
  case NOT:
    {
      int p = right->polarity(str);
      if (p == 1) return 0;
      if (p == 0) return 1;
      if (p == -1) return -1;
      if (p == 2) return 2;
    }
    break;
  case OR: case AND:
    {
      int p1 = left->polarity(str);
      int p2 = right->polarity(str);
      if (p1 == -1 && p2 == -1) return -1;
      if (p1 == 2 || p2 == 2) return 2;
      if (p1 == 1 && p2 == 0) return 2;
      if (p1 == 0 && p2 == 1) return 2;
      if (p1 != -1) return p1;
      if (p2 != -1) return p2;
    }
    break;
  case IMPLIES:
    {
      int p1 = left->polarity(str);
      int p2 = right->polarity(str);
      if (p1 == -1 && p2 == -1) return -1;
      if (p1 == 2 || p2 == 2) return 2;
      if (p1 == p2) return 2;
      if (p1 != -1) return !p1;
      if (p2 != -1) return p2;
    }
    break;
  case ORN: case ANDN:
    {
      unsigned int nneg = 0, nnone = 0, npos = 0, nboth = 0;
      for (unsigned int i = 0; i < fmls.size(); i++) {
	int p = fmls[i]->polarity(str);
	if (p == -1) nnone++;
	else if (p == 0) nneg++;
	else if (p == 1) npos++;
	else nboth++;
      }
      if (nnone == fmls.size()) return -1;
      if (nboth > 0) return 2;
      if (nneg > 0 && npos > 0) return 2;
      if (nneg > 0) return 0;
      if (npos > 0) return 1;
    }
  }
  return -1;
}

unsigned int Formula::atomsIn(const map<string, int>& valuation) const
{
  switch (op) {
  case ATOM:
    {
      if (valuation.find(atom) != valuation.end())
	return 1;
      else
	return 0;
    }
    break;
  case NOT:
    {
      return right->atomsIn(valuation);
    }
    break;
  case OR: case AND: case IMPLIES:
    {
      return left->atomsIn(valuation) + right->atomsIn(valuation);
    }
    break;
  case ORN: case ANDN:
    {
      unsigned int ret = 0;
      for (unsigned int i = 0; i < fmls.size(); i++)
	ret += fmls[i]->atomsIn(valuation);
      return ret;
    }
  }
  return 0;
}

unsigned int Formula::atomsOut(const map<string, int>& valuation) const
{
  switch (op) {
  case ATOM:
    {
      if (valuation.find(atom) == valuation.end())
	return 1;
      else
	return 0;
    }
    break;
  case NOT:
    {
      return right->atomsOut(valuation);
    }
    break;
  case OR: case AND: case IMPLIES:
    {
      return left->atomsOut(valuation) + right->atomsOut(valuation);
    }
    break;
  case ORN: case ANDN:
    {
      unsigned int ret = 0;
      for (unsigned int i = 0; i < fmls.size(); i++)
	ret += fmls[i]->atomsOut(valuation);
      return ret;
    }
  }
  return 0;
}

unsigned int Formula::atomsOut(const set<string>& atomset) const
{
  switch (op) {
  case ATOM:
    {
      if (atomset.find(atom) == atomset.end())
	return 1;
      else
	return 0;
    }
    break;
  case NOT:
    {
      return right->atomsOut(atomset);
    }
    break;
  case OR: case AND: case IMPLIES:
    {
      return left->atomsOut(atomset) + right->atomsOut(atomset);
    }
    break;
  case ORN: case ANDN:
    {
      unsigned int ret = 0;
      for (unsigned int i = 0; i < fmls.size(); i++)
	ret += fmls[i]->atomsOut(atomset);
      return ret;
    }
  }
  return 0;
}

double Formula::distanceFrom(const map<string, int>& valuation,
			     const map<string, int>& atom_dist) const
{
  switch (op) {
  case ATOM:
    {
      double d = 6E+23;
      map<string, int>::const_iterator v;
      for (v = valuation.begin(); v != valuation.end(); v++) {
	string key = atom + "," + v->first;
	map<string, int>::const_iterator it = atom_dist.find(key);
	if (it != atom_dist.end()) {
	  if ((double) it->second < d)
	    d = (double) it->second;
	}
      }
      return d;
    }
    break;
  case NOT:
    {
      return right->distanceFrom(valuation, atom_dist);
    }
    break;
  case OR: case AND: case IMPLIES:
    {
      double l = left->distanceFrom(valuation, atom_dist);
      double r = right->distanceFrom(valuation, atom_dist);
      if (l < r)
	return l;
      return r;
    }
    break;
  case ORN: case ANDN:
    {
      double min = 6E+23;
      for (unsigned int i = 0; i < fmls.size(); i++)
	if (fmls[i]->distanceFrom(valuation, atom_dist) < min)
	  min = fmls[i]->distanceFrom(valuation, atom_dist);
      return min;
    }
  }
  return 6E+23;
}


// Utility functions

// Used only in parsing
enum parsed_type {PARSE_OPEN, PARSE_OPER, PARSE_FORM};
struct parsed_item {
  parsed_type type;
  Formula *formula;
  Formula::opType op;
};

// Parse a formula from a string. The formula (and its subformulas
// except atoms) must be enclosed in parenthesis. Returns a newly
// allocated pointer to the formula represented by the string or a
// null pointer if the string is not a valid formula.
Formula *parse(const string& s)
{
  
  Formula *retval;
  unsigned int i;
  stack<parsed_item> S;

  for (i = 0; i < s.size(); ) {
    parsed_item item;
    if (s[i] == ' ')
      i++;
    else if (s[i] == '(') {
      item.type = PARSE_OPEN;
      S.push(item);
      i++;
    }
    else if (isalnum(s[i])) {
      string a;
      while (i < s.size() && (isalnum(s[i]) || s[i] == '_' || s[i] == ',')) {
	a += s[i];
	i++;
      }
      item.type = PARSE_FORM;
      if (!S.empty() && S.top().type == PARSE_OPER &&
	  S.top().op == Formula::NOT) {
	item.formula = new Formula(Formula::NOT, new Formula(a));
	S.pop();
      }
      else
	item.formula = new Formula(a);
      S.push(item);
    }
    else if (s[i] == '!') {
      item.type = PARSE_OPER;
      item.op = Formula::NOT;
      S.push(item);
      i++;
    }
    else if (s[i] == '&') {
      item.type = PARSE_OPER;
      item.op = Formula::AND;
      S.push(item);
      i++;
    }
    else if (s[i] == '|') {
      item.type = PARSE_OPER;
      item.op = Formula::OR;
      S.push(item);
      i++;
    }
    else if (i+1 < s.size() && s[i] == '-' && s[i+1] == '>') {
      item.type = PARSE_OPER;
      item.op = Formula::IMPLIES;
      S.push(item);
      i = i + 2;
    }
    else if (s[i] == ')') {
      Formula *result;

      assert(!S.empty() && S.top().type == PARSE_FORM);

      vector<Formula *> fmls;
//        Formula *right = S.top().formula;
      fmls.push_back(S.top().formula);
      S.pop();

      assert(!S.empty() &&
	     (S.top().type == PARSE_OPEN || S.top().type == PARSE_OPER));
      
      Formula::opType op = S.top().op;
      while (S.top().type == PARSE_OPER) {
	assert(op == S.top().op);
	S.pop();

	assert(!S.empty() && S.top().type == PARSE_FORM);

	fmls.insert(fmls.begin(), S.top().formula);
	S.pop();
      }

      assert(!S.empty() && S.top().type == PARSE_OPEN);

      S.pop();

      if (fmls.size() > 2) {
	assert (op == Formula::AND || op == Formula::OR);
	if (op == Formula::AND)
	  op = Formula::ANDN;
	else
	  op = Formula::ORN;
	result = new Formula(op, fmls);
      }
      else if (fmls.size() == 2)
	result = new Formula(op, fmls[0], fmls[1]);
      else // fmls.size() == 1
	result = fmls[0];
	

//        if (S.top().type == PARSE_OPER) {
//  	Formula::opType op = S.top().op;
//  	S.pop();

//  	assert(!S.empty() && S.top().type == PARSE_FORM);

//  	Formula *left = S.top().formula;
//  	S.pop();
//  	result = new Formula(op, left, right);
//        }
//        else
//  	result = right;

//        assert(!S.empty() && S.top().type == PARSE_OPEN);

//        S.pop();
      
      item.type = PARSE_FORM;
      if (!S.empty() && S.top().type == PARSE_OPER &&
	  S.top().op == Formula::NOT) {
	item.formula = new Formula(Formula::NOT, result);
	S.pop();
      }
      else
	item.formula = result;
      S.push(item);
      i++;
    }
  }

  assert(!S.empty() && S.top().type == PARSE_FORM);
  retval = S.top().formula;
  S.pop();
  assert(S.empty());
  
  return retval;
}
