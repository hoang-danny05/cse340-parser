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

static const bool DEBUGGING = false;
bool vecContains(std::vector<Token>, Token);
int vecAddTo(std::vector<Token>*, std::vector<Token>*);

class Rule {
public: 
  Rule(Token, std::vector<Token>);
  Rule();
  void Print();
  bool isNull();
  bool hasPrefix(std::vector<Token>);

  Token LHS; 
  std::vector<Token> RHS;

  int compare(Rule *other) {

    if ((other->LHS.lexeme.compare(this->LHS.lexeme) < 0)) {
      return -1;
    }
    else if ((other->LHS.lexeme.compare(this->LHS.lexeme) > 0)) {
      return 1;
    }

    for (int i = 0; i < other->RHS.size() && i < this->RHS.size(); i++) {
      if ((other->RHS.at(i).lexeme.compare(this->RHS.at(i).lexeme) < 0)) {
        return -1;
      }
      else if ((other->RHS.at(i).lexeme.compare(this->RHS.at(i).lexeme) > 0)) {
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
    std::vector<Token> newRHS;
    for (int i = cutLength; i < RHS.size(); i++) {
      newRHS.push_back(RHS[i]);
    }
    RHS = newRHS;
  }

private:
  Token tmp;
};

// will have its own lexical analyzer
class ContextFreeGrammar {
public:
  ContextFreeGrammar();
  void readGrammar();
  void init();

  void Print();
  void PrintTokens();
  void PrintNullable();
  void PrintFirst();
  void PrintFollow();

  std::vector<Rule> rules;
  std::vector<Token> nonterminals;
  std::vector<Token> terminals;

  std::vector<Token> nullable;

  std::map<std::string, std::vector<Token>> first;
  std::map<std::string, std::vector<Token>> follow;

  // helpers 

  std::vector<Rule> getRulesWith(Token);
  std::vector<Rule> popRulesWith(Token);
  std::vector<Rule> popRulesWithPrefix(Token, std::vector<Token>);

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
    Token LHS = lexer.GetToken();
    std::vector<Token> RHS = std::vector<Token>();
    
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
        RHS = std::vector<Token>();
        //std::cout << "NEW RHS size:"<< RHS.size()<< std::endl;
      }
      else {
        RHS.push_back(tmp);
        // std::cout << "RHS size:"<< RHS.size()<< std::endl;
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
bool vecContains(std::vector<Token> vec, Token item) {
  for(int i = 0; i < vec.size(); i++) {
    if (vec.at(i).lexeme == item.lexeme)
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

  std::vector<Token> nonterm_set;

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

void ContextFreeGrammar::PrintTokens() {
  for (int i = 0; i < terminals.size(); i++) {
    std::cout << terminals.at(i).lexeme << " ";
  }
  for (int i = 0; i < nonterminals.size(); i++) {
    std::cout << nonterminals.at(i).lexeme << " ";
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
    //   std::cout<< rule.LHS.lexeme << "is NOT null\n";
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
      for (Token t : rule.RHS) {
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
  for (Token order : nonterminals) {
    for (Token t : nullable ) {
      if (t.lexeme == order.lexeme) {
        std::cout << t.lexeme;
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
int vecAddTo(std::vector<Token>* from, std::vector<Token>* to) {
  int changesMade = 0;

  // for term in termsToAdd
  for (Token term : *from) {
    if (!vecContains(*to, term)) 
    {
      to->push_back(term);
      //first.at(rule.LHS.lexeme).push_back(term);
      changesMade++;
    }
  }

  return changesMade;
}

void ContextFreeGrammar::initFirst() {
  if (DEBUGGING)
  std::cout << "Initialize map\n";
  for (Token nonterm : nonterminals) {
    first.insert({nonterm.lexeme, std::vector<Token>()});
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
      !vecContains(first.at(rule.LHS.lexeme), rule.RHS.at(0))
    )
    {
      first.at(rule.LHS.lexeme).push_back(rule.RHS.at(0));
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

      for (Token r : rule.RHS) {
        // if r is term AND 
        // if r is not in first(LHS)

        if(
          vecContains(terminals, r)
        )
        {
          if (DEBUGGING)
            std::cout << "Terminal found: " << r.lexeme << "\n";

          if (!vecContains(first.at(rule.LHS.lexeme), r)) {
            first.at(rule.LHS.lexeme).push_back(r);
            changesMade++;
          }
          break;
        }
        else if (vecContains(nullable, r))
        {
          //NOTE: adds first[r.lexeme] to first[rule.LHS.lexeme]
          if (DEBUGGING)
            std::cout << "Nullable nonterm: " << r.lexeme << "\n";
          std::vector<Token>* termsToAdd = &first.at(r.lexeme);
          std::vector<Token>* termsInSet = &first.at(rule.LHS.lexeme);
          changesMade += vecAddTo(termsToAdd, termsInSet);

          continue;
        }
        else {
          if (DEBUGGING)
            std::cout << "Non-Nullable nonterm: " << r.lexeme << "\n";
          // for (Token t: terminals) {
          //   std::cout << t.lexeme << ", ";
          // }
          // std::cout << std::endl;
          //NOTE: adds first[r.lexeme] to first[rule.LHS.lexeme]
          std::vector<Token>* termsToAdd = &first.at(r.lexeme);
          std::vector<Token>* termsInSet = &first.at(rule.LHS.lexeme);
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
  for (Token tok : nonterminals){
    std::string lhs = tok.lexeme;
    std::vector<Token> terms = first.at(lhs);
    std::cout << "FIRST(" << lhs << ") = { ";

    int termCount = 0;
    // in order of terminals
    for(Token order : terminals) {
      for(Token t : terms) {
        if (t.lexeme == order.lexeme) {
          std::cout << t.lexeme;
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
  for (Token nonterm : nonterminals) {
    follow.insert({nonterm.lexeme, std::vector<Token>()});
  }

  Token eof;
  eof.lexeme = "$";

  //TODO: add eof to first nonterm
  Token starter = nonterminals.at(0);
  follow.at(starter.lexeme).push_back(eof);


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
      Token base = rule.RHS.at(i);
      // don't need follow set of terminals 
      if (vecContains(terminals, base)) {
        continue;
      }

      // lives as long it is nullable
      for(int j = i+1; j < rule.RHS.size(); j++) {
        Token follower = rule.RHS.at(j);
        if (vecContains(terminals, follower)) {
          std::vector<Token> term = {follower};
          std::vector<Token> *followTarget = &follow.at(base.lexeme);
          vecAddTo(&term, followTarget);
          break;
        }
        else if (vecContains(nullable, follower)) {
          std::vector<Token> *terms = &first.at(follower.lexeme);
          std::vector<Token> *followTarget = &follow.at(base.lexeme);
          if (j == rule.RHS.size()) { // if it is the end
            terms->push_back(eof);
          } 
          vecAddTo(terms, followTarget);
          continue;
        }
        else { // NonNull Nonterm
          std::vector<Token> *terms = &first.at(follower.lexeme);
          std::vector<Token> *followTarget = &follow.at(base.lexeme);
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

      std::vector<Token> *inherit = &follow.at(rule.LHS.lexeme);
      std::vector<Token> termsToAdd;
      // add follow(LHS)
      vecAddTo(inherit, &termsToAdd);

      for (int k = rule.RHS.size()-1; k >= 0; k--) {
        Token current = rule.RHS.at(k);

        if(vecContains(terminals, current)) { // ignore terminals
          termsToAdd.clear();
          termsToAdd.push_back(current);
          continue;
        }
        else if (vecContains(nullable, current)) {
          // add to follow(current)
          std::vector<Token> *target = &follow.at(current.lexeme);
          changesMade += vecAddTo(&termsToAdd, target);
          
          // add first(me) to collection
          std::vector<Token> *newTarget = &first.at(current.lexeme);
          vecAddTo(newTarget, &termsToAdd);

          continue;
        }
        else { // nonterm nullable
          // add to follow(current)
          std::vector<Token> *target = &follow.at(current.lexeme);
          changesMade += vecAddTo(&termsToAdd, target);

          // because not nullable
          termsToAdd.clear();

          // add first(me) to collection
          std::vector<Token> *newTarget = &first.at(current.lexeme);
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
  Token eof;
  eof.lexeme = "$";
  std::vector<Token> termOrder = { eof };
  for (Token tok : terminals) {
    termOrder.push_back(tok);
  }

  // for (lhs, terms) in first
  for (Token tok : nonterminals){
    std::string lhs = tok.lexeme;
    std::vector<Token> terms = follow.at(lhs);
    std::cout << "FOLLOW(" << lhs << ") = { ";

    int termCount = 0;
    // in order of terminals
    for(Token order : termOrder) {
      for(Token t : terms) {
        if (t.lexeme == order.lexeme) {
          std::cout << t.lexeme;
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
std::vector<Rule> ContextFreeGrammar::getRulesWith(Token lhs) {
  std::vector<Rule> ret;

  for (Rule rule : rules) {
    if (rule.LHS == lhs) {
      ret.push_back(rule);
    }
  }

  return ret;
}

// returns all rules with a certain LHS
std::vector<Rule> ContextFreeGrammar::popRulesWith(Token lhs) {
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

std::vector<Rule> ContextFreeGrammar::popRulesWithPrefix(Token lhs, std::vector<Token> prefix) {
  std::vector<Rule> ret;
  std::vector<Rule> newRules;

  for (Rule rule : rules) {
    if ((rule.LHS.lexeme == lhs.lexeme) && (rule.hasPrefix(prefix))) {
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


std::vector<Token> longestCommonPrefix(const std::vector<Rule>& vecs) 
{
  std::vector<Token> prefix;
  
  for (int i = 0; i < vecs.size(); i++) {
    for (int j = i+1; j < vecs.size(); j++) {
      std::vector<Token> candidate;
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
// rule stuff
/////////////////////////////////////////////////////////////////////////


Rule::Rule(Token lhs, std::vector<Token> rhs) {
  LHS = lhs;
  RHS = rhs;
}

Rule::Rule() {
}


// void Rule::Print() {
//   std::cout << "\tRULE: " << LHS.lexeme << " --> ";
//   for(int i = 0; i < RHS.size(); i++) {
//     std::cout << RHS.at(i).lexeme << " ";
//   }
//   std::cout << "\n\n";
// }

void Rule::Print() {
  std::cout << LHS.lexeme << " -> ";
  for(int i = 0; i < RHS.size(); i++) {
    std::cout << RHS.at(i).lexeme << " ";
  }
  std::cout << "#\n";
}

bool Rule::isNull() {
  return RHS.empty();
}

bool Rule::hasPrefix(std::vector<Token> prefix){
  if(RHS.size() < prefix.size()) {
    return false;
  }
  for (int i = 0; i < prefix.size(); i++) {
    if (!(prefix.at(i).lexeme == RHS.at(i).lexeme))
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

  std::vector<Token> operatingNonterminals = cfg.nonterminals;

  while (!cfg.nonterminals.empty()) {
    for (Token nonterm : cfg.nonterminals) {
      if (DEBUGGING)
        std::cout << "starting nonterm: " << nonterm.lexeme << std::endl;
      std::vector<Token> prefix = longestCommonPrefix(cfg.getRulesWith(nonterm));

      if (prefix.size() > 0) {
        if(DEBUGGING) {
          std::cout << "prefix exist! \nprf:" << prefix.size() << "\n";
          for (Token t : prefix) {
            cout << t.lexeme << ", ";
          }
          cout<< "worked\n";
        }

        // remove rules with prefix
        std::vector<Rule> withPrefix = cfg.popRulesWithPrefix(nonterm, prefix);

        // add rule (prefix)(newTok)
        Token newTok;
        newTok.lexeme = nonterm.lexeme;
        newTok.lexeme.append("1");
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
          for (Token tok : operatingNonterminals) {
            cout << "> nt: " << tok.lexeme << "\n";
          }
        }
        cfg_prime.nonterminals.push_back(nonterm);
        vector<Token> newNonterm;
        for (Token nt : operatingNonterminals) {
          if (nt.lexeme != nonterm.lexeme)
            newNonterm.push_back(nt);
        }
        operatingNonterminals = newNonterm;
        if (DEBUGGING)
          cout << "Removed nonterm " << nonterm.lexeme << "\n";
      } // end else
    } // for(nonterm in nonterminal)
    cfg.nonterminals = operatingNonterminals;

    //exit(1);
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

