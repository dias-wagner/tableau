// php: generates the php_n family (Pigeon Hole Principle with n
// holes) of theorems.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <utility>

#include <iostream>
#include <cstdlib>

#include "formula.h"

using namespace std;

//
// Usage: php -from n0 -to n [-n]
//
// Generates all the formulae php_n0, php_(n0+1), ..., php_n
//
// -n: generates formulae w/o conjunction and implication
//

void usage()
{
  cout << "Usage: php -from n0 -to n [-n]" << endl;
  return;
}

int main(int argc, char **argv)
{
  int arg, n0 = -1, nn = -1;
  bool syntax = false, neg = false;

  for (arg = 1; ! syntax && arg < argc; arg++) {
    if (strcmp(argv[arg], "-from") == 0) {
      if (arg+1 < argc) {
	n0 = atoi(argv[arg+1]);
	arg++;
      }
      else
	syntax = true;
    }
    else if (strcmp(argv[arg], "-to") == 0) {
      if (arg+1 < argc) {
	nn = atoi(argv[arg+1]);
	arg++;
      }
      else
	syntax = true;
    }
    else if (strcmp(argv[arg], "-n") == 0) {
      neg = true;
    }
    else
      syntax = true;
  }

  if (n0 < 1 || nn < 1 || n0 > nn) {
    cout << "Error: n0 and n must be greater than zero and n0 < n" << endl;
    syntax = true;
  }

  if (syntax) {
    usage();
    return 1;
  }

  int n;

  for (n = n0; n <= nn; n++) {
    int i, j, k;
    Formula *A, *B;

    map<pair<int, int>, Formula *> P;
    for (i = 0; i <= n; i++)
      for (j = 0; j < n; j++) {
	char atom[10];
	sprintf(atom, "p%d,%d", i, j);
	P[make_pair(i, j)] = new Formula(string(atom));
      }
    
    // A: every source is mapped to some destination
    
    A = NULL;
    vector<Formula *> fml_and;
    for (i = 0; i <= n; i++) {
      vector<Formula *> fml_or;
      for (j = 0; j < n; j++)
	fml_or.push_back(P[make_pair(i, j)]);
      
      if (! neg) {
	if (fml_or.size() == 1)
	  fml_and.push_back(fml_or[0]);
	else if (fml_or.size() == 2)
	  fml_and.push_back(new Formula(Formula::OR, fml_or[0], fml_or[1]));
	else
	  fml_and.push_back(new Formula(Formula::ORN, fml_or));
      }
      else {
	if (fml_or.size() == 1)
	  fml_and.push_back(new Formula(Formula::NOT, fml_or[0]));
	else if (fml_or.size() == 2)
	  fml_and.push_back(new Formula(Formula::NOT, new Formula(Formula::OR,
								  fml_or[0],
								  fml_or[1])));
	else
	  fml_and.push_back(new Formula(Formula::NOT, new Formula(Formula::ORN,
								  fml_or)));
      }
    }

    if (! neg) {
      if (fml_and.size() == 1)
	A = fml_and[0];
      else if (fml_and.size() == 2)
	A = new Formula(Formula::AND, fml_and[0], fml_and[1]);
      else
	A = new Formula(Formula::ANDN, fml_and);
    }
    else {
      if (fml_and.size() == 1)
	A = new Formula(Formula::NOT, fml_and[0]);
      else if (fml_and.size() == 2)
	A = new Formula(Formula::NOT, new Formula(Formula::OR,
						  fml_and[0],
						  fml_and[1]));
      else
	A = new Formula(Formula::NOT, new Formula(Formula::ORN, fml_and));
    }

    // B: at least one destination receives more than one source
    
    B = NULL;
    vector<Formula *> fml_or;
    for (j = 0; j < n; j++)
      for (i = 0; i < n; i++)
	for (k = i+1; k <= n; k++)
	  if (! neg)
	    fml_or.push_back(new Formula(Formula::AND,
					 P[make_pair(i, j)],
					 P[make_pair(k, j)]));
	  else
	    fml_or.push_back(new Formula(Formula::NOT,
				 new Formula(Formula::OR,
				     new Formula(Formula::NOT,
						 P[make_pair(i, j)]),
				     new Formula(Formula::NOT,
						 P[make_pair(k, j)]))));
    
    if (fml_or.size() == 1)
      B = fml_or[0];
    else if (fml_or.size() == 2)
      B = new Formula(Formula::OR, fml_or[0], fml_or[1]);
    else
      B = new Formula(Formula::ORN, fml_or);

    Formula *php;

    if (! neg)
      php = new Formula(Formula::IMPLIES, A, B);
    else
      php = new Formula(Formula::OR, new Formula(Formula::NOT, A), B);

    char filename[30];
    if (! neg)
      sprintf(filename, "php%d.prove", n);
    else
      sprintf(filename, "phpn%d.prove", n);

    ofstream f(filename);
    
    f << "F" << php->toString() << endl;

    f.close();

    //delete php;
  }

  return 0;
}
