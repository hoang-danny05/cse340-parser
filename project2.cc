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
ContextFreeGrammar cfg;

bool vecContains(vector<string> vec, string item) {
  for(int i = 0; i < vec.size(); i++) {
    if (vec.at(i)  == item )
      return true;
  }
  return false;
}


//////////////////////////////////////////////
// Task 5 helpers
//////////////////////////////////////////////

// lexicograpic

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
      if (rules->at(j).compare2(&rules->at(i)) > 0) {
        temp = rules->at(j);
        rules->at(j) = rules->at(i);
        rules->at(i) = temp;
        continue;
      }
    }
  }
}

// recommended helpers

int length_of_common_prefix(Rule rule1, Rule rule2) {
  int i = 0;
  // cout << "common prefix len for[\n";
  // rule1.Print();
  // rule2.Print();
  // cout<<"]:";
  for (; i < rule1.RHS.size() && i < rule2.RHS.size(); i++) {
    if (!(rule1.RHS[i] == rule2.RHS[i])) {
      return (i);
    }
  }
  return (i);
}


int longest_match(const ContextFreeGrammar* grammar, const Rule rule) {
  int longestMatch = 0;
  for (Rule r : grammar->getRulesWith(rule.LHS)) {
    if (rule.RHS == r.RHS) {
      continue;
    }
    if (length_of_common_prefix(r, rule) > longestMatch) {
      longestMatch = length_of_common_prefix(r, rule);
    }
  }
  return longestMatch;
}

// // <1 if rule1 is before
// // >1 if rule1 is after
// int compareByLongestMatch(ContextFreeGrammar* grammar, Rule rule1, Rule rule2) {
//   cout << "comp: [\n";
//   rule1.Print();
//   rule2.Print();
//   cout  << "]\n";
//   if (rule1.LHS.compare(rule2.LHS) > 0) {
//     cout << "before!\n\n";
//     return -1;
//   }
//   if (rule1.LHS.compare(rule2.LHS) < 0) {
//     cout << "after!\n\n";
//     return 1;
//   }
//   if (longest_match(grammar, rule1) < longest_match(grammar, rule2)) 
//     return -1;
//   if (longest_match(grammar, rule1) > longest_match(grammar, rule2)) 
//     return 1;
//   return rule1.compare(&rule2);
// }

string generateTokenNotIn(const string&, const vector<string>);

// sort rules by this order:
// LHS
// longest prefix
// RHS

void sortRulesByMatch(ContextFreeGrammar* grammar) {
  vector<Rule> rules = grammar->rules;
  //sort by lexicographic order
  sortRules2(&grammar->rules);
  
  // sort by the longest prefix
  for (int i = 1; i < grammar->rules.size(); ++i) {
    Rule item = grammar->rules[i];
    int key = longest_match(grammar, item);
    int j = i - 1;
    // while (j >= 0 && arr[j] > key) {
    //                  arr(j) goes after key
    // cout << "Sort by longest match of:" << endl;
    // grammar->rules[i].Print();
    // grammar->rules[j].Print();
    // cout<< longest_match(grammar, grammar->rules[j]), "\n\n";
    for (; j >= 0 && longest_match(grammar, grammar->rules[j]) < longest_match(grammar, item); j--) {
      // cout<<"Shift left\n";
      grammar->rules[j+1] = grammar->rules[j];
    }
    // arr[j + 1] = key;
    grammar->rules[j+1] = item;
  }

  // grammar->Print();

  // sort by LHS
  for (int i = 1; i < grammar->rules.size(); ++i) {
    Rule item = grammar->rules[i];
    string key = item.LHS;
    int j = i - 1;

    // while (j >= 0 && arr[j] > key) {
    //                  arr(j) goes after key
    for (; j >= 0 && grammar->rules[j].LHS.compare(key) > 0; j--) {
        // arr[j + 1] = arr[j];
        grammar->rules[j+1] = grammar->rules[j];
    }
    // arr[j + 1] = key;
    grammar->rules[j+1] = item;
  }

  // for (int i = 0; i < rules.size(); i++) {
  //   Rule temp;
  //   for (int j = i+1; j < rules.size(); j++) {
  //     if (compareByLongestMatch(grammar, rules[i], rules[j]) < 0) {
  //       cout << "! swap\n";
  //       temp = grammar->rules[j];
  //       grammar->rules[j] = grammar->rules[i];
  //       grammar->rules[i] = temp;
  //       continue;
  //     }
  //   }
  // }

}

void debug() {
  cfg.Print();
  sortRulesByMatch(&cfg);
  cfg.Print();

}

string generateNewTokenNotIn(const string& nonterm, const vector<string>& nonterminals) {
  string A_new;
  for (int k = 1; k < 100; k++) {
    A_new  = nonterm  + to_string(k);
    if (!(vecContains(nonterminals, A_new))) {
      break;
    }
  }
  return A_new;
}



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
  vector<string> newNonterminals;
  ContextFreeGrammar cfg_prime;
  cfg.sortNonterminals();

  for(string nonterm : cfg.nonterminals) {
    bool changeMade = false;
    while (true) {
      sortRulesByMatch(&cfg);
      const vector<Rule> rulesToCheck = cfg.getRulesWith(nonterm);
      if (rulesToCheck.size() == 0) {
        break;
      }

      int prefixLen = longest_match(&cfg, rulesToCheck[0]);

      if (prefixLen == 0) { // no common prefix!
        for (Rule r : cfg.popRulesWith(nonterm))
          cfg_prime.rules.push_back(r);
        break;
      }

      vector<string> prefix = rulesToCheck[0].getPrefix(prefixLen);
      string newTok = generateNewTokenNotIn(nonterm, cfg.nonterminals);
      //cout << "NEWTOK:" << newTok << endl;
      cfg.nonterminals.push_back(newTok);

      vector<Rule> commonPrefixRules = cfg.popRulesWithPrefix(nonterm, prefix);
      for (Rule r : commonPrefixRules) {
        r.LHS = newTok;
        cfg_prime.rules.push_back(r.substitutePrefix(prefixLen, {}));
      }
      // cout << "hello?\n";


      // cout << "Nonterm:" << nonterm << endl;
      // for (Rule rule : cfg.getRulesWith(nonterm))
      //   rule.Print();
      
      // Rule (A) -> (prefix) (A_new)
      Rule newRule = Rule(nonterm, prefix) ;
      newRule.RHS.push_back(newTok);
      cfg.rules.push_back(newRule);
    }
  }
  cfg_prime.nonterminals = cfg.nonterminals;
  sortRules2(&cfg_prime.rules);
  cfg_prime.Print();
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
  string A_new = generateNewTokenNotIn(A_i, grammar->nonterminals);
  // for (int k = 1; k < 100; k++) {
  //   A_new  = A_i  + to_string(k);
  //   if (!(vecContains(grammar->nonterminals, A_new))) {
  //     //grammar->nonterminals.push_back(newTok);
  //     break;
  //   }
  // }

  // add null rule
  // A_new -> null
  grammar->rules.push_back(Rule(A_new, vector<string>()));

  // the form A_i -> A_i (alpha)
  // to
  // A_new -> alpha A_new
  for (Rule offender : offendingRules) {
    offender.extractPrefix(1);
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
          
            // debugging
        case 7: debug();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

