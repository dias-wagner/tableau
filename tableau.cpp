/*****************************************************************************
 * tableau.cpp
 *
 * Definitions for the generic tableau.
 *****************************************************************************/

#include <cstdio>
#include <algorithm>
#include <iterator>
#include <string>

#include <cassert>


#include "tableau.h"


//////////////////////////////////////////////////////////////////////////////
// Members of class SignedFormula.
//////////////////////////////////////////////////////////////////////////////

SignedFormula::SignedFormula(SignedFormula::Sign s, Formula *fml)
{
  sign = s;
  formula = fml;

  switch(fml->op) {
  case Formula::NOT:
    ty = ALPHA;
    break;
  case Formula::ANDN: case Formula::AND:
    ty = (s==S_T)?ALPHA:BETA;
    break;
  case Formula::ORN: case Formula::OR: case Formula::IMPLIES:
    ty = (s==S_T)?BETA:ALPHA;
    break;
  default: // ATOM
    ty = LITERAL;
  }
}

SignedFormula::fmlType SignedFormula::type() const
{
  return ty;
}

string SignedFormula::toString() const
{
  string retval;
  if (sign == S_F)
    retval = "F ";
  else
    retval = "T ";

  retval += formula->toString();

  return retval;
}

int SignedFormula::value(map<string, int>& valuation) const
{
  int val = formula->value(valuation);
  if (val == -1)
    return -1;
  else {
    if (sign == S_T)
      return val;
    else
      return (val==1)?0:1;
  }
}

int SignedFormula::polarity(const string& str) const
{
  int p = formula->polarity(str);
  if (p == -1) return -1;
  if (p == 0) return (sign==S_T)?0:1;
  if (p == 1) return (sign==S_T)?1:0;
  if (p == 2) return 2;
  return -1;
}

unsigned int SignedFormula::atomsIn(const map<string, int>& valuation) const
{
  return formula->atomsIn(valuation);
} 

unsigned int SignedFormula::atomsOut(const map<string, int>& valuation) const
{
  return formula->atomsOut(valuation);
} 

unsigned int SignedFormula::atomsOut(const set<string>& atomset) const
{
  return formula->atomsOut(atomset);
} 

double SignedFormula::distanceFrom(const map<string, int>& valuation,
				   const map<string, int>& atom_dist) const
{
  return formula->distanceFrom(valuation, atom_dist);
}

//////////////////////////////////////////////////////////////////////////////
// Members of class TableauStrategy.
//////////////////////////////////////////////////////////////////////////////
TableauStrategy::TableauStrategy()
{
  _items = _alphas = _betas = _lits = NULL;
  _fw_done = false;
}

TableauStrategy::~TableauStrategy() { }


bool TableauStrategy::init(const string& tableau_id,
			   vector<SignedFormula *> *items,
			   vector<SignedFormula *> *alphas,
			   vector<SignedFormula *> *betas,
			   vector<SignedFormula *> *lits)
{
  unsigned int i;

  bool closed = false;
  id = tableau_id;
  _items = items;
  _alphas = alphas;
  _betas = betas;
  _lits = lits;

  _mlits.clear();

  for (i = 0; i < _lits->size(); i++) {
    if ((*_lits)[i]->type() == SignedFormula::LITERAL) {
      string atom = (*_lits)[i]->formula->atom;
      SignedFormula::Sign sign = (*_lits)[i]->sign;
      if (sign == SignedFormula::S_T) {
	if (_mlits.find(atom) == _mlits.end())
	  _mlits[atom] = 2;
	else
	  _mlits[atom] = _mlits[atom] | 2;
      }
      else {
	if (_mlits.find(atom) == _mlits.end())
	  _mlits[atom] = 1;
	else
	  _mlits[atom] = _mlits[atom] | 1;
      }
      if (_mlits[atom] == 3)
	closed = true;
    }
  }

  // *** Floyd-Warshall all pair shortest path algorithm ***

  if (! _fw_done) {    
    vector<Formula *> vt, vf;
    Formula *ft, *ff, *fml;
    
    for (i = 0; i < _items->size(); i++) {
      if ((*_items)[i]->sign == SignedFormula::S_T)
	vt.push_back((*_items)[i]->formula);
      else
	vf.push_back((*_items)[i]->formula);
    }
    
    if (vt.size() == 0) ft = NULL;
    else if (vt.size() == 1) ft = vt[0];
    else if (vt.size() == 2) ft = new Formula(Formula::AND, vt[0], vt[1]);
    else ft = new Formula(Formula::ANDN, vt);
    
    if (vf.size() == 0) ff = NULL;
    else if (vf.size() == 1) ff = vf[0];
    else if (vf.size() == 2) ff = new Formula(Formula::OR, vf[0], vf[1]);
    else ff = new Formula(Formula::ORN, vf);
    
    assert(ft || ff);
    
    if (ft == NULL)
      fml = ff;
    else if (ff == NULL)
      fml = ft;
    else
      fml = new Formula(Formula::IMPLIES, ft, ff);
    
    //    cout << fml->toString() << endl;
    
    unsigned int sz_op = fml->size(false);
    
    unsigned int j;
    
    // Matrix of nodes.
    int **M;
    
    M = (int **) malloc(sz_op * sizeof(int *));
    for (i = 0; i < sz_op; i++) {
      M[i] = (int *) malloc(sz_op * sizeof(int));
      for (j = 0; j < sz_op; j++)
	M[i][j] = 2 * sz_op;
      M[i][i] = 0;
    }

    _max_atom_dist = 2 * sz_op;

    
    // Maps the occurrences of the atoms in the nodes.
    map<string, set<int> > atom2node;
    
    int nextnode = 0;
    init_floyd_warshall(fml, vt.size(), vf.size(),
			M, atom2node, -1, &nextnode, 0);
    
    unsigned int k;

    for (k = 0; k < sz_op; k++) {
      for (i = 0; i < sz_op; i++)
	for (j = 0; j < sz_op; j++) {
	  int dist = M[i][k] + M[k][j];
	  if (dist < M[i][j]) {
	    M[i][j] = dist;
	  }
	}
    }
    
//     // Printing...

//     for (i = 0; i < sz_op; i++) {
//       for (j = 0; j < sz_op; j++)
// 	cout << M[i][j] << " ";
//       cout << endl;
//     }
    
//     cout << endl;
    
//     for (map<string, set<int> >::const_iterator it = atom2node.begin();
// 	 it != atom2node.end(); it++) {
//       cout << it->first << " ";
//       copy(it->second.begin(), it->second.end(),
// 	   ostream_iterator<int>(cout, " "));
//       cout << endl;
//     }

    // Calculating the distance between the atoms. The distance
    // between a and b is the minimum distance between an occurrence
    // of a and an occurrence of b.

    _atom_dist.clear();

    set<string> atomset;

    map<string, set<int> >::const_iterator a, b;
    for (a = atom2node.begin(); a != atom2node.end(); a++) {
      atomset.insert(a->first);
      for (b = a; b != atom2node.end(); b++) {
	if (b == a) {
	  string key(a->first);
	  key.append(",");
	  key.append(a->first);
	  _atom_dist[key] = 0;
	}
	else {
	  set<int>::const_iterator as, bs;
	  int d = M[*a->second.begin()][*b->second.begin()];
	  for (as = a->second.begin(); as != a->second.end(); as++)
	    for (bs = b->second.begin(); bs != b->second.end(); bs++) {
	      if (M[*as][*bs] < d)
		d = M[*as][*bs];
	    }
	  string key1(a->first);
	  key1.append(",");
	  key1.append(b->first);	
	  string key2(b->first);
	  key2.append(",");
	  key2.append(a->first);	
	  _atom_dist[key1] = d;
	  _atom_dist[key2] = d;
	}
      }
    }
    
    set<string>::const_iterator m, n, p;
    for (m = atomset.begin(); m != atomset.end(); m++) {
      for (n = atomset.begin(); n != atomset.end(); n++) {
	for (p = atomset.begin(); p != atomset.end(); p++) {
	  string np(*n + "," + *p), nm(*n + "," + *m), mp(*m + "," + *p); 
	  assert(_atom_dist.find(np) != _atom_dist.end());
	  assert(_atom_dist.find(nm) != _atom_dist.end());
	  assert(_atom_dist.find(mp) != _atom_dist.end());
	  if (_atom_dist[nm] + _atom_dist[mp] < _atom_dist[np])
	    _atom_dist[np] = _atom_dist[nm] + _atom_dist[mp];
	}
      }
    }
    
//     map<string, int>::const_iterator dit;
//     for (dit = _atom_dist.begin(); dit != _atom_dist.end(); dit++)
//       cout << dit->first << " " << dit->second << endl;
    
    // Freeing resources...
    
    for (i = 0; i < sz_op; i++)
      free(M[i]);
    free(M);
    
    _fw_done = true;
  }

  return closed;
}

void TableauStrategy::init_floyd_warshall(Formula *fml, int nt, int nf,
					  int **M,
					  map<string, set<int> >& atom2node,
					  int parent,
					  int *nextnode, int level)
{
  switch (fml->op) {
  case Formula::ATOM:
    {
      if (parent != -1)
	atom2node[fml->atom].insert(parent);
    }
    break;
  case Formula::NOT:
    {
      if (parent != -1) {
	if (level == 2)
	  M[parent][*nextnode] = M[*nextnode][parent] = 1000000;
	else
	  M[parent][*nextnode] = M[*nextnode][parent] = 1;
      }
      int thisnode = *nextnode;
      *nextnode = *nextnode + 1;
      init_floyd_warshall(fml->right, nt, nf,
			  M, atom2node, thisnode, nextnode, level+1);
    }
    break;
  case Formula::AND: case Formula::OR: case Formula::IMPLIES:
    {
      if (parent != -1) {
	if (level == 2)
	  M[parent][*nextnode] = M[*nextnode][parent] = 1000000;
	else
	  M[parent][*nextnode] = M[*nextnode][parent] = 1;
      }
      int thisnode = *nextnode;
      *nextnode = *nextnode + 1;
      init_floyd_warshall(fml->left, nt, nf,
			  M, atom2node, thisnode, nextnode, level+1);
      init_floyd_warshall(fml->right, nt, nf,
			  M, atom2node, thisnode, nextnode, level+1);
    }
    break;
  case Formula::ANDN: case Formula::ORN:
    {
      if (parent != -1) {
	if (level == 2)
	  M[parent][*nextnode] = M[*nextnode][parent] = 1000000;
	else
	  M[parent][*nextnode] = M[*nextnode][parent] = 1;
      }
      int thisnode = *nextnode;
      *nextnode = *nextnode + 1;
      for (unsigned int j = 0; j < fml->fmls.size(); j++)
	init_floyd_warshall(fml->fmls[j], nt, nf,
			    M, atom2node, thisnode, nextnode, level+1);
    }
    break;
  }

  return;
}

bool TableauStrategy::classify(unsigned int& index)
{
  assert(_items != NULL && _alphas != NULL && _betas != NULL && _lits != NULL);

  unsigned int i;
  bool closed = false;

  for (i = index; i < _items->size(); i++) {
    SignedFormula::fmlType ty = (*_items)[i]->type();
    switch (ty) {
    case SignedFormula::ALPHA:
      if (find(_alphas->begin(), _alphas->end(), (*_items)[i]) ==
	  _alphas->end())
	_alphas->push_back((*_items)[i]);
      break;
    case SignedFormula::BETA:
      if (find(_betas->begin(), _betas->end(), (*_items)[i]) == _betas->end())
	_betas->push_back((*_items)[i]);
      break;
    case SignedFormula::LITERAL:
      if (find(_lits->begin(), _lits->end(), (*_items)[i]) == _lits->end()) {
	_lits->push_back((*_items)[i]);

	string atom = (*_items)[i]->formula->atom;
	SignedFormula::Sign sign = (*_items)[i]->sign;
	if (sign == SignedFormula::S_T) {
	  if (_mlits.find(atom) == _mlits.end())
	    _mlits[atom] = 2;
	  else
	    _mlits[atom] = _mlits[atom] | 2;
	}
	else {
	  if (_mlits.find(atom) == _mlits.end())
	    _mlits[atom] = 1;
	  else
	    _mlits[atom] = _mlits[atom] | 1;
	}
	if (_mlits[atom] == 3)
	  closed = true;
      }
      break;
    }
  }

  index = i;

  return closed;
}

unsigned int TableauStrategy::chooseAlpha() { return 0; }

unsigned int TableauStrategy::chooseBeta() { return 0; }


//////////////////////////////////////////////////////////////////////////////
// Members of class Tableau.
//////////////////////////////////////////////////////////////////////////////

Tableau::Tableau(const string& id, SignedFormula *fml,
		 Tableau *parent)
{
  _items.push_back(fml);
  _parent = parent;
  _id = id;
}

Tableau::Tableau(const string& id, const vector<SignedFormula *>& fmls,
		 Tableau *parent)
{
  _items = fmls;
  _parent = parent;
  _id = id;
}

Tableau::~Tableau() { }

void Tableau::setStrategy(TableauStrategy *strategy)
{
  _strategy = strategy;
}

string Tableau::toString(int level) const
{
  string s;
  unsigned int i;

  for(i = 0; i < _items.size(); i++) {
    char si[11];
    sprintf(si, "%d", i);
    s += string(level, ' ') + si + " " + _items[i]->toString() + "\n";
  }

  for (i = 0; i < _children.size(); i++)
    if (_children[i] != NULL)
      s += _children[i]->toString(level+2);

  return s;
}

unsigned int Tableau::countNodes()
{
  unsigned int i, total = 1;
  for (i = 0; i < _children.size(); i++)
    if (_children[i] != NULL)
      total += _children[i]->countNodes();
  return total;
}

unsigned int Tableau::countFormulae()
{
  unsigned int i, total = _items.size();
  for (i = 0; i < _children.size(); i++)
    if (_children[i] != NULL)
      total += _children[i]->countFormulae();
  return total;
}


bool Tableau::applyRule(unsigned int index,
			const vector<SignedFormula *>& in,
			vector<SignedFormula *>& out)
{
  if (index < _rules.size())
    return (*_rules[index])(in, out);
  return false;
}
