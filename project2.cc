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
#include "lexer.h"

////////////////////////////////////////////////////
/// START cfg definition
////////////////////////////////////////////////////
using namespace std;

static const bool DEBUGGING = false;
bool vecContains(vector<string>, string);
int vecAddTo(vector<string>*, vector<string>*);

class Rule {
public: 
  Rule(string, vector<string>);
  Rule();
  void Print();
  bool isNull();
  bool hasPrefix(vector<string>);

  string LHS; 
  vector<string> RHS;

  int compare(const Rule *other) {

    if ((other->LHS .compare(this->LHS ) < 0)) {
      return -1;
    }
    else if ((other->LHS .compare(this->LHS ) > 0)) {
      return 1;
    }

    for (int i = 0; i < other->RHS.size() && i < this->RHS.size(); i++) {
      if ((other->RHS.at(i) .compare(this->RHS.at(i) ) < 0)) {
        return -1;
      }
      else if ((other->RHS.at(i) .compare(this->RHS.at(i) ) > 0)) {
        return 1;
      }
    }

    if (this->RHS.size() == other->RHS.size()) {
      return 0;
    }
    return (other->RHS.size() > this->RHS.size());
  }

  void cutBeginning(int cutLength) {
    //std::cout << "len: " << cutLength << std::endl;
    std::vector<string> newRHS;
    for (int i = cutLength; i < RHS.size(); i++) {
      newRHS.push_back(RHS[i]);
    }
    RHS = newRHS;
  }

  bool operator ==(const Rule& other) {
    return (this->compare(&other)) == 0;
  }

  std::vector<string> afterPrefix(int prefixLen) {
    std::vector<string> ret;
    for (int i = prefixLen; i < RHS.size(); i++) {
      ret.push_back(RHS[i]);
    }
    return ret;
  }

private:
  string tmp;
};

// will have its own lexical analyzer
class ContextFreeGrammar {
public:
  ContextFreeGrammar();
  void readGrammar();
  void init();

  void Print();
  void Printstrings();
  void PrintNullable();
  void PrintFirst();
  void PrintFollow();

  std::vector<Rule> rules;
  std::vector<string> nonterminals;
  std::vector<string> terminals;

  std::vector<string> nullable;

  std::map<std::string, std::vector<string>> first;
  std::map<std::string, std::vector<string>> follow;

  // helpers 

  std::vector<Rule> getRulesWith(string);
  std::vector<Rule> popRulesWith(string);
  std::vector<Rule> popRulesWithPrefix(string, std::vector<string>);

private:
  void initTokens();
  void initNullable();
  void initFirst();
  void initFollow();
};
////////////////////////////////////////////////////
/// END cfg header
////////////////////////////////////////////////////


ContextFreeGrammar::ContextFreeGrammar() {};

// read grammar

void ContextFreeGrammar::readGrammar() {
  LexicalAnalyzer lexer = LexicalAnalyzer();

  int count = 0;
  const int limit = 300;

  for (Token peek = lexer.peek(1);
    (peek.token_type != END_OF_FILE) && (peek.token_type != HASH) && (count <= 50);)
  {
    string LHS = lexer.GetToken().lexeme;
    std::vector<string> RHS = std::vector<string>();
    
    if (lexer.GetToken().token_type != ARROW) {
      std::cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";
    }

    for (int i = 0; i < limit; i++) {
      if (i == limit)
        std::cout << "[CFG] Exceeded rule length of tokens!!!\n";
      
      Token tmp = lexer.GetToken();

      if (tmp.token_type == STAR) {
        Rule rule = Rule(LHS, RHS);
        if (DEBUGGING) {
          std::cout << "\nAdding terminated rule\n";
          rule.Print();
        }
        this->rules.push_back(rule);
        // std::cout << "Size:"<< rules.size()<< std::endl;
        break;
      }
      else if (tmp.token_type == OR) {
        Rule rule = Rule(LHS, RHS);
        if (DEBUGGING) {
          std::cout << "\nAdding OR rule\n";
          rule.Print();
        }
        this->rules.push_back(rule);
        RHS = std::vector<string>();
        //std::cout << "NEW RHS size:"<< RHS.size()<< std::endl;
      }
      else {
        RHS.push_back(tmp.lexeme);
      }
    } //end for each OR rule
    count++;

   peek = lexer.peek(1);
  } // end for each line

  if (DEBUGGING)
    std::cout << "Final Size:"<< this->rules.size()<< std::endl;
  if (count == limit) {
   std::cout << "WARNING: CFG rule limit exceeded!\n";
  }
}

void ContextFreeGrammar::Print() {
  std::cout << "total rules: "<< rules.size() << "  \n";
  std::cout << "###############################################\n";
  std::cout << "DISPLAYING CFG: \n";
  std::cout << "###############################################\n";
  for (int i = 0; i < rules.size(); i++) {
    rules.at(i).Print();
  }
}

// initialize

void ContextFreeGrammar::init() {
  initTokens();
  initNullable();
  initFirst();
  initFollow();
}

/////////////////////////////////////////////////////////////////////////
// PART 1: nonterminals and terminals
/////////////////////////////////////////////////////////////////////////

// checks lexeme, nothing else!
bool vecContains(std::vector<string> vec, string item) {
  for(int i = 0; i < vec.size(); i++) {
    if (vec.at(i)  == item )
      return true;
  }
  return false;
}

void ContextFreeGrammar::initTokens() {
  // must be called AFTER constructor
  
  if ((!terminals.empty()) || (!nonterminals.empty())) {
    std::cout << "WARNING: initTokens called twice. returning\n";
    return;
  }

  std::vector<string> nonterm_set;

  //nonterminals
  for (int i = 0; i < rules.size(); i++) {
    Rule rule = rules.at(i);
    if (!vecContains(nonterm_set, rule.LHS)) {
      nonterm_set.push_back(rule.LHS);
    }
  }

  //nonterminals
  for (int i = 0; i < rules.size(); i++) {
    Rule rule = rules.at(i);

    // if it is a nonterm, but not already in the order

    if (
      vecContains(nonterm_set, rule.LHS) && 
      (!vecContains(nonterminals, rule.LHS))
    ) {
      nonterminals.push_back(rule.LHS);
    }

    for (int j = 0; j < rule.RHS.size(); j++) {
      if (
        vecContains(nonterm_set, rule.RHS.at(j)) && 
        (!vecContains(nonterminals, rule.RHS.at(j)))
      ) {
        nonterminals.push_back(rule.RHS.at(j));
      }
    }
  }

  //terminals
  for (int i = 0; i < rules.size(); i++) {
    Rule rule = rules.at(i);


    for (int j = 0; j < rule.RHS.size(); j++) {
      if (
        (!vecContains(nonterminals, rule.RHS.at(j))) && 
        (!vecContains(terminals, rule.RHS.at(j)))
      ) // if it is not a nonterm + not already in the list 
      {
        terminals.push_back(rule.RHS.at(j));
      }
    }
  }
}

void ContextFreeGrammar::Printstrings() {
  for (int i = 0; i < terminals.size(); i++) {
    std::cout << terminals.at(i)  << " ";
  }
  for (int i = 0; i < nonterminals.size(); i++) {
    std::cout << nonterminals.at(i)  << " ";
  }
  std::cout << std::endl;
}

/////////////////////////////////////////////////////////////////////////
// PART 2: NULLLABLE
/////////////////////////////////////////////////////////////////////////

void ContextFreeGrammar::initNullable() {
  for (Rule rule : rules) { // i forgot foreach existed
    if (rule.isNull() && (!vecContains(nullable, rule.LHS))) {
      nullable.push_back(rule.LHS);
    }
    // else {
    //   std::cout<< rule.LHS  << "is NOT null\n";
    // }
  }

  int last_len = 0;
  while (true) {
    if (last_len == nullable.size()) {
      break;
    }
    else {
      last_len = nullable.size();
    }

    for (Rule rule : rules) {
      // no duplicates
      if(vecContains(nullable, rule.LHS)) {
        continue;
      }

      bool isNull = true;
      for (string t : rule.RHS) {
        if (vecContains(terminals, t)) { //actually not necessary but you never know
          isNull = false;
          break;
        }
        if(!vecContains(nullable, t)) {
          isNull = false;
          break;
        }
      }
      if (isNull) {
        nullable.push_back(rule.LHS);
      }
    }
  }
}

void ContextFreeGrammar::PrintNullable() {
  std::cout << "Nullable = { ";
  
  int nullCount = 0;
  for (string order : nonterminals) {
    for (string t : nullable ) {
      if (t  == order ) {
        std::cout << t ;
        //std::cout << "comp: " << nullCount << " == " << nullable.size() << "\n";
        if (nullCount != nullable.size() -1)
          std::cout << " , ";
        nullCount +=1;
        break;
      }
    }
  }

  std::cout << " }\n";
}

/////////////////////////////////////////////////////////////////////////
// PART 3: FIRST SETS
/////////////////////////////////////////////////////////////////////////

//NOTE: USE THIS AS SET ADDITION, NOTHING ELSE
// returns changesMade
int vecAddTo(std::vector<string>* from, std::vector<string>* to) {
  int changesMade = 0;

  // for term in termsToAdd
  for (string term : *from) {
    if (!vecContains(*to, term)) 
    {
      to->push_back(term);
      //first.at(rule.LHS ).push_back(term);
      changesMade++;
    }
  }

  return changesMade;
}

void ContextFreeGrammar::initFirst() {
  if (DEBUGGING)
  std::cout << "Initialize map\n";
  for (string nonterm : nonterminals) {
    first.insert({nonterm , std::vector<string>()});
  }

  if (DEBUGGING)
  std::cout << "Initial map terminals\n";
  for (Rule rule : rules) {
    // if RHS is not empty AND 
    // if the first character of the RHS terminal AND
    // if the terminal is not already in the first set
    if(
      ((rule.RHS.size() > 0) && 
      vecContains(terminals, rule.RHS.at(0))) &&
      !vecContains(first.at(rule.LHS ), rule.RHS.at(0))
    )
    {
      first.at(rule.LHS ).push_back(rule.RHS.at(0));
    }
  }

  if (DEBUGGING)
  std::cout << "Starting complex loop \n";
  while (true) {
    int changesMade = 0;
    for (Rule rule : rules) {

      //for each rule [rule]
      //for each RHS char [r]
      //if [r] is terminal 
      //  add r to FIRST(rule.LHS)
      //  break 
      //if [rule] is nullable 
      //  continue
      //else 
      //  nonTermNotNull += 1;

      for (string r : rule.RHS) {
        // if r is term AND 
        // if r is not in first(LHS)

        if(
          vecContains(terminals, r)
        )
        {
          if (DEBUGGING)
            std::cout << "Terminal found: " << r  << "\n";

          if (!vecContains(first.at(rule.LHS ), r)) {
            first.at(rule.LHS ).push_back(r);
            changesMade++;
          }
          break;
        }
        else if (vecContains(nullable, r))
        {
          //NOTE: adds first[r ] to first[rule.LHS ]
          if (DEBUGGING)
            std::cout << "Nullable nonterm: " << r  << "\n";
          std::vector<string>* termsToAdd = &first.at(r );
          std::vector<string>* termsInSet = &first.at(rule.LHS );
          changesMade += vecAddTo(termsToAdd, termsInSet);

          continue;
        }
        else {
          if (DEBUGGING)
            std::cout << "Non-Nullable nonterm: " << r  << "\n";
          // for (string t: terminals) {
          //   std::cout << t  << ", ";
          // }
          // std::cout << std::endl;
          //NOTE: adds first[r ] to first[rule.LHS ]
          std::vector<string>* termsToAdd = &first.at(r );
          std::vector<string>* termsInSet = &first.at(rule.LHS );
          changesMade += vecAddTo(termsToAdd, termsInSet);
          break;
        }

      }


    } // end for(rules)

    if (changesMade == 0) {
      if (DEBUGGING)
        std::cout<<"Ending FIRST Loops\n";
      break;
    }
    else {
      if (DEBUGGING)
        std::cout<<"Next FIRST Loop\n";
    }
  } // end while
}

void ContextFreeGrammar::PrintFirst() {

  // for (lhs, terms) in first
  for (string tok : nonterminals){
    std::string lhs = tok ;
    std::vector<string> terms = first.at(lhs);
    std::cout << "FIRST(" << lhs << ") = { ";

    int termCount = 0;
    // in order of terminals
    for(string order : terminals) {
      for(string t : terms) {
        if (t  == order ) {
          std::cout << t ;
          if (termCount != terms.size() - 1) {
            std::cout << ", ";
          }
          termCount++;
          break;
        }
      }
    }
    std::cout << " }\n";
  }
}

/////////////////////////////////////////////////////////////////////////
// PART 4: FOLLOW SETS
/////////////////////////////////////////////////////////////////////////

void ContextFreeGrammar::initFollow() {
  if (DEBUGGING)
  std::cout << "follow: Initialize map\n";
  for (string nonterm : nonterminals) {
    follow.insert({nonterm , std::vector<string>()});
  }

  string eof;
  eof  = "$";

  //TODO: add eof to first nonterm
  string starter = nonterminals.at(0);
  follow.at(starter ).push_back(eof);


  //    forwards
  //    j = 1; i+j < size; j++
  //    if (RHS[i+j] is terminal)
  //      add RHS[i+j] to follow(r)
  //      break
  //    elif (RHS[i+j] is nullable)
  //      add first(RHS[i+j]) to follow(r)
  //      continue 
  //    else (RHS[i+j] is non-null)
  //      add first(RHS[i+j]) to follow(r)
  //      break 
  for (Rule rule : rules) {
    for(int i = 0; i < rule.RHS.size(); i++) {
      string base = rule.RHS.at(i);
      // don't need follow set of terminals 
      if (vecContains(terminals, base)) {
        continue;
      }

      // lives as long it is nullable
      for(int j = i+1; j < rule.RHS.size(); j++) {
        string follower = rule.RHS.at(j);
        if (vecContains(terminals, follower)) {
          std::vector<string> term = {follower};
          std::vector<string> *followTarget = &follow.at(base );
          vecAddTo(&term, followTarget);
          break;
        }
        else if (vecContains(nullable, follower)) {
          std::vector<string> *terms = &first.at(follower );
          std::vector<string> *followTarget = &follow.at(base );
          if (j == rule.RHS.size()) { // if it is the end
            terms->push_back(eof);
          } 
          vecAddTo(terms, followTarget);
          continue;
        }
        else { // NonNull Nonterm
          std::vector<string> *terms = &first.at(follower );
          std::vector<string> *followTarget = &follow.at(base );
          vecAddTo(terms, followTarget);
          break;
        }
      }
    }
  }

  if (DEBUGGING)
  std::cout << "Starting complex loop \n";
  while (true) {
    int changesMade = 0;
    for (Rule rule : rules) {
      /*
       * FOLLOW SET RULES 
       *
       * follow(s) has eof 
       * A -> aBC 
       *  - FIRST(C) in FOLLOW(B)
       * A -> aB
       *  - FOLLOW(A) in FOLLOW(B)
       * A -> aBC 
       *  - C in nullable 
       *  - FOLLOW(A) in FOLLOW(B)
      */

      //for each rule [rule]
      //for each RHS char r = RHS[i]
      //if [r] is nonterminal 
      //  null = true
      //  while (not end of rule):
      //
      //    forwards
      //    j = 1; i+j < size; j++
      //    if (RHS[i+j] is terminal)
      //      add RHS[i+j] to follow(r)
      //      break
      //    elif (RHS[i+j] is nullable)
      //      add first(RHS[i+j]) to follow(r)
      //      continue 
      //    else (RHS[i+j] is non-null)
      //      add first(RHS[i+j]) to follow(r)
      //      break 
      //
      //  backwards, once per rule
      //  k = size-1, k>0, k--; offset from r 
      //    if (RHS[k] is nonterm, nullable):
      //      add follow(LHS) to follow(RHS[k])
      //      continue 
      //    if (RHS[k] is nonterm, nonnull)
      //      add follow(LHS) to follow(RHS[k])
      //      break 
      //    else terminal 
      //      break

      //TODO: add backwards traverse
      //  backwards, once per rule
      //  k = size-1, k>0, k--; offset from r 
      //    if (RHS[k] is nonterm, nullable):
      //      add follow(LHS) to follow(RHS[k])
      //      continue 
      //    if (RHS[k] is nonterm, nonnull)
      //      add follow(LHS) to follow(RHS[k])
      //      remove termsToAdd
      //      continue
      //    else terminal 
      //      remove termsToAdd
      //      continue

      std::vector<string> *inherit = &follow.at(rule.LHS );
      std::vector<string> termsToAdd;
      // add follow(LHS)
      vecAddTo(inherit, &termsToAdd);

      for (int k = rule.RHS.size()-1; k >= 0; k--) {
        string current = rule.RHS.at(k);

        if(vecContains(terminals, current)) { // ignore terminals
          termsToAdd.clear();
          termsToAdd.push_back(current);
          continue;
        }
        else if (vecContains(nullable, current)) {
          // add to follow(current)
          std::vector<string> *target = &follow.at(current );
          changesMade += vecAddTo(&termsToAdd, target);
          
          // add first(me) to collection
          std::vector<string> *newTarget = &first.at(current );
          vecAddTo(newTarget, &termsToAdd);

          continue;
        }
        else { // nonterm nullable
          // add to follow(current)
          std::vector<string> *target = &follow.at(current );
          changesMade += vecAddTo(&termsToAdd, target);

          // because not nullable
          termsToAdd.clear();

          // add first(me) to collection
          std::vector<string> *newTarget = &first.at(current );
          vecAddTo(newTarget, &termsToAdd);

          continue;
        }
      }

    } // end for(rules)

    if (changesMade == 0) {
      if (DEBUGGING)
        std::cout<<"Ending FOLLOW Loops\n";
      break;
    }
    else {
      if (DEBUGGING)
        std::cout<<"Next FOLLOW Loop\n";
    }
  } // end while

}

void ContextFreeGrammar::PrintFollow() {
  string eof;
  eof  = "$";
  std::vector<string> termOrder = { eof };
  for (string tok : terminals) {
    termOrder.push_back(tok);
  }

  // for (lhs, terms) in first
  for (string tok : nonterminals){
    std::string lhs = tok ;
    std::vector<string> terms = follow.at(lhs);
    std::cout << "FOLLOW(" << lhs << ") = { ";

    int termCount = 0;
    // in order of terminals
    for(string order : termOrder) {
      for(string t : terms) {
        if (t  == order ) {
          std::cout << t ;
          if (termCount != terms.size() - 1) {
            std::cout << ", ";
          }
          termCount++;
          break;
        }
      }
    }
    std::cout << " }\n";
  }
}

/////////////////////////////////////////////////////////////////////////
// Task 5 helpers
/////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////
// rule stuff
/////////////////////////////////////////////////////////////////////////


Rule::Rule(string lhs, std::vector<string> rhs) {
  LHS = lhs;
  RHS = rhs;
}

Rule::Rule() {
}


// void Rule::Print() {
//   std::cout << "\tRULE: " << LHS  << " --> ";
//   for(int i = 0; i < RHS.size(); i++) {
//     std::cout << RHS.at(i)  << " ";
//   }
//   std::cout << "\n\n";
// }

void Rule::Print() {
  std::cout << LHS  << " -> ";
  for(int i = 0; i < RHS.size(); i++) {
    std::cout << RHS.at(i)  << " ";
  }
  std::cout << "#\n";
}

bool Rule::isNull() {
  return RHS.empty();
}

bool Rule::hasPrefix(std::vector<string> prefix){
  if(RHS.size() < prefix.size()) {
    return false;
  }
  for (int i = 0; i < prefix.size(); i++) {
    if (!(prefix.at(i)  == RHS.at(i) ))
      return false;
  }
  return true;
}


////////////////////////////////////////////////////
/// END cfg definition
////////////////////////////////////////////////////


using namespace std;

ContextFreeGrammar cfg;

// read grammar
void ReadGrammar() {
  cfg.readGrammar();
  if (DEBUGGING)
    cfg.Print();
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
  ContextFreeGrammar cfg_prime;
  // sort nonterminals 
  for (int i = 0; i < cfg.nonterminals.size(); i++) {
    for (int j = i+1; j < cfg.nonterminals.size(); j++) {
      if (cfg.nonterminals[i].compare(cfg.nonterminals[j]) > 0) {
        string temp = cfg.nonterminals[i];
        cfg.nonterminals[i] = cfg.nonterminals[j];
        cfg.nonterminals[j] = temp;
      }
    }
  }

  // for (string t : cfg.nonterminals) {
  //   cout << t  << ", ";
  // }

  vector<string> nonterminals;



  // For nonterm
  // there are two types of rules:

  // forall alpha, beta
  //
  // nonterm -> (nonterm) (alpha)
  // nonterm -> (beta)
  //    beta does not have nonterm
  //
  // given newNT symbol (aka nonterm1)
  // equal to:
  // 
  // nonterm -> (beta) (newNT)
  // newNT -> (alpha) (newNT)
  // newNT -> epsilon

  // group similar nonterminals
  map<string, vector<Rule>> Rules;
  for (string nt : cfg.nonterminals) {
    Rules.insert({nt , cfg.getRulesWith(nt)});
  }


  /* 
  for int i..n
    for int j..i
      for rule [r] in Rules[nonterm[i]]
        if r.has!Prefix(nonterm[j]):                                 
          let (after_prefix) be r.RHS[j:]                           TODO: Rule::afterPrefix() -> rule
          remove r from Rules[nonterm[i]]                           TODO: vecRemoveItem(vec, item)
          forall rule [sub_r] in Rules[nonterm[j]]:
            if sub_r has form {nonterm[j] -> (delta)}
              let (delta) be sub_r.RHS
              add rule {nonterm[j] -> (delta)()} to Rules[nonterm[i]]
]         
  */      
  // bool changed = true;
  // while(changed) {
  //
  // }

  for(int i = 0; i < cfg.nonterminals.size(); i++) {
    const string nonterm_i = cfg.nonterminals[i] ;
    for(int j = 0; j < i; j++) {
      const string nonterm_j = cfg.nonterminals[j] ;
      vector<Rule> operatingRules = Rules[nonterm_i];
      for (Rule r: Rules[nonterm_i]) {
        operatingRules = Rules[cfg.nonterminals[i] ];
        if(r.hasPrefix({cfg.nonterminals[j]})) {
          vector<string> after_prefix = r.afterPrefix(1);
          // going to remove offending rule
          vecRemoveItem(&operatingRules, r);

          // substitute offending rules
          for (Rule sub_r : Rules[cfg.nonterminals[j] ]) {
            // if sub_r has form.. but i don't understand delta
            if (true) {
              vector<string> delta = sub_r.RHS;
              Rule newRule;
              newRule.LHS = cfg.nonterminals[i];
              newRule.RHS = delta;
              for (string t : after_prefix) {
                newRule.RHS.push_back(t);
              }
              operatingRules.push_back(newRule);
            }
          }
        } // if has prefix
      }// for r : rule starting with nonterminals[i]
      Rules[nonterm_i] = operatingRules;
    }// for j

    //TODO: remove immediate left recursion

    // vector<Rule> hasRecursion = cfg.popRulesWithPrefix(cfg.nonterminals[i], {cfg.nonterminals[i]});
    // vector<Rule> noRecursion = cfg.popRulesWith(cfg.nonterminals[i]);
    vector<Rule> hasRecursion;
    vector<Rule> noRecursion;
    vector<Rule> splitMe = Rules[nonterm_i];
    for (Rule r: splitMe) {
      if (r.RHS.size() > 0 && r.hasPrefix({cfg.nonterminals[i]})) {
        hasRecursion.push_back(r);
      } else {
        noRecursion.push_back(r);
      }
    }


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
    
    string newTok;
    for (int k = 1; k < 10; k++) {
      newTok  = cfg.nonterminals[i]  + to_string(k);
      break;
      // if (!(vecContains(cfg_prime.nonterminals, newTok))) {
      //   break;
      // }
    }
    vector<Rule> newRules;

    for (int i = 0; i < hasRecursion.size(); i++) {
      Rule newRule;
      vector<string> alpha = hasRecursion[i].afterPrefix(1);
      newRule.LHS = newTok;
      newRule.RHS = alpha;
      newRule.RHS.push_back(newTok);
      newRules.push_back(newRule);
    }
    // epsilon
    Rule newRule;
    newRule.LHS = newTok;
    newRules.push_back(newRule);

    for (Rule betaRule : noRecursion) {
      betaRule.RHS.push_back(newTok);
      newRules.push_back(betaRule);
    }

  } // for i


  // for (string nonterm: cfg.nonterminals) {
  //   vector<Rule> hasRecursion = cfg.popRulesWithPrefix(nonterm, {nonterm});
  //   vector
  // }

  
  cout << "DONE\n";
  for (string nonterm : cfg.nonterminals) {
    sortRules(&Rules[nonterm ]);
    for (Rule r : Rules[nonterm ]) {
      r.Print();
    }
  }
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

