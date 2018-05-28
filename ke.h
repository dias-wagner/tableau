/*****************************************************************************
 * ke.h
 *
 * Class declarations for tableau KE.
 *****************************************************************************/

#ifndef __KE_H__
#define __KE_H__

#include <string>
#include <vector>

#include "formula.h"


// alpha rules
bool KE_alpha_E_NOT_OR(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out);
bool KE_alpha_E_NOT_ORN(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out);
bool KE_alpha_E_AND(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out);
bool KE_alpha_E_ANDN(const vector<SignedFormula *>& in,
		  vector<SignedFormula *>& out);
bool KE_alpha_E_NOT_IMPLIES(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);
bool KE_alpha_E_NOT_NOT(const vector<SignedFormula *>& in,
		     vector<SignedFormula *>& out);
bool KE_alpha_E_NOT(const vector<SignedFormula *>& in,
		 vector<SignedFormula *>& out);

// beta rules.
bool KE_beta_E_OR_1(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out);
bool KE_beta_E_OR_2(const vector<SignedFormula *>& in,
		    vector<SignedFormula *>& out);
bool KE_beta_E_ORN(const vector<SignedFormula *>& in,
		   vector<SignedFormula *>& out);
bool KE_beta_E_NOT_AND_1(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);
bool KE_beta_E_NOT_AND_2(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);
bool KE_beta_E_NOT_ANDN(const vector<SignedFormula *>& in,
			vector<SignedFormula *>& out);
bool KE_beta_E_IMPLIES_1(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);
bool KE_beta_E_IMPLIES_2(const vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);


//////////////////////////////////////////////////////////////////////////////
// Encapsulates the KE tableau default strategy. It:
// - Analyses all the alphas first, top down;
// - Then, tries to analyse the first beta, top down, with a literal as
//   secondary;
// - Then, applies the PB rule with the first beta non analysed.
//////////////////////////////////////////////////////////////////////////////

class KEStrategy : public TableauStrategy
{
 public:
  KEStrategy();
  virtual ~KEStrategy();

  virtual unsigned int chooseAlpha();

  // Returns true (and updates the flag) if there is an applicable
  // beta with a literal as secondary.
  virtual bool hasApplicableBeta();

  virtual unsigned int chooseBeta();

  // Returns the literal that goes as secondary of the chosen beta.
  virtual unsigned int chooseLit();

  // Returns a pointer to a newly allocated formula on which the PB
  // will be applied.
  virtual Formula *choosePB();

  // 0=alpha; 1=beta; 2=PB; -1=none.
  virtual int nextRule();

 protected:
  // maps a formula to a tableau id if the formula has
  // been applied in the tableau.
  map<string, string> appliedPB;

  bool hasAppBeta;
  unsigned int indexAppBeta;
  unsigned int indexAppLit;
  unsigned int indexAppPB;
};


//////////////////////////////////////////////////////////////////////////////
// Encapsulates a KE tableau strategy that:
// - Analyses all the alphas first, top down;
// - Then, tries to analyse the first beta, top down, with a literal as
//   secondary;
// - Then, applies the PB rule with the first beta non analysed with value=*
//   with respect to the valuation given by the literals in the branch.
//////////////////////////////////////////////////////////////////////////////

class KEValuationStrategy : public KEStrategy
{
 public:
  KEValuationStrategy();
  virtual ~KEValuationStrategy();

  virtual Formula *choosePB();
};


//////////////////////////////////////////////////////////////////////////////
// Encapsulates a KE tableau strategy that:
// - Analyses all the alphas first, top down;
// - Then, tries to analyse the first beta, top down, with a literal as
//   secondary;
// - Then, applies the PB rule with a subformula of the first beta non
//   analysed with the properties:
//    1. must have value=* with respect to the valuation given by the literals
//       in the branch, and
//    2. for some atom A, must have an occurrence of A with the inverse
//       polarity of the literal where A occurs in the branch.
//////////////////////////////////////////////////////////////////////////////

class KEPolarityStrategy : public KEStrategy
{
 public:
  KEPolarityStrategy();
  virtual ~KEPolarityStrategy();

  virtual Formula *choosePB();
};


//////////////////////////////////////////////////////////////////////////////
// Encapsulates a KE tableau.
//////////////////////////////////////////////////////////////////////////////

class KETableau : public Tableau
{
 public:
  KETableau(const string& id, SignedFormula *fml,
	    KETableau *parent = NULL);
  KETableau(const string& id, const vector<SignedFormula *>& fmls,
	    KETableau *parent = NULL);
  ~KETableau() { }

  virtual void setStrategy(KEStrategy *strategy);

  virtual bool close();

 protected:
  enum enumRule {A_E_NOT_OR=0, A_E_NOT_ORN=1,
		 A_E_AND=2, A_E_ANDN=3,
		 A_E_NOT_IMPLIES=4, A_E_NOT_NOT=5, A_E_NOT=6,
		 B_E_OR_1=7, B_E_OR_2=8, B_E_ORN=9,
		 B_E_NOT_AND_1=10, B_E_NOT_AND_2=11, B_E_NOT_ANDN=12,
		 B_E_IMPLIES_1=13, B_E_IMPLIES_2=14};

  virtual bool applyRule(enumRule r,
			 vector<SignedFormula *>& in,
			 vector<SignedFormula *>& out);

  // Performs pre-close operations.
  virtual void preClose() { }

  // Performs post-close operations.
  virtual void postClose() { }

  // Create a child tableau.
  virtual void createChild(const string& id, SignedFormula *fml);

  // Auxiliary vectors containing the alpha, the beta formulas and the
  // literals in vector _items.
  vector<SignedFormula *> _alphas, _betas, _lits;

  // Indicates if the tableau is closed
  bool _closed;

 private:
  // Strategy object
  KEStrategy *_strategy;
};

#endif
