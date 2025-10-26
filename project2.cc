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

bool vecContains(std::vector<string> vec, string item) {
  for(int i = 0; i < vec.size(); i++) {
    if (vec.at(i)  == item )
      return true;
  }
  return false;
}


// checks lexeme, nothing else!

// returns all rules with a certain LHS
std::vector<Rule> ContextFreeGrammar::getRulesWith(string lhs) {
  std::vector<Rule> ret;

  for (Rule rule : rules) {
    if (rule.LHS == lhs) {
      ret.push_back(rule);
    }
  }

  return ret;
}

// returns all rules with a certain LHS
std::vector<Rule> ContextFreeGrammar::popRulesWith(string lhs) {
  std::vector<Rule> ret;
  std::vector<Rule> newRules;

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

std::vector<Rule> ContextFreeGrammar::popRulesWithPrefix(string lhs, std::vector<string> prefix) {
  std::vector<Rule> ret;
  std::vector<Rule> newRules;

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

void sortRules(std::vector<Rule>* rules) {
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


std::vector<string> longestCommonPrefix(const std::vector<Rule>& vecs) 
{
  std::vector<string> prefix;
  
  for (int i = 0; i < vecs.size(); i++) {
    for (int j = i+1; j < vecs.size(); j++) {
      std::vector<string> candidate;
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


void vecRemoveItem(std::vector<Rule> *rules, Rule rule) {
  std::vector<Rule> newRules;
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
    cfg.Printstrings();
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

  std::vector<string> operatingNonterminals = cfg.nonterminals;
  int step_count = 0;

  while (!cfg.nonterminals.empty()) {
    for (string nonterm : cfg.nonterminals) {
      if (DEBUGGING)
        std::cout << "starting nonterm: " << nonterm  << std::endl;
      std::vector<string> prefix = longestCommonPrefix(cfg.getRulesWith(nonterm));

      if (prefix.size() > 0) {
        if(DEBUGGING) {
          std::cout << "prefix exist! \nlen:" << prefix.size() << "\n";
          cout<< "Prefix:\n\t";
          for (string t : prefix) {
            cout << t  << ", ";
          }
          cout<<endl;
        }

        // remove rules with prefix
        std::vector<Rule> withPrefix = cfg.popRulesWithPrefix(nonterm, prefix);

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

// Task 6: eliminate left recursion
void Task6()
{
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

