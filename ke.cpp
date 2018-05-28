/*****************************************************************************
 * ke.cpp
 *
 * Definitions for tableau KE.
 *****************************************************************************/

#include <cassert>
#include <cstdio>
#include <algorithm>

#include "tableau.h"
#include "ke.h"


//////////////////////////////////////////////////////////////////////////////
// Members of class KEStrategy.
//////////////////////////////////////////////////////////////////////////////

KEStrategy::KEStrategy() : TableauStrategy()
{
  hasAppBeta = false;
}

KEStrategy::~KEStrategy() { }

unsigned int KEStrategy::chooseAlpha() { return 0; }

bool KEStrategy::hasApplicableBeta()
{

  //  cout << "hasAppBeta:" << endl;

  hasAppBeta = false;
  indexAppBeta = 0;

  unsigned int i, j;
  for (i = 0; i < _betas->size() && ! hasAppBeta; i++) {
    for (j = 0; j < _lits->size() && ! hasAppBeta; j++) {
      vector<SignedFormula *> dummyin, dummyout;
      dummyin.push_back((*_betas)[i]);
      dummyin.push_back((*_lits)[j]);      
      hasAppBeta = hasAppBeta
	|| KE_beta_E_OR_1     (dummyin, dummyout)
	|| KE_beta_E_OR_2     (dummyin, dummyout)
	|| KE_beta_E_ORN      (dummyin, dummyout)
	|| KE_beta_E_NOT_AND_1(dummyin, dummyout)
	|| KE_beta_E_NOT_AND_2(dummyin, dummyout)
	|| KE_beta_E_NOT_ANDN (dummyin, dummyout)
	|| KE_beta_E_IMPLIES_1(dummyin, dummyout)
	|| KE_beta_E_IMPLIES_2(dummyin, dummyout);
      if (hasAppBeta) {
	indexAppBeta = i;
	indexAppLit = j;
	//	cout << "Beta: " << (*_betas)[i]->toString()
	//	     << " with " << (*_lits)[j]->toString() << endl;
      }
    }
  }

  return hasAppBeta;
}

unsigned int KEStrategy::chooseBeta() { return indexAppBeta; }

unsigned int KEStrategy::chooseLit() { return indexAppLit; }

Formula *KEStrategy::choosePB()
{
  Formula *ret;
  
  switch ((*_betas)[indexAppPB]->formula->op) {
  case Formula::OR:
  case Formula::AND:
  case Formula::IMPLIES:
    ret = new Formula(*((*_betas)[indexAppPB]->formula->left));
    break;
  case Formula::ORN:
  case Formula::ANDN:
    ret = new Formula(*((*_betas)[indexAppPB]->formula->fmls[0]));
    break;
  default:
    ret = NULL;
  }

  assert(ret != NULL);
  appliedPB[(*_betas)[indexAppPB]->toString()] = id;
    
  return ret;
}

// 0=alpha; 1=beta; 2=PB; -1=none.
int KEStrategy::nextRule()
{
  if (! _alphas->empty())
    return 0; // alpha
  else if (hasApplicableBeta())
    return 1; // beta
  else if (! _betas->empty()) {
    unsigned int choice = 0;
    while (choice < _betas->size()) {
      map<string, string>::const_iterator it =
	appliedPB.find((*_betas)[choice]->toString());
      if (it != appliedPB.end()) {
	//	cout << it->first;
	if (it->second == id.substr(0, it->second.length())) {
	  //	  cout << ": applied in " << it->second << endl;
	  choice++;
	}
	else {
	  //	  cout << ": not applied" << endl;
	  break;
	}
      }
      else
	break;
    }
    if (choice == _betas->size())
      return -1; // none
    else {
      indexAppPB = choice;
      return 2; // PB
    }
  }
  else
    return -1; // none
}


//////////////////////////////////////////////////////////////////////////////
// Members of class KEValuationStrategy.
//////////////////////////////////////////////////////////////////////////////

KEValuationStrategy::KEValuationStrategy() : KEStrategy() { }

KEValuationStrategy::~KEValuationStrategy() { }

Formula *KEValuationStrategy::choosePB()
{
  unsigned int k;

  // Construct the valuation <atom, value>, with value in {*, 0, 1} (* = -1)
  map<string, int> valuation;
  for (k = 0; k < _lits->size(); k++)
    if ((*_lits)[k]->type() == SignedFormula::LITERAL) {
      if ((*_lits)[k]->sign == SignedFormula::S_F)
	valuation[(*_lits)[k]->formula->atom] = 0;
      else // S_T
	valuation[(*_lits)[k]->formula->atom] = 1;
    }
  
  // Check valuation against _betas. We'll choose the formula with
  // minimum distance from the valuation.

  unsigned int choice = indexAppPB;
  double min = (*_betas)[indexAppPB]->distanceFrom(valuation, _atom_dist);
  unsigned int minind = indexAppPB;

  for (k = indexAppPB; k < _betas->size(); k++) {
    map<string, string>::const_iterator it =
      appliedPB.find((*_betas)[k]->toString());
    if ((it == appliedPB.end()
	 || it->second != id.substr(0, it->second.length())) &&
	(*_betas)[k]->value(valuation) < 1) {
      double dfv = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
      if (dfv < min) {
	min = dfv;
	minind = k;
      }
    }
  }

  choice = minind;

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
    ret = NULL;
  }

  assert (ret != NULL);
  appliedPB[(*_betas)[choice]->toString()] = id;
  
  return ret;
}


//////////////////////////////////////////////////////////////////////////////
// Members of class KEPolarityStrategy.
//////////////////////////////////////////////////////////////////////////////

KEPolarityStrategy::KEPolarityStrategy() : KEStrategy() { }

KEPolarityStrategy::~KEPolarityStrategy() { }

Formula *KEPolarityStrategy::choosePB()
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
  
  // Check valuation against _betas. We'll choose the formula with
  // minimum distance from the valuation given by the lits of the node.
  
  choice = indexAppPB;
  double minv, minp;
  unsigned int minindv, minindp;
  
  minv = minp = (double) _max_atom_dist;
  minindv = minindp = indexAppPB;
  
  for (k = indexAppPB; k < _betas->size(); k++) {
    map<string, string>::const_iterator it =
      appliedPB.find((*_betas)[k]->toString());
    if ((it == appliedPB.end()
	 || it->second != id.substr(0, it->second.length())) &&
	(*_betas)[k]->value(valuation) < 1) {
      double dfvv = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
      if (dfvv < minv) {
	minv = dfvv;
	minindv = k;
      }
      for (map<string, int>::const_iterator vit = valuation.begin();
	   vit != valuation.end(); vit++)
	if ((*_betas)[k]->polarity(vit->first) == -vit->second) {
  	  double dfvp = (*_betas)[k]->distanceFrom(valuation, _atom_dist);
  	  if (dfvp < minp) {
	    minp = dfvp;
	    minindp = k;
	  }
	}
    }
  }

  if (minp > (double) _max_atom_dist - 0.5)
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

  return ret;
}


//////////////////////////////////////////////////////////////////////////////
// Members of class KETableau.
//////////////////////////////////////////////////////////////////////////////

KETableau::KETableau(const string& id, SignedFormula *fml,
		     KETableau *parent)
  : Tableau(id, fml, parent)
{
  _closed = false;

  // Initialization of _alphas, _betas and _lits
  if (parent != NULL) {
    _alphas.insert(_alphas.end(),
		   parent->_alphas.begin(), parent->_alphas.end());
    _betas.insert(_betas.end(), parent->_betas.begin(), parent->_betas.end());
    _lits.insert(_lits.end(), parent->_lits.begin(), parent->_lits.end());  
  }

  // Initialization of _rules
  _rules.push_back(&KE_alpha_E_NOT_OR);
  _rules.push_back(&KE_alpha_E_NOT_ORN);
  _rules.push_back(&KE_alpha_E_AND);
  _rules.push_back(&KE_alpha_E_ANDN);
  _rules.push_back(&KE_alpha_E_NOT_IMPLIES);
  _rules.push_back(&KE_alpha_E_NOT_NOT);
  _rules.push_back(&KE_alpha_E_NOT);
  _rules.push_back(&KE_beta_E_OR_1);
  _rules.push_back(&KE_beta_E_OR_2);
  _rules.push_back(&KE_beta_E_ORN);
  _rules.push_back(&KE_beta_E_NOT_AND_1);
  _rules.push_back(&KE_beta_E_NOT_AND_2);
  _rules.push_back(&KE_beta_E_NOT_ANDN);
  _rules.push_back(&KE_beta_E_IMPLIES_1);
  _rules.push_back(&KE_beta_E_IMPLIES_2);

  //  cout << "NEW BRANCH " << id << endl;

  // if this is a child tableau, try to apply each beta with fml

  if (parent != NULL) {
    for (unsigned int i = 0; i < _betas.size(); i++) {
      vector<SignedFormula *> in, out;
      in.push_back(_betas[i]);
      in.push_back(fml);
      // One or none of these calls will succeed
      bool success = false;
      success = success
	|| applyRule(B_E_OR_1, in, out)
	|| applyRule(B_E_OR_2, in, out)
	|| applyRule(B_E_ORN, in, out)
	|| applyRule(B_E_NOT_AND_1, in, out)
	|| applyRule(B_E_NOT_AND_2, in, out)
	|| applyRule(B_E_NOT_ANDN, in, out)
	|| applyRule(B_E_IMPLIES_1, in, out)
	|| applyRule(B_E_IMPLIES_2, in, out);
      
      if (success) {
	_items.insert(_items.end(), out.begin(), out.end());
	//	cout << "beta: " << _betas[i]->toString() << endl;
	vector<SignedFormula *>::iterator it = _betas.begin();
	for (unsigned int m = 0; m < i; m++) it++;
	_betas.erase(it);
	i--;
      }
    }
  }
}

KETableau::KETableau(const string& id, const vector<SignedFormula *>& fmls,
		     KETableau *parent)
  : Tableau(id, fmls, parent)
{
  _closed = false;
  
  // Initialization of alphas, _betas and _lits
  if (parent != NULL) {
    _alphas.insert(_alphas.end(),
		   parent->_alphas.begin(), parent->_alphas.end());    
    _betas.insert(_betas.end(), parent->_betas.begin(), parent->_betas.end());
    _lits.insert(_lits.end(), parent->_lits.begin(), parent->_lits.end());  
  }

  // Initialization of _rules
  _rules.push_back(&KE_alpha_E_NOT_OR);
  _rules.push_back(&KE_alpha_E_NOT_ORN);
  _rules.push_back(&KE_alpha_E_AND);
  _rules.push_back(&KE_alpha_E_ANDN);
  _rules.push_back(&KE_alpha_E_NOT_IMPLIES);
  _rules.push_back(&KE_alpha_E_NOT_NOT);
  _rules.push_back(&KE_alpha_E_NOT);
  _rules.push_back(&KE_beta_E_OR_1);
  _rules.push_back(&KE_beta_E_OR_2);
  _rules.push_back(&KE_beta_E_ORN);
  _rules.push_back(&KE_beta_E_NOT_AND_1);
  _rules.push_back(&KE_beta_E_NOT_AND_2);
  _rules.push_back(&KE_beta_E_NOT_ANDN);
  _rules.push_back(&KE_beta_E_IMPLIES_1);
  _rules.push_back(&KE_beta_E_IMPLIES_2);
  
  //  cout << "NEW BRANCH " << id << endl;
}

void KETableau::setStrategy(KEStrategy *strategy) {
  // Initialization of the strategy object
  _strategy = strategy;
  _strategy->init(_id, &_items, &_alphas, &_betas, &_lits);
}

bool KE_alpha_E_NOT_OR(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::OR)) return false;

  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->left))));
  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool KE_alpha_E_NOT_ORN(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::ORN)) return false;

  for(unsigned int i = 0; i < in[0]->formula->fmls.size(); i++)
    out.push_back(new SignedFormula(SignedFormula::S_F,
				    new Formula(*(in[0]->formula->fmls[i]))));

  return true;
}

bool KE_alpha_E_AND(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::AND)) return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->left))));
  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool KE_alpha_E_ANDN(const vector<SignedFormula *>& in,
		  vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::ANDN)) return false;

  for(unsigned int i = 0; i < in[0]->formula->fmls.size(); i++)
    out.push_back(new SignedFormula(SignedFormula::S_T,
				    new Formula(*(in[0]->formula->fmls[i]))));

  return true;
}

bool KE_alpha_E_NOT_IMPLIES(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::IMPLIES))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->left))));
  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool KE_alpha_E_NOT_NOT(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::NOT)) return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool KE_alpha_E_NOT(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::NOT)) return false;

  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool KE_beta_E_OR_1(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (! (primary && primary->sign == SignedFormula::S_T && 
	 primary->formula->op == Formula::OR &&
	 secondary && secondary->sign == SignedFormula::S_F &&
	 primary->formula->left->toString() == secondary->formula->toString()))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(primary->formula->right))));

  return true;
}

bool KE_beta_E_OR_2(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (! (primary && primary->sign == SignedFormula::S_T &&
	 primary->formula->op == Formula::OR &&
	 secondary && secondary->sign == SignedFormula::S_F &&
	 primary->formula->right->toString()==secondary->formula->toString()))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(primary->formula->left))));

  return true;
}

bool KE_beta_E_ORN(const vector<SignedFormula *>& in,
		   vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (primary && primary->sign == SignedFormula::S_T &&
      primary->formula->op == Formula::ORN &&
      secondary && secondary->sign == SignedFormula::S_F) {
    vector<Formula *> newfmls;
    bool found = false;
    unsigned int i, ind;
    for (i = 0; ! found && i < primary->formula->fmls.size(); i++)
      if (primary->formula->fmls[i]->toString() ==
	  secondary->formula->toString()) {
	found = true;
	ind = i;
      }
    if (found) {
      for (i = 0; i < primary->formula->fmls.size(); i++)
	if (i != ind)
	  newfmls.push_back(primary->formula->fmls[i]);
      if (newfmls.size() > 2)
	out.push_back(new SignedFormula(SignedFormula::S_T,
					new Formula(Formula::ORN, newfmls)));
      else if (newfmls.size() == 2)
	out.push_back(new SignedFormula(SignedFormula::S_T,
					new Formula(Formula::OR, newfmls[0],
						    newfmls[1])));
      else
	out.push_back(new SignedFormula(SignedFormula::S_T, newfmls[0]));
      return true;
    }
    return false;
  }
  return false;
}


bool KE_beta_E_NOT_AND_1(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (! (primary && primary->sign == SignedFormula::S_F &&
	 primary->formula && primary->formula->op == Formula::AND &&
	 secondary->sign == SignedFormula::S_T &&
	 primary->formula->left->toString() == secondary->formula->toString()))
    return false;
  
  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(primary->formula->right))));
  
  return true;
}

bool KE_beta_E_NOT_AND_2(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (! (primary && primary->sign == SignedFormula::S_F &&
	 primary->formula && primary->formula->op == Formula::AND &&
	 secondary->sign == SignedFormula::S_T &&
	 primary->formula->right->toString()==secondary->formula->toString()))
    return false;
  
  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(primary->formula->left))));

  return true;
}

bool KE_beta_E_NOT_ANDN(const vector<SignedFormula *>& in,
			vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (primary && primary->sign == SignedFormula::S_F &&
      primary->formula && primary->formula->op == Formula::ANDN &&
      secondary->sign == SignedFormula::S_T) {
    vector<Formula *> newfmls;
    bool found = false;
    unsigned int i, ind;
    for (i = 0; ! found && i < primary->formula->fmls.size(); i++)
      if (primary->formula->fmls[i]->toString() ==
	  secondary->formula->toString()) {
	found = true;
	ind = i;
      }
    if (found) {
      for (i = 0; i < primary->formula->fmls.size(); i++)
	if (i != ind)
	  newfmls.push_back(primary->formula->fmls[i]);
      if (newfmls.size() > 2)
	out.push_back(new SignedFormula(SignedFormula::S_F,
					new Formula(Formula::ANDN, newfmls)));
      else if (newfmls.size() == 2)
	out.push_back(new SignedFormula(SignedFormula::S_F,
					new Formula(Formula::AND,
						    newfmls[0], newfmls[1])));
      else
	out.push_back(new SignedFormula(SignedFormula::S_F, newfmls[0]));
      return true;
    }
    return false;
  }
  return false;
}

bool KE_beta_E_IMPLIES_1(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (! (primary && primary->sign == SignedFormula::S_T &&
	 primary->formula->op == Formula::IMPLIES &&
	 secondary->sign == SignedFormula::S_T &&
	 primary->formula->left->toString() == secondary->formula->toString()))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(primary->formula->right))));

  return true;
}

bool KE_beta_E_IMPLIES_2(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out)
{
  if (in.size() != 2) return false;

  SignedFormula *primary, *secondary;
  primary = in[0];
  secondary = in[1];

  if (! (primary && primary->sign == SignedFormula::S_T &&
	 primary->formula->op == Formula::IMPLIES &&
	 secondary && secondary->sign == SignedFormula::S_F &&
	 primary->formula->right->toString()==secondary->formula->toString()))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(primary->formula->left))));

  return true;
}

bool KETableau::applyRule(KETableau::enumRule r,
			  vector<SignedFormula *>& in,
			  vector<SignedFormula *>& out)
{
  bool result;

  result = Tableau::applyRule((unsigned int) r, in, out);
  
  return result;
}

bool KETableau::close()
{
  preClose();

  unsigned int i = 0;

  if (_closed) {
    postClose();
    return true;
  }

  _closed = _strategy->classify(i);
  
  if (_closed) {
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
	
	// exactly one of these will succeed
	bool success = false;
	success = success || applyRule(A_E_NOT_OR, in, out);
	success = success || applyRule(A_E_NOT_ORN, in, out);
	success = success || applyRule(A_E_AND, in, out);
	success = success || applyRule(A_E_ANDN, in, out);
	success = success || applyRule(A_E_NOT_IMPLIES, in, out);
	success = success || applyRule(A_E_NOT_NOT, in, out);
	success = success || applyRule(A_E_NOT, in, out);
	
	_items.insert(_items.end(), out.begin(), out.end());
	//	cout << success << " " << index << " " << _alphas.size() << endl;
	vector<SignedFormula *>::iterator it = _alphas.begin() + index;
	_alphas.erase(it);
	
	_closed = _strategy->classify(i);
	if (_closed) {
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
	  //      cout << "CLOSED BRANCH " << _id << endl;
	  postClose();
	  return true;
	}
	else {
	  postClose();
	  return true;
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

void KETableau::createChild(const string& id, SignedFormula *fml)
{
  _children.push_back(new KETableau(id, fml, this));
  ((KETableau *) _children[_children.size()-1])->setStrategy(_strategy);
}
