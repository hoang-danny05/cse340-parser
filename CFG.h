
#ifndef __CFG__H_
#define __CFG__H_

#include "lexer.h"
#include <vector>
#include <string>


class Rule {
public: 
  Rule(Token, std::vector<Token>);
  void Print();
  bool isNull();

  Token LHS; 
  std::vector<Token> RHS;
private:
  Token tmp;
};

// will have its own lexical analyzer
class ContextFreeGrammar {
public:
  ContextFreeGrammar();
  void Print();
  void PrintTokens();

  std::vector<Rule> rules;
  // std::vector<Token> tokens;
  std::vector<Token> nonterminals;
  std::vector<Token> terminals;
private:
  void initTokens();
  bool vecContains(std::vector<Token>, Token);
};

#endif
