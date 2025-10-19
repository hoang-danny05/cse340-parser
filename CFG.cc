#include "CFG.h"
#include "lexer.h"
#include <cctype>
#include <iostream>


ContextFreeGrammar::ContextFreeGrammar() {
  // shared lexer across rules
  LexicalAnalyzer lexer = LexicalAnalyzer();

  int count = 0;

  //Token peek = lexer.peek(1);

  for (Token peek = lexer.peek(1);
    (peek.token_type != END_OF_FILE) && (peek.token_type != HASH) && (count <= 50);)
  {
    std::cout << "STARTING NEW LOOP ASSHOLE\n";
    std::cout << "Peek val: " << peek.token_type << std::endl;
    std::cout << "Cond. val: " << ((peek.token_type != HASH) && (count <= 50)) << std::endl;
    Rule rule = Rule(&lexer);
    rules.push_back(rule);
    count++;

    peek = lexer.peek(1);
    std::cout << "peek val: " << peek.token_type << std::endl;
    std::cout << "eqv val: " << ((peek.token_type != HASH) && (count <= 50)) << std::endl;
  }

  if (count == 50) {
    std::cout << "WARNING: CFG rule limit exceeded!\n";
  }
}

void ContextFreeGrammar::Print() {
  std::cout << "DISPLAYING CFG: \n";
  for (int i = 0; i < rules.size(); i++) {
    rules.at(i).Print();
  }
}

Rule::Rule(LexicalAnalyzer* lexer) {
  tmp = lexer->GetToken();
  
  if (tmp.token_type == HASH || tmp.token_type == END_OF_FILE) {
    std::cout << "ERROR: Cannot make RULE from EOF.\n";
    std::exit(-1);
  }

  LHS = tmp;

  while (true) {
    tmp = lexer->GetToken();

    if(tmp.token_type == STAR)
      break;

    RHS.push_back(tmp);
  }
}

void Rule::Print() {
  std::cout << "Rule: " << LHS.lexeme << " --> ";
  for(int i = 0; i < RHS.size(); i++) {
    std::cout << RHS.at(i).lexeme << " ";
  }
  std::cout << " |END|\n";
}
