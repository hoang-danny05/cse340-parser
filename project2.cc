/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <cctype>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include "CFG.h"

using namespace std;

//////////////////////////////////////////////
// Task 5 helpers
//////////////////////////////////////////////

bool vecContains(vector<string> vec, string item) {
  for(int i = 0; i < vec.size(); i++) {
    if (vec.at(i)  == item )
      return true;
  }
  return false;
}


// checks lexeme, nothing else!

// returns all rules with a certain LHS
vector<Rule> ContextFreeGrammar::getRulesWith(string lhs) {
  vector<Rule> ret;

  for (Rule rule : rules) {
    if (rule.LHS == lhs) {
      ret.push_back(rule);
    }
  }

  return ret;
}

// returns all rules with a certain LHS
vector<Rule> ContextFreeGrammar::popRulesWith(string lhs) {
  vector<Rule> ret;
  vector<Rule> newRules;

  for (Rule rule : rules) {
    if (rule.LHS == lhs) {
      ret.push_back(rule);
    }
    else {
      newRules.push_back(rule);
    }
  }

  rules = newRules;
  return ret;
}

vector<Rule> ContextFreeGrammar::popRulesWithPrefix(string lhs, vector<string> prefix) {
  vector<Rule> ret;
  vector<Rule> newRules;

  for (Rule rule : rules) {
    if ((rule.LHS  == lhs ) && (rule.hasPrefix(prefix))) {
      ret.push_back(rule);
    }
    else {
      newRules.push_back(rule);
    }
  }

  rules = newRules;
  return ret;
}

void sortRules(vector<Rule>* rules) {
  for (int i = 0; i < rules->size(); i++) {
    Rule temp;
    for (int j = i+1; j < rules->size(); j++) {
      if (rules->at(j).compare(&rules->at(i)) > 0) {
        temp = rules->at(j);
        rules->at(j) = rules->at(i);
        rules->at(i) = temp;
        continue;
      }
    }
  }
}

void sortRules2(vector<Rule>* rules) {
  for (int i = 0; i < rules->size(); i++) {
    Rule temp;
    for (int j = i+1; j < rules->size(); j++) {
      if (rules->at(j).compare(&rules->at(i)) > 0) {
        temp = rules->at(j);
        rules->at(j) = rules->at(i);
        rules->at(i) = temp;
        continue;
      }
    }
  }
}

vector<string> longestCommonPrefix(const vector<Rule>& vecs) 
{
  vector<string> prefix;
  
  for (int i = 0; i < vecs.size(); i++) {
    for (int j = i+1; j < vecs.size(); j++) {
      vector<string> candidate;
      Rule rule1 = vecs[i];
      Rule rule2 = vecs[j];
      for (int k = 0; k < rule1.RHS.size() && k < rule2.RHS.size(); k++){
        if (rule1.RHS[k] == rule2.RHS[k]) {
          candidate.push_back(rule1.RHS[k]);
        }
        else {
          break;
        }
      }
      // switch if larger
      if (candidate.size() > prefix.size())
        prefix = candidate;
    }
  }

  return prefix;
}



/////////////////////////////////////////////////////////////////////////
// Task 6 helpers
/////////////////////////////////////////////////////////////////////////


void vecRemoveItem(vector<Rule> *rules, Rule rule) {
  vector<Rule> newRules;
  for (Rule r : *rules) {
    if(!(rule == r))
      newRules.push_back(r);
  }
  *rules = newRules;
}

//////////////////////////////////////////////
// end helpers
//////////////////////////////////////////////

ContextFreeGrammar cfg;

// read grammar
void ReadGrammar() {
  cfg.readGrammar();
  cfg.init();
}

/* 
 * Task 1: 
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
*/
void Task1()
{
    cfg.PrintTokens();
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
*/
void Task2()
{
    cfg.PrintNullable();
}

// Task 3: FIRST sets
void Task3()
{
    cfg.PrintFirst();
}

// Task 4: FOLLOW sets
void Task4()
{
    cfg.PrintFollow();
}

// Task 5: left factoring
void Task5()
{  
  ContextFreeGrammar cfg_prime;
  // hoping these clone the vectors
  cfg_prime.terminals = cfg.terminals;

  vector<string> operatingNonterminals = cfg.nonterminals;
  int step_count = 0;

  while (!cfg.nonterminals.empty()) {
    for (string nonterm : cfg.nonterminals) {
      if (DEBUGGING)
        cout << "starting nonterm: " << nonterm  << endl;
      vector<string> prefix = longestCommonPrefix(cfg.getRulesWith(nonterm));

      if (prefix.size() > 0) {
        if(DEBUGGING) {
          cout << "prefix exist! \nlen:" << prefix.size() << "\n";
          cout<< "Prefix:\n\t";
          for (string t : prefix) {
            cout << t  << ", ";
          }
          cout<<endl;
        }

        // remove rules with prefix
        vector<Rule> withPrefix = cfg.popRulesWithPrefix(nonterm, prefix);

        // add rule (prefix)(newTok)

        string newTok;
        for (int k = 1; k < 10; k++) {
          newTok  = nonterm  + to_string(k);
          if (!(vecContains(cfg_prime.nonterminals, newTok))) {
            break;
          }
        }

        cfg_prime.nonterminals.push_back(newTok);

        Rule condensedRule = Rule(nonterm, prefix);
        condensedRule.RHS.push_back(newTok);
        cfg.rules.push_back(condensedRule);

        // add rules with new prefix
        for (Rule rule : withPrefix) {
          rule.LHS = newTok;
          rule.cutBeginning(prefix.size());
          cfg_prime.rules.push_back(rule);
        }
        if(DEBUGGING) {
          cout << "New rules:\n";
          for (Rule r : cfg.rules) {
            cout << "> rl: ";
            r.Print();
          }
        }
      }
      else {
        // No left factoring possible!!
        // Remove rules with nonterm 
        // add to cfg_prime.rules
        for (Rule rule : cfg.popRulesWith(nonterm)) {
          cfg_prime.rules.push_back(rule);
        }
        // Remove nonterm from Nonterminals and move to cfg_prime.nonterminals
        if(DEBUGGING) {
          cout << "Before copying nonterms\n";
          for (string tok : operatingNonterminals) {
            cout << "> nt: " << tok  << "\n";
          }
        }
        cfg_prime.nonterminals.push_back(nonterm);
        vector<string> newNonterm;
        for (string nt : operatingNonterminals) {
          if (nt  != nonterm )
            newNonterm.push_back(nt);
        }
        operatingNonterminals = newNonterm;
        if (DEBUGGING)
          cout << "Removed nonterm " << nonterm  << "\n";
      } // end else
    } // for(nonterm in nonterminal)
    cfg.nonterminals = operatingNonterminals;

    if (DEBUGGING) {
      cout << "**after iteration " << ++step_count << endl;
      for (Rule rule : cfg_prime.rules) {
        rule.Print();
      }
    }

  } // end while

  sortRules(&cfg_prime.rules);
  for (Rule rule : cfg_prime.rules) {
    rule.Print();
  }
}

Rule substituteStartWith(const Rule&, const Rule&);
void eliminateIndirectRecursion(string, string, ContextFreeGrammar*);
void eliminateDirectRecursion(string, ContextFreeGrammar*);

Rule substituteStartWith(const Rule& replacee, const Rule& replacer) {
  if (replacee.RHS.size() < 1)
    std::exit(1); 
  Rule newRule = Rule(replacee.LHS, replacer.RHS);
  for (int i = 1; i < replacee.RHS.size(); i++){
    newRule.RHS.push_back(replacee.RHS[i]);
  }
  return newRule;
}

void eliminateIndirectRecursion(
  string A_i,  // current nonterminal
  string A_j,  // target of replacement
  ContextFreeGrammar* grammar
) {
  vector<Rule> offendingRules = grammar->popRulesWithPrefix(A_i, {A_j});
  vector<Rule> substitutionRules = grammar-> getRulesWith(A_j);

  // cout << "OFFENDER\n";
  // for (Rule offender : offendingRules) {
  //   offender.Print();
  // }
  // cout << "SUBBER\n";
  // for (Rule offender : substitutionRules) {
  //   offender.Print();
  // }



  for (Rule offender : offendingRules) {
    for (Rule substitution : substitutionRules) {
      // make offender Ai -> Aj alpha
      // Ai -> delta alpha
      // where Aj -> delta
      // cout << "Substituting[\n";
      // substitution.Print();
      // offender.Print();
      // cout << "]\n";
      grammar->rules.push_back(substituteStartWith(offender, substitution));
      // grammar->Print();
    }
  }
  sortRules(&grammar->rules);
}

void eliminateDirectRecursion(
  string A_i,
  ContextFreeGrammar* grammar
) {
  vector<Rule> offendingRules = grammar->popRulesWithPrefix(A_i, {A_i});

  if (offendingRules.size() == 0) {
    return;
  }

  vector<Rule> substitutionRules = grammar->popRulesWith(A_i);

  // cout << "OFFENDER\n";
  // for (Rule offender : offendingRules) {
  //   offender.Print();
  // }
  // cout << "SUSTITUTER:\n";
  // for (Rule offender : substitutionRules) {
  //   offender.Print();
  // }

  // generate new token
  string A_new;
  for (int k = 1; k < 10; k++) {
    A_new  = A_i  + to_string(k);
    if (!(vecContains(grammar->nonterminals, A_new))) {
      //grammar->nonterminals.push_back(newTok);
      break;
    }
  }

  // add null rule
  // A_new -> null
  grammar->rules.push_back(Rule(A_new, vector<string>()));

  // the form A_i -> A_i (alpha)
  // to
  // A_new -> alpha A_new
  for (Rule offender : offendingRules) {
    offender.cutBeginning(1);
    offender.LHS = A_new;
    offender.RHS.push_back(A_new);
    grammar->rules.push_back(offender);
  }

  // the form A_i -> (beta)
  // to 
  // A_i -> (beta) (A_new)
  for (Rule substitution : substitutionRules) {
    substitution.RHS.push_back(A_new);
    grammar->rules.push_back(substitution);
  }
  sortRules(&grammar->rules);
}

// Task 6: eliminate left recursion
void Task6()
{
  // sort nonterminals
  for (int i = 0; i < cfg.nonterminals.size(); i++) {
    for (int j = i+1; j < cfg.nonterminals.size(); j++) {
      if (cfg.nonterminals[i].compare(cfg.nonterminals[j]) > 0){
        string temp = cfg.nonterminals[i];
        cfg.nonterminals[i] = cfg.nonterminals[j];
        cfg.nonterminals[j] = temp;
      }
    }
  }
  sortRules(&cfg.rules);
  //cfg.PrintTokens();


  // indirect left recursion, no rule can start with a rule before
  for (int i = 0; i < cfg.nonterminals.size(); i++) {
    for (int j = 0; j < i; j++) {
      eliminateIndirectRecursion(
        cfg.nonterminals[i],
        cfg.nonterminals[j],
        &cfg
      );
    }
    eliminateDirectRecursion(
      cfg.nonterminals[i],
      &cfg
    );
  }
  sortRules2(&cfg.rules);
  cfg.Print();


    /*
     * A->A(alpha)
     * A->(beta)
     *
     * goes to 
     *
     * A-> (beta)A1
     * A1 -> (alpha)A1 | epsilon
     *
     *
     * Ex)
     *
     * A->Aa
     * A->b 
     * ==
     * A->bA1 
     * A1->aA1 | epsilon
     *
     * A->Aa
     * A->Ac
     * A->b 
     * ==
     * A->bA1
     * A1->aA1 | cA1 | epsilon 
     *
     * A-> Aa
     * A-> b
     * A-> d
     * ==
     * A-> bA1 | dA1 
     * A1->aA1 | epsilon
     *
     * */
    
}



    
int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: Task1();
            break;

        case 2: Task2();
            break;

        case 3: Task3();
            break;

        case 4: Task4();
            break;

        case 5: Task5();
            break;
        
        case 6: Task6();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

