// h: generates the h_n family of theorems.

#include <cstdio>
#include <cstring>
#include <fstream>

#include <iostream>
#include <cstdlib>

using namespace std;

#include "formula.h"

//
// Usage: h -from n0 -to n [-n]
//
// Generates all the formulae h_n0, h_(n0+1), ..., h_n
//
// -n: generates formulae w/o implication and conjunction
//

void usage()
{
  cout << "Usage: h -from n0 -to n [-n]" << endl;
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

  int n, power=1;

  for (n = 1; n < n0; n++)
    power *= 2;

  for (n = n0; n <= nn; n++) {
    int i, j;
    Formula *H;
    vector<Formula *> conjunctions;

    power *= 2;

    for (i = 0; i < power; i++) {
      vector<Formula *> literals;
      int mask=1;
      for (j = 1; j <= n; j++) {
	char atom[10];
	sprintf(atom, "p%d", j);
	if (i & mask) {
	  if (! neg)
	    literals.push_back(new Formula(string(atom)));
	  else
	    literals.push_back(new Formula(Formula::NOT,
					   new Formula(string(atom))));
	}
	else {
	  if (! neg)
	    literals.push_back(new Formula(Formula::NOT,
					   new Formula(string(atom))));
	  else
	    literals.push_back(new Formula(string(atom)));
	}
	mask = mask << 1;
      }

      if (literals.size() == 1) {
	if (! neg)
	  conjunctions.push_back(literals[0]);
	else
	  conjunctions.push_back(new Formula(Formula::NOT, literals[0]));
      }
      else if (literals.size() == 2) {
	if (! neg)
	  conjunctions.push_back(new Formula(Formula::AND,
					     literals[0], literals[1]));
	else
	  conjunctions.push_back(new Formula(Formula::NOT,
					     new Formula(Formula::OR,
							 literals[0],
							 literals[1])));
      }
      else {
	if (! neg)
	  conjunctions.push_back(new Formula(Formula::ANDN, literals));
	else
	  conjunctions.push_back(new Formula(Formula::NOT,
					     new Formula(Formula::ORN,
							 literals)));
      }
    }

    if (conjunctions.size() == 1)
      H = conjunctions[0];
    else if (conjunctions.size() == 2)
      H = new Formula(Formula::OR, conjunctions[0], conjunctions[1]);
    else
      H = new Formula(Formula::ORN, conjunctions);
    
    char filename[30];
    if (! neg)
      sprintf(filename, "h%d.prove", n);
    else
      sprintf(filename, "hn%d.prove", n);

    ofstream f(filename);
    
    f << "F" << H->toString() << endl;

    f.close();

    delete H;
  }

  return 0;
}
