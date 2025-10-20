#include "CFG.h"
#include "lexer.h"
#include <cctype>
#include <iostream>
#include <cstring>
#include <vector>


ContextFreeGrammar::ContextFreeGrammar() {
  LexicalAnalyzer lexer = LexicalAnalyzer();

  int count = 0;

  for (Token peek = lexer.peek(1);
    (peek.token_type != END_OF_FILE) && (peek.token_type != HASH) && (count <= 50);)
  {
    Token LHS = lexer.GetToken();
    std::vector<Token> RHS = std::vector<Token>();
    
    if (lexer.GetToken().token_type != ARROW) {
      std::cout << "[CFG] Expected Arrow!\n";
    }

    for (int i = 0; i < 50; i++) {
      if (i == 50)
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
  if (count == 50) {
   std::cout << "WARNING: CFG rule limit exceeded!\n";
  }

  initTokens();
  initNullable();
  initFirst();
  //initFollow();
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

/////////////////////////////////////////////////////////////////////////
// PART 1: nonterminals and terminals
/////////////////////////////////////////////////////////////////////////

// checks lexeme, nothing else!
bool ContextFreeGrammar::vecContains(std::vector<Token> vec, Token item) {
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
int ContextFreeGrammar::vecAddTo(std::vector<Token>* from, std::vector<Token>* to) {
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

}

void ContextFreeGrammar::PrintFollow() {

}

/////////////////////////////////////////////////////////////////////////
// rule stuff
/////////////////////////////////////////////////////////////////////////


Rule::Rule(Token lhs, std::vector<Token> rhs) {
  LHS = lhs;
  RHS = rhs;
}

void Rule::Print() {
  std::cout << "\tRULE: " << LHS.lexeme << " --> ";
  for(int i = 0; i < RHS.size(); i++) {
    std::cout << RHS.at(i).lexeme << " ";
  }
  std::cout << "\n\n";
}

bool Rule::isNull() {
  return RHS.empty();
}
