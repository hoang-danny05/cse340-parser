#include "CFG.h"
#include "lexer.h"
#include <cctype>
#include <iostream>
#include <cstring>


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
        std::cout << "\nAdding terminated rule\n";
        Rule rule = Rule(LHS, RHS);
        rule.Print();
        this->rules.push_back(rule);
        // std::cout << "Size:"<< rules.size()<< std::endl;
        break;
      }
      else if (tmp.token_type == OR) {
        std::cout << "\nAdding OR rule\n";
        Rule rule = Rule(LHS, RHS);
        rule.Print();
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

  std::cout << "Final Size:"<< this->rules.size()<< std::endl;
  if (count == 50) {
    std::cout << "WARNING: CFG rule limit exceeded!\n";
  }

  initTokens();
  initNullable();
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

  //nonterminals
  for (int i = 0; i < rules.size(); i++) {
    Rule rule = rules.at(i);
    if (!vecContains(nonterminals, rule.LHS)) {
      nonterminals.push_back(rule.LHS);
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
  for (Token t : nullable) {
    std::cout << t.lexeme << " ";
  }
  std::cout << "}\n";
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
