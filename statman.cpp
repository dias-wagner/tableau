// statman: generates the S_n family of theorems.

#include <cstdio>
#include <cstring>
#include <fstream>

#include <iostream>
#include <cstdlib>

using namespace std;

#include "formula.h"

//
// Usage: statman -from n0 -to n [-n]
//
// Generates all the formulae S_n0, S_(n0+1), ..., S_n
//
// -n: generates formulae w/o implication and conjunction
//

void usage()
{
  cout << "Usage: statman -from n0 -to n [-n]" << endl;
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
    vector<Formula *> A, B, F;

    unsigned int k, j;
    for (k = 0; k < (unsigned int) n-1; k++) {
      vector<Formula *> vAND;
      for (j = 0; j <= k; j++) {
	char cj[10], dj[10];
	sprintf(cj, "c%d", j);
	sprintf(dj, "d%d", j);
	if (! neg)
	  vAND.push_back(new Formula(Formula::OR,
				     new Formula(cj),
				     new Formula(dj)));
	else
	  vAND.push_back(new Formula(Formula::NOT,
				     new Formula(Formula::OR,
						 new Formula(cj),
						 new Formula(dj))));
      }
      if (! neg) {
	if (vAND.size() == 1)
	  F.push_back(vAND[0]);
	else if (vAND.size() == 2)
	  F.push_back(new Formula(Formula::AND, vAND[0], vAND[1]));
	else
	  F.push_back(new Formula(Formula::ANDN, vAND));
      }
      else {
	if (vAND.size() == 1)
	  F.push_back(new Formula(Formula::NOT, vAND[0]));
	else if (vAND.size() == 2)
	  F.push_back(new Formula(Formula::NOT,
				  new Formula(Formula::OR, vAND[0], vAND[1])));
	else
	  F.push_back(new Formula(Formula::NOT,
				  new Formula(Formula::ORN, vAND)));
      }
    }

    for (j = 0; j < (unsigned int) n; j++) {
      char cj[10], dj[10];
      sprintf(cj, "c%d", j);
      sprintf(dj, "d%d", j);
      if (j == 0) {
	A.push_back(new Formula(cj));
	B.push_back(new Formula(dj));
      }
      else {
	if (! neg) {
	  A.push_back(new Formula(Formula::IMPLIES,
				  F[j-1], new Formula(cj)));
	  B.push_back(new Formula(Formula::IMPLIES,
				  F[j-1], new Formula(dj)));
	}
	else {
	  A.push_back(new Formula(Formula::OR,
				  new Formula(Formula::NOT, F[j-1]),
				  new Formula(cj)));
	  B.push_back(new Formula(Formula::OR,
				  new Formula(Formula::NOT, F[j-1]),
				  new Formula(dj)));
	}
      }
    }

    char filename[30];
    if (! neg)
      sprintf(filename, "statman%d.prove", n);
    else
      sprintf(filename, "statmann%d.prove", n);

    ofstream f(filename);

    vector<Formula *> X;
    for (j = 0; j < (unsigned int) n; j++)
      X.push_back(new Formula(Formula::OR, A[j], B[j]));
    
    char cn[10], dn[10];
    sprintf(cn, "c%d", n-1);
    sprintf(dn, "d%d", n-1);

    X.push_back(new Formula(Formula::OR,
			    new Formula(cn),
			    new Formula(dn)));

    for (j = 0; j < X.size(); j++) {
      if (j != X.size()-1)
	f << "T";
      else
	f << "F";
      f << X[j]->toString() << endl;
    }

    f.close();
  }

  return 0;
}
