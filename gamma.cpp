// gamma: generates the gamma_n family of theorems.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include <iostream>
#include <cstdlib>

using namespace std;

#include "formula.h"

//
// Usage: gamma -from n0 -to n [-n] [-l num]
//
// Generates all the formulae gamma_n0, gamma_(n0+1), ..., gamma_n
//
// -n: generates formulae w/o conjunction and implication
// -l num: adds to G a set of num% * |G| random irrelevant formulae,
//         where G is the original set of formulae

void usage()
{
  cout << "Usage: gamma -from n0 -to n [-n] [-l num]" << endl;
  return;
}

int main(int argc, char **argv)
{
  int arg, n0 = -1, nn = -1;
  unsigned int l = 0;
  bool syntax = false, neg = false;
  bool has_l = false;

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
    else if (strcmp(argv[arg], "-l") == 0) {
      if (arg+1 < argc) {
	has_l = true;
	l = atoi(argv[arg+1]);
	arg++;
      }
      else
	syntax = true;
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

  if (l < 0)
    l = 0;

  int n;

  unsigned int seed;

  if (l > 0) {
    printf("Type any number... ");
    scanf("%d", &seed);
    srand(seed);
  }

  for (n = n0; n <= nn; n++) {
    int i;
    vector<Formula *> gamma;

    // For the random generation of formulae, we must have:
    // - The vector of relevant atoms
    vector<string> r_atoms;
    // - The vector of irrelevant atoms
    vector<string> ir_atoms;
    // - The minimum and the maximum formula size (measured approx. in
    // #connectives)
    unsigned int r_min = 1, r_max = 3;

    r_atoms.push_back("a1");
    r_atoms.push_back("b1");
    ir_atoms.push_back("c1");
    ir_atoms.push_back("d1");

    for (i = 1; i <= n; i++) {
      char ai[10], bi[10], ai1[10], bi1[10];
      char ci1[10], di1[10];
      sprintf(ai, "a%d", i);
      sprintf(bi, "b%d", i);
      sprintf(ai1, "a%d", i+1);
      sprintf(bi1, "b%d", i+1);

      sprintf(ci1, "c%d", i+1);
      sprintf(di1, "d%d", i+1);

      r_atoms.push_back(ai1);
      r_atoms.push_back(bi1);
      ir_atoms.push_back(ci1);
      ir_atoms.push_back(di1);
      
      if (! neg) {
	gamma.push_back(new Formula(Formula::IMPLIES,
				    new Formula(string(ai)),
				    new Formula(Formula::OR,
						new Formula(string(ai1)),
						new Formula(string(bi1)))));
	gamma.push_back(new Formula(Formula::IMPLIES,
				    new Formula(string(bi)),
				    new Formula(Formula::OR,
						new Formula(string(ai1)),
						new Formula(string(bi1)))));
      }
      else {
	gamma.push_back(new Formula(Formula::OR,
				    new Formula(Formula::NOT,
						new Formula(string(ai))),
				    new Formula(Formula::OR,
						new Formula(string(ai1)),
						new Formula(string(bi1)))));
	gamma.push_back(new Formula(Formula::OR,
				    new Formula(Formula::NOT,
						new Formula(string(bi))),
				    new Formula(Formula::OR,
						new Formula(string(ai1)),
						new Formula(string(bi1)))));
      }
    }
    
    Formula *a1b1 = new Formula(Formula::OR,
				new Formula("a1"),
				new Formula("b1"));
    char an1[10], bn1[10];
    char cn1[10], dn1[10];

    sprintf(an1, "a%d", n+1);
    sprintf(bn1, "b%d", n+1);

    sprintf(cn1, "c%d", n+1);
    sprintf(dn1, "d%d", n+1);

    r_atoms.push_back(an1);
    r_atoms.push_back(bn1);
    ir_atoms.push_back(cn1);
    ir_atoms.push_back(dn1);

    Formula *an1bn1 = new Formula(Formula::OR,
				  new Formula(string(an1)),
				  new Formula(string(bn1)));

    // Generation of random formulae
    vector<Formula *> l_fml;
    for (unsigned int ll = 0; ll < l; ll++) {
      unsigned int size = r_min+(int) ((double)(r_max*rand())/(RAND_MAX+1.0));
      Formula *r_fml;
      unsigned int ind =
	(int) ((double)(ir_atoms.size()-1*rand())/(RAND_MAX+1.0));
      
      r_fml = new Formula(ir_atoms[ind]);

      for (unsigned int j = 0; j < size; j++) {
	// Choose the operator
	unsigned int op = 1+(int) ((double)(4.0*rand())/(RAND_MAX+1.0));
	switch (op) {
	case 1: // NOT
	  {
	    r_fml = new Formula(Formula::NOT, r_fml);
	  }
	  break;
	case 2: // AND
	  {
	    if (rand() % 2) {
	      ind = (int) ((double)(r_atoms.size()-1*rand())/(RAND_MAX+1.0));
	      r_fml = new Formula(Formula::AND,
				  new Formula(r_atoms[ind]), r_fml);
	    }
	    else {
	      ind = (int) ((double)(ir_atoms.size()-1*rand())/(RAND_MAX+1.0));
	      r_fml = new Formula(Formula::AND,
				  new Formula(ir_atoms[ind]), r_fml);
	    }

	  }
	  break;
	case 3: // OR
	  {
	    ind = (int) ((double)(ir_atoms.size()-1*rand())/(RAND_MAX+1.0));
	    r_fml = new Formula(Formula::OR,
				new Formula(ir_atoms[ind]), r_fml);
	  }
	  break;
	case 4: // IMPLIES
	  {
	    ind = (int) ((double)(r_atoms.size()-1*rand())/(RAND_MAX+1.0));
	    if (rand() % 2)
	      r_fml = new Formula(Formula::IMPLIES,
				  new Formula(r_atoms[ind]), r_fml);
	    else
	      r_fml = new Formula(Formula::IMPLIES, r_fml,
				  new Formula(r_atoms[ind]));
	  }
	  break;
	}
      }
      l_fml.push_back(r_fml);
    }
    char filename[30], cn[2], cl[2];
    sprintf(cn, "%s", neg?"n":"");
    sprintf(cl, "%s", has_l?"l":"");

    sprintf(filename, "gamma%s%s%d.prove", cn, cl, n);

    ofstream f(filename);
    
    f << "T" << a1b1->toString() << endl;
    for (unsigned int j = 0; j < gamma.size() || j < l_fml.size(); j++) {
      if (j < gamma.size())
	f << "T" << gamma[j]->toString() << endl;
      if (j < l_fml.size())
	f << "T" << l_fml[j]->toString() << endl;
    }
    f << "F" << an1bn1->toString() << endl;

    f.close();

    delete a1b1;
    for (unsigned int j = 0; j < gamma.size(); j++)
      delete gamma[j];

    for (unsigned int j = 0; j < l_fml.size(); j++)
      delete l_fml[j];    

    delete an1bn1;
  }

  return 0;
}
