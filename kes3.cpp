/*****************************************************************************
 * kes3.cpp
 *
 * Definitions for tableau KE-S3.
 *****************************************************************************/

#include <cassert>
#include <cstdio>
#include <algorithm>

#include "tableau.h"
#include "kes3.h"


//////////////////////////////////////////////////////////////////////////////
// Members of class KES3Strategy.
//////////////////////////////////////////////////////////////////////////////

KES3Strategy::KES3Strategy() : KEPolarityStrategy() { }

KES3Strategy::~KES3Strategy() { }

Formula *KES3Strategy::choosePB()
{
  unsigned int k, choice;

  // Construct the valuation <atom, value>, with value in {*, 0, 1} (* = -1)
  map<string, int> valuation;
  for (k = 0; k < _lits->size(); k++)
    if ((*_lits)[k]->type() == SignedFormula::LITERAL) {
      if ((*_lits)[k]->sign == SignedFormula::S_F)
	valuation[(*_lits)[k]->formula->atom] = 0;
      else // S_T
	valuation[(*_lits)[k]->formula->atom] = 1;
    }
  
  // Check valuation against _betas. We'll choose the formula with the
  // lowest number of atoms not ocurring in S and with minimum
  // distance from the valuation given by the lits of the node.
  
  choice = indexAppPB;
  unsigned int minv, minp;
  double mindv = (double) _max_atom_dist, mindp = (double) _max_atom_dist;
  unsigned int minindv, minindp;
  
  minv = minp = _betas->size();
  minindv = minindp = indexAppPB;
  
  for (k = indexAppPB; k < _betas->size(); k++) {
    map<string, string>::const_iterator it =
      appliedPB.find((*_betas)[k]->toString());
    if ((it == appliedPB.end()
	 || it->second != id.substr(0, it->second.length())) &&
	(*_betas)[k]->value(valuation) < 1) {
      unsigned int aovv = (*_betas)[k]->atomsOut(((KES3Tableau *)tab)->_S);
      double dfv = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
      if (aovv < minv && dfv < mindv) {
	minv = aovv;
	minindv = k;
	mindv = dfv;
      }
      else if (aovv == minv && dfv < mindv) {
	mindv = dfv;
	minindv = k;
      }
      for (map<string, int>::const_iterator vit = valuation.begin();
	   vit != valuation.end(); vit++)
	if ((*_betas)[k]->polarity(vit->first) == -vit->second) {
	  unsigned int aovp =
	    (*_betas)[k]->atomsOut(((KES3Tableau *)tab)->_S);
	  double dfv = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
	  if (aovp < minp && dfv < mindp) {
	    minp = aovp;
	    minindp = k;
	    mindp = dfv;
	  }
	  else if (aovp == minp && dfv < mindp) {
	    mindp = dfv;
	    minindp = k;
	  }
	}
    }
  }

  if (minp == _betas->size())
    choice = minindv;
  else
    choice = minindp;

  Formula *ret;
  
  switch ((*_betas)[choice]->formula->op) {
  case Formula::OR:
  case Formula::AND:
  case Formula::IMPLIES:
    ret = new Formula(*((*_betas)[choice]->formula->left));
    break;
  case Formula::ORN:
  case Formula::ANDN:
    ret = new Formula(*((*_betas)[choice]->formula->fmls[0]));
    break;
  default:
    ret =  NULL;
  }

  assert(ret != NULL);
  appliedPB[(*_betas)[choice]->toString()] = id;

  // cout << "PB: " << ret->toString() << endl;

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// Members of class KES3AENOTLastStrategy.
//////////////////////////////////////////////////////////////////////////////

KES3AENOTLastStrategy::KES3AENOTLastStrategy() : KES3Strategy() {}

KES3AENOTLastStrategy::~KES3AENOTLastStrategy() { }

bool KES3AENOTLastStrategy::hasApplicableSimpleAlpha()
{
  for (unsigned int i = 0; i < _alphas->size(); i++) {
    set<string>& Set = ((KES3Tableau *) tab)->_S;
    if (
	(! ((*_alphas)[i]->sign == SignedFormula::S_T &&
	    (*_alphas)[i]->formula->op == Formula::NOT))
	||
	(
	 ((*_alphas)[i]->formula->right->op == Formula::ATOM) &&
	 (Set.find(((*_alphas)[i]->formula->right->atom)) != Set.end())
	)
       ) {
      indexAppAlpha = i;
      hasAppSimpleAlpha = true;
      return true;
    }
  }
  hasAppSimpleAlpha = false;
  return false;
}

bool KES3AENOTLastStrategy::hasApplicableAENOT()
{
  // Construct the valuation <atom, value>, with value in {*, 0, 1} (* = -1)
  map<string, int> valuation;
  for (unsigned int k = 0; k < _lits->size(); k++)
    if ((*_lits)[k]->type() == SignedFormula::LITERAL) {
      if ((*_lits)[k]->sign == SignedFormula::S_F)
	valuation[(*_lits)[k]->formula->atom] = 0;
      else // S_T
	valuation[(*_lits)[k]->formula->atom] = 1;
    }
  
  // We'll choose the formula with the lowest number of atoms outside
  // S and with minimum distance from the valuation given by the lits
  // of the node.

  unsigned int min;
  double mind = (double) _max_atom_dist;

  hasAppAENOT = false;
  for (unsigned int i = 0; i < _alphas->size(); i++)
    if ((*_alphas)[i]->sign == SignedFormula::S_T &&
	(*_alphas)[i]->formula->op == Formula::NOT) {
      unsigned int aout = (*_alphas)[i]->atomsOut(((KES3Tableau *)tab)->_S);
      if (! hasAppAENOT) { // first time entering here
	min = aout;
	indexAppAlpha = i;
	hasAppAENOT = true;
      }
      if (aout < min) {
	min = aout;
	indexAppAlpha = i;
      }
      else if (aout == min) {
  	double dist = (*_alphas)[i]->distanceFrom(valuation, _atom_dist);
  	if (dist < mind) {
  	  mind = dist;
	  indexAppAlpha = i;
	}
      }
    }
  return hasAppAENOT;
}

unsigned int KES3AENOTLastStrategy::chooseAlpha() { return indexAppAlpha; }

Formula *KES3AENOTLastStrategy::choosePB()
{
  unsigned int k, choice;

  // Construct the valuation <atom, value>, with value in {*, 0, 1} (* = -1)
  map<string, int> valuation;
  for (k = 0; k < _lits->size(); k++)
    if ((*_lits)[k]->type() == SignedFormula::LITERAL) {
      if ((*_lits)[k]->sign == SignedFormula::S_F)
	valuation[(*_lits)[k]->formula->atom] = 0;
      else // S_T
	valuation[(*_lits)[k]->formula->atom] = 1;
    }
  
  // Check valuation against _betas. We'll choose the formula with the
  // lowest number of atoms not ocurring in S and with minimum
  // distance from the valuation given by the lits of the node.
  
  choice = indexAppPB;
  unsigned int minv, minp;
  double mindv = (double) _max_atom_dist, mindp = (double) _max_atom_dist;
  unsigned int minindv, minindp;
  
  minv = minp = _betas->size();
  minindv = minindp = indexAppPB;
  
  for (k = indexAppPB; k < _betas->size(); k++) {
    map<string, string>::const_iterator it =
      appliedPB.find((*_betas)[k]->toString());
    if ((it == appliedPB.end()
	 || it->second != id.substr(0, it->second.length())) &&
	(*_betas)[k]->value(valuation) < 1) {
      unsigned int aovv = (*_betas)[k]->atomsOut(((KES3Tableau *)tab)->_S);
      double dfv = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
      if (aovv < minv && dfv < mindv) {
	minv = aovv;
	minindv = k;
	mindv = dfv;
      }
      else if (aovv == minv && dfv < mindv) {
	mindv = dfv;
	minindv = k;
      }
      for (map<string, int>::const_iterator vit = valuation.begin();
	   vit != valuation.end(); vit++)
	if ((*_betas)[k]->polarity(vit->first) == -vit->second) {
	  unsigned int aovp =
	    (*_betas)[k]->atomsOut(((KES3Tableau *)tab)->_S);
	  double dfv = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
	  if (aovp < minp && dfv < mindp) {
	    minp = aovp;
	    minindp = k;
	    mindp = dfv;
	  }
	  else if (aovp == minp && dfv < mindp) {
	    mindp = dfv;
	    minindp = k;
	  }
	}
    }
  }

  if (minp == _betas->size())
    choice = minindv;
  else
    choice = minindp;

  Formula *ret;
  
  switch ((*_betas)[choice]->formula->op) {
  case Formula::OR:
  case Formula::AND:
  case Formula::IMPLIES:
    ret = new Formula(*((*_betas)[choice]->formula->left));
    break;
  case Formula::ORN:
  case Formula::ANDN:
    ret = new Formula(*((*_betas)[choice]->formula->fmls[0]));
    break;
  default:
    ret =  NULL;
  }

  assert(ret != NULL);
  appliedPB[(*_betas)[choice]->toString()] = id;

  // cout << "PB: " << ret->toString() << endl;

  return ret;
}

int KES3AENOTLastStrategy::nextRule()
{
  if (hasApplicableSimpleAlpha()) {
    //    cout << "SimpleAlpha: " << id << endl;
    return 0; // alpha
  }
  else if (hasApplicableBeta()) {
    //    cout << "Beta: " << id << endl;
    return 1; // beta
  }
  else if (! _betas->empty()) {
    unsigned int choice = 0;
    while (choice < _betas->size()) {
      map<string, string>::const_iterator it =
	appliedPB.find((*_betas)[choice]->toString());
      if (it != appliedPB.end()) {
	//	cout << it->second << " | " << id << endl;
	if (it->second == id.substr(0, it->second.length()))
	  choice++;
	else
	  break;
      }
      else
	break;
    }
    if (choice == _betas->size()) {
      if (hasApplicableAENOT()) {
	//	cout << "AENot1: " << id << endl;
	return 0;
      }
      else {
	//	cout << "NONE 1: " << id << endl;
	return -1; // none
      }
    }
    else {
      indexAppPB = choice;
      //      cout << "PB: " << id << endl;
      return 2; // PB
    }
  }
  else if (hasApplicableAENOT()) {
    //    cout << "AENot2: " << id << endl;
    return 0;
  }
  else {
    //    cout << "NONE 2: " << id << endl;
    return -1;
  }
}


//////////////////////////////////////////////////////////////////////////////
// Members of class KES3Tableau.
//////////////////////////////////////////////////////////////////////////////

KES3Tableau::KES3Tableau(const string& id, SignedFormula *fml,
			 KES3Tableau *parent)
  : KETableau(id, fml, parent) { }

KES3Tableau::KES3Tableau(const string& id, const vector<SignedFormula *>& fmls,
			 KES3Tableau *parent)
  : KETableau(id, fmls, parent) { }

void KES3Tableau::setStrategy(KES3Strategy *strategy) {
  // Initialization of the strategy object
  _strategy = strategy;
  _strategy->init(_id, &_items, &_alphas, &_betas, &_lits);
  _strategy->setTableau(this);
}

bool KES3Tableau::applyRule(KES3Tableau::enumRule r,
			    vector<SignedFormula *>& in,
			    vector<SignedFormula *>& out)
{
  bool result;
  
  result = Tableau::applyRule((unsigned int) r, in, out);

  if (result && r == A_E_NOT) {
      InsertAtoms(out[0]->formula);
      mS[_items.size()-1] = _S;
  }
  
  return result;
}

string KES3Tableau::toString(int level) const
{
  string s;
  unsigned int i;
  map<unsigned int, set<string> >::const_iterator mit = mS.begin();

  for(i = 0; i < _items.size(); i++) {
    char si[11];
    sprintf(si, "%d", i);
    s += string(level, ' ') + si + " " + _items[i]->toString();
    if (mit != mS.end() && i == mit->first) {
      s += "   S = { ";
      for (set<string>::const_iterator sit = mit->second.begin();
	   sit != mit->second.end(); sit++)
	s += (*sit) + " ";
      s += "}";
      mit++;
    }
    s += "\n";
  }

  for (i = 0; i < _children.size(); i++)
    if (_children[i] != NULL)
      s += _children[i]->toString(level+2);

  return s;
}

void KES3Tableau::preClose()
{
  if (_parent) {
    KES3Tableau *parent = (KES3Tableau *) _parent;
    _S.insert(parent->_S.begin(), parent->_S.end());
  }
}

bool KES3Tableau::close()
{
  preClose();

  unsigned int i = 0;

  if (_closed) {
    //    cout << "CLOSED BRANCH " << _id << endl;
    postClose();
    return true;
  }

  _closed = _strategy->classify(i);
  
  if (_closed) {
    //    cout << "CLOSED BRANCH " << _id << endl;
    postClose();
    return true;
  }

  int nextRule = _strategy->nextRule();

  while (nextRule != -1) {
    switch (nextRule) {
    case 0: // alpha
      {
	unsigned int index = _strategy->chooseAlpha();
	vector<SignedFormula *> in, out;
	in.push_back(_alphas[index]);

	//	cout << toString() << endl;
	//	cout << "alpha: " << _alphas[index]->toString() << endl;
	
	// exactly one of these will succeed
	applyRule(A_E_NOT_OR, in, out);
	applyRule(A_E_NOT_ORN, in, out);
	applyRule(A_E_AND, in, out);
	applyRule(A_E_ANDN, in, out);
	applyRule(A_E_NOT_IMPLIES, in, out);
	applyRule(A_E_NOT_NOT, in, out);
	applyRule(A_E_NOT, in, out);
	
	_items.insert(_items.end(), out.begin(), out.end());
	vector<SignedFormula *>::iterator it = _alphas.begin() + index;

	_alphas.erase(it);

	_closed = _strategy->classify(i);
	if (_closed) {
	  //	  cout << "CLOSED BRANCH " << _id << endl;
	  postClose();
	  return true;
	}
      }
      break;
    case 1: // beta
      {
	unsigned int index = _strategy->chooseBeta();
	unsigned int indexL = _strategy->chooseLit();
	vector<SignedFormula *> in, out;
	
	in.push_back(_betas[index]);
	in.push_back(_lits[indexL]);

	// exactly one of these will succeed
	applyRule(B_E_OR_1, in, out);
	applyRule(B_E_OR_2, in, out);
	applyRule(B_E_ORN, in, out);
	applyRule(B_E_NOT_AND_1, in, out);
	applyRule(B_E_NOT_AND_2, in, out);
	applyRule(B_E_NOT_ANDN, in, out);
	applyRule(B_E_IMPLIES_1, in, out);
	applyRule(B_E_IMPLIES_2, in, out);
	
	_items.insert(_items.end(), out.begin(), out.end());
	vector<SignedFormula *>::iterator it = _betas.begin() + index;
	_betas.erase(it);

	_closed = _strategy->classify(i);
	if (_closed) {
	  //	  cout << "CLOSED BRANCH " << _id << endl;
	  postClose();
	  return true;
	}
      }
      break;
    case 2: // PB
      {
	Formula *x = _strategy->choosePB();
	
	assert(x != NULL);
	
	// Obs: You cannot create the second child before the call to
	// close() of the previous child. It leads to a sobreposition of
	// the members of _strategy.
	bool closed1, closed2;	
	createChild(_id + "-1", new SignedFormula(SignedFormula::S_T, x));
	closed1 = _children[0]->close();
	setStrategy(_strategy);
	if (closed1) {
	  createChild(_id + "-2", new SignedFormula(SignedFormula::S_F, x));
	  closed2 = _children[1]->close();
	  setStrategy(_strategy);
	}

	if (closed1 && closed2) {
	  //	  cout << "CLOSED BRANCH " << _id << endl;
	  postClose();
	  return true;
	}
	else {
	  postClose();
	  return false;
	}
      }
      break;
    default:
      {
	postClose();
	return false;
      }
    }
    nextRule = _strategy->nextRule();
  }
  postClose();
  return false;
}

void KES3Tableau::postClose()
{
  if (_parent) {
    KES3Tableau *parent = (KES3Tableau *) _parent;
    parent->_S.insert(_S.begin(), _S.end());
  }
}

set<string> KES3Tableau::S() const
{
  return _S;
}

void KES3Tableau::InsertAtoms(Formula *f)
{
  switch(f->op) {
  case Formula::ANDN: case Formula::ORN:
    {
      for (unsigned int i = 0; i < f->fmls.size(); i++)
	InsertAtoms(f->fmls[i]);
    }
    break;
  case Formula::AND: case Formula::OR: case Formula::IMPLIES:
    {
      InsertAtoms(f->left);
      InsertAtoms(f->right);
    }
    break;
  case Formula::NOT:
    InsertAtoms(f->right);
    break;
  case Formula::ATOM:
    _S.insert(f->atom);
  }
}

void KES3Tableau::createChild(const string& id, SignedFormula *fml)
{
  _children.push_back(new KES3Tableau(id, fml, this));
  ((KES3Tableau *) _children[_children.size()-1])->setStrategy(_strategy);
}
