#include <cctype>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include "lexer.h"
#include "CFG.h"


ContextFreeGrammar::ContextFreeGrammar() {};

// read grammar


void ContextFreeGrammar::readGrammar() {
  LexicalAnalyzer lexer = LexicalAnalyzer();

  int count = 0;
  const int limit = 1000;

  for (Token peek = lexer.peek(1);
    (peek.token_type != END_OF_FILE) && (peek.token_type != HASH) && (count <= 50);)
  {
    if (peek.token_type == ERROR) {
      std::cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";
      std::exit(-1);
    }

    string LHS = lexer.GetToken().lexeme;
    std::vector<string> RHS = std::vector<string>();
    
    if (lexer.GetToken().token_type != ARROW) {
      std::cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";
      std::exit(-1);
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
  //  std::cout << "WARNING: CFG rule limit exceeded!\n";
    std::cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";
    std::exit(-1);
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
// bool vecContains(std::vector<string> vec, string item) {
//   for(int i = 0; i < vec.size(); i++) {
//     if (vec.at(i)  == item )
//       return true;
//   }
//   return false;
// }

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
