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


void throwException() {
  cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";
  exit(-1);
}

void ContextFreeGrammar::readGrammar() {
  LexicalAnalyzer lexer = LexicalAnalyzer();

  // for each start of line 
  for (Token peek = lexer.peek(1);(peek.token_type == ID);peek = lexer.peek(1))
  {
    if (peek.token_type == ERROR) {
      throwException();
    }

    string LHS = lexer.GetToken().lexeme;
    vector<string> RHS = vector<string>();
    
    if (lexer.GetToken().token_type != ARROW) {
      throwException();
    }

    while (true) {
      
      Token tmp = lexer.GetToken();

      if (tmp.token_type == STAR) {
        Rule rule = Rule(LHS, RHS);
        if (DEBUGGING) {
          cout << "\nAdding terminated rule\n";
          rule.Print();
        }
        this->rules.push_back(rule);
        // cout << "Size:"<< rules.size()<< endl;
        break;
      }
      else if (tmp.token_type == OR) {
        Rule rule = Rule(LHS, RHS);
        if (DEBUGGING) {
          cout << "\nAdding OR rule\n";
          rule.Print();
        }
        this->rules.push_back(rule);
        RHS = vector<string>();
        //cout << "NEW RHS size:"<< RHS.size()<< endl;
      }
      else {
        RHS.push_back(tmp.lexeme);
      }
    } //end for each OR rule
  } // end for each line

  if (lexer.GetToken().token_type != HASH) {
    throwException();
  }

  if (DEBUGGING)
    cout << "Final Size:"<< this->rules.size()<< endl;
}

void ContextFreeGrammar::Print() {
  cout << "total rules: "<< rules.size() << "  \n";
  cout << "###############################################\n";
  cout << "DISPLAYING CFG: \n";
  cout << "###############################################\n";
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
// bool vecContains(vector<string> vec, string item) {
//   for(int i = 0; i < vec.size(); i++) {
//     if (vec.at(i)  == item )
//       return true;
//   }
//   return false;
// }

void ContextFreeGrammar::initTokens() {
  // must be called AFTER constructor
  
  if ((!terminals.empty()) || (!nonterminals.empty())) {
    cout << "WARNING: initTokens called twice. returning\n";
    return;
  }

  vector<string> nonterm_set;

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
    cout << terminals.at(i)  << " ";
  }
  for (int i = 0; i < nonterminals.size(); i++) {
    cout << nonterminals.at(i)  << " ";
  }
  cout << endl;
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
    //   cout<< rule.LHS  << "is NOT null\n";
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
  cout << "Nullable = { ";
  
  int nullCount = 0;
  for (string order : nonterminals) {
    for (string t : nullable ) {
      if (t  == order ) {
        cout << t ;
        //cout << "comp: " << nullCount << " == " << nullable.size() << "\n";
        if (nullCount != nullable.size() -1)
          cout << " , ";
        nullCount +=1;
        break;
      }
    }
  }

  cout << " }\n";
}

/////////////////////////////////////////////////////////////////////////
// PART 3: FIRST SETS
/////////////////////////////////////////////////////////////////////////



void ContextFreeGrammar::initFirst() {
  if (DEBUGGING)
  cout << "Initialize map\n";
  for (string nonterm : nonterminals) {
    first.insert({nonterm , vector<string>()});
  }

  if (DEBUGGING)
  cout << "Initial map terminals\n";
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
  cout << "Starting complex loop \n";
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
            cout << "Terminal found: " << r  << "\n";

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
            cout << "Nullable nonterm: " << r  << "\n";
          vector<string>* termsToAdd = &first.at(r );
          vector<string>* termsInSet = &first.at(rule.LHS );
          changesMade += vecAddTo(termsToAdd, termsInSet);

          continue;
        }
        else {
          if (DEBUGGING)
            cout << "Non-Nullable nonterm: " << r  << "\n";
          // for (string t: terminals) {
          //   cout << t  << ", ";
          // }
          // cout << endl;
          //NOTE: adds first[r ] to first[rule.LHS ]
          vector<string>* termsToAdd = &first.at(r );
          vector<string>* termsInSet = &first.at(rule.LHS );
          changesMade += vecAddTo(termsToAdd, termsInSet);
          break;
        }

      }


    } // end for(rules)

    if (changesMade == 0) {
      if (DEBUGGING)
        cout<<"Ending FIRST Loops\n";
      break;
    }
    else {
      if (DEBUGGING)
        cout<<"Next FIRST Loop\n";
    }
  } // end while
}

void ContextFreeGrammar::PrintFirst() {

  // for (lhs, terms) in first
  for (string tok : nonterminals){
    string lhs = tok ;
    vector<string> terms = first.at(lhs);
    cout << "FIRST(" << lhs << ") = { ";

    int termCount = 0;
    // in order of terminals
    for(string order : terminals) {
      for(string t : terms) {
        if (t  == order ) {
          cout << t ;
          if (termCount != terms.size() - 1) {
            cout << ", ";
          }
          termCount++;
          break;
        }
      }
    }
    cout << " }\n";
  }
}

/////////////////////////////////////////////////////////////////////////
// PART 4: FOLLOW SETS
/////////////////////////////////////////////////////////////////////////

void ContextFreeGrammar::initFollow() {
  if (DEBUGGING)
  cout << "follow: Initialize map\n";
  for (string nonterm : nonterminals) {
    follow.insert({nonterm , vector<string>()});
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
          vector<string> term = {follower};
          vector<string> *followTarget = &follow.at(base );
          vecAddTo(&term, followTarget);
          break;
        }
        else if (vecContains(nullable, follower)) {
          vector<string> *terms = &first.at(follower );
          vector<string> *followTarget = &follow.at(base );
          if (j == rule.RHS.size()) { // if it is the end
            terms->push_back(eof);
          } 
          vecAddTo(terms, followTarget);
          continue;
        }
        else { // NonNull Nonterm
          vector<string> *terms = &first.at(follower );
          vector<string> *followTarget = &follow.at(base );
          vecAddTo(terms, followTarget);
          break;
        }
      }
    }
  }

  if (DEBUGGING)
  cout << "Starting complex loop \n";
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

      vector<string> *inherit = &follow.at(rule.LHS );
      vector<string> termsToAdd;
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
          vector<string> *target = &follow.at(current );
          changesMade += vecAddTo(&termsToAdd, target);
          
          // add first(me) to collection
          vector<string> *newTarget = &first.at(current );
          vecAddTo(newTarget, &termsToAdd);

          continue;
        }
        else { // nonterm nullable
          // add to follow(current)
          vector<string> *target = &follow.at(current );
          changesMade += vecAddTo(&termsToAdd, target);

          // because not nullable
          termsToAdd.clear();

          // add first(me) to collection
          vector<string> *newTarget = &first.at(current );
          vecAddTo(newTarget, &termsToAdd);

          continue;
        }
      }

    } // end for(rules)

    if (changesMade == 0) {
      if (DEBUGGING)
        cout<<"Ending FOLLOW Loops\n";
      break;
    }
    else {
      if (DEBUGGING)
        cout<<"Next FOLLOW Loop\n";
    }
  } // end while

}

void ContextFreeGrammar::PrintFollow() {
  string eof;
  eof  = "$";
  vector<string> termOrder = { eof };
  for (string tok : terminals) {
    termOrder.push_back(tok);
  }

  // for (lhs, terms) in first
  for (string tok : nonterminals){
    string lhs = tok ;
    vector<string> terms = follow.at(lhs);
    cout << "FOLLOW(" << lhs << ") = { ";

    int termCount = 0;
    // in order of terminals
    for(string order : termOrder) {
      for(string t : terms) {
        if (t  == order ) {
          cout << t ;
          if (termCount != terms.size() - 1) {
            cout << ", ";
          }
          termCount++;
          break;
        }
      }
    }
    cout << " }\n";
  }
}

/////////////////////////////////////////////////////////////////////////
// Task 5 helpers
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// rule stuff
/////////////////////////////////////////////////////////////////////////


Rule::Rule(string lhs, vector<string> rhs) {
  LHS = lhs;
  RHS = rhs;
}

Rule::Rule() {
}


// void Rule::Print() {
//   cout << "\tRULE: " << LHS  << " --> ";
//   for(int i = 0; i < RHS.size(); i++) {
//     cout << RHS.at(i)  << " ";
//   }
//   cout << "\n\n";
// }

void Rule::Print() {
  cout << LHS  << " -> ";
  for(int i = 0; i < RHS.size(); i++) {
    cout << RHS.at(i)  << " ";
  }
  cout << "#\n";
}

bool Rule::isNull() {
  return RHS.empty();
}

bool Rule::hasPrefix(vector<string> prefix){
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
