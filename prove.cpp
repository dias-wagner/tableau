#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <sys/time.h>

#include <cstring>

#include "formula.h"
#include "tableau.h"
#include "analytic.h"
#include "ke.h"
#include "kes3.h"

using namespace std;

//
// Usage: prove [-m analytic[+BU]*|ke[+V|P]|kes3[+PB]] [-v] -f file
//
// * - default
//

void usage()
{
  cout << "Usage: prove [-m analytic[+BU]*|ke[+V|P]|kes3[+PB]] [-v] -f %.prove|%.cnf" << endl;
  return;
}


bool readProve(const string& file, vector<SignedFormula *>& v)
{
  string s_fml;
  char sign;
  Formula *p_fml;

  ifstream in(file.c_str());

  in >> sign;
  in >> s_fml;
  while (!in.eof()) {
    p_fml = parse(s_fml);
    if (p_fml) {
      v.push_back(new SignedFormula(
			sign=='T'?SignedFormula::S_T:SignedFormula::S_F,
			p_fml));
    }
    else {
      in.close();
      return false;
    }
    in >> sign;
    in >> s_fml;
  }
  in.close();
  return true;
}


// Assumes:
// - lines with less than (or) 256 characters
// - each clause occurs in a line

bool readCNF(const string& file, vector<SignedFormula *>& v)
{
  ifstream in(file.c_str());
  char buffer[256];
  string line;

  int vars, clauses, cl=0;

  while (! in.eof() && cl != clauses) {
    in.getline(buffer, 256);
    if (buffer[0] == 'c') // comment line
      continue;
    else if (buffer[0] == 'p') // problem line
      sscanf(buffer, "p cnf %d %d\n", &vars, &clauses);
    else { // clause line
      vector<int> vv;
      int n;
      char *sn = strtok(buffer, " \t");
      n = atoi(sn);
      vv.push_back(n);
      while (n != 0) {
	sn = strtok(NULL, " \t");
	n = atoi(sn);
	if (n != 0) vv.push_back(n);
      }
      Formula *fml = NULL;
      for (unsigned int i = 0; i < vv.size(); i++) {
	sprintf(buffer, "x%d", abs(vv[i]));
	if (fml == NULL) {
	  if (vv[i] > 0)
	    fml = new Formula(string(buffer));
	  else
	    fml = new Formula(Formula::NOT, new Formula(string(buffer)));
	}
	else {
	  if (vv[i] > 0)
	    fml = new Formula(Formula::OR, fml, new Formula(string(buffer)));
	  else
	    fml = new Formula(Formula::OR,
			      fml,
			      new Formula(Formula::NOT,
					  new Formula(string(buffer))));
	}
      }
      v.push_back(new SignedFormula(SignedFormula::S_T, fml));
      cl++;
    }
  }
  return true;
}


int main(int argc, char **argv)
{
  string method = "analytic", file = "";
  bool syntax = false, verbose = false, cnf = false;
  int arg;
  
  for (arg = 1; ! syntax && arg < argc; arg++) {
    if (strcmp(argv[arg], "-v") == 0)
      verbose = true;
    else if (strcmp(argv[arg], "-m") == 0) {
      if (arg+1 < argc && 
	  (strcmp(argv[arg+1], "analytic") == 0 ||
	   strcmp(argv[arg+1], "analytic+BU") == 0 ||
	   strcmp(argv[arg+1], "ke") == 0 ||
	   strcmp(argv[arg+1], "ke+V") == 0 ||
	   strcmp(argv[arg+1], "ke+P") == 0 ||
	   strcmp(argv[arg+1], "kes3") == 0 ||
	   strcmp(argv[arg+1], "kes3+PB") == 0)) {
	method = argv[arg+1];
	arg++;
      }
      else
	syntax = true;
    }
    else if (strcmp(argv[arg], "-f") == 0) {
      if (arg+1 < argc) {
	file = argv[arg+1];
	if (file.size() > 4 && file.substr(file.size()-4, 4) == ".cnf")
	  cnf = true;
	else if (file.size() > 5 && file.substr(file.size()-6, 6) == ".prove")
	  cnf = false;
	else
	  syntax = true;
	arg++;
      }
      else
	syntax = true;
    }
    else
      syntax = true;
  }
  
  if (file == "")
    syntax = true;
  
  if (syntax) {
    usage();
    return 1;
  }
  
  vector<SignedFormula *> v;

  bool read_ok;
  if (cnf)
    read_ok = readCNF(file, v);
  else 
    read_ok = readProve(file, v);

  Tableau *tab;
  
  if (method == "analytic") {
    tab = new AnalyticTableau("1", v);
    ((AnalyticTableau *) tab)->setStrategy(new AnalyticStrategy());
  }
  else if (method == "analytic+BU") {
    tab = new AnalyticTableau("1", v);
    ((AnalyticTableau *) tab)->setStrategy(new AnalyticBottomUpStrategy());
  }
  else if (method == "ke") {
    tab = new KETableau("1", v);
    ((KETableau *) tab)->setStrategy(new KEStrategy());
  }
  else if (method == "ke+V") {
    tab = new KETableau("1", v); 
    ((KETableau *) tab)->setStrategy(new KEValuationStrategy());
  }
  else if (method == "ke+P") {
    tab = new KETableau("1", v); 
    ((KETableau *) tab)->setStrategy(new KEPolarityStrategy());
  }
  else if (method == "kes3") {
    tab = new KES3Tableau("1", v); 
    ((KES3Tableau *) tab)->setStrategy(new KES3Strategy());
  }
  else if (method == "kes3+PB") {
    tab = new KES3Tableau("1", v); 
    ((KES3Tableau *) tab)->setStrategy(new KES3AENOTLastStrategy());
  }
  else { // default
    tab = new KETableau("1", v); 
    ((KETableau *) tab)->setStrategy(new KEPolarityStrategy());
  }
  
  if (verbose) {
    cout << endl;
    cout << tab->toString() << endl;
    cout << "-------------------------------" << endl << endl;
  }

  struct timeval startt, endt;

  gettimeofday(&startt, NULL);

  bool closed = tab->close();

  gettimeofday(&endt, NULL);

  if (endt.tv_usec < startt.tv_usec) {
    endt.tv_sec--;
    endt.tv_usec += 1000000;
  }

  char elapsed[20];
  sprintf(elapsed, "%ld.%06ld",
	  endt.tv_sec - startt.tv_sec, endt.tv_usec - startt.tv_usec);

  if (verbose) {
    if (closed)
      cout << tab->toString() << "x" << endl;
    else
      cout << tab->toString() << endl;
    
    cout << endl
	 << "Total number of nodes:    " << tab->countNodes() << endl
	 << "Total number of formulae: " << tab->countFormulae() << endl
	 << "Elapsed time (s):         " << elapsed << endl;
  }
  else {
    cout //<< (closed ? 1 : 0) << "\t"
	 << tab->countNodes() << " & "
	 << tab->countFormulae() << " & "
	 << elapsed << " & ";
    if (method.substr(0, 4) == "kes3")
      cout << ((KES3Tableau *) tab)->S().size() << " & ";//endl;
    else
      cout << " & ";//endl;
  }

  delete tab;

  return 0;
}
