/*****************************************************************************
 * analytic.cpp
 *
 * Definitions for analytic tableau.
 *
 *****************************************************************************/


#include <cassert>
#include <cstdio>
#include <algorithm>

#include "analytic.h"


//////////////////////////////////////////////////////////////////////////////
// Members of class AnalyticStrategy.
//////////////////////////////////////////////////////////////////////////////

AnalyticStrategy::AnalyticStrategy()
  : TableauStrategy() { }

AnalyticStrategy::~AnalyticStrategy() { }
  
unsigned int AnalyticStrategy::chooseAlpha()
{
  return 0;
}

unsigned int AnalyticStrategy::chooseBeta()
{
  return 0;
}

int AnalyticStrategy::nextRule()
{
  if (! _alphas->empty())
    return 0; // alpha
  else {
    if (! _betas->empty())
      return 1; // beta
    else
      return -1; // none
  }
  return -1; // none
}


//////////////////////////////////////////////////////////////////////////////
// Members of class AnalyticBottomUpStrategy.
//////////////////////////////////////////////////////////////////////////////

AnalyticBottomUpStrategy::AnalyticBottomUpStrategy()
  : AnalyticStrategy() { }

AnalyticBottomUpStrategy::~AnalyticBottomUpStrategy() { }
  
unsigned int AnalyticBottomUpStrategy::chooseAlpha()
{
  return _alphas->size() - 1;
}

unsigned int AnalyticBottomUpStrategy::chooseBeta()
{
  return _betas->size() - 1;
}


//////////////////////////////////////////////////////////////////////////////
// Members of class AnalyticTableau.
//////////////////////////////////////////////////////////////////////////////

AnalyticTableau::AnalyticTableau(const string& id, SignedFormula *fml,
				 AnalyticTableau *parent)
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
  _rules.push_back(&alpha_E_NOT_OR);
  _rules.push_back(&alpha_E_NOT_ORN);
  _rules.push_back(&alpha_E_AND);
  _rules.push_back(&alpha_E_ANDN);
  _rules.push_back(&alpha_E_NOT_IMPLIES);
  _rules.push_back(&alpha_E_NOT_NOT);
  _rules.push_back(&alpha_E_NOT);
  _rules.push_back(&beta_E_OR);
  _rules.push_back(&beta_E_ORN);
  _rules.push_back(&beta_E_NOT_AND);
  _rules.push_back(&beta_E_NOT_ANDN);
  _rules.push_back(&beta_E_IMPLIES);
}

AnalyticTableau::AnalyticTableau(const string& id,
				 const vector<SignedFormula *>& fmls,
				 AnalyticTableau *parent)
  : Tableau(id, fmls, parent)
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
  _rules.push_back(&alpha_E_NOT_OR);
  _rules.push_back(&alpha_E_NOT_ORN);
  _rules.push_back(&alpha_E_AND);
  _rules.push_back(&alpha_E_ANDN);
  _rules.push_back(&alpha_E_NOT_IMPLIES);
  _rules.push_back(&alpha_E_NOT_NOT);
  _rules.push_back(&alpha_E_NOT);
  _rules.push_back(&beta_E_OR);
  _rules.push_back(&beta_E_ORN);
  _rules.push_back(&beta_E_NOT_AND);
  _rules.push_back(&beta_E_NOT_ANDN);
  _rules.push_back(&beta_E_IMPLIES);
}

void AnalyticTableau::setStrategy(AnalyticStrategy *strategy) {
  // Initialization of the strategy object
  _strategy = strategy;
  _strategy->init(_id, &_items, &_alphas, &_betas, &_lits);
}

bool alpha_E_NOT_OR(const vector<SignedFormula *>& in,
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

bool alpha_E_NOT_ORN(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::ORN)) return false;

  for(unsigned int i = 0; i < in[0]->formula->fmls.size(); i++)
    out.push_back(new SignedFormula(SignedFormula::S_F,
				    new Formula(*(in[0]->formula->fmls[i]))));

  return true;
}

bool alpha_E_AND(const vector<SignedFormula *>& in,
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

bool alpha_E_ANDN(const vector<SignedFormula *>& in,
		  vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::ANDN)) return false;

  for(unsigned int i = 0; i < in[0]->formula->fmls.size(); i++)
    out.push_back(new SignedFormula(SignedFormula::S_T,
				    new Formula(*(in[0]->formula->fmls[i]))));

  return true;
}

bool alpha_E_NOT_IMPLIES(const vector<SignedFormula *>& in,
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

bool alpha_E_NOT_NOT(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::NOT)) return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool alpha_E_NOT(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::NOT)) return false;

  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool beta_E_OR(const vector<SignedFormula *>& in,
	       vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::OR))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->left))));
  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool beta_E_ORN(const vector<SignedFormula *>& in,
		vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::ORN))
    return false;

  for(unsigned int i = 0; i < in[0]->formula->fmls.size(); i++)
    out.push_back(new SignedFormula(SignedFormula::S_T,
				    new Formula(*(in[0]->formula->fmls[i]))));

  return true;
}

bool beta_E_NOT_AND(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::AND))
    return false;
  
  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->left))));
  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool beta_E_NOT_ANDN(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_F &&
	 in[0]->formula && in[0]->formula->op == Formula::ANDN))
    return false;

  for(unsigned int i = 0; i < in[0]->formula->fmls.size(); i++)
    out.push_back(new SignedFormula(SignedFormula::S_F,
				    new Formula(*(in[0]->formula->fmls[i]))));

  return true;
}

bool beta_E_IMPLIES(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out)
{
  if (! (in.size() == 1 && in[0] && in[0]->sign == SignedFormula::S_T &&
	 in[0]->formula && in[0]->formula->op == Formula::IMPLIES))
    return false;

  out.push_back(new SignedFormula(SignedFormula::S_F,
				  new Formula(*(in[0]->formula->left))));
  out.push_back(new SignedFormula(SignedFormula::S_T,
				  new Formula(*(in[0]->formula->right))));

  return true;
}

bool AnalyticTableau::applyRule(AnalyticTableau::enumRule r,
				vector<SignedFormula *>& in,
				vector<SignedFormula *>& out)
{
  bool result;

  result = Tableau::applyRule((unsigned int) r, in, out);
  
  return result;
}

bool AnalyticTableau::close()
{
  unsigned int c = 0;

  if (_closed)
    return true;

  _closed = _strategy->classify(c);

  if (_closed)
    return true;
  
  int nextRule = _strategy->nextRule();
  
  while (nextRule != -1) {
    switch (nextRule) {
    case 0: // alpha
      {
	unsigned int index = _strategy->chooseAlpha();
	vector<SignedFormula *> in, out;
	in.push_back(_alphas[index]);
	
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
	
	_closed = _strategy->classify(c);
	if (_closed)
	  return true;
      }
      break;
    case 1: // beta
      {
	unsigned int index = _strategy->chooseBeta();
	vector<SignedFormula *> in, out;
	
	in.push_back(_betas[index]);
	
	// exactly one of these will succeed
	applyRule(B_E_OR, in, out);
	applyRule(B_E_ORN, in, out);
	applyRule(B_E_NOT_AND, in, out);
	applyRule(B_E_NOT_ANDN, in, out);
	applyRule(B_E_IMPLIES, in, out);
	
	vector<SignedFormula *>::iterator it = _betas.begin() + index;
	_betas.erase(it);
    
	// Obs: You cannot create the second child before the call to
	// close() of the previous child. It leads to a sobreposition of
	// the members of _strategy.
	bool closed = true;
	for (unsigned int ind = 0; ind < out.size() && closed; ind++) {
	  char cid[1000];
	  sprintf(cid, "%s-%d", _id.c_str(), ind+1);
	  createChild(cid, out[ind]);
	  if (! _children[ind]->close())
	    closed = false;
	  setStrategy(_strategy);
	}
	return closed;
      }
      break;
    default:
      return false;
    }
    nextRule = _strategy->nextRule();
  }
  return false;
}

void AnalyticTableau::createChild(const string& id, SignedFormula *fml)
{
  _children.push_back(new AnalyticTableau(id, fml, this));
  ((AnalyticTableau *) _children[_children.size()-1])->setStrategy(_strategy);
}
