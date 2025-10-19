#include "CFG.h"
#include "lexer.h"
#include <cctype>
#include <iostream>


ContextFreeGrammar::ContextFreeGrammar() {
  LexicalAnalyzer lexer = LexicalAnalyzer();
  //rules = std::vector<Rule>();

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
        rules.push_back(rule);
        // std::cout << "Size:"<< rules.size()<< std::endl;
        break;
      }
      else if (tmp.token_type == OR) {
        std::cout << "\nAdding OR rule\n";
        Rule rule = Rule(LHS, RHS);
        rule.Print();
        rules.push_back(rule);
        RHS = std::vector<Token>();
        //std::cout << "NEW RHS size:"<< RHS.size()<< std::endl;
      }
      else {
        RHS.push_back(tmp);
        // std::cout << "RHS size:"<< RHS.size()<< std::endl;
      }
    }
    count++;

    peek = lexer.peek(1);
    // std::cout << "peek val: " << peek.token_type << std::endl;
    // std::cout << "eqv val: " << ((peek.token_type != HASH) && (count <= 50)) << std::endl;
  }

  std::cout << "Final Size:"<< rules.size()<< std::endl;
  if (count == 50) {
    std::cout << "WARNING: CFG rule limit exceeded!\n";
  }
}

void ContextFreeGrammar::Print() {
  std::cout << "DISPLAYING CFG: \n";
  std::cout << "total rules: "<< rules.size() << "  \n";
  for (int i = 0; i < rules.size(); i++) {
    std::cout << "iter?\n";
    rules.at(i).Print();
  }
}

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
