/*****************************************************************************
 * analytic.h
 *
 * Class declarations for analytic tableau.
 *****************************************************************************/

#ifndef __ANALYTIC_H__
#define __ANALYTIC_H__

#include <vector>

#include "formula.h"
#include "tableau.h"


// alpha rules
bool alpha_E_NOT_OR(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out);
bool alpha_E_NOT_ORN(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out);
bool alpha_E_AND(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out);
bool alpha_E_ANDN(const vector<SignedFormula *>& in,
		  vector<SignedFormula *>& out);
bool alpha_E_NOT_IMPLIES(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);
bool alpha_E_NOT_NOT(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out);
bool alpha_E_NOT(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out);

// beta rules
bool beta_E_OR(const vector<SignedFormula *>& in,
	       vector<SignedFormula *>& out);
bool beta_E_ORN(const vector<SignedFormula *>& in,
		vector<SignedFormula *>& out);
bool beta_E_NOT_AND(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out);
bool beta_E_NOT_ANDN(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out);
bool beta_E_IMPLIES(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out);


//////////////////////////////////////////////////////////////////////////////
// Encapsulates the analytic tableau default strategy. It:
// - Analyses all the alphas first, top down;
// - Then, analyses the first beta, top down.
//////////////////////////////////////////////////////////////////////////////

class AnalyticStrategy : public TableauStrategy
{
 public:
  AnalyticStrategy();
  ~AnalyticStrategy();
  
  virtual unsigned int chooseAlpha();
  virtual unsigned int chooseBeta();
  virtual int nextRule();
};


//////////////////////////////////////////////////////////////////////////////
// Encapsulates an analytic tableau strategy that:
// - Analyses all the alphas first, bottom up;
// - Then, analyses the first beta, bottom up.
//////////////////////////////////////////////////////////////////////////////

class AnalyticBottomUpStrategy : public AnalyticStrategy
{
 public:
  AnalyticBottomUpStrategy();
  ~AnalyticBottomUpStrategy();

  virtual unsigned int chooseAlpha();
  virtual unsigned int chooseBeta();
};


//////////////////////////////////////////////////////////////////////////////
// Encapsulates an analytic tableau.
//////////////////////////////////////////////////////////////////////////////

class AnalyticTableau : public Tableau
{
 public:
  AnalyticTableau(const string& id, SignedFormula *fml,
		  AnalyticTableau *parent = NULL);
  AnalyticTableau(const string& id, const vector<SignedFormula *>& fmls,
		  AnalyticTableau *parent = NULL);
  ~AnalyticTableau() { }

  // Sets the strategy object.
  virtual void setStrategy(AnalyticStrategy *strategy);

  bool close();

 protected:  
  enum enumRule {A_E_NOT_OR=0, A_E_NOT_ORN,
		 A_E_AND, A_E_ANDN,
		 A_E_NOT_IMPLIES, A_E_NOT_NOT, A_E_NOT,
		 B_E_OR, B_E_ORN,
		 B_E_NOT_AND, B_E_NOT_ANDN,
		 B_E_IMPLIES};

  bool applyRule(AnalyticTableau::enumRule r,
		 vector<SignedFormula *>& in, vector<SignedFormula *>& out);

  // Create a child tableau
  virtual void createChild(const string& id, SignedFormula *fml);

  // Auxiliary vectors containing the alpha, the beta formulas and the
  // literals in vector _items.
  vector<SignedFormula *> _alphas, _betas, _lits;

  // Indicates if the tableau is closed
  bool _closed;
  
 private:
  // Strategy object
  AnalyticStrategy *_strategy;
};

#endif
